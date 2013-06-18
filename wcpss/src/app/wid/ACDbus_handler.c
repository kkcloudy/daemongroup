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
* ACDbus_handler.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/
#include <string.h>
#include <time.h>		/* Huangleilei add for ASXXZFI-1622 */
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <dirent.h>
#include "CWAC.h"
#include "CWCommon.h"
#include "ACMultiHomedSocket.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "hmd/hmdpub.h"
#include "CWStevens.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "ACDbus_handler.h"
#include "ACDbus.h"
#include "iplist.h"
#include "ACBak.h"
#include "ACIPv6Addr.h"
#include "ACLoadbanlance.h"
#include <syslog.h>
#include "AC.h"
#include <math.h>

APScanningSetting gapscanset = {0};
int wids_judge_policy = 7;
white_mac_list *pwhite_mac_list = NULL;
white_mac_list *pblack_mac_list = NULL;
unsigned char wirelessdata_switch = 0;
unsigned char wireddata_switch = 0;
unsigned char apstatistics = 0;
unsigned int apstatisticsinterval = 1800;
unsigned char aphotreboot = 0;
wid_wids_set gwids = {0};

unsigned char gwidsinterval = 1;
unsigned char gprobethreshold = 0;
unsigned char gotherthreshold = 30;
unsigned int glasttimeinblack = 300;

update_wtp_list * updatewtplist = NULL;
unsigned int checkwtpcount = 0;
int gtrapflag = 1;
int gtrap_ap_run_quit_trap_switch = 0;
int gtrap_ap_cpu_trap_switch = 0;
int gtrap_ap_mem_trap_switch = 0;
int gtrap_rrm_change_trap_switch = 0;
int gtrap_flash_write_fail_trap_switch = 0;
int gtrap_channel_device_ap_switch = 0; /*zhaoruijia,tranlate  neighbor_channel_interference to 1.3,start*/
int gtrap_channel_device_interference_switch = 0;
int gtrap_channel_terminal_interference_switch = 0;
int gtrap_rogue_ap_threshold_switch = 0;
int gtrap_wireless_interface_down_switch = 0;
int gtrap_channel_count_minor_switch = 0;
int gtrap_channel_change_switch = 0;
unsigned char updatemaxfailcount = 1;
unsigned char gwidspolicy = 0;
wid_wids_device *wids_ignore_list = NULL;
unsigned char countermeasurecount = 0;
unsigned char sta_deauth_message_reportswitch = 0;
unsigned char sta_flow_information_reportswitch  = 0;
unsigned char g_radio_5g_sw = 0;

int istryreadipv6addr = 0;
int istrybindipv6addr = 0;
unsigned char gmacfilterflag = 0;
unsigned char gessidfilterflag = 0;
extern unsigned int sample_infor_interval;
extern unsigned char gWIDLOGHN;//qiuchen
int wid_dbug_trap_ssid_key_conflict(unsigned int wtpid,unsigned char radio_l_id, unsigned char wlan1, unsigned char wlan2);

#define CFDISK_MIN_MEM (1000)
#define SYSTEM_MIN_MEM (300000)


int ACDBUS_MSGQ;

//xm add
OUI_LIST_S 			g_oui_list = {0,NULL};
ESSID_LIST_S 		g_essid_list = {0,NULL};
ATTACK_MAC_LIST_S	g_attack_mac_list = {0,NULL};
int setWtpNoRespToStaProReq(unsigned int wtpid,unsigned char l_radioid,unsigned char wlanid,unsigned int policy);
int setWtpUniMutiBroCastIsolation(unsigned int wtpid,unsigned char radioid,unsigned char wlanid,unsigned char policy);
int setWtpUniMutiBroCastRate(unsigned int wtpid,unsigned char radioid,unsigned char wlanid,unsigned int rate);
void free_maclist(struct acl_config *conf, struct maclist *list);

CWBool check_wtpid_func(unsigned int WTPID){
	if(WTPID >= WTP_NUM){
		wid_syslog_err("<error> invalid wtpid:%d\n",WTPID);
		return CW_FALSE;
	}else{
		return CW_TRUE;
	}
}
CWBool check_wlanid_func(unsigned int WLANID){
	if(WLANID >= WLAN_NUM){
		wid_syslog_err("<error> invalid wlanid:%d\n",WLANID);
		return CW_FALSE;
	}else{
		return CW_TRUE;
	}
}
CWBool check_bssid_func(unsigned int BSSID){
	if(BSSID >= BSS_NUM){
		wid_syslog_err("<error> invalid wlanid:%d\n",BSSID);
		return CW_FALSE;
	}else if(AC_BSS[BSSID] == NULL){
		wid_syslog_err("<error> AC_BSS[%d] is NULL\n",BSSID);
		return CW_FALSE;
	}else{
		return CW_TRUE;
	}
}
CWBool check_g_radioid_func(unsigned int RADIOID){
	if(RADIOID >= G_RADIO_NUM){
		wid_syslog_err("<error> invalid g_radioid:%d\n",RADIOID);
		return CW_FALSE;
	}else{
		return CW_TRUE;
	}
}
CWBool check_l_radioid_func(unsigned int RADIOID){
	if(RADIOID >= L_RADIO_NUM){
		wid_syslog_err("<error> invalid l_radioid:%d\n",RADIOID);
		return CW_FALSE;
	}else{
		return CW_TRUE;
	}
}
void wid_apstatsinfo_init(unsigned int WTPID){			//xiaodawei add for apstatsinfo init, 20110107
	int i = 0;
	for(i=0; i<TOTAL_AP_IF_NUM; i++){
		AC_WTP[WTPID]->apstatsinfo[i].radioId = TOTAL_AP_IF_NUM+1;
		AC_WTP[WTPID]->apstatsinfo[i].type = TOTAL_AP_IF_NUM+1;
		AC_WTP[WTPID]->apstatsinfo[i].wlanId = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_bytes = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_bytes = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_drop = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_drop = 0;

		
		AC_WTP[WTPID]->apstatsinfo[i].rx_packets = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_packets = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_errors = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_errors = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_rate = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_rate = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_crcerr = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_badcrypt = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_badmic = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_phyerr = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_error_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_error_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_drop_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_drop_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_band= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_band= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_unicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_unicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_broadcast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_broadcast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_multicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_multicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_unicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_unicast = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_broadcast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_broadcast = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_multicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_multicast = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_mgmt =0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_mgmt =0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_mgmt =0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_mgmt =0;
		AC_WTP[WTPID]->apstatsinfo[i].is_refuse_lowrssi = 0;  //fengwenchao add  for chinamobile-177,20111122
	}
}
 int mac_format_check
 (
	 char* str,
	 int len
 ) 
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
			 if((':'!=c)&&('-'!=c))
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

 int parse_mac_addr(char* input,MACADDR* macAddr) {
 	
	int i = 0;
	char cur = 0,value = 0;
	
	if((NULL == input)||(NULL == macAddr)) {
		return -1;
	}
	if(-1 == mac_format_check(input,strlen(input))) {
		return -1;
	}
	
	for(i = 0; i <6;i++) {
		cur = *(input++);
		if((cur == ':') ||(cur == '-')){
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
		macAddr->arEther[i] = value;
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
		macAddr->arEther[i] = (macAddr->arEther[i]<< 4)|value;
	}
	
	return 0;
} 

int wid_update_bss_to_wifi(unsigned int bssindex,unsigned int WTPIndex,unsigned char flag){
	if(1)//((AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE)||(AC_BSS[bssindex]->BSS_IF_POLICY == WLAN_INTERFACE))
	{
		struct sockaddr *sa;
		struct sockaddr *sa2;
		int ret = -1;									
		IF_info ifinfo;
		unsigned char wlan_id = 0;
		int fd = open("/dev/wifi0", O_RDWR);
		wid_syslog_debug_debug(WID_DEFAULT,"***%s, fd:%d,flag=%d. ***\n",__func__,fd,flag);
	
		if(fd < 0)
		{
			wid_syslog_err("*** open /dev/wifi0 fail ret:%d ***\n",fd);
			return -1;//create failure
		}
		sa = (struct sockaddr *)&gWTPs[WTPIndex].address;
		sa2 = (struct sockaddr*)&(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr);
		memset(&ifinfo, 0, sizeof(IF_info));
		ifinfo.apip = ((struct sockaddr_in *)sa)->sin_addr.s_addr;
		ifinfo.apport = ((struct sockaddr_in *)sa)->sin_port +1;
		ifinfo.acport = htons(5247);
		ifinfo.BSSIndex = bssindex;
		if(flag == 0){
			ifinfo.WLANID = 0;/*resolve send capwap message,before ath be created*/	
		}else{
			ifinfo.WLANID = AC_BSS[bssindex]->WlanID;	

		}
		wlan_id = AC_BSS[bssindex]->WlanID; 
		if((AC_WLAN[wlan_id] != NULL)&& (AC_WLAN[wlan_id]->want_to_delete != 1) && (AC_WLAN[wlan_id]->SecurityType == OPEN)&&(AC_WLAN[wlan_id]->EncryptionType == NONE))
			ifinfo.protect_type = 0;
		else
		{
			ifinfo.protect_type = 1;
			if(AC_WLAN[wlan_id] == NULL)
				wid_syslog_warning("<warning>,%s,%d,wlanid  is  NULL!\n",__func__,__LINE__);
			else if (AC_WLAN[wlan_id]->want_to_delete == 1)				/* Huangleilei check deleting wlan */
			{
				wid_syslog_crit("<critical>, %s %d, you want to delete this wlan!", __func__, __LINE__);
				close(fd);
				return -1;
			}
		}  //fengwenchao add 20111220
		if(AC_BSS[bssindex]->vlanid != 0){
			ifinfo.vlanid = AC_BSS[bssindex]->vlanid;
		}else if(AC_BSS[bssindex]->wlan_vlanid != 0){
			ifinfo.vlanid = AC_BSS[bssindex]->wlan_vlanid;
		}else{
			ifinfo.vlanid = 0;
		}
		ifinfo.vrid = local*MAX_INSTANCE +vrrid;
		if(AC_BSS[bssindex]->BSS_TUNNEL_POLICY == CW_802_DOT_3_TUNNEL){
				ifinfo.f802_3 = 1;
			wid_syslog_info("%s,ifinfo.f802_3 = %d.\n",__func__,ifinfo.f802_3);
		}else{
			ifinfo.f802_3 = 0;
			wid_syslog_info("%s,AC_BSS[%d]->BSS_TUNNEL_POLICY =%d, not dot3,set ifinfo.f802_3 = %d.\n",__func__,bssindex,AC_BSS[bssindex]->BSS_TUNNEL_POLICY,ifinfo.f802_3);
		}
		ifinfo.wsmswitch = wsmswitch;
		ifinfo.vlanSwitch = vlanSwitch;
		if((AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE)||(AC_BSS[bssindex]->BSS_IF_POLICY == WLAN_INTERFACE))
			ifinfo.if_policy = 1;
		else
			ifinfo.if_policy = 0;
		memcpy(ifinfo.apmac, AC_WTP[WTPIndex]->WTPMAC, MAC_LEN);
		memcpy(ifinfo.bssid,  AC_BSS[bssindex]->BSSID, MAC_LEN);
		memcpy(ifinfo.ifname, AC_WTP[WTPIndex]->BindingIFName,strlen(AC_WTP[WTPIndex]->BindingIFName));
		memcpy(ifinfo.apname,AC_WTP[WTPIndex]->WTPNAME,strlen(AC_WTP[WTPIndex]->WTPNAME));		
		if(AC_WLAN[wlan_id] != NULL && AC_WLAN[wlan_id]->want_to_delete != 1)  //fengwenchao add 20111220
		{
			memcpy(ifinfo.essid ,AC_WLAN[wlan_id]->ESSID ,strlen(AC_WLAN[wlan_id]->ESSID));
			ifinfo.Eap1XServerSwitch = AC_WLAN[wlan_id]->eap_mac_switch;
			memset(ifinfo.Eap1XServerMac,0,MAC_LEN);
			memcpy(ifinfo.Eap1XServerMac,AC_WLAN[wlan_id]->eap_mac2,MAC_LEN);
		}
		else
		{
			ifinfo.Eap1XServerSwitch = 0;
			memset(ifinfo.Eap1XServerMac,0,MAC_LEN);
			wid_syslog_warning("<warning>,%s,%d,wlanid  is NULL!\n",__func__,__LINE__);
		}
		char __str[128];			
		memset(__str,0,128);
		char *str = "lo";	//fengwenchao modify 20110525
		str = sock_ntop_r(((struct sockaddr*)&(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr)), __str);
		wid_syslog_info("WTP %d on Interface %s (%d)\n",WTPIndex, str, gWTPs[WTPIndex].interfaceIndex);
		if(sa->sa_family != AF_INET6){										
			ifinfo.isIPv6 = 0;
			ifinfo.apip = ((struct sockaddr_in *)sa)->sin_addr.s_addr;
			ifinfo.apport = ((struct sockaddr_in *)sa)->sin_port +1;
			struct sockaddr_in	*sin = (struct sockaddr_in *) sa2;
			unsigned int v_ip = sin->sin_addr.s_addr;
			wid_syslog_info("v_ip %d.%d.%d.%d.\n", \
						(v_ip >> 24) & 0xFF, (v_ip >> 16) & 0xFF,(v_ip >> 8) & 0xFF, v_ip & 0xFF);
			ifinfo.acip = v_ip;
		}else{
			ifinfo.isIPv6 = 1;
			memcpy(ifinfo.apipv6,&((struct sockaddr_in6 *) sa)->sin6_addr,sizeof(struct in6_addr));
			ifinfo.apport = ((struct sockaddr_in6 *)sa)->sin6_port +1;
			memcpy(ifinfo.acipv6,&((struct sockaddr_in6 *) sa2)->sin6_addr,sizeof(struct in6_addr));
		}
		
		ret = ioctl(fd, WIFI_IOC_IF_UPDATE, &ifinfo);
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** update_BSS_L3_Interface ret:%d ***\n",ret);
		close(fd);
		if(ret < 0)
		{
			wid_syslog_err("*** update_BSS_L3_Interface fail ret:%d ***\n",ret);
			return -1;
		}
	
	}
	return 0;

}
/*fengwenchao add 20111219*/
int wid_update_wtp_bss_infov2(int wtpid,unsigned int BSSIndex)
{
//	int m =0;int n = 0;
	if(AC_WTP[wtpid] != NULL)
	{	
		if((check_bssid_func(BSSIndex))&&(AC_BSS[BSSIndex] != NULL)&&(AC_WTP[wtpid] != NULL))
		{
			wid_update_bss_to_wifi(BSSIndex,wtpid,1);/*resolve send capwap message,before ath be created*/	
		}
	}
	return 0;
}

/*fengwenchao add end*/
/*fengwenchao add 20111028*/
int check_whether_in_ebr(unsigned int index,int wtpid, unsigned int radioid,unsigned char wlanid, int *ebr_id)
{
	#define PATH_MAX_LEN 256  
	//#define SYSFS_CLASS_NET "/sys/class/net/"
	//#define DEV_BRIDGE_IFINDEX "brport/bridge/ifindex"
	 unsigned int ret = 0;
	 char br_ifindex_str[PATH_MAX_LEN] = {0};
	 char bppath[PATH_MAX_LEN] = {0};
	 char ifpath[PATH_MAX_LEN] = {0};
	 char ifname[PATH_MAX_LEN] = {0};
	 FILE *fd =NULL;
	int slot_id;
	
	fd = fopen("/dbm/local_board/slot_id", "r");
	if (fd == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT, "<error>:open /dbm/local_board/slot_id failed\n");
		return SYSTEM_CMD_ERROR;
	}
	fscanf(fd, "%d", &slot_id);
	fclose(fd);
	
	wid_syslog_debug_debug(WID_DEFAULT, "%d\n", slot_id);

	
	 sprintf(ifname,"r%d-%d-%d-%d.%d",slot_id,index,wtpid,radioid,wlanid);
	 wid_syslog_debug_debug(WID_DEFAULT,"ifname  =  %s \n",ifname);
	 DIR * dir = NULL;
	 FILE *f = NULL;
	/* if(!ifname || !ebr_id) return INTERFACE_RETURN_CODE_ERROR;
	 *ebr_id = 0;*/
	 sprintf(bppath,"/sys/class/net/%s/brport", ifname);
	 dir = opendir(bppath);
	 
	if (dir)
	{
		  sprintf(ifpath,"/sys/class/net/%s/brport/bridge/ifindex", ifname);
		  f = fopen(ifpath, "r");
		/*read first,check and write back and append */
		  if(f)
		  {
			if(fgets(br_ifindex_str,256,f))
			{
				//printf("br_ifindex_str  =  %s\n", br_ifindex_str);
				*ebr_id = strtoul(br_ifindex_str, NULL, 10);
				//printf("the interface is member of br, ebr_id (%#x)\n", *ebr_id);
			 }
		  	 fclose(f);
		}

		  closedir(dir);  
	}

	char ifindex_name[256] = {0};
	if((*ebr_id > 0)&&(if_indextoname(*ebr_id,ifindex_name)))
	{
		if(!strncasecmp(ifindex_name,"ebr",3))
		{
			ret = RADIO_IN_EBR;
		   	wid_syslog_debug_debug(WID_DEFAULT,"<error>RADIO_IN_EBR \n");
			//wid_syslog_info("error,interface in  %s,please del it from bridge\n",ifindex_name);	
		}
		
	}
	
	 return ret;
}
/*fengwenchao add end*/

int set_balance_probe_extension_command(int wtpid, char * command)
{//xm add 09.5.13
	msgq msg;
//	struct msgqlist *elem;

	if(AC_WTP[wtpid]->WTP_Radio[0]->excommand!=NULL){
		free(AC_WTP[wtpid]->WTP_Radio[0]->excommand);
		AC_WTP[wtpid]->WTP_Radio[0]->excommand = NULL;
	}

	AC_WTP[wtpid]->WTP_Radio[0]->excommand = (char*)malloc(strlen(command)+1);
	memset(AC_WTP[wtpid]->WTP_Radio[0]->excommand, 0, strlen(command)+1);
	memcpy(AC_WTP[wtpid]->WTP_Radio[0]->excommand, command, strlen(command));
	
	memset((char*)&msg, 0, sizeof(msg));
	msg.mqid = wtpid%THREAD_NUM +1;
	msg.mqinfo.WTPID = wtpid;
	msg.mqinfo.type = CONTROL_TYPE;
	msg.mqinfo.subtype = WTP_S_TYPE;
	msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;
	memcpy(msg.mqinfo.u.WtpInfo.value, command, strlen(command));

	if(AC_WTP[wtpid]->WTPStat == 5){	
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){			
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}//delete unuseful code
	/*else{
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){			
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(wtpid, elem);
	}*/

	return 0;	
}


int balance_probe_extend_command(unsigned char wlanid,unsigned char state){
//xm add 09.5.13
	int i=0,j=0;
	unsigned int wtpid=0;
	char *command=NULL;
	

	if(wlanid<1||wlanid>=WLAN_NUM)
		return -1;
	
	if(state!=0&&state!=1)
		return -1;

	if(AC_WLAN[wlanid]==NULL)
		return -1;

	command = (char *)malloc(sizeof(char)*50);
	memset(command,0,50);

	if(state==1)
		strncpy(command,"echo 1 > /proc/sys/dev/wifi0/traffic_balance",44);
	else if(state==0)
		strncpy(command,"echo 0 > /proc/sys/dev/wifi0/traffic_balance",44);
	else
		return -1;

	for(i=0;i<WTP_NUM;i++)
		for(j=0;j<L_RADIO_NUM;j++){
			if(AC_WLAN[wlanid]->S_WTP_BSS_List[i][j]!=0){
				
				wtpid=AC_WLAN[wlanid]->S_WTP_BSS_List[i][j]/(L_BSS_NUM*L_RADIO_NUM);
				set_balance_probe_extension_command(wtpid,command);
			}
		}
		
	free(command);	

	return 0;
}


int wid_trap_remote_restart(unsigned int wtpid);
void wid_init_wtp_info_in_create(unsigned int WTPID);
int wid_radio_set_chainmask(unsigned int RadioID, unsigned char type); // zhangshu add for set chainmask
int wid_set_ap_eth_if_mtu(unsigned int wtpid,unsigned char eth_index);   //fengwenchao add 20110126 for XJDEV-32  from 2.0

int WID_CREATE_NEW_WLAN(char *WlanName, unsigned char WlanID,unsigned char *ESSID,unsigned char *ESSID_STR,unsigned char cnFlag){
	int i = 0, j = 0;
	unsigned int essid_len = 0;
	AC_WLAN[WlanID] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
	memset(AC_WLAN[WlanID], 0 ,sizeof(WID_WLAN));
	AC_WLAN[WlanID]->WlanName = (char*)malloc(strlen(WlanName)+1);
	memset(AC_WLAN[WlanID]->WlanName,0,strlen(WlanName)+1);
	memcpy(AC_WLAN[WlanID]->WlanName,WlanName,strlen(WlanName));
	essid_len = strlen((char *)ESSID);
	if(essid_len <= ESSID_LENGTH){  //fengwenchao modify 20111013 for AXSSZFI-477,AUTELAN-2546
		AC_WLAN[WlanID]->ESSID = (char*)malloc(essid_len + 1);	
		memset(AC_WLAN[WlanID]->ESSID,0,essid_len + 1);
		memcpy(AC_WLAN[WlanID]->ESSID,ESSID,essid_len);
	}
	else{
		AC_WLAN[WlanID]->ESSID = (char*)malloc(ESSID_LENGTH);	
		memset(AC_WLAN[WlanID]->ESSID,0,ESSID_LENGTH);
		memcpy(AC_WLAN[WlanID]->ESSID,ESSID,(ESSID_LENGTH-1));
	}
	AC_WLAN[WlanID]->chinaEssid = cnFlag;
	if(cnFlag == 1){
		if(strlen((char *)ESSID_STR)< ESSID_DEFAULT_LEN){
			AC_WLAN[WlanID]->ESSID_CN_STR = (unsigned char*)malloc(ESSID_DEFAULT_LEN);	
			memset(AC_WLAN[WlanID]->ESSID_CN_STR,0,ESSID_DEFAULT_LEN);
			memcpy(AC_WLAN[WlanID]->ESSID_CN_STR,ESSID_STR,strlen((char *)ESSID_STR));
		}else{
			AC_WLAN[WlanID]->ESSID_CN_STR = (unsigned char*)malloc(strlen((char *)ESSID_STR)+1);	
			memset(AC_WLAN[WlanID]->ESSID_CN_STR,0,strlen((char *)ESSID_STR)+1);
			memcpy(AC_WLAN[WlanID]->ESSID_CN_STR,ESSID_STR,strlen((char *)ESSID_STR));
		}
	}
	AC_WLAN[WlanID]->WlanID = WlanID;
	AC_WLAN[WlanID]->AAW = 1;
	for(i=0; i<WTP_MAX_MAX_NUM; i++)
		for(j=0; j<L_RADIO_NUM; j++)
		AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] = 0;
	//memset(AC_WLAN[WlanID]->S_WTP_BSS_List, 0, WTP_NUM);
	AC_WLAN[WlanID]->D_WTPADD_List[0] = 0;
	AC_WLAN[WlanID]->L3_IF_Index[0] = 0;
	AC_WLAN[WlanID]->Status = 1;
	AC_WLAN[WlanID]->ifcount = 0;
	AC_WLAN[WlanID]->Wlan_Ifi = NULL;
	AC_WLAN[WlanID]->CMD = 1;
	AC_WLAN[WlanID]->SecurityID = 0;
	AC_WLAN[WlanID]->HideESSid = 0;
	AC_WLAN[WlanID]->wlan_if_policy = NO_INTERFACE;
	memset(AC_WLAN[WlanID]->WlanL3IFName,0, ETH_IF_NAME_LEN);
	AC_WLAN[WlanID]->SecurityType = 0;
	AC_WLAN[WlanID]->EncryptionType = 0;
	AC_WLAN[WlanID]->asic_hex = 0;
	AC_WLAN[WlanID]->KeyLen = 0;
	AC_WLAN[WlanID]->SecurityIndex= 1;
	memset(AC_WLAN[WlanID]->WlanKey, 0, DEFAULT_LEN);
	AC_WLAN[WlanID]->IpLen = 0;
	memset(AC_WLAN[WlanID]->AsIp, 0, DEFAULT_LEN);
	AC_WLAN[WlanID]->AECerLen = 0;
	memset(AC_WLAN[WlanID]->AECerPath, 0, DEFAULT_LEN);
	AC_WLAN[WlanID]->ASCerLen = 0;
	memset(AC_WLAN[WlanID]->ASCerPath, 0, DEFAULT_LEN);
	AC_WLAN[WlanID]->wlan_max_allowed_sta_num=65536;
	AC_WLAN[WlanID]->balance_para=ac_flow_num_balance_flag.num_balance_para;
	AC_WLAN[WlanID]->flow_balance_para=ac_flow_num_balance_flag.flow_balance_para;
	AC_WLAN[WlanID]->wlan_send_traffic_limit = 0;	//
	AC_WLAN[WlanID]->wlan_traffic_limit = 0;
	AC_WLAN[WlanID]->multi_user_optimize_switch = 0;
	AC_WLAN[WlanID]->wlan_station_average_traffic_limit = 0;
	AC_WLAN[WlanID]->wlan_station_average_send_traffic_limit = 0;

	if(ac_flow_num_balance_flag.state !=0){
		AC_WLAN[WlanID]->balance_switch=1;
		AC_WLAN[WlanID]->balance_method=ac_flow_num_balance_flag.state;
		balance_probe_extend_command(WlanID,1);
	}else{
		AC_WLAN[WlanID]->balance_switch=0;
		AC_WLAN[WlanID]->balance_method=0;
	}
	
	AC_WLAN[WlanID]->Roaming_Policy=0;
	AC_WLAN[WlanID]->isolation_policy=1;
	AC_WLAN[WlanID]->multicast_isolation_policy=1;
	AC_WLAN[WlanID]->bridge_mcast_solicit_stat = 1;     /* the default config is 1 as ebr, set by sysctl.conf. zhangdi@autelan.com 2013-05-31 */
	AC_WLAN[WlanID]->bridge_ucast_solicit_stat = 1;
	AC_WLAN[WlanID]->sameportswitch=0;
	AC_WLAN[WlanID]->vlanid = 0;
	AC_WLAN[WlanID]->wlan_1p_priority = 0;
	AC_WLAN[WlanID]->tunnel_wlan_vlan = NULL;
	AC_WLAN[WlanID]->WDSStat = 0;
	AC_WLAN[WlanID]->wds_mesh = 0;
	AC_WLAN[WlanID]->StartService.times = -1;	
	AC_WLAN[WlanID]->StopService.times = -1;
	
	AC_WLAN[WlanID]->hotspot_id = 0;
	//weichao add 2011.10.31
	AC_WLAN[WlanID]->flow_check = 0;
	AC_WLAN[WlanID]->no_flow_time = 900;
	AC_WLAN[WlanID]->limit_flow = 10240;
	/* zhangshu add for eap mac initial,2010-10-22 */
	AC_WLAN[WlanID]->eap_mac_switch = 0;
	AC_WLAN[WlanID]->eap_mac = (unsigned char*)malloc(18);
	memset(AC_WLAN[WlanID]->eap_mac,0,18);
	memcpy(AC_WLAN[WlanID]->eap_mac,"0",1);
	AC_WLAN[WlanID]->wlan_muti_rate = gWLAN_UNI_MUTI_BRO_CAST.rate; //fengwenchao modify 20120323
	AC_WLAN[WlanID]->wlan_noResToStaProReqSW =0;
	AC_WLAN[WlanID]->wlan_muti_bro_cast_sw =gWLAN_UNI_MUTI_BRO_CAST.multicast_broadcast_policy; //fengwenchao modify 20120323
	AC_WLAN[WlanID]->wlan_unicast_sw = gWLAN_UNI_MUTI_BRO_CAST.unicast_policy; //fengwenchao modify 20120323
	AC_WLAN[WlanID]->wlan_wifi_sw = gWLAN_UNI_MUTI_BRO_CAST.wifi_policy; //fengwenchao modify 20120323
	AC_WLAN[WlanID]->bss_allow_max_sta_num = gWLAN_MAX_ALLOWED_STA_NUM_FOR_BSS;//fengwenchao add 20120323
	AC_WLAN[WlanID]->wlan_ath_l2_isolation = gWLAN_ATH_L2_ISOLATION; //fengwenchao add 20120323
	AC_WLAN[WlanID]->wlan_sta_static_arp_policy = gWLAN_STA_STATIC_ARP_POLICY.policy;//fengwenchao add 20120323
	memset(AC_WLAN[WlanID]->wlan_arp_ifname,0,ETH_IF_NAME_LEN);//fengwenchao add 20120323
	AC_WLAN[WlanID]->wlan_limit_sta_rssi = gWLAN_LIMIT_STA_RSSI;//fengwenchao add 20120323
	AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
	
	AsdWsm_WLANOp(WlanID, WID_ADD, 0);
//	printf("%s,%s,%d,%d,%d,%d,%d,%d\n",AC_WLAN[WlanID]->WlanName,AC_WLAN[WlanID]->ESSID,AC_WLAN[WlanID]->WlanID,AC_WLAN[WlanID]->AAW,
//										AC_WLAN[WlanID]->S_WTPADD_List[0],AC_WLAN[WlanID]->D_WTPADD_List[0],AC_WLAN[WlanID]->L3_IF_Index[0],AC_WLAN[WlanID]->Status);
	wid_syslog_debug_debug(WID_DEFAULT,"create a new wlan:%s,%s,%d,%d,%d,%d",AC_WLAN[WlanID]->WlanName,AC_WLAN[WlanID]->ESSID,AC_WLAN[WlanID]->WlanID,AC_WLAN[WlanID]->AAW,AC_WLAN[WlanID]->L3_IF_Index[0],AC_WLAN[WlanID]->Status);
	return 0;
}
/* if spending too much time to delete the special wlan,
  * use the struct below to notice the user when return 
  * from free_wlan thread
  */
struct free_wlan_thread_stuct		/* Huangleilei add for ASXXZFI-1622 */
{
	int vty_fd;
	FILE *fp;
	unsigned char WlanID;
};
void * free_wlan(void * arg)
{
	int i = 0, j = 0;
	int WlanID = 0;
	int ret = 0;
	unsigned int radioid;
	WlanID = (int) arg;
	if (WlanID & 0x80000000)
	{
		wid_pid_write_v2("free_AC_WLAN", 0, vrrid);
		wid_syslog_info("%s %d pid: %u", __func__, __LINE__, (unsigned int)getpid());
		WlanID &= 0x000000FF;
	}	
	wid_syslog_info( "%s %d WlanID: %u", __func__, __LINE__, WlanID);
	struct timeval tpstart, tpend;
	struct timeval tpdel;
	struct timeval time_check_start;
	struct timeval time_check_end;
	gettimeofday(&tpstart, NULL);
	{
		for( i = 0 ; i < WTP_NUM; i ++)
		{
			if (AC_WTP[ i ] != NULL && AC_WTP[ i ]->WTPStat == CW_ENTER_RUN)
			{
				gettimeofday(&time_check_start, NULL);
				for(j = 0 ; j < AC_WTP[ i ]->RadioCount; j ++)
				{
					if (AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0
						&& AC_WTP[i]->WTP_Radio[j]->BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] % L_BSS_NUM] != NULL)
					{
						while (AC_WTP[i]->WTP_Radio[j]->BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] % L_BSS_NUM]->enable_wlan_flag == 1
							&& AC_WTP[i]->WTPStat == CW_ENTER_RUN)
						{
							usleep(10000);
						}
						gettimeofday(&time_check_end, NULL);
						if (time_check_end.tv_sec - time_check_start.tv_sec > CW_REACCESS_INTERVAL_DEFAULT/*(CW_RETRANSMIT_INTERVAL_DEFAULT * CW_MAX_RETRANSMIT_DEFAULT)*/)
						{
							wid_syslog_debug_debug(WID_DEFAULT, "%s %d too much time to wait: 25 seconds", __func__, __LINE__);
							goto maybe_get_time_exprired;
						}
					}
				}
			}
			maybe_get_time_exprired:
				;						// do nothing, goto next circle
		}
	}

	for(i=0;i<WTP_NUM;i++)
		if(AC_WTP[i] != NULL){
			for(j=0;j<AC_WTP[i]->RadioCount;j++){
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
				{
					radioid = i*L_RADIO_NUM+j;
					ret = WID_DELETE_WLAN_APPLY_RADIO(radioid,WlanID);
					if(ret != 0)
						wid_syslog_err("WID_DELETE_WLAN_APPLY_RADIO radio %d delete wlan %d error(ret = %d)\n",radioid,WlanID, ret);
					continue;
				}
			}
		}
	if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE){
		Delete_Wlan_L3_BR_Interface(WlanID);
	}
	struct WID_TUNNEL_WLAN_VLAN * wlan_vlan = AC_WLAN[WlanID]->tunnel_wlan_vlan;
	while(wlan_vlan != NULL)
	{
		AC_WLAN[WlanID]->tunnel_wlan_vlan = wlan_vlan->ifnext;
		free(wlan_vlan);
		wlan_vlan = NULL;
		wlan_vlan = AC_WLAN[WlanID]->tunnel_wlan_vlan;
	}
	wid_auto_ap_if *iflist= NULL;
	iflist = g_auto_ap_login.auto_ap_if;
	while(iflist != NULL)
	{
		for(i=0; i<L_BSS_NUM; i++)
		{
			if(WlanID == iflist->wlanid[i])
			{
				iflist->wlanid[i] = 0;
				for(j=i;((j+1)<L_BSS_NUM)&&(j<(iflist->wlannum));j++)
				{
					iflist->wlanid[j] = iflist->wlanid[j+1];
				}
				iflist->wlannum--;
				break;
			}
		}	
			iflist = iflist->ifnext;
	}
	struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
	while(wlan_ifi != NULL)
	{
		AC_WLAN[WlanID]->Wlan_Ifi = wlan_ifi->ifi_next;
		free(wlan_ifi);
		wlan_ifi = NULL;
		wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
	}
	free(AC_WLAN[WlanID]->WlanName);
	AC_WLAN[WlanID]->WlanName = NULL;
	free(AC_WLAN[WlanID]->ESSID);
	AC_WLAN[WlanID]->ESSID = NULL;
	free(AC_WLAN[WlanID]->eap_mac);//zhangshu add
	AC_WLAN[WlanID]->eap_mac= NULL;
	free(AC_WLAN[WlanID]);
	AC_WLAN[WlanID] = NULL;
	gettimeofday(&tpend, NULL);
	if (tpend.tv_usec < tpstart.tv_usec)
		tpdel.tv_sec = tpend.tv_sec - tpstart.tv_sec - 1;
	else
		tpdel.tv_sec = tpend.tv_sec - tpstart.tv_sec;
	tpdel.tv_usec = (tpend.tv_usec < tpstart.tv_usec) ? (tpstart.tv_usec - tpend.tv_usec) : (tpend.tv_usec - tpstart.tv_usec);
	wid_syslog_info("%s %d: before delete wlan has waited %d seconds %d usecond [ wlanid: %d ]", __func__, __LINE__, tpdel.tv_sec, tpdel.tv_usec, WlanID);

	 return (void *)0;
}
int WID_DELETE_WLAN(unsigned char WlanID){
	int i, j,ret = 0;
	int create_thread_flag = 0;
	unsigned int radioid;
	int wlanid_temp = 0;
	if(AC_WLAN[WlanID]->Status == 0){
		return WLAN_BE_ENABLE;
	}	

	/*fengwenchao add 20120509 for onlinebug-271*/
	for(i=0;i<WTP_NUM;i++)
	{
		if(AC_WTP[i] != NULL)
		{
			for(j=0;j<AC_WTP[i]->RadioCount;j++)
			{
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
				{
					radioid = i*L_RADIO_NUM+j;
					ret = DELETE_WLAN_CHECK_APPLY_RADIO(radioid,WlanID);
					if(ret != 0)
						return ret;
				}
			}
		}
	}
	/*fengwenchao add end*/
	

	AsdWsm_WLANOp(WlanID, WID_DEL, 0);	
	sleep(1);
	/* if the user want to delete this wlan, set this flag to 1, 
	 * and ignore this wlan's information for all the new created wtps
	 * Huangleilei add it for AXSSZFI-1622 , move for AXSSZFI-1740
	 */
	 if (AC_WLAN[WlanID]->want_to_delete == 0)
	 {
		AC_WLAN[WlanID]->want_to_delete = 1;
	 }
	 else
	 {
	 	return WID_WANT_TO_DELETE_WLAN;
	 }
	time_t before_wait;
	time(&before_wait);
	struct timeval tpstart, tpend;
	struct timeval tpdel;
	gettimeofday(&tpstart, NULL);
	{
		
	for(i=0;i<WTP_NUM;i++)
		{
			if (AC_WTP[i] != NULL && AC_WTP[i]->WTPStat == CW_ENTER_RUN)
			{
				for(j = 0;j < AC_WTP[i]->RadioCount; j ++)
				{
					if ((AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
						&& AC_WTP[i]->WTP_Radio[j]->BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] % L_BSS_NUM] != NULL)
					{
						while (/*AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0 */
							AC_WTP[i]->WTP_Radio[j]->BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] % L_BSS_NUM]->enable_wlan_flag == 1
							&& AC_WTP[i]->WTPStat == CW_ENTER_RUN)
						{
							usleep(10000);

							/* if delete-wlan-operation spends too much time (more than 3 seconds) in this thread, 
							 * it will create a new thread to spend it, allowing the parent thread retrun to DBUS the resut
							 */
							gettimeofday(&tpend, NULL);
							if (tpend.tv_usec < tpstart.tv_usec)
								tpdel.tv_sec = tpend.tv_sec - tpstart.tv_sec - 1;
							else
								tpdel.tv_sec = tpend.tv_sec - tpstart.tv_sec;
							tpdel.tv_usec = (tpend.tv_usec < tpstart.tv_usec) ? (tpstart.tv_usec - tpend.tv_usec) : (tpend.tv_usec - tpstart.tv_usec);
							if (tpdel.tv_sec >= 3)
							{
								wid_syslog_debug_debug(WID_DEFAULT, "%s %d: I did not want to wait it....it spend too much time: this is %d seconds %d usecond", __func__, __LINE__, tpdel.tv_sec, tpdel.tv_usec);
								create_thread_flag = 1;
								wid_syslog_debug_debug(WID_DEFAULT, "[%s %d] now, attend create a new thread to do the next operation, because of too much time spending", __func__, __LINE__);
								goto create_thread_flag_out;
								break;
							}
						}
					}
				}
			}
		}
	}
	
create_thread_flag_out:
	wlanid_temp = WlanID;
	wid_syslog_debug_debug(WID_DEFAULT, "%s %d WlanID: %u", __func__, __LINE__, wlanid_temp);
	wlanid_temp &= 0x000000FF;		/* if delete wlan by create a new thread, then the first bit was setted to '1' */
	//struct free_wlan_thread_stuct free_wlan_data;
	//free_wlan_data.WlanID = WlanID;
	//free_wlan_data.vty_fd = fd;
	if (create_thread_flag == 1)
	{
		CWThread free_wlan_thread;	
		wlanid_temp |= 0x80000000;
				
		if(!CWErr(CWCreateThread(&free_wlan_thread, free_wlan, (void *)(wlanid_temp),0)))
		{
			wid_syslog_crit("%s %d Error starting free_wlan Thread", __func__, __LINE__);
			exit(1);
		}
		wid_syslog_debug_debug(WID_DEFAULT, "%s %d create a new thread to delete wlan %d information", __func__, __LINE__, WlanID);
		return DELETE_WLAN_SPEN_TOO_MUCH_TIME;
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT, "%s %d delete wlan %d information", __func__, __LINE__, WlanID);
		free_wlan((void *)wlanid_temp);
		return 0;
	}

	//fengwenchao add end
	return 0;

}

#if 0
int WID_CREATE_NEW_WTP(char *WTPNAME, unsigned int WTPID, char* WTPSN, char* WTPModel,int issn){

	if(gStaticWTPs >= WTP_NUM)
	{
		return WTP_OVER_MAX_NUM;//
	}

	//trap test
	//int tmpvalue = 500;
	//wid_dbus_trap_wtp_code_start(WTPID);
	//wid_dbus_trap_wtp_electrify_register_circle(WTPID, tmpvalue);	
	//wid_dbus_trap_wtp_ap_power_off(WTPID);
	//wid_dbus_trap_wtp_ip_change_alarm(WTPID);
	//wid_dbus_trap_wtp_ap_power_off(WTPID);


	int num, i, j, k,m,ii,jj;
	char RadioType[L_RADIO_NUM];
	char RadioPowerType[L_RADIO_NUM];
	unsigned int RadioExternFlag[L_RADIO_NUM];
	unsigned int gwtpid;
	if(!check_wtpid_func(WTPID)){
		wid_syslog_err("%s\n",__func__);
		return 1;
	}else{
	}
	AC_WTP[WTPID] = (WID_WTP*)malloc(sizeof(WID_WTP));
	memset(AC_WTP[WTPID], 0, sizeof(WID_WTP));
	AC_WTP[WTPID]->WTPNAME = (char*)malloc(strlen(WTPNAME)+1);	
	memset(AC_WTP[WTPID]->WTPNAME,0,strlen(WTPNAME)+1);
	memcpy(AC_WTP[WTPID]->WTPNAME,WTPNAME,strlen(WTPNAME));	
	AC_WTP[WTPID]->WTPIP= (char*)malloc(DEFAULT_LEN);	
	memset(AC_WTP[WTPID]->WTPIP,0,DEFAULT_LEN);

	AC_WTP[WTPID]->WTPID = WTPID;
	AC_WTP[WTPID]->wtp_allowed_max_sta_num=64; //xm
	AC_WTP[WTPID]->wtp_triger_num=1; //xm
	AC_WTP[WTPID]->wtp_flow_triger=0; //xm
	AC_WTP[WTPID]->EchoTimer = gEchoRequestTimer;
	AC_WTP[WTPID]->ap_sta_wapi_report_interval = 10;
	AC_WTP[WTPID]->APGroupID = 0;

	if(issn == 1)
	{
		AC_WTP[WTPID]->WTPMAC = (unsigned char*)malloc(7);	
		memset(AC_WTP[WTPID]->WTPMAC,0,7);
		
		AC_WTP[WTPID]->WTPSN = (char*)malloc(NAS_IDENTIFIER_NAME);
		memset(AC_WTP[WTPID]->WTPSN, 0, NAS_IDENTIFIER_NAME);
		//AC_WTP[WTPID]->WTPSN = (char*)malloc(strlen(WTPSN)+1);
		//memset(AC_WTP[WTPID]->WTPSN, 0, strlen(WTPSN)+1);
		memcpy(AC_WTP[WTPID]->WTPSN, WTPSN, strlen(WTPSN));
	}
	else
	{
		/*default code
		AC_WTP[WTPID]->WTPSN = (char*)malloc(DEFAULT_SN_LENTH+1);
		memset(AC_WTP[WTPID]->WTPSN, 0, DEFAULT_SN_LENTH+1);
		memcpy(AC_WTP[WTPID]->WTPSN, gdefaultsn, DEFAULT_SN_LENTH);
		*/
		AC_WTP[WTPID]->WTPSN = (char*)malloc(NAS_IDENTIFIER_NAME);
		memset(AC_WTP[WTPID]->WTPSN, 0, NAS_IDENTIFIER_NAME);
		memcpy(AC_WTP[WTPID]->WTPSN, gdefaultsn, 20);
		/*used to test ,avoid the point of sn error*/
		AC_WTP[WTPID]->WTPMAC = (unsigned char*)malloc(MAC_LEN+1);	
		memset(AC_WTP[WTPID]->WTPMAC,0,(MAC_LEN+1));
		memcpy(AC_WTP[WTPID]->WTPMAC,(unsigned char *)WTPSN,MAC_LEN);
	}	
	
	AC_WTP[WTPID]->WTPModel = (char*)malloc(strlen(WTPModel)+1);
	memset(AC_WTP[WTPID]->WTPModel, 0, strlen(WTPModel)+1);
	memcpy(AC_WTP[WTPID]->WTPModel, WTPModel, strlen(WTPModel));
	AC_WTP[WTPID]->updateversion = NULL;
	AC_WTP[WTPID]->updatepath = NULL;
	AC_WTP[WTPID]->isipv6addr = 0;
	
	AC_WTP[WTPID]->sendsysstart = 2;	
	AC_WTP[WTPID]->WTPStat = 7;
	AC_WTP[WTPID]->isused = 0;
	AC_WTP[WTPID]->quitreason = WTP_UNUSED;
	//AC_WTP[WTPID]->isBinddingWlan = 0;
	//AC_WTP[WTPID]->BindingWlanCount = 0;
	AC_WTP[WTPID]->tunnel_mode = CW_LOCAL_BRIDGING;
	AC_WTP[WTPID]->CTR_ID = 0;
	AC_WTP[WTPID]->DAT_ID = 0;
	AC_WTP[WTPID]->BindingSock= -1;//initialize binding socket
	AC_WTP[WTPID]->BindingSystemIndex= -1;//initialize binding interface system index
	memset(AC_WTP[WTPID]->BindingIFName,0, ETH_IF_NAME_LEN);
	//AC_WTP[WTPID]->Wlan_Id = NULL;
	AC_WTP[WTPID]->wtp_login_mode = 0;
	AC_WTP[WTPID]->WFR_Index = WTPID*L_RADIO_NUM;
	AC_WTP[WTPID]->CMD = (WID_CMD*)malloc(sizeof(WID_CMD));
	AC_WTP[WTPID]->CMD->CMD = 0;
	AC_WTP[WTPID]->CMD->setCMD = 0;
	AC_WTP[WTPID]->CMD->wlanCMD = 0;
	AC_WTP[WTPID]->NeighborAPInfos = NULL;
	
	AC_WTP[WTPID]->apcminfo.ap_cpu_info_head = NULL;
	AC_WTP[WTPID]->apcminfo.ap_mem_info_head = NULL;
	AC_WTP[WTPID]->apcminfo.ap_snr_info_head = NULL;
	AC_WTP[WTPID]->apcminfo.ap_cpu_info_length = 0;
	AC_WTP[WTPID]->apcminfo.ap_mem_info_length = 0;
	AC_WTP[WTPID]->apcminfo.ap_snr_info_length = 0;
	
	AC_WTP[WTPID]->rouge_ap_infos = NULL;	
	AC_WTP[WTPID]->wids_device_list = NULL;	
	AC_WTP[WTPID]->rx_echocount = 0;
	AC_WTP[WTPID]->ap_ipadd = 0;
	AC_WTP[WTPID]->ap_gateway = 0;
	AC_WTP[WTPID]->ap_mask_new = 0;
	AC_WTP[WTPID]->ap_dnsfirst = 0;
	AC_WTP[WTPID]->ap_dnssecend = 0;
	AC_WTP[WTPID]->resetflag = 0;
	AC_WTP[WTPID]->ap_mask = 0;
	AC_WTP[WTPID]->sysver = NULL;
	AC_WTP[WTPID]->ver = NULL;
	AC_WTP[WTPID]->codever = NULL;
	AC_WTP[WTPID]->add_time = NULL;
	AC_WTP[WTPID]->imagedata_time = 0;
	AC_WTP[WTPID]->config_update_time = 0;
	AC_WTP[WTPID]->ElectrifyRegisterCircle = 0;
	AC_WTP[WTPID]->updateStat = 0;
	AC_WTP[WTPID]->updatefailcount = 0;
	AC_WTP[WTPID]->updatefailstate = 0;
	AC_WTP[WTPID]->manual_update_time = 0;	
	AC_WTP[WTPID]->location = NULL;
	memset(AC_WTP[WTPID]->wep_flag,0,WTP_WEP_NUM);
	AC_WTP[WTPID]->ControlList = NULL;
	AC_WTP[WTPID]->ControlWait = NULL;
	//AC_WTP[WTPID]->netid = NULL;
	AC_WTP[WTPID]->netid = (char*)malloc(sizeof(char)*12);
	memset(AC_WTP[WTPID]->netid, 0, 12);
	memcpy(AC_WTP[WTPID]->netid, "defaultcode", 11);
	memset(AC_WTP[WTPID]->cpuType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(AC_WTP[WTPID]->cpuType,"soc",3);
	memset(AC_WTP[WTPID]->memType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(AC_WTP[WTPID]->memType,"flash",5);
	memset(AC_WTP[WTPID]->flashType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(AC_WTP[WTPID]->flashType,"flash",5);/*wuwl add.when wtp didn't into run ,display this*/
	for(j = 0;j < L_RADIO_NUM; j++)
	{
		AC_WTP[WTPID]->CMD->radioid[j] = 0;
		for(k = 0;k < WLAN_NUM; k++)
		{
			AC_WTP[WTPID]->CMD->radiowlanid[j][k] = 0;
		}
	}
	AC_WTP[WTPID]->wtp_trap_switch = 1;/*enable*/
	AC_WTP[WTPID]->wtp_trap_lev = 1;
	AC_WTP[WTPID]->wtp_cpu_use_threshold = 10;
	AC_WTP[WTPID]->wtp_mem_use_threshold = 30;
	AC_WTP[WTPID]->wtp_rogue_ap_threshold = 10;
	AC_WTP[WTPID]->wtp_rogue_terminal_threshold = 1;

	AC_WTP[WTPID]->apstatisticsinterval = 10;

	AC_WTP[WTPID]->neighborchannelrssithold = -70;//zhaoruijia,20100825,add
	AC_WTP[WTPID]->samechannelrssithold = -80;//zhaoruijia,20100825,add
    AC_WTP[WTPID]->neighborchannel_trap_flag = 0;
	AC_WTP[WTPID]->samechannel_trap_flag = 0;

	AC_WTP[WTPID]->ntp_interval = 3600;/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3*/
	AC_WTP[WTPID]->ntp_state = 1; /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3*/

	AC_WTP[WTPID]->wid_trap.ignore_percent = 0; // zhangshu add 2010-08-27 
	AC_WTP[WTPID]->wid_trap.ignore_switch = 0;  // zhangshu add 2010-08-27 

    AC_WTP[WTPID]->ntp_trap_flag = 0;/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3*/

	
		/*
		if((AC_WLAN[k] != NULL)&&(AC_WLAN[k]->Status == 0))
		{	
			//changed by weiay 20080617
			if(AC_WTP[WTPID]->BindingSystemIndex != -1)
			{
				printf("***wtp index is %d\n",AC_WTP[WTPID]->BindingSystemIndex);
				if(AC_WLAN[k]->Wlan_Ifi != NULL)
				{
					struct ifi * wlan_ifi = AC_WLAN[k]->Wlan_Ifi;
					while(wlan_ifi != NULL)
					{
						printf("*** wlan index is %d\n",wlan_ifi->ifi_index);
						if(AC_WTP[WTPID]->BindingSystemIndex == wlan_ifi->ifi_index)
						{
							break;
						}
						wlan_ifi = wlan_ifi->ifi_next;
					}
					
					if(wlan_ifi == NULL)
					{
						printf("*** wtp binding interface doesn't match with wlan binding interface **\n");
						return -1;
					}
					else
					{
						AC_WTP[WTPID]->CMD->wlanid[k] = 1;
						AC_WTP[WTPID]->CMD->wlanCMD += 1;
					}
				}
				else
				{
					AC_WTP[WTPID]->CMD->wlanid[k] = 1;
					AC_WTP[WTPID]->CMD->wlanCMD += 1;
				}
			}
			//changedn end
			else
			{
				AC_WTP[WTPID]->CMD->wlanid[k] = 1;
				AC_WTP[WTPID]->CMD->wlanCMD += 1;
			}
		}
	}
	*/
	AC_WTP[WTPID]->CMD->staCMD = -1;
	memset(AC_WTP[WTPID]->CMD->StaInf, 0, 8);
	AC_WTP[WTPID]->CMD->keyCMD = -1;
	AC_WTP[WTPID]->CMD->key.BSSIndex = 0;
	AC_WTP[WTPID]->CMD->key.WTPID = 0;
	AC_WTP[WTPID]->CMD->key.key_idx = 0;
	AC_WTP[WTPID]->CMD->key.key_len = 0;
	AC_WTP[WTPID]->CMD->key.cipher = 0;
	memset(AC_WTP[WTPID]->CMD->key.StaAddr, 0, MAC_LEN);
	memset(AC_WTP[WTPID]->CMD->key.key, 0, DEFAULT_LEN);
	for(i=0;i<L_RADIO_NUM;i++)
	{
		AC_WTP[WTPID]->WTP_Radio[i] = NULL;
	}
	CWConfigVersionInfo *pnode = gConfigVersionInfo;
	num = -1;
	while(pnode != NULL)
	{
		//printf("wtp model:%s \n",WTPModel);
		//printf("pnode->str_ap_model:%s \n",pnode->str_ap_model);
		//printf("pnode->str_ap_code:%s \n",pnode->str_ap_code);
		if((strcmp(pnode->str_ap_model,WTPModel) == 0)||(strcmp(pnode->str_ap_code,WTPModel) == 0))
		{
			num = pnode->radio_num;
			
			AC_WTP[WTPID]->APCode= (char*)malloc(strlen(pnode->str_ap_code)+1);
			memset(AC_WTP[WTPID]->APCode, 0, strlen(pnode->str_ap_code)+1);
			memcpy(AC_WTP[WTPID]->APCode, pnode->str_ap_code, strlen(pnode->str_ap_code));
			//printf("model:%s innercode:%s\n",WTPModel,AC_WTP[WTPID]->APCode);
			//save radio type 
			if((strlen(pnode->str_ap_code) > 5)&&(memcmp(pnode->str_ap_code, "ALTAI", 5) == 0)){
				AC_WTP[WTPID]->sendsysstart = 0;
			}
			for(m=0;((m<num)&&(m<L_RADIO_NUM));m++)
			{
				RadioType[m] = pnode->radio_info[m].radio_type;
				RadioPowerType[m] = pnode->radio_info[m].txpower;
				RadioExternFlag[m] = pnode->radio_info[m].extern_flag;
			}
			break;
		}
		pnode = pnode->next;
	}	
	wid_syslog_debug_debug(WID_DEFAULT,"wtp model:%s radio count is:%d\n",WTPModel,num);
	if(num == -1)
	{
		free(AC_WTP[WTPID]->WTPMAC);
		AC_WTP[WTPID]->WTPMAC = NULL;
		free(AC_WTP[WTPID]->WTPIP);
		AC_WTP[WTPID]->WTPIP = NULL;
		free(AC_WTP[WTPID]->WTPNAME);
		AC_WTP[WTPID]->WTPNAME = NULL;
		free(AC_WTP[WTPID]->WTPSN);
		AC_WTP[WTPID]->WTPSN = NULL;
		free(AC_WTP[WTPID]->WTPModel);
		AC_WTP[WTPID]->WTPModel = NULL;
		free(AC_WTP[WTPID]->netid);
		AC_WTP[WTPID]->netid = NULL;
		free(AC_WTP[WTPID]->CMD);
		AC_WTP[WTPID]->CMD = NULL;
		free(AC_WTP[WTPID]);
		AC_WTP[WTPID] = NULL;
		return 1;
	}

	gwtpid = WTPID*L_RADIO_NUM;
	AC_WTP[WTPID]->RadioCount = num;
  //AC_WTP[WTPID]->channelsendtimes = 1;   /*wuwl add for send channel_cont only one time at the beginning of the wtp access*/
	for(i=0; ((i<num)&&(i<L_RADIO_NUM)); i++){	
		AC_RADIO[gwtpid] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
		memset(AC_RADIO[gwtpid], 0, sizeof(WID_WTP_RADIO));
		AC_RADIO[gwtpid]->WTPID = WTPID;
		AC_RADIO[gwtpid]->Radio_G_ID = gwtpid;
		AC_RADIO[gwtpid]->Radio_L_ID = i;
		AC_RADIO[gwtpid]->Radio_Chan = 0;
		AC_RADIO[gwtpid]->ishighpower = RadioPowerType[i];
		if(AC_RADIO[gwtpid]->ishighpower == 1){
			AC_RADIO[gwtpid]->Radio_TXP = 27;
		}
		else{
			AC_RADIO[gwtpid]->Radio_TXP = 20;
		}
		AC_RADIO[gwtpid]->Radio_TXPOF= 0;/*wuwl add */
		AC_RADIO[gwtpid]->channelsendtimes = 1;	 /*wuwl add for send channel_cont only one time at the beginning of the wtp access*/

		if(RadioType[i] == 7){
			AC_RADIO[gwtpid]->Radio_Type = 5;
		}else{
			AC_RADIO[gwtpid]->Radio_Type = RadioType[i];
		}

		AC_RADIO[gwtpid]->Radio_Type_Bank = RadioType[i];

		if((AC_RADIO[gwtpid]->Radio_Type & 0x02) == 0x02)
		{
			AC_RADIO[gwtpid]->Radio_Chan = 149;
		}

		AC_RADIO[gwtpid]->Support_Rate_Count = 0;
		AC_RADIO[gwtpid]->Radio_Rate = NULL;
		//AC_RADIO[gwtpid]->Radio_Rate = 0;// 54m/bps//sz1121 add channel 0,rate 0
		AC_RADIO[gwtpid]->FragThreshold = 2346;
		AC_RADIO[gwtpid]->BeaconPeriod = 100;
		AC_RADIO[gwtpid]->IsShortPreamble = 1;
		AC_RADIO[gwtpid]->DTIMPeriod = 1; //
		AC_RADIO[gwtpid]->ShortRetry = 7; //
		AC_RADIO[gwtpid]->LongRetry = 4; //
		AC_RADIO[gwtpid]->rtsthreshold = 2346;
		AC_RADIO[gwtpid]->AdStat = 2;
		AC_RADIO[gwtpid]->OpStat = 2;
		AC_RADIO[gwtpid]->CMD = 0x0;
		for(ii=0;ii<L_BSS_NUM;ii++){
			AC_RADIO[gwtpid]->BSS[ii] = NULL;
		}
		AC_RADIO[gwtpid]->QOSID = 0;
		AC_RADIO[gwtpid]->QOSstate = 0;
		AC_RADIO[gwtpid]->bandwidth = 108;
		AC_RADIO[gwtpid]->auto_channel = 0;
		AC_RADIO[gwtpid]->auto_channel_cont = 0;
		AC_RADIO[gwtpid]->txpowerautostate = 1;
		AC_RADIO[gwtpid]->wifi_state = 1;
		AC_RADIO[gwtpid]->channelchangetime = 0;
		AC_RADIO[gwtpid]->excommand = NULL;
		AC_RADIO[gwtpid]->diversity = 0;//default is 0
		AC_RADIO[gwtpid]->txantenna = 1;//default is main
		AC_RADIO[gwtpid]->isBinddingWlan = 0;
		AC_RADIO[gwtpid]->BindingWlanCount = 0;
		AC_RADIO[gwtpid]->upcount = 0;
		AC_RADIO[gwtpid]->downcount = 0;
		AC_RADIO[gwtpid]->guardinterval = 1;
		AC_RADIO[gwtpid]->mcs = 0;
		AC_RADIO[gwtpid]->cwmode = 0;
		AC_RADIO[gwtpid]->Wlan_Id = NULL;
		AC_RADIO[gwtpid]->rx_data_deadtime = 10;
		AC_RADIO[gwtpid]->REFlag = RadioExternFlag[i];//zhanglei add for A8
		AC_WTP[WTPID]->WTP_Radio[i] = AC_RADIO[gwtpid];
		memset(AC_RADIO[gwtpid]->br_ifname,0,L_BSS_NUM*IF_NAME_MAX);
			
		/*11n set begin*/
		AC_RADIO[gwtpid]->Ampdu.Able = 1;/*enable*/
		AC_RADIO[gwtpid]->Ampdu.AmpduLimit = 65535;
		AC_RADIO[gwtpid]->MixedGreenfield.Mixed_Greenfield = 1;/*enable*/
		AC_RADIO[gwtpid]->channel_offset =0;
		AC_RADIO[gwtpid]->tx_chainmask_state_value = 3;

		/*a8 chushihua start*/
		if(AC_RADIO[gwtpid]->REFlag == 1){
			AC_RADIO[gwtpid]->sector_state_value = 0 ;
			AC_RADIO[gwtpid]->inter_vap_able = 0;
			AC_RADIO[gwtpid]->intra_vap_able = 0;
			AC_RADIO[gwtpid]->keep_alive_period = 3600;
			AC_RADIO[gwtpid]->keep_alive_idle_time = 3600;
			AC_RADIO[gwtpid]->congestion_avoidance = 0;

		}
	
		for(ii=0;ii<SECTOR_NUM;ii++)
		{	
			//CW_CREATE_OBJECT_ERR(AC_RADIO[gwtpid]->sector[ii],WID_oem_sector,return NULL;);
			AC_RADIO[gwtpid]->sector[ii] = (WID_oem_sector*)malloc(sizeof(WID_oem_sector));
			AC_RADIO[gwtpid]->sector[ii]->state = 0;
			AC_RADIO[gwtpid]->sector[ii]->tx_power = 0;
		}
		for(jj=0;jj<TX_CHANIMASK_NUM;jj++)
		{	
			AC_RADIO[gwtpid]->tx_chainmask[jj] = (WID_oem_tx_chainmask*)malloc(sizeof(WID_oem_tx_chainmask));
			AC_RADIO[gwtpid]->tx_chainmask[jj]->state = 0;
		}
		if((AC_RADIO[gwtpid]->Radio_Type & 0x08) == 0x08)//if mode is 11n,beacon interval set to 400
		{
			AC_RADIO[gwtpid]->BeaconPeriod = 400;
			AC_RADIO[gwtpid]->diversity = 1;// 11 n default is 1
			AC_RADIO[gwtpid]->txantenna = 0;// 11n default is 0
		}
		//set default support rate list 11g&11b/g 12,11b 4
		AC_RADIO[gwtpid]->Support_Rate_Count = 12;

		//memory leak
		//AC_RADIO[gwtpid]->Radio_Rate = (struct Support_Rate_List *)malloc(sizeof(struct Support_Rate_List));
		AC_RADIO[gwtpid]->Radio_Rate = create_support_rate_list(1);//here add 10 first
		
		//printf("/////the list has %d element////\n",AC_RADIO[gwtpid]->Support_Rate_Count);
		
		int rate = 10;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 20;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 55;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 60;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 90;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 110;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 120;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 180;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 240;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 360;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 480;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 540;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
		/*add 11n rate 1300,3000*/
		if((AC_RADIO[gwtpid]->Radio_Type & 0x08) == 0x08){
			rate = 1300;
			AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
			
			rate = 3000;
			AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
		}
		delete_rate_from_list(AC_RADIO[gwtpid]->Radio_Rate,0);
		AC_RADIO[gwtpid]->Support_Rate_Count = length_of_rate_list(AC_RADIO[gwtpid]->Radio_Rate);
		AC_RADIO[gwtpid]->txpowerstep = 1;//zhaoruijia,20100917,add radio txpower step 
		//printf("/////the list has %d element////\n",AC_RADIO[gwtpid]->Support_Rate_Count);
		//display_support_rate_list(AC_RADIO[gwtpid]->Radio_Rate);
		gwtpid++;
	}
	wid_init_wtp_info_in_create(WTPID);
	AC_WTP[WTPID]->wids_statist.floodingcount = 0;
	AC_WTP[WTPID]->wids_statist.sproofcount = 0;
	AC_WTP[WTPID]->wids_statist.weakivcount = 0;
	gStaticWTPs++;
	//only for test no release code //////////////////////////////////
	
	//AC_WTP[WTPID]->NeighborAPInfos = create_ap_info_list(3);
	//AC_WTP[WTPID]->rouge_ap_infos = create_ap_info_list_test(1);
	//display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	//display_ap_info_list(AC_WTP[WTPID]->rouge_ap_infos);

	//merge_ap_list(AC_WTP[WTPID]->NeighborAPInfos,&(AC_WTP[WTPID]->rouge_ap_infos));

	//char *mac = "222222";
	//struct Neighbor_AP_ELE *papnode = create_mac_elem(mac);
	
	//printf("##############################################\n");
	
	//insert_elem_into_ap_list((AC_WTP[WTPID]->NeighborAPInfos),papnode);
	//delete_elem_from_ap_list(&(AC_WTP[WTPID]->NeighborAPInfos),papnode);
	//destroy_ap_info_list(&(AC_WTP[WTPID]->NeighborAPInfos));
	/*
	if(AC_WTP[WTPID]->NeighborAPInfos == NULL)
	{
		printf("NeighborAPInfos == NULL\n");
	}
	else
	{
		printf("NeighborAPInfos != NULL\n");
	}

	if(AC_WTP[WTPID]->rouge_ap_infos== NULL)
	{
		printf("rouge_ap_infos == NULL\n");
	}
	else
	{
		printf("rouge_ap_infos != NULL\n");
	}
	
	display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	display_ap_info_list(AC_WTP[WTPID]->rouge_ap_infos);

	*/

	//display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	//destroy_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	//printf("////////////001//////////\n");
	/*
	
	AC_WTP[WTPID]->NeighborAPInfos = create_ap_info_list(2);
	display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	destroy_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);

	printf("////////////002//////////\n");

	AC_WTP[WTPID]->NeighborAPInfos = create_ap_info_list(3);
	display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	destroy_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);

	/////////////////////////////////////////////////////
	
	*/


	//AC_WTP[WTPID]->wids_device_list = create_wids_info_list(3);

	
	return 0 ;
	
}
#endif

int WID_CREATE_NEW_WTP(char *WTPNAME, unsigned int WTPID, unsigned char* WTPSN, char* WTPModel,int issn, int apcodeflag, char* code){

	if((gStaticWTPs >= WTP_NUM)||((gStaticWTPs > gMaxWTPs)))
	{
		return WTP_OVER_MAX_NUM;//
	}

	//trap test
	//int tmpvalue = 500;
	//wid_dbus_trap_wtp_code_start(WTPID);
	//wid_dbus_trap_wtp_electrify_register_circle(WTPID, tmpvalue);	
	//wid_dbus_trap_wtp_ap_power_off(WTPID);
	//wid_dbus_trap_wtp_ip_change_alarm(WTPID);
	//wid_dbus_trap_wtp_ap_power_off(WTPID);


	int num, i, j, k,m,ii,jj;
	char RadioType[L_RADIO_NUM];
	char RadioPowerType[L_RADIO_NUM];
	char RadioChainmask[L_RADIO_NUM];//zhangshu add
	unsigned int RadioExternFlag[L_RADIO_NUM];
	unsigned int gwtpid;
	if(!check_wtpid_func(WTPID)){
		wid_syslog_err("%s\n",__func__);
		return 1;
	}else{
	}
	AC_WTP[WTPID] = (WID_WTP*)malloc(sizeof(WID_WTP));
	memset(AC_WTP[WTPID], 0, sizeof(WID_WTP));
	AC_WTP[WTPID]->WTPNAME = (char*)malloc(strlen(WTPNAME)+1);	
	memset(AC_WTP[WTPID]->WTPNAME,0,strlen(WTPNAME)+1);
	memcpy(AC_WTP[WTPID]->WTPNAME,WTPNAME,strlen(WTPNAME));	
	AC_WTP[WTPID]->WTPIP= (char*)malloc(DEFAULT_LEN);	
	memset(AC_WTP[WTPID]->WTPIP,0,DEFAULT_LEN);

	AC_WTP[WTPID]->WTPID = WTPID;
	AC_WTP[WTPID]->wtp_allowed_max_sta_num=gWTP_MAX_STA; //xm/*wcl modify for globle variable*/
	AC_WTP[WTPID]->wtp_triger_num=1; //xm
	AC_WTP[WTPID]->dhcp_snooping = gDHCP_SNOOPING;/*wcl add for globle variable*/
	AC_WTP[WTPID]->wtp_flow_triger=gWTP_FLOW_TRIGER; //xm/*wcl modify for globle variable*/
	AC_WTP[WTPID]->EchoTimer = gEchoRequestTimer;
	AC_WTP[WTPID]->ap_sta_wapi_report_interval = gAP_STA_WAPI_REPORT_INTERVAL;/*wcl modify for globle variable*/
	AC_WTP[WTPID]->ap_sta_wapi_report_switch = gAP_STA_WAPI_REPORT_SWITCH;/*wcl add for globle variable*/
	AC_WTP[WTPID]->APGroupID = 0;

	if(issn == 1)
	{
		AC_WTP[WTPID]->WTPMAC = (unsigned char*)malloc(7);	
		memset(AC_WTP[WTPID]->WTPMAC,0,7);
		
		AC_WTP[WTPID]->WTPSN = (char*)malloc(NAS_IDENTIFIER_NAME);
		memset(AC_WTP[WTPID]->WTPSN, 0, NAS_IDENTIFIER_NAME);
		//AC_WTP[WTPID]->WTPSN = (char*)malloc(strlen(WTPSN)+1);
		//memset(AC_WTP[WTPID]->WTPSN, 0, strlen(WTPSN)+1);
		memcpy(AC_WTP[WTPID]->WTPSN, WTPSN, strlen((char *)WTPSN));
	}
	else
	{
		/*default code
		AC_WTP[WTPID]->WTPSN = (char*)malloc(DEFAULT_SN_LENTH+1);
		memset(AC_WTP[WTPID]->WTPSN, 0, DEFAULT_SN_LENTH+1);
		memcpy(AC_WTP[WTPID]->WTPSN, gdefaultsn, DEFAULT_SN_LENTH);
		*/
		AC_WTP[WTPID]->WTPSN = (char*)malloc(NAS_IDENTIFIER_NAME);
		memset(AC_WTP[WTPID]->WTPSN, 0, NAS_IDENTIFIER_NAME);
		memcpy(AC_WTP[WTPID]->WTPSN, gdefaultsn, 20);
		/*used to test ,avoid the point of sn error*/
		AC_WTP[WTPID]->WTPMAC = (unsigned char*)malloc(MAC_LEN+1);	
		memset(AC_WTP[WTPID]->WTPMAC,0,(MAC_LEN+1));
		memcpy(AC_WTP[WTPID]->WTPMAC,(unsigned char *)WTPSN,MAC_LEN);
	}	
	
	AC_WTP[WTPID]->WTPModel = (char*)malloc(strlen(WTPModel)+1);
	memset(AC_WTP[WTPID]->WTPModel, 0, strlen(WTPModel)+1);
	memcpy(AC_WTP[WTPID]->WTPModel, WTPModel, strlen(WTPModel));
	AC_WTP[WTPID]->updateversion = NULL;
	AC_WTP[WTPID]->updatepath = NULL;
	AC_WTP[WTPID]->isipv6addr = 0;
	
	AC_WTP[WTPID]->sendsysstart = 2;	
	AC_WTP[WTPID]->WTPStat = 7;
	AC_WTP[WTPID]->isused = 0;
	AC_WTP[WTPID]->unused_flag = 0;
	AC_WTP[WTPID]->quitreason = WTP_UNUSED;
	//AC_WTP[WTPID]->isBinddingWlan = 0;
	//AC_WTP[WTPID]->BindingWlanCount = 0;
	AC_WTP[WTPID]->tunnel_mode = CW_LOCAL_BRIDGING;
	AC_WTP[WTPID]->CTR_ID = 0;
	AC_WTP[WTPID]->DAT_ID = 0;
	AC_WTP[WTPID]->BindingSock= -1;//initialize binding socket
	AC_WTP[WTPID]->BindingSystemIndex= -1;//initialize binding interface system index
	memset(AC_WTP[WTPID]->BindingIFName,0, ETH_IF_NAME_LEN);
	//AC_WTP[WTPID]->Wlan_Id = NULL;
	AC_WTP[WTPID]->wtp_login_mode = 0;
	AC_WTP[WTPID]->WFR_Index = WTPID*L_RADIO_NUM;
	AC_WTP[WTPID]->CMD = (WID_CMD*)malloc(sizeof(WID_CMD));
	AC_WTP[WTPID]->CMD->CMD = 0;
	AC_WTP[WTPID]->CMD->setCMD = 0;
	AC_WTP[WTPID]->CMD->wlanCMD = 0;
	AC_WTP[WTPID]->NeighborAPInfos = NULL;
	
	AC_WTP[WTPID]->apcminfo.ap_cpu_info_head = NULL;
	AC_WTP[WTPID]->apcminfo.ap_mem_info_head = NULL;
	AC_WTP[WTPID]->apcminfo.ap_snr_info_head = NULL;
	AC_WTP[WTPID]->apcminfo.ap_cpu_info_length = 0;
	AC_WTP[WTPID]->apcminfo.ap_mem_info_length = 0;
	AC_WTP[WTPID]->apcminfo.ap_snr_info_length = 0;
	
	AC_WTP[WTPID]->rouge_ap_infos = NULL;	
	AC_WTP[WTPID]->wids_device_list = NULL;	
	AC_WTP[WTPID]->rx_echocount = 0;
	AC_WTP[WTPID]->ap_ipadd = 0;
	AC_WTP[WTPID]->ap_gateway = 0;
	AC_WTP[WTPID]->ap_mask_new = 0;
	AC_WTP[WTPID]->ap_dnsfirst = 0;
	AC_WTP[WTPID]->ap_dnssecend = 0;
	AC_WTP[WTPID]->resetflag = 0;
	AC_WTP[WTPID]->ap_mask = 0;
	AC_WTP[WTPID]->sysver = NULL;
	AC_WTP[WTPID]->ver = NULL;
	AC_WTP[WTPID]->codever = NULL;
	AC_WTP[WTPID]->ApReportVer = NULL;    /*fengwenchao add 20110216*/
	AC_WTP[WTPID]->add_time = NULL;
	AC_WTP[WTPID]->imagedata_time = 0;
	AC_WTP[WTPID]->config_update_time = 0;
	AC_WTP[WTPID]->ElectrifyRegisterCircle = 0;
	AC_WTP[WTPID]->updateStat = 0;
	AC_WTP[WTPID]->updatefailcount = 0;
	AC_WTP[WTPID]->updatefailstate = 0;
	AC_WTP[WTPID]->manual_update_time = 0;	
	AC_WTP[WTPID]->location = NULL;
	AC_WTP[WTPID]->option60_param = NULL;
	//memset(AC_WTP[WTPID]->wep_flag,0,WTP_WEP_NUM);
	AC_WTP[WTPID]->ControlList = NULL;
	AC_WTP[WTPID]->ControlWait = NULL;
	AC_WTP[WTPID]->sta_ip_report = gSTAINFOREPORT ; /*wcl add for globle variable*/
	//AC_WTP[WTPID]->netid = NULL;
	AC_WTP[WTPID]->netid = (char*)malloc(sizeof(char)*12);
	memset(AC_WTP[WTPID]->netid, 0, 12);
	memcpy(AC_WTP[WTPID]->netid, "defaultcode", 11);
	memset(AC_WTP[WTPID]->cpuType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(AC_WTP[WTPID]->cpuType,"soc",3);
	memset(AC_WTP[WTPID]->memType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(AC_WTP[WTPID]->memType,"flash",5);
	memset(AC_WTP[WTPID]->flashType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(AC_WTP[WTPID]->flashType,"flash",5);/*wuwl add.when wtp didn't into run ,display this*/
	for(j = 0;j < L_RADIO_NUM; j++)
	{
		AC_WTP[WTPID]->CMD->radioid[j] = 0;
		for(k = 0;k < WLAN_NUM; k++)
		{
			AC_WTP[WTPID]->CMD->radiowlanid[j][k] = 0;
		}
	}
	AC_WTP[WTPID]->wtp_trap_switch = 1;/*enable*/
	AC_WTP[WTPID]->wtp_seqnum_switch = 1; /*wcl add*/
	AC_WTP[WTPID]->wtp_trap_lev = 1;
	AC_WTP[WTPID]->wtp_cpu_use_threshold = gWTP_CPU_USE_THRESHOLD;/*wcl modify for globle variable*/
	AC_WTP[WTPID]->wtp_mem_use_threshold = gWTP_MEM_USE_THRESHOLD;/*wcl modify for globle variable*/
	AC_WTP[WTPID]->wtp_rogue_ap_threshold = gWTP_ROGUE_AP_THRESHOLD;/*wcl modify for globle variable*/
	AC_WTP[WTPID]->wtp_rogue_terminal_threshold = gWTP_ROGUE_TERMINAL_THRESHOLD;/*wcl modify for globle variable*/

	AC_WTP[WTPID]->apstatisticsinterval = apstatisticsinterval;/*wcl modify for globle variable*/
	AC_WTP[WTPID]->dhcp_snooping = gDHCP_SNOOPING;/*wcl add for globle variable*/

	AC_WTP[WTPID]->neighborchannelrssithold = gNEIGHBORCHANNELRSSITHOLD;//zhaoruijia,20100825,add /*wcl modify for goble variable*/
	AC_WTP[WTPID]->samechannelrssithold = gSAMECHANNELRSSITHOLD;//zhaoruijia,20100825,add/*wcl modify for goble variable*/
    AC_WTP[WTPID]->neighborchannel_trap_flag = 0;
	AC_WTP[WTPID]->channel_device_interference_flag = 0;   /*fengwenchao add 20110221 for wid_dbus_trap_wtp_channel_device_interference*/
	AC_WTP[WTPID]->ap_rogue_threshold_flag = 1;            /*fengwenchao add 20110221 for wid_dbus_trap_ap_rogue_threshold */
	AC_WTP[WTPID]->ac_discovery_danger_ap_flag = 1;        /*fengwenchao add 20110221 for wid_dbus_trap_wtp_ac_discovery_danger_ap*/ 
	AC_WTP[WTPID]->find_wids_attack_flag = 1;              /*fengwenchao add 20110221 for wid_dbus_trap_wtp_find_wids_attack */
	AC_WTP[WTPID]->channel_count_minor_flag = 0;           /*fengwenchao add 20110221 for wid_dbus_trap_wtp_channel_count_minor */
	AC_WTP[WTPID]->samechannel_trap_flag = 0;
	AC_WTP[WTPID]->sta_ip_report = gSTAINFOREPORT ; /*wcl add for globle variable*/
	AC_WTP[WTPID]->ap_sta_wapi_report_switch = gAP_STA_WAPI_REPORT_SWITCH;/*wcl add for globle variable*/

	AC_WTP[WTPID]->ntp_interval = gNTP_INTERVAL;/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3*//*wcl modify for globle variable*/
	AC_WTP[WTPID]->ntp_state = gNTP_STATE; /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3*//*wcl modify for globle variable*/
	AC_WTP[WTPID]->elem_num = 0;
	AC_WTP[WTPID]->wid_trap.ignore_percent = 0; // zhangshu add 2010-08-27 
	AC_WTP[WTPID]->wid_trap.ignore_switch = 0;  // zhangshu add 2010-08-27 
	AC_WTP[WTPID]->wid_trap.rogue_terminal_trap_flag = 0;  // fengwenchao add 20111116 for AXSSZFI-558
	/* zhangshu add for init terminal dis info, 2010-10-08 */
	AC_WTP[WTPID]->ter_dis_info.reportpkt = gTER_DIS_INFOREPORTPKT;/*wcl modify for globle variable*/
	AC_WTP[WTPID]->ter_dis_info.reportswitch= gTER_DIS_INFOREPORTSWITCH;/*wcl modify for globle variable*/
	AC_WTP[WTPID]->ter_dis_info.sta_trap_count = gTER_DIS_INFOSTA_TRAP_COUNT;/*wcl modify for globle variable*/

    AC_WTP[WTPID]->ntp_trap_flag = 0;/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3*/

	AC_WTP[WTPID]->radio_5g_sw = g_radio_5g_sw;
	
		/*
		if((AC_WLAN[k] != NULL)&&(AC_WLAN[k]->Status == 0))
		{	
			//changed by weiay 20080617
			if(AC_WTP[WTPID]->BindingSystemIndex != -1)
			{
				printf("***wtp index is %d\n",AC_WTP[WTPID]->BindingSystemIndex);
				if(AC_WLAN[k]->Wlan_Ifi != NULL)
				{
					struct ifi * wlan_ifi = AC_WLAN[k]->Wlan_Ifi;
					while(wlan_ifi != NULL)
					{
						printf("*** wlan index is %d\n",wlan_ifi->ifi_index);
						if(AC_WTP[WTPID]->BindingSystemIndex == wlan_ifi->ifi_index)
						{
							break;
						}
						wlan_ifi = wlan_ifi->ifi_next;
					}
					
					if(wlan_ifi == NULL)
					{
						printf("*** wtp binding interface doesn't match with wlan binding interface **\n");
						return -1;
					}
					else
					{
						AC_WTP[WTPID]->CMD->wlanid[k] = 1;
						AC_WTP[WTPID]->CMD->wlanCMD += 1;
					}
				}
				else
				{
					AC_WTP[WTPID]->CMD->wlanid[k] = 1;
					AC_WTP[WTPID]->CMD->wlanCMD += 1;
				}
			}
			//changedn end
			else
			{
				AC_WTP[WTPID]->CMD->wlanid[k] = 1;
				AC_WTP[WTPID]->CMD->wlanCMD += 1;
			}
		}
	}
	*/
	AC_WTP[WTPID]->CMD->staCMD = -1;
	memset(AC_WTP[WTPID]->CMD->StaInf, 0, 8);
	AC_WTP[WTPID]->CMD->keyCMD = -1;
	AC_WTP[WTPID]->CMD->key.BSSIndex = 0;
	AC_WTP[WTPID]->CMD->key.WTPID = 0;
	AC_WTP[WTPID]->CMD->key.key_idx = 0;
	AC_WTP[WTPID]->CMD->key.key_len = 0;
	AC_WTP[WTPID]->CMD->key.cipher = 0;
	memset(AC_WTP[WTPID]->CMD->key.StaAddr, 0, MAC_LEN);
	memset(AC_WTP[WTPID]->CMD->key.key, 0, DEFAULT_LEN);
	for(i=0;i<L_RADIO_NUM;i++)
	{
		AC_WTP[WTPID]->WTP_Radio[i] = NULL;
	}
	CWConfigVersionInfo *pnode = gConfigVersionInfo;
	num = -1;
	while(pnode != NULL)
	{
		//printf("wtp model:%s \n",WTPModel);
		//printf("pnode->str_ap_model:%s \n",pnode->str_ap_model);
		//printf("pnode->str_ap_code:%s \n",pnode->str_ap_code);

		//book modify, 2011-10-12
		if(strcmp(pnode->str_ap_model,WTPModel) == 0)
		{
			num = pnode->radio_num;
			AC_WTP[WTPID]->apifinfo.eth_num = pnode->eth_num;    //fengwenchao add 20110407
			AC_WTP[WTPID]->apcodeflag = 0;

			if(code != NULL)
			{
			    if(AC_WTP[WTPID]->APCode != NULL){
					free(AC_WTP[WTPID]->APCode);
					AC_WTP[WTPID]->APCode = NULL;
				}
                AC_WTP[WTPID]->APCode = (char*)malloc(strlen(code)+1);
                memset(AC_WTP[WTPID]->APCode, 0, strlen(code)+1);
                memcpy(AC_WTP[WTPID]->APCode, code, strlen(code));
			        
			}
			//printf("model:%s innercode:%s\n",WTPModel,AC_WTP[WTPID]->APCode);
			//save radio type 
			/*
			if((pnode->str_ap_code != NULL)&&(strlen(pnode->str_ap_code) > 5)&&(memcmp(pnode->str_ap_code, "ALTAI", 5) == 0)){
				AC_WTP[WTPID]->sendsysstart = 0;
			}
			*/
			for(m=0;((m<num)&&(m<L_RADIO_NUM));m++)
			{
				RadioType[m] = pnode->radio_info[m].radio_type;
				RadioPowerType[m] = pnode->radio_info[m].txpower;
				RadioExternFlag[m] = pnode->radio_info[m].extern_flag;
				RadioChainmask[m] = pnode->radio_info[m].chainmask_num;//zhangshu add
			}
			break;
		}
		pnode = pnode->next;
	}	
	wid_syslog_debug_debug(WID_DEFAULT,"wtp model:%s radio count is:%d\n",WTPModel,num);
	if(num == -1)
	{
		free(AC_WTP[WTPID]->WTPMAC);
		AC_WTP[WTPID]->WTPMAC = NULL;
		free(AC_WTP[WTPID]->WTPIP);
		AC_WTP[WTPID]->WTPIP = NULL;
		free(AC_WTP[WTPID]->WTPNAME);
		AC_WTP[WTPID]->WTPNAME = NULL;
		free(AC_WTP[WTPID]->WTPSN);
		AC_WTP[WTPID]->WTPSN = NULL;
		free(AC_WTP[WTPID]->WTPModel);
		AC_WTP[WTPID]->WTPModel = NULL;
		free(AC_WTP[WTPID]->netid);
		AC_WTP[WTPID]->netid = NULL;
		free(AC_WTP[WTPID]->CMD);
		AC_WTP[WTPID]->CMD = NULL;
		free(AC_WTP[WTPID]);
		AC_WTP[WTPID] = NULL;
		return 1;
	}

	gwtpid = WTPID*L_RADIO_NUM;
	AC_WTP[WTPID]->RadioCount = num;
  //AC_WTP[WTPID]->channelsendtimes = 1;   /*wuwl add for send channel_cont only one time at the beginning of the wtp access*/
	for(i=0; ((i<num)&&(i<L_RADIO_NUM)); i++){	
		AC_RADIO[gwtpid] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
		memset(AC_RADIO[gwtpid], 0, sizeof(WID_WTP_RADIO));
		AC_RADIO[gwtpid]->WTPID = WTPID;
		AC_RADIO[gwtpid]->Radio_G_ID = gwtpid;
		AC_RADIO[gwtpid]->Radio_L_ID = i;
		AC_RADIO[gwtpid]->Radio_Chan = 0;
		AC_RADIO[gwtpid]->ishighpower = RadioPowerType[i];
		AC_RADIO[gwtpid]->Radio_country_code = gCOUNTRYCODE; /*wcl add for OSDEVTDPB-31*/
		if(AC_RADIO[gwtpid]->ishighpower == 1){
			AC_RADIO[gwtpid]->Radio_TXP = 27;
		}
		else{
			AC_RADIO[gwtpid]->Radio_TXP = 20;
		}
		AC_RADIO[gwtpid]->Radio_TXPOF= 0;/*wuwl add */
		AC_RADIO[gwtpid]->channelsendtimes = 1;	 /*wuwl add for send channel_cont only one time at the beginning of the wtp access*/

		if(RadioType[i] == 7){
			AC_RADIO[gwtpid]->Radio_Type = 5;
		}else{
			AC_RADIO[gwtpid]->Radio_Type = RadioType[i];
		}

		AC_RADIO[gwtpid]->Radio_Type_Bank = RadioType[i];

		if((AC_RADIO[gwtpid]->Radio_Type & 0x02) == 0x02)
		{
			AC_RADIO[gwtpid]->Radio_Chan = 149;
		}

		AC_RADIO[gwtpid]->Support_Rate_Count = 0;
		AC_RADIO[gwtpid]->txpowerstep = 1;//zhaoruijia,20100917,add radio txpower step 
		AC_RADIO[gwtpid]->Radio_Rate = NULL;
		//AC_RADIO[gwtpid]->Radio_Rate = 0;// 54m/bps//sz1121 add channel 0,rate 0
		AC_RADIO[gwtpid]->FragThreshold = 2346;
		AC_RADIO[gwtpid]->BeaconPeriod = 100;
		AC_RADIO[gwtpid]->IsShortPreamble = 1;
		AC_RADIO[gwtpid]->DTIMPeriod = 1; //
		AC_RADIO[gwtpid]->ShortRetry = 7; //
		AC_RADIO[gwtpid]->LongRetry = 4; //
		AC_RADIO[gwtpid]->rtsthreshold = 2346;//zhangshu modify for rts initial, Huang Leilei change it for AXSSZFI-1406, 2012-01-09
		AC_RADIO[gwtpid]->AdStat = 2;
		AC_RADIO[gwtpid]->OpStat = 2;
		AC_RADIO[gwtpid]->CMD = 0x0;
		for(ii=0;ii<L_BSS_NUM;ii++){
			AC_RADIO[gwtpid]->BSS[ii] = NULL;
		}
		AC_RADIO[gwtpid]->QOSID = 0;
		AC_RADIO[gwtpid]->QOSstate = 0;
		AC_RADIO[gwtpid]->bandwidth = gBANDWIDTH;/*wcl modify for globle variable*/
		AC_RADIO[gwtpid]->auto_channel = 0;
		AC_RADIO[gwtpid]->auto_channel_cont = 0;
		AC_RADIO[gwtpid]->txpowerautostate = 1;
		AC_RADIO[gwtpid]->wifi_state = 1;
		AC_RADIO[gwtpid]->radio_countermeasures_flag = 0;  //fengwenchao add 20110325
		AC_RADIO[gwtpid]->channelchangetime = 0;
		AC_RADIO[gwtpid]->excommand = NULL;
		AC_RADIO[gwtpid]->diversity = 0;//default is 0
		AC_RADIO[gwtpid]->txantenna = 1;//default is main
		AC_RADIO[gwtpid]->isBinddingWlan = 0;
		AC_RADIO[gwtpid]->BindingWlanCount = 0;
		AC_RADIO[gwtpid]->upcount = 0;
		AC_RADIO[gwtpid]->downcount = 0;
		AC_RADIO[gwtpid]->guardinterval = 1;
		//AC_RADIO[gwtpid]->mcs = 0;   fengwenchao modify 20110411
		AC_RADIO[gwtpid]->cwmode = 0;
		AC_RADIO[gwtpid]->Wlan_Id = NULL;
		AC_RADIO[gwtpid]->REFlag = RadioExternFlag[i];//zhanglei add for A8
		AC_WTP[WTPID]->WTP_Radio[i] = AC_RADIO[gwtpid];
		memset(AC_RADIO[gwtpid]->br_ifname,0,WLAN_NUM*IF_NAME_MAX);
		/*wcl add for RDIR-33*/
		AC_RADIO[gwtpid]->ack.Op = ACK_timeout;
		AC_RADIO[gwtpid]->ack.Type = RADIO;
		AC_RADIO[gwtpid]->ack.state = 0;
		AC_RADIO[gwtpid]->ack.distance = 0;
		/*wcl add end*/
		memset(AC_RADIO[gwtpid]->wep_flag,0,WTP_WEP_NUM);	//fengwenchao change
		/*11n set begin*/
		AC_RADIO[gwtpid]->Ampdu.Op = Ampdu_op;//zhangshu add
		AC_RADIO[gwtpid]->Ampdu.Type = RADIO; //zhangshu add
		AC_RADIO[gwtpid]->Ampdu.Able = 1;/*enable*/
		AC_RADIO[gwtpid]->Ampdu.AmpduLimit = 65535;
		AC_RADIO[gwtpid]->Ampdu.subframe= 32;  //zhangshu add  2010-10-09
		if((AC_RADIO[gwtpid]->Radio_Type == 10)||(AC_RADIO[gwtpid]->Radio_Type == 12))  //fengwenchao add 20120716 for autelan-3057
			AC_RADIO[gwtpid]->MixedGreenfield.Mixed_Greenfield = 1;/*enable*///book modify
		else
			AC_RADIO[gwtpid]->MixedGreenfield.Mixed_Greenfield = 0;
		AC_RADIO[gwtpid]->channel_offset =0;
		AC_RADIO[gwtpid]->radio_disable_flag = 0;/*fengwenchao add 20110920 for radio disable config save*/
		AC_RADIO[gwtpid]->chainmask_num = RadioChainmask[i];//zhangshu add for default chainmask number
		memset(AC_RADIO[gwtpid]->mcs_list,0,L_BSS_NUM);  //fengwenchao add 20120314 for requirements-407
		AC_RADIO[gwtpid]->mcs_count = 1;
		if(0 == AC_RADIO[gwtpid]->chainmask_num)  //use for no chainmask xml configuration
		    AC_RADIO[gwtpid]->chainmask_num = 1;
		
		if(2 == AC_RADIO[gwtpid]->chainmask_num)//zhangshu add
		{
		    AC_RADIO[gwtpid]->tx_chainmask_state_value = 3;
		    AC_RADIO[gwtpid]->rx_chainmask_state_value = 3; 
		}
		else if(3 == AC_RADIO[gwtpid]->chainmask_num)
		{
		    AC_RADIO[gwtpid]->tx_chainmask_state_value = 7;
		    AC_RADIO[gwtpid]->rx_chainmask_state_value = 7; 
		}
		else
		{
		    AC_RADIO[gwtpid]->tx_chainmask_state_value = 1;
		    AC_RADIO[gwtpid]->rx_chainmask_state_value = 1; 
		}
		/*fengwenchao add 20120314 forrequirements-407*/
		int j = 0;
		if(AC_RADIO[gwtpid]->chainmask_num == 1)
		{
			for(j=0;j<8; j++)
			{
				AC_RADIO[gwtpid]->mcs_list[j] = j;
			}
			AC_RADIO[gwtpid]->mcs_count = 8;
		}
		else if(AC_RADIO[gwtpid]->chainmask_num == 2)
		{
			for(j=0;j<16; j++)
			{
				AC_RADIO[gwtpid]->mcs_list[j] = j;
			}
			AC_RADIO[gwtpid]->mcs_count = 16;
		}
		else if(AC_RADIO[gwtpid]->chainmask_num == 3)
		{
			for(j=0;j<24; j++)
			{
				AC_RADIO[gwtpid]->mcs_list[j] = j;
			}
			AC_RADIO[gwtpid]->mcs_count = 24;
		}
		/*fengwenchao add end*/
		/* zhangshu add for amsdu init, 2010-10-09 */
		AC_RADIO[gwtpid]->Amsdu.Op = Amsdu_op;
		AC_RADIO[gwtpid]->Amsdu.Type = RADIO;
		AC_RADIO[gwtpid]->Amsdu.Able = 0;/*disable*/
		AC_RADIO[gwtpid]->Amsdu.AmsduLimit = 4000;
		AC_RADIO[gwtpid]->Amsdu.subframe = 32;

		/*a8 chushihua start*/
		if(AC_RADIO[gwtpid]->REFlag == 1){
			AC_RADIO[gwtpid]->sector_state_value = 0 ;
			AC_RADIO[gwtpid]->inter_vap_able = 0;
			AC_RADIO[gwtpid]->intra_vap_able = 0;
			AC_RADIO[gwtpid]->keep_alive_period = 3600;
			AC_RADIO[gwtpid]->keep_alive_idle_time = 3600;
			AC_RADIO[gwtpid]->congestion_avoidance = 0;

		}
	
		for(ii=0;ii<SECTOR_NUM;ii++)
		{	
			//CW_CREATE_OBJECT_ERR(AC_RADIO[gwtpid]->sector[ii],WID_oem_sector,return NULL;);
			AC_RADIO[gwtpid]->sector[ii] = (WID_oem_sector*)malloc(sizeof(WID_oem_sector));
			AC_RADIO[gwtpid]->sector[ii]->state = 0;
			AC_RADIO[gwtpid]->sector[ii]->tx_power = 0;
		}
		for(jj=0;jj<TX_CHANIMASK_NUM;jj++)
		{	
			AC_RADIO[gwtpid]->tx_chainmask[jj] = (WID_oem_tx_chainmask*)malloc(sizeof(WID_oem_tx_chainmask));
			AC_RADIO[gwtpid]->tx_chainmask[jj]->state = 0;
		}
		if((AC_RADIO[gwtpid]->Radio_Type & 0x08) == 0x08)//if mode is 11n,beacon interval set to 400
		{
			AC_RADIO[gwtpid]->BeaconPeriod = 400;
			AC_RADIO[gwtpid]->diversity = 1;// 11 n default is 1
			AC_RADIO[gwtpid]->txantenna = 0;// 11n default is 0
		}
		//set default support rate list 11g&11b/g 12,11b 4
		AC_RADIO[gwtpid]->Support_Rate_Count = 12;

		//memory leak
		//AC_RADIO[gwtpid]->Radio_Rate = (struct Support_Rate_List *)malloc(sizeof(struct Support_Rate_List));
		AC_RADIO[gwtpid]->Radio_Rate = create_support_rate_list(1);//here add 10 first
		
		//printf("/////the list has %d element////\n",AC_RADIO[gwtpid]->Support_Rate_Count);
		
		int rate = 10;
		if ((AC_RADIO[gwtpid]->Radio_Type & 0x01)
			|| (AC_RADIO[gwtpid]->Radio_Type & 0x04))
		{
			 rate = 10;
			AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 20;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 55;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
				
				rate = 110;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
		}

				rate = 60;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);

		rate = 540;
		AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
		/*add 11n rate 1300,3000*/
		if((AC_RADIO[gwtpid]->Radio_Type & 0x08) == 0x08){
			rate = 1300;
			AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
			
			rate = 3000;
			AC_RADIO[gwtpid]->Radio_Rate = insert_rate_into_list(AC_RADIO[gwtpid]->Radio_Rate,rate);
		}
		delete_rate_from_list(AC_RADIO[gwtpid]->Radio_Rate,0);
		AC_RADIO[gwtpid]->Support_Rate_Count = length_of_rate_list(AC_RADIO[gwtpid]->Radio_Rate);
		//printf("/////the list has %d element////\n",AC_RADIO[gwtpid]->Support_Rate_Count);
		//display_support_rate_list(AC_RADIO[gwtpid]->Radio_Rate);


		AC_RADIO[gwtpid]->StartService.times = -1;
		AC_RADIO[gwtpid]->StopService.times = -1;





		gwtpid++;
	}
	wid_init_wtp_info_in_create(WTPID);
	AC_WTP[WTPID]->wids_statist.floodingcount = 0;
	AC_WTP[WTPID]->wids_statist.sproofcount = 0;
	AC_WTP[WTPID]->wids_statist.weakivcount = 0;
	gStaticWTPs++;
	//only for test no release code //////////////////////////////////
	
	//AC_WTP[WTPID]->NeighborAPInfos = create_ap_info_list(3);
	//AC_WTP[WTPID]->rouge_ap_infos = create_ap_info_list_test(1);
	//display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	//display_ap_info_list(AC_WTP[WTPID]->rouge_ap_infos);

	//merge_ap_list(AC_WTP[WTPID]->NeighborAPInfos,&(AC_WTP[WTPID]->rouge_ap_infos));

	//char *mac = "222222";
	//struct Neighbor_AP_ELE *papnode = create_mac_elem(mac);
	
	//printf("##############################################\n");
	
	//insert_elem_into_ap_list((AC_WTP[WTPID]->NeighborAPInfos),papnode);
	//delete_elem_from_ap_list(&(AC_WTP[WTPID]->NeighborAPInfos),papnode);
	//destroy_ap_info_list(&(AC_WTP[WTPID]->NeighborAPInfos));
	/*
	if(AC_WTP[WTPID]->NeighborAPInfos == NULL)
	{
		printf("NeighborAPInfos == NULL\n");
	}
	else
	{
		printf("NeighborAPInfos != NULL\n");
	}

	if(AC_WTP[WTPID]->rouge_ap_infos== NULL)
	{
		printf("rouge_ap_infos == NULL\n");
	}
	else
	{
		printf("rouge_ap_infos != NULL\n");
	}
	
	display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	display_ap_info_list(AC_WTP[WTPID]->rouge_ap_infos);

	*/

	//display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	//destroy_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	//printf("////////////001//////////\n");
	/*
	
	AC_WTP[WTPID]->NeighborAPInfos = create_ap_info_list(2);
	display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	destroy_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);

	printf("////////////002//////////\n");

	AC_WTP[WTPID]->NeighborAPInfos = create_ap_info_list(3);
	display_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);
	destroy_ap_info_list(AC_WTP[WTPID]->NeighborAPInfos);

	/////////////////////////////////////////////////////
	
	*/


	//AC_WTP[WTPID]->wids_device_list = create_wids_info_list(3);

	
	return 0 ;
	
}


int WID_DELETE_WTP(unsigned int WTPID){

	if(AC_WTP[WTPID]->isused == 1)
	{
		printf("*** error this WTP is used and active, you can not delete this ***\n");
		printf("*** if you want to delete please unused it first ***\n");
		return WTP_ID_BE_USED;
	}
	int num = AC_WTP[WTPID]->RadioCount;
	int i;
	int j;
	int ii;
	int k= 0;
	int bssindex = 0;
	unsigned char wlanid = 0;
	int ebr_id = 0;
	unsigned int RID = AC_WTP[WTPID]->WFR_Index;
	char macAddr[MAC_LEN];
	for(i=0; i<num; i++){
		for(k=0; k<L_BSS_NUM; k++)
		{
			if(AC_RADIO[RID]->BSS[k] != NULL)
			{
				wlanid = AC_RADIO[RID]->BSS[k]->WlanID;
				/*fengwenchao add 20111221 for ISSUES-850*/
				#if 0
				if((AC_RADIO[RID])&&(!strncasecmp(AC_RADIO[RID]->br_ifname[wlanid],"ebr",3)))
				{
					char ebrname[ETH_IF_NAME_LEN];
					memset(ebrname,0,ETH_IF_NAME_LEN);
					memcpy(ebrname,AC_RADIO[RID]->br_ifname[wlanid],strlen(AC_RADIO[RID]->br_ifname[wlanid]));
					 ret = Check_Interface_Exist(ebrname,&quitreason);
					 if(ret == 0)
					 	return RADIO_IN_EBR;
				}
				#endif
				if(check_whether_in_ebr(vrrid,WTPID,i,wlanid,&ebr_id))
				{
					wid_syslog_debug_debug(WID_DBUS,"<error> %s  check interface in ebr\n",__func__);
					return RADIO_IN_EBR;
				}
				/*fengwenchao add end*/
			}
		}
		RID++;
	}
	memset(macAddr,0,MAC_LEN);
	if(AC_WTP[WTPID] != NULL){
		memcpy(macAddr,AC_WTP[WTPID]->WTPMAC,MAC_LEN);
	}else{
		wid_syslog_warning("%s,%d,AC_WTP[%d] is %p.\n",__func__,__LINE__,WTPID,AC_WTP[WTPID]);
	}
	RID = AC_WTP[WTPID]->WFR_Index;
	for(i=0; ((i<num)&&(AC_RADIO[RID]!=NULL)); i++){
		wid_syslog_debug_debug(WID_DBUS,"%s,%d.\n",__func__,__LINE__);

		//free bss 20080806
		for(k=0; k<L_BSS_NUM; k++)
		{
			if(AC_RADIO[RID]->BSS[k] != NULL)
			{
				wlanid = AC_RADIO[RID]->BSS[k]->WlanID;
				if(AC_RADIO[RID]->BSS[k]->BSS_IF_POLICY == BSS_INTERFACE)
				{
					//should added delete l3 interface
					Delete_BSS_L3_Interface(AC_RADIO[RID]->BSS[k]->BSSIndex);
				}
				if(AC_RADIO[RID]->BSS[k]->BSS_IF_POLICY == WLAN_INTERFACE)
				{
					//remove interface from wlan br
					Del_BSS_L3_Interface_BR(AC_RADIO[RID]->BSS[k]->BSSIndex);
					//should added delete l3 interface
					Delete_BSS_L3_Interface(AC_RADIO[RID]->BSS[k]->BSSIndex);
				}
				bssindex = AC_RADIO[RID]->BSS[k]->BSSIndex;
				//weichao add 
				if( AC_BSS[bssindex]->acl_conf != NULL){	
					if( AC_BSS[bssindex]->acl_conf->accept_mac != NULL) {
						free_maclist(AC_BSS[bssindex]->acl_conf,AC_BSS[bssindex]->acl_conf->accept_mac);
						AC_BSS[bssindex]->acl_conf->accept_mac = NULL;
						}
					if( AC_BSS[bssindex]->acl_conf->deny_mac != NULL) {
						free_maclist(AC_BSS[bssindex]->acl_conf,AC_BSS[bssindex]->acl_conf->deny_mac);
						AC_BSS[bssindex]->acl_conf->deny_mac = NULL;
						}
					free(AC_BSS[bssindex]->acl_conf);
					AC_BSS[bssindex]->acl_conf = NULL;
				}
				wlanid = AC_RADIO[RID]->BSS[k]->WlanID;
				if((AC_WLAN[wlanid])&&(AC_WLAN[wlanid]->S_WTP_BSS_List[WTPID][i] == bssindex))
					AC_WLAN[wlanid]->S_WTP_BSS_List[WTPID][i] = 0;

				AC_RADIO[RID]->BSS[k]->WlanID = 0;
				AC_RADIO[RID]->BSS[k]->Radio_G_ID = 0;
				AC_RADIO[RID]->BSS[k]->Radio_L_ID = 0;
				AC_RADIO[RID]->BSS[k]->State = 0;
				AC_RADIO[RID]->BSS[k]->BSSIndex = 0;
				memset(AC_RADIO[RID]->BSS[k]->BSSID, 0, 6);
				free(AC_RADIO[RID]->BSS[k]->BSSID);
				AC_RADIO[RID]->BSS[k]->BSSID = NULL;
				free(AC_RADIO[RID]->BSS[k]);
				AC_RADIO[RID]->BSS[k] = NULL;
				AC_WTP[WTPID]->WTP_Radio[i]->BSS[k]=NULL;
				
				AC_BSS[bssindex] = NULL;
			}

		}
		//added end 20080806
		
		if (AC_WTP[WTPID]->WTP_Radio[i]->Support_Rate_Count != 0)
		{
			AC_WTP[WTPID]->WTP_Radio[i]->Support_Rate_Count = 0;
			destroy_support_rate_list(AC_WTP[WTPID]->WTP_Radio[i]->Radio_Rate);

		}
		AC_WTP[WTPID]->WTP_Radio[i] = NULL;
		
		struct wlanid *wlan_id = AC_RADIO[RID]->Wlan_Id;
		struct wlanid *wlan_id_next = NULL;
		
		while(wlan_id != NULL)
		{			
			wlan_id_next = wlan_id->next;
		
			CW_FREE_OBJECT(wlan_id);
		
			wlan_id = wlan_id_next;
		}
		for(j=0;j<SECTOR_NUM;j++)
		{	
			if(AC_RADIO[RID]->sector[j]){
				free(AC_RADIO[RID]->sector[j]);
				AC_RADIO[RID]->sector[j] = NULL;
			}
		}
		for(ii=0;ii<TX_CHANIMASK_NUM;ii++)
		{	
			if(AC_RADIO[RID]->tx_chainmask[ii]){
				free(AC_RADIO[RID]->tx_chainmask[ii]);
				AC_RADIO[RID]->tx_chainmask[ii] = NULL;
			}
		}
		
		free(AC_RADIO[RID]);
		AC_RADIO[RID] = NULL;
		
		RID++;
	}
	free(AC_WTP[WTPID]->WTPIP);
	AC_WTP[WTPID]->WTPIP = NULL;
	free(AC_WTP[WTPID]->WTPMAC);
	AC_WTP[WTPID]->WTPMAC = NULL;
	free(AC_WTP[WTPID]->CMD);
	AC_WTP[WTPID]->CMD = NULL;
	free(AC_WTP[WTPID]->WTPSN);
	AC_WTP[WTPID]->WTPSN = NULL;
	free(AC_WTP[WTPID]->WTPNAME);
	AC_WTP[WTPID]->WTPNAME = NULL;
	free(AC_WTP[WTPID]->WTPModel);
	AC_WTP[WTPID]->WTPModel = NULL;
	if (NULL == AC_WTP[WTPID]->APCode)
	{
	free(AC_WTP[WTPID]->APCode);
	AC_WTP[WTPID]->APCode = NULL;
	}
	free(AC_WTP[WTPID]->add_time);
	AC_WTP[WTPID]->add_time = NULL;
	free(AC_WTP[WTPID]->quit_time);
	AC_WTP[WTPID]->quit_time = NULL;
	free(AC_WTP[WTPID]->location);
	AC_WTP[WTPID]->location = NULL;
	free(AC_WTP[WTPID]->netid);
	AC_WTP[WTPID]->netid = NULL;
	if (NULL != AC_WTP[WTPID]->option60_param)
	{
	free(AC_WTP[WTPID]->option60_param);
	AC_WTP[WTPID]->option60_param = NULL;
	}
//	CW_FREE_OBJECT(AC_WTP[WTPID]->sysver);
//	CW_FREE_OBJECT(AC_WTP[WTPID]->ver);
//	CW_FREE_OBJECT(AC_WTP[WTPID]->codever);
	AC_WTP[WTPID]->sysver = NULL;
	AC_WTP[WTPID]->ver = NULL;
	AC_WTP[WTPID]->codever = NULL;

	if(AC_WTP[WTPID]->apcminfo.ap_cpu_info_head != NULL)
	{	
		struct ap_cpu_info * p = AC_WTP[WTPID]->apcminfo.ap_cpu_info_head;
		struct ap_cpu_info * tmp = NULL;
		
		while(p!=NULL){
			tmp = p;
			p = p->next;
			tmp->next =NULL;
			free(tmp);
			tmp = NULL;
			AC_WTP[WTPID]->apcminfo.ap_cpu_info_length--;
		}
	}
	if(AC_WTP[WTPID]->apcminfo.ap_mem_info_head != NULL)
	{	
		struct ap_cpu_info * p = AC_WTP[WTPID]->apcminfo.ap_mem_info_head;
		struct ap_cpu_info * tmp = NULL;
		
		while(p!=NULL){
			tmp = p;
			p = p->next;
			tmp->next =NULL;
			free(tmp);
			tmp = NULL;
			AC_WTP[WTPID]->apcminfo.ap_mem_info_length--;
		}
	}
	if(AC_WTP[WTPID]->apcminfo.ap_snr_info_head != NULL)
	{	
		struct ap_snr_info * p = AC_WTP[WTPID]->apcminfo.ap_snr_info_head;
		struct ap_snr_info * tmp = NULL;
		
		while(p!=NULL){
			tmp = p;
			p = p->next;
			tmp->next =NULL;
			free(tmp);
			tmp = NULL;
			AC_WTP[WTPID]->apcminfo.ap_snr_info_length--;
			printf("ap_snr_info_length:%d",AC_WTP[WTPID]->apcminfo.ap_snr_info_length);
		}
	}
	/*fengwenchao add 20111118 for GM-3*/
	if(AC_WTP[WTPID]->heart_time.heart_time_value_head != NULL)
	{
		struct heart_time_value_head * p = AC_WTP[WTPID]->heart_time.heart_time_value_head;
		struct heart_time_value_head * tmp = NULL;
		
		while(p!=NULL)
		{
			tmp = p;
			p = p->next;
			tmp->next =NULL;
			free(tmp);
			tmp = NULL;
			AC_WTP[WTPID]->heart_time.heart_time_value_length--;
			printf("heart_time_value_length:%d",AC_WTP[WTPID]->heart_time.heart_time_value_length);
		}
	}
	/*fengwenchao add end*/
	CWThreadMutexLock(&(gWTPs[WTPID].RRMThreadMutex));
	if((AC_WTP[WTPID]->NeighborAPInfos) != NULL)
	{
		destroy_ap_info_list(&(AC_WTP[WTPID]->NeighborAPInfos));
	}
	if((AC_WTP[WTPID]->rouge_ap_infos) != NULL)
	{
		destroy_ap_info_list(&(AC_WTP[WTPID]->rouge_ap_infos));
	}
	CWThreadMutexUnlock(&(gWTPs[WTPID].RRMThreadMutex)); 

	//added by weiay 20080630
	/*
	struct wlanid *wlan_id = NULL;
	while(wlan_id != NULL)
	{
		AC_WTP[WTPID]->Wlan_Id = wlan_id->next;
		free(wlan_id);
		wlan_id = NULL;
		wlan_id = AC_WTP[WTPID]->Wlan_Id;
		
	}
	*/
	//added end
	if(AC_WTP[WTPID]->APGroupID != 0){
		del_ap_group_member(AC_WTP[WTPID]->APGroupID,WTPID);
	}
	free(AC_WTP[WTPID]);
	AC_WTP[WTPID] = NULL;

	gStaticWTPs--;
	
	struct wtp_con_info * con_info = NULL;
	con_info = malloc(sizeof(struct wtp_con_info));
	memset(con_info,0,sizeof(struct wtp_con_info));
	memcpy(con_info->wtpmac,macAddr,MAC_LEN);
	wid_syslog_info("%s,%d,con_info->wtpmac:%2X:%2X:%2X:%2X:%2X:%2X.\n",__func__,__LINE__,con_info->wtpmac[0],con_info->wtpmac[1],con_info->wtpmac[2],con_info->wtpmac[3],con_info->wtpmac[4],con_info->wtpmac[5]);
	con_info->wtpindex = WTPID;
	con_info->wtpindex2 = WTPID;
	wid_del_conflict_wtpinfo(con_info);
	if(con_info){
		free(con_info);
		con_info = NULL;
	}
	return 0;

}
//added by weiay 20080626
int WID_SUSPEND_WTP(unsigned int WTPID)
{
	char i=0,result=0;
	msgq msg;
	if(gWTPs[WTPID].isNotFree){ 	
		gWTPs[WTPID].isRequestClose = CW_TRUE;		
		syslog_wtp_log(WTPID, 0, "WTP UNUSED", 0);
		if(gWIDLOGHN & 0x01)
			syslog_wtp_log_hn(WTPID,0,3);
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPID%THREAD_NUM + 1;
		msg.mqinfo.WTPID = WTPID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_REBOOT;
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}else{
		for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
		{
			if(gConfigVersionUpdateInfo[i] != NULL)
			{
				result = 1;
				break;
			}
		}
		if((result != 0)&&(find_in_wtp_list(WTPID) == CW_TRUE))
		{
			CWTimerCancel(&(gWTPs[WTPID].updateTimer),0);
			delete_wtp_list(WTPID);
		}
		
		if((AC_WTP[WTPID] != NULL)&&(AC_WTP[WTPID]->isused == 1)&&(AC_WTP[WTPID]->unused_flag == 1))
		{
				AC_WTP[WTPID]->isused = 0;
				AC_WTP[WTPID]->unused_flag = 0;
		}
	}
	CWThreadMutexLock(&(gWTPs[WTPID].WTPThreadControllistMutex));
	WID_CLEAN_CONTROL_LIST(WTPID);
	CWThreadMutexUnlock(&(gWTPs[WTPID].WTPThreadControllistMutex));

	return 0;
}
//
int WID_DISABLE_WLAN(unsigned char WlanID){
	
	msgq msg;
//	struct msgqlist *elem;
	if(AC_WLAN[WlanID]->Status == 0){
		AC_WLAN[WlanID]->Status = 1;		
		AsdWsm_WLANOp(WlanID, WID_MODIFY, 0);
	}
	else
		return 0;
	int m = 0, m1 = 0;	
	wid_syslog_debug_debug(WID_DEFAULT,"wlan %d need delete",WlanID);	
	for(m = 0; m < WTP_NUM; m++){
		if(AC_WTP[m] != NULL)
			for(m1 = 0; m1 < AC_WTP[m]->RadioCount; m1++){//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				wid_syslog_debug_debug(WID_DEFAULT,"m %d m1 %d bssindex %d need delete\n",m,m1,AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1]);	
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1] != 0)
				{	unsigned int BSSIndex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1];
					if((check_bssid_func(BSSIndex))&&(AC_BSS[BSSIndex] != NULL)&&(AC_WTP[m] != NULL)){
						Wsm_BSSOp(BSSIndex, WID_DEL, 1);
					//	AsdWsm_BSSOp(BSSIndex, WID_DEL, 1);//Qiuchen change it. when disable wlan,ASD will delete all bss in WLAN_OP--WID_MODIFY
						wid_update_bss_to_wifi(BSSIndex,m,0);/*resolve send capwap message,before ath be created*/	
						memset((char*)&msg, 0, sizeof(msg));
						wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id match success**\n");
						msg.mqid = m%THREAD_NUM+1;
						msg.mqinfo.WTPID = m;
						msg.mqinfo.type = CONTROL_TYPE;
						msg.mqinfo.subtype = WLAN_S_TYPE;
						msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_DEL;
						msg.mqinfo.u.WlanInfo.WLANID = WlanID;
						msg.mqinfo.u.WlanInfo.Radio_L_ID = m1;//zhanglei wait for M-radio
						
						msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1];

						if((AC_WTP[m]->WTPStat == 5)&&(AC_WTP[m]->CMD->radiowlanid[m1][WlanID] == 2)){ 
							if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
								wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
								perror("msgsnd");
							}
						}//delete unuseful code
						/*else{
							elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
							if(elem == NULL){
								wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
								perror("malloc");
								return 0;
							}
							memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
							elem->next = NULL;
							memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
							WID_INSERT_CONTROL_LIST(m, elem);
						}*/
						int type = 1;//manual
						int flag = 0;//disable

						if(gtrapflag>=4){
							wid_dbus_trap_ap_ath_error(m,m1,WlanID,type,flag);
						}
						//wid_syslog_debug_debug("AC_WTP[%d]->CMD->wlanid[%d]:%d	wlanCMD:%d\n",m,WlanID, AC_WTP[m]->CMD->wlanid[WlanID],AC_WTP[m]->CMD->wlanCMD);

						AC_BSS[BSSIndex]->downcount++; //book add, 2011-1-24
						/*fengwenchao add 20121213 for onlinebug-767*/
						AC_WTP[m]->CMD->wlanCMD -= 1;
						AC_WTP[m]->CMD->radiowlanid[m1][WlanID] = 0;
						if(AC_BSS[BSSIndex] != NULL){
							AC_BSS[BSSIndex]->State = 0;
							if(AC_BSS[BSSIndex]->BSSID != NULL){
								memset(AC_BSS[BSSIndex]->BSSID, 0, 6);
							}
							if(AC_BSS[BSSIndex]->BSS_TUNNEL_POLICY == CW_802_IPIP_TUNNEL)
							{
								delete_ipip_tunnel(BSSIndex);
							}
						}			
						/*fengwenchao add end*/						
						continue;
					}else if(((check_bssid_func(BSSIndex))&&(AC_BSS[BSSIndex] == NULL))||(AC_WTP[m] == NULL)){
						AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1] = 0;
						continue;
					}else if((AC_WTP[m] != NULL)&&(AC_WTP[m]->CMD->radiowlanid[m1][WlanID] == 1)){
						AC_WTP[m]->CMD->wlanCMD -= 1;
						AC_WTP[m]->CMD->radiowlanid[m1][WlanID] = 0;
					}	
				}
			}
	}

	return 0;
}

//weichao add 2011.11.03
int WLAN_FLOW_CHECK(unsigned char WlanID)
{
	

	msgq msg;
//	struct msgqlist *elem;

	int m = 0, m1 = 0;	
	for(m = 0; m < WTP_NUM; m++){
		if(AC_WTP[m] != NULL)
			for(m1 = 0; m1 < AC_WTP[m]->RadioCount; m1++){
				wid_syslog_debug_debug(WID_DEFAULT,"m %d m1 %d bssindex %d need delete\n",m,m1,AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1]);	
				if((AC_WLAN[WlanID]->SecurityType == IEEE8021X)||(AC_WLAN[WlanID]->SecurityType ==WPA_E)||(AC_WLAN[WlanID]->SecurityType ==WPA2_E)||(AC_WLAN[WlanID]->SecurityType ==MD5))
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1] != 0)
				{	unsigned int BSSIndex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1];
					if((check_bssid_func(BSSIndex))&&(AC_BSS[BSSIndex] != NULL)&&(AC_WTP[m] != NULL)){
													
							msg.mqid = m%THREAD_NUM+1;
							msg.mqinfo.WTPID = m;
							msg.mqinfo.type = CONTROL_TYPE;
							msg.mqinfo.subtype = WTP_S_TYPE;
							msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_FLOW_CHECK;
							msg.mqinfo.u.WlanInfo.WLANID = WlanID;
							msg.mqinfo.u.WlanInfo.Radio_L_ID = m1;
							msg.mqinfo.u.WlanInfo.flow_check = AC_WLAN[WlanID]->flow_check;
							msg.mqinfo.u.WlanInfo.no_flow_time = AC_WLAN[WlanID]->no_flow_time;
							msg.mqinfo.u.WlanInfo.limit_flow = AC_WLAN[WlanID]->limit_flow;
						
						if(AC_WTP[m]->WTPStat == 5){	
							if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
								wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
								perror("msgsnd");
							}
						}//delete unuseful code
						/*else{
							elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
							if(elem == NULL){			
								wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
								perror("malloc");
								return 0;
							}
							
							memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
							elem->next = NULL;
							memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
							
							WID_INSERT_CONTROL_LIST(m, elem);
						}*/
						
						
							

					}
					
				}
				
			}
		}
	return 0; 
}

int WID_wds_disable(unsigned char WlanID,unsigned char wds_mesh)
{
	msgq msg;
	
	if(AC_WLAN[WlanID]->Status == 1)
	{
		return WLAN_BE_DISABLE;
	}
	

	if(AC_WLAN[WlanID]->WDSStat== 1)
	{
		AC_WLAN[WlanID]->WDSStat= 0;
		if((wds_mesh&(0x02)) !=0)
			AC_WLAN[WlanID]->wds_mesh = 1;
		else
			AC_WLAN[WlanID]->wds_mesh = 0;
	}
	else
	{
		return 0;
	}
	

	
	int m = 0, m1 = 0;	
	for(m = 0; m < WTP_NUM; m++)
	{
		if(AC_WTP[m] != NULL)
		{
			for(m1 = 0; m1 < AC_WTP[m]->RadioCount; m1++)
			{				
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1] != 0)
				{	
					unsigned int BSSIndex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][m1];
					if((check_bssid_func(BSSIndex))&&(AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->WDSStat == WDS_SOME)){
						continue;
					}
					if((AC_BSS[BSSIndex] != NULL)&&(AC_WTP[m] != NULL))
					{
						//if(AC_WTP[m]->CMD->radiowlanid[m1][WlanID] != 0)	//run state
						{
							msg.mqid = m%THREAD_NUM+1;
							msg.mqinfo.WTPID = m;
							msg.mqinfo.type = CONTROL_TYPE;
							msg.mqinfo.subtype = WDS_S_TYPE;
							msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_DISABLE;
							msg.mqinfo.u.WlanInfo.WLANID = WlanID;
							msg.mqinfo.u.WlanInfo.Radio_L_ID = m1;
							if(AC_WTP[m]->WTPStat == 5)
							{	
								if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
									wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
									perror("msgsnd");
								}
							}
							
							AC_BSS[BSSIndex]->WDSStat = DISABLE;
						}
					
					}
				
				}
			}
		}
	}

	return 0;
}


int WID_wds_enable(unsigned char WlanID,unsigned char wds_mesh)
{
	msgq msg;
//	struct msgqlist *elem;
	
	if(AC_WLAN[WlanID]->Status == 1)
	{
		return WLAN_BE_DISABLE;
	}		
	
	if(AC_WLAN[WlanID]->WDSStat== 0)
	{
		AC_WLAN[WlanID]->WDSStat= 1;
		if((wds_mesh&(0x02)) !=0)
			AC_WLAN[WlanID]->wds_mesh = 1;
		else
			AC_WLAN[WlanID]->wds_mesh = 0;
	}
	else
	{
		return 0;
	}


	
	int m = 0,n=0;

	for(m = 0; m < WTP_NUM; m++)
	{
		if((AC_WTP[m]!=NULL))
		{
			for(n = 0; n < AC_WTP[m]->RadioCount; n++)
			{
				
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n] != 0)
				{	
					
					unsigned int BSSIndex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][n];
					if((check_bssid_func(BSSIndex))&&(AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->WDSStat == WDS_SOME)){
						continue;
					}
					if((AC_BSS[BSSIndex] != NULL)&&(AC_WTP[m] != NULL))
					{
						
						//if(AC_WTP[m]->CMD->radiowlanid[n][WlanID] != 0)	//run state
						{
							
							//printf("*** 3333333 ***\n");
							msg.mqid = m%THREAD_NUM+1;
							msg.mqinfo.WTPID = m;
							msg.mqinfo.type = CONTROL_TYPE;
							msg.mqinfo.subtype = WDS_S_TYPE;
							msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_ENABLE;
							msg.mqinfo.u.WlanInfo.WLANID = WlanID;
							msg.mqinfo.u.WlanInfo.Radio_L_ID = n;//zhanglei wait for M-radio
							if(AC_WTP[m]->WTPStat == 5){	
								if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
									wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
									perror("msgsnd");
								}
							}//delete unuseful code
							/*else{
								elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
								if(elem == NULL){
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									perror("malloc");
									return 0;
								}
								memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
								elem->next = NULL;
								memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
								WID_INSERT_CONTROL_LIST(m, elem);

							
							}*/

							AC_BSS[BSSIndex]->WDSStat = WDS_ANY;
						}

					}
				
				}
			}
		}
	}

	
	AC_WLAN[WlanID]->WDSStat = 1;		

	return 0;
}
int WID_ENABLE_WLAN(unsigned char WlanID){
	WTPQUITREASON quitreason = WTP_INIT;
	msgq msg;
//	struct msgqlist *elem;
	unsigned int bssindex;	
	char buf[DEFAULT_LEN];
	struct wds_bssid *wds = NULL;
	unsigned char pcy = 0;
	msgq msg2;
	struct msgqlist *elem2;
	
	msgq msg4 ;
//	struct msgqlist *elem4 = NULL; 
	char *command = NULL;
	char *ath_str = NULL;
	if(AC_WLAN[WlanID]->Wlan_Ifi == NULL)
	{
	//	printf("*** you must binding interface ***\n");
		wid_syslog_warning("<warning>,%s,%d,INTERFACE_NOT_BE_BINDED\n",__func__,__LINE__);
		//return INTERFACE_NOT_BE_BINDED;
	}
	if(AC_WLAN[WlanID]->Status == 0)
		return 0;
	if (AC_WLAN[WlanID]->want_to_delete == 1)		/* Huangleilei add for ASXXZFI-1622 */
	{
		wid_syslog_err("operator want to delete wlan %d", WlanID);
		return -1;
	}
	////////////////////////////////////////////////////
	if(AC_WLAN[WlanID]->wlan_if_policy != NO_INTERFACE)
	{
		int ret = -1;
		int i,j;
		char ifiname[ETH_IF_NAME_LEN-1];
		memset(ifiname,0,ETH_IF_NAME_LEN-1);
		
		if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)
		{
			//snprintf(ifiname,ETH_IF_NAME_LEN,"WLAN%d",WlanID);
			if(local)
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlanl%d-%d-%d",slotid,vrrid,WlanID);
			else
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlan%d-%d-%d",slotid,vrrid,WlanID);				
			ret = Check_Interface_Config(ifiname,&quitreason);
//			printf("%s\n",ifiname);
			if(ret != 0)
			{
				return L3_INTERFACE_ERROR;
			}
		}
		else if(AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE)
		{
			for(i=0; i<WTP_NUM; i++)
			{
				if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))
				{
					for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
					{
						if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
						{
							int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
							int wtpid = (bssindex/L_BSS_NUM)/L_RADIO_NUM;
							int radioid = j;
						
							memset(ifiname,0,ETH_IF_NAME_LEN-1);
							//snprintf(ifiname,ETH_IF_NAME_LEN,"BSS%d",AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]);
							//snprintf(ifiname,ETH_IF_NAME_LEN,"radio%d-%d-%d.%d",vrrid,wtpid,radioid,WlanID);
							if(local)
								snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,wtpid,radioid,WlanID);
							else
								snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpid,radioid,WlanID);
							ret = Check_Interface_Config(ifiname,&quitreason);
//							printf("%s\n",ifiname);
							if(ret != 0)
							{
								return L3_INTERFACE_ERROR;
							}

						}
					}
				}

			}

		}
		else
		{
			wid_syslog_debug_debug(WID_DEFAULT,"can not present but i am not sure\n");
		}
	}
	else//local mode to put wlan-vlan to bss
	{
		int i,j;
		for(i=0; i<WTP_NUM; i++)
			{
				/*if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))*/
				if(AC_WTP[i]!=NULL)
				{
					for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
					{
						if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
						{
							int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
							if((check_bssid_func(bssindex))&&(AC_BSS[bssindex])){
							AC_BSS[bssindex]->wlan_vlanid = AC_WLAN[WlanID]->vlanid;
							}
						}
					}
				}
			}
		//printf("put wlan vlan to the bss\n");
	}
	//check wep wlan 
	if((AC_WLAN[WlanID]->EncryptionType == WEP)&&(AC_WLAN[WlanID]->SecurityType != IEEE8021X))
	{
		int i,j,i1;
		for(i=0; i<WTP_NUM; i++)
			{
				if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))
				{
					for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
					{
						if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
						{
							int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
//							int wtpid = bssindex/(L_RADIO_NUM*L_BSS_NUM);
							//fengwenchao change begin
							if((check_bssid_func(bssindex))/*&&(AC_BSS[bssindex]->keyindex == 0)*/)//wlan is wep but bss is not,so put wep keyindex to the bss
							{
								//修改前代码开始
								/*for(i1=0;i1<WTP_WEP_NUM;i1++)
								{
									if(AC_WTP[wtpid]->wep_flag[i1] == 0)
									{
										AC_WTP[wtpid]->wep_flag[i1] = bssindex;
										AC_BSS[bssindex]->keyindex = i1+1;
										break;
									}
								}

								if(i1 == WTP_WEP_NUM-1)//no room to put wep wlan
								{
									//not process , when add wlan just to avoid add this bss
								}*/	
								//修改前代码结束
								
								//修改后代码开始
								for(i1=0;i1<WTP_WEP_NUM;i1++)
								{
									if(AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1] != 0)
									{
										if((AC_WLAN[WlanID]->SecurityIndex == AC_BSS[AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1]]->keyindex)&&(bssindex != AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1]))
										{
											return SECURITYINDEX_IS_SAME;
										}	
									}
									if(AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1] == bssindex)
									{
										AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1] = 0;
										AC_BSS[bssindex]->keyindex = 0;							
									}
								}
								i1 = AC_WLAN[WlanID]->SecurityIndex-1;
								if(i1 < WTP_WEP_NUM)
								{
									if(AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1] == 0)
									{
										AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1] = bssindex;
										AC_BSS[bssindex]->keyindex = AC_WLAN[WlanID]->SecurityIndex;
										break;
									}
									if(i1 == WTP_WEP_NUM-1)//no room to put wep wlan
									{
										//not process , when add wlan just to avoid add this bss
									}
								}
								//修改后代码结束
							}
							//fengwenchao change end
						}
					}
				}

			}
	}
	else//wlan is not wep
	{
		int i,j,i1;
		for(i=0; i<WTP_NUM; i++)
		{
			if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))
			{
				for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				{
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
//						int wtpid = bssindex/(L_RADIO_NUM*L_BSS_NUM);
					
						if((check_bssid_func(bssindex))&&(AC_BSS[bssindex]->keyindex != 0))//wlan is none but bss is wep,so remove the bss key index
						{
							for(i1=0;i1<WTP_WEP_NUM;i1++)
							{
								if(AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1] == bssindex)
								{
									AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[i1] = 0;
									AC_BSS[bssindex]->keyindex = 0;
									break;
								}
							}
						}

					}
				}
			}

		}
	}
	/*if the ac recive wlan response but wlan state is disable,bss state is not same to the wlan*/
	AC_WLAN[WlanID]->Status = 0;
	////////////////////////////////////////////////////////////
	int m = 0,n=0;
	wid_syslog_debug_debug(WID_DEFAULT,"ONE wlan need create WLAN %d\n",WlanID);
	for(m = 0; m < WTP_NUM; m++)
		if((AC_WTP[m]!=NULL)&&(AC_WTP[m]->isused == 1))
		{
			for(n = 0; n < AC_WTP[m]->RadioCount; n++)
			{
				if((AC_WTP[m]->WTP_Radio[n]->isBinddingWlan == 1)&&(AC_WTP[m]->CMD->radiowlanid[n][WlanID]==0))
				{
			//changed by weiay 20080617
				wid_syslog_debug_debug(WID_DEFAULT,"***wtp binding index is %d\n",AC_WTP[m]->BindingSystemIndex);
#ifdef _CheckBindingIf_
				if(AC_WLAN[WlanID]->Wlan_Ifi != NULL)//default Wlan_Ifi equal NULL
				{
					struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
					while(wlan_ifi != NULL)
					{
						wid_syslog_debug_debug(WID_DEFAULT,"*** wlan index is %d\n",wlan_ifi->ifi_index);
						if(AC_WTP[m]->BindingSystemIndex == wlan_ifi->ifi_index)
						{
							break;
						}
						wlan_ifi = wlan_ifi->ifi_next;
					}
					
					if(wlan_ifi == NULL)
					{
						wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding interface doesn't match with wlan binding interface **\n");
						continue;
					}
				else
#else
				{			
#endif
					{
						//wid_syslog_debug_debug("*** wtp binding interface match with wlan binding interface success**\n");
						//AC_WTP[m]->CMD->wlanCMD += 1;
						//AC_WTP[m]->CMD->wlanid[WlanID] = 1;
						//Added by weiay 20080630
						if(AC_WTP[m]->WTP_Radio[n]->isBinddingWlan == 1)
						{
							//added by weiay 20080630
							struct wlanid *wlan_id = AC_WTP[m]->WTP_Radio[n]->Wlan_Id;
							while(wlan_id != NULL)
							{	
								if(wlan_id->wlanid == WlanID)
								{
									break;
								}
								wlan_id = wlan_id->next;
							}
							if(wlan_id != NULL)
							{
								WID_WTP_SSID_KEY_CONFLICT(AC_WTP[m]->WTP_Radio[n]->Radio_G_ID,WlanID);
								if((AC_WLAN[WlanID]->EncryptionType == WEP)&&(AC_WLAN[WlanID]->SecurityType != IEEE8021X))   /*fengwenchao modify 20110309 排除802.1x*/
								{
									if((check_bssid_func(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]))&&(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->keyindex == 0))
									{
										wid_syslog_debug_debug(WID_DEFAULT,"wlan is wep ,but no room to put bss %d to the wep bss,so not add this bss\n",AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]);
										break;
									}
								}
								//printf("*** wtp binding wlan id match success**\n");
								wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id match success**\n");
								msg.mqid = m%THREAD_NUM+1;
								msg.mqinfo.WTPID = m;
								msg.mqinfo.type = CONTROL_TYPE;
								msg.mqinfo.subtype = WLAN_S_TYPE;
								msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
								msg.mqinfo.u.WlanInfo.WLANID = WlanID;
								msg.mqinfo.u.WlanInfo.Radio_L_ID = n;//zhanglei wait for M-radio

								msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
								memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
								memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
								msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
								msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
								msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
								msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;/* 0 asic; 1 hex*/
								msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy; 		/*Roaming (1 enable /0 disable)*/
								memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
								//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
								memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,strlen(AC_WLAN[WlanID]->ESSID));
								msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][n];
								
								if(AC_WTP[m]->WTPStat == 5){	
									if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
										wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
										perror("msgsnd");
									}
								}//delete unuseful code
								/*else{
									elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
									if(elem == NULL){
										wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
										perror("malloc");
										return 0;
									}
									memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
									elem->next = NULL;
									memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
									WID_INSERT_CONTROL_LIST(m, elem);
								}*/
								//wei chao add flow check 2011.11.03
								
							if((AC_WLAN[WlanID]->SecurityType == IEEE8021X)||(AC_WLAN[WlanID]->SecurityType ==WPA_E)||(AC_WLAN[WlanID]->SecurityType ==WPA2_E)||(AC_WLAN[WlanID]->SecurityType ==MD5))
							{
									msgq msg3;
//									struct msgqlist *elem3;
									msg3.mqid = m%THREAD_NUM+1;
									msg3.mqinfo.WTPID = m;
									msg3.mqinfo.type = CONTROL_TYPE;
									msg3.mqinfo.subtype = WTP_S_TYPE;
									msg3.mqinfo.u.WtpInfo.Wtp_Op = WTP_FLOW_CHECK;
									msg3.mqinfo.u.WlanInfo.WLANID = WlanID;
									msg3.mqinfo.u.WlanInfo.Radio_L_ID = n;
									msg3.mqinfo.u.WlanInfo.flow_check = AC_WLAN[WlanID]->flow_check;
									msg3.mqinfo.u.WlanInfo.no_flow_time = AC_WLAN[WlanID]->no_flow_time;
									msg3.mqinfo.u.WlanInfo.limit_flow = AC_WLAN[WlanID]->limit_flow;
									if(AC_WTP[m]->WTPStat == 5){	
										if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg3, sizeof(msg.mqinfo), 0) == -1){
											wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
											perror("msgsnd");
										}
									}//delete unuseful code
								/*else{
									elem3 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
									if(elem3 == NULL){			
										wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
										perror("malloc");
										return 0;
									}
									
									memset((char*)&(elem3->mqinfo), 0, sizeof(msgqdetail));
									elem3->next = NULL;
									memcpy((char*)&(elem3->mqinfo),(char*)&(msg3.mqinfo),sizeof(msg3.mqinfo));
									WID_INSERT_CONTROL_LIST(m, elem3);
								}*/
							}
							//weichao add 
								command = (char *)malloc(sizeof(char)*100);
								if(NULL == command)
								{							
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									return 0;
								}
								memset(command,0,100);
								ath_str = (char *)malloc(sizeof(char)*20);
								if(NULL == ath_str)
								{
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									return 0;
								}
								memset(ath_str,0,20);
								memset(&msg4,0,sizeof(msg4));
								msg4.mqid = m%THREAD_NUM +1;
								msg4.mqinfo.WTPID = m;
								msg4.mqinfo.type = CONTROL_TYPE;
								msg4.mqinfo.subtype = WTP_S_TYPE;
								msg4.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;
							
								sprintf(ath_str,"ath.%d-%d",n,WlanID);
								sprintf(command,"ifconfig %s down;iwpriv %s inact %u;ifconfig %s up",ath_str,ath_str,AC_WLAN[WlanID]->ap_max_inactivity,ath_str);
								memcpy(msg4.mqinfo.u.WtpInfo.value, command, strlen(command));
								wid_syslog_debug_debug(WID_DEFAULT,"the command is : %s\n",command);
								
								if(AC_WTP[m]->WTPStat == 5){	
									if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg4, sizeof(msg4.mqinfo), 0) == -1){
										wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
										perror("msgsnd");
									}
								}//delete unuseful code
								/*else{
									elem4 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
									if(elem4 == NULL){
										wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
										perror("malloc");
										if(command){
											free(command);
											command = NULL;
										}
										if(ath_str){
											free(ath_str);
											ath_str = NULL;
										}
										return 0;
									}
									memset((char*)&(elem4->mqinfo), 0, sizeof(msgqdetail));
									elem4->next = NULL;
									memcpy((char*)&(elem4->mqinfo),(char*)&(msg4.mqinfo),sizeof(msg4.mqinfo));
									WID_INSERT_CONTROL_LIST(m, elem4);
								}*/
							
							
							free(command);
							command = NULL;
							free(ath_str);
							ath_str = NULL;

								//add wds state info
								
								if((check_bssid_func(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]))&&(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->WDSStat == WDS_ANY))
								{
									msg2.mqid = m%THREAD_NUM +1;
									msg2.mqinfo.WTPID = m;
									msg2.mqinfo.type = CONTROL_TYPE;
									msg2.mqinfo.subtype = WDS_S_TYPE;
									msg2.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_ENABLE;
									msg2.mqinfo.u.WlanInfo.WLANID = WlanID;
									msg2.mqinfo.u.WlanInfo.Radio_L_ID = n;
									
									elem2 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
									if(elem2 == NULL){
										wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
										perror("malloc");
										return 0;
									}
									memset((char*)&(elem2->mqinfo), 0, sizeof(msgqdetail));
									elem2->next = NULL;
									memcpy((char*)&(elem2->mqinfo),(char*)&(msg2.mqinfo),sizeof(msg2.mqinfo));
									WID_INSERT_CONTROL_LIST(m, elem2);
								}
								//added end
								else{
									bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][n];
									if((check_bssid_func(bssindex))&&(AC_BSS[bssindex]!=NULL) && (AC_BSS[bssindex]->WDSStat == WDS_SOME)){
										wds = AC_BSS[bssindex]->wds_bss_list;
										while(wds != NULL){
											memset(buf,0,DEFAULT_LEN);
											sprintf(buf,"/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",AC_BSS[bssindex]->Radio_L_ID,WlanID,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
//											printf("/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",AC_BSS[bssindex]->Radio_L_ID,WlanID,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
											wid_radio_set_extension_command(m,buf);
											wds = wds->next;
										}
									}
								}
								int type = 1;//manual
								int flag = 1;//enable
								if(gtrapflag>=4){
									wid_dbus_trap_ap_ath_error(m,n,WlanID,type,flag);
									}

							    /* send eap switch & mac to ap ,zhangshu add 2010-10-22 */
                            	char apcmd[WID_SYSTEM_CMD_LENTH];
                            	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
                                bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][n];
                            	if((AC_WLAN[WlanID]->eap_mac_switch==1)&&(AC_WLAN[WlanID]->wlan_if_policy==NO_INTERFACE)&&(AC_BSS[bssindex]!=NULL)&&(AC_BSS[bssindex]->BSS_IF_POLICY==NO_INTERFACE))
                            	{
                        		    sprintf(apcmd,"set_eap_mac ath.%d-%d %s",n,WlanID,AC_WLAN[WlanID]->eap_mac);
                        		}
                        		else
                        		{
                        		    sprintf(apcmd,"set_eap_mac ath.%d-%d 0",n,WlanID);
                        		}
                            	wid_syslog_debug_debug(WID_DEFAULT,"Enable Wlan: set eap mac cmd %s\n",apcmd);
                            	wid_radio_set_extension_command(m,apcmd);
                            	/* end */

                            	/* zhangshu add , 2011-1-7 */
                            	if((check_bssid_func(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]))&&(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])] != NULL))
                            	{
                            		bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][n];
                            		wid_syslog_debug_debug(WID_DEFAULT,"!@#$ AC_BSS[%d]->traffic_limit_able = %d\n",bssindex,AC_BSS[bssindex]->traffic_limit_able);
                            		if(AC_BSS[bssindex]->traffic_limit_able == 1){
                            		    WID_Save_Traffic_Limit(bssindex, m);
                            		}
                            		AC_BSS[bssindex]->upcount++;//book add, 2011-1-25
                            	}
								/*fengwenchao add 20120220 for autelan-2841*/
								 if((AC_WTP[m]->WTP_Radio[n]->MixedGreenfield.Mixed_Greenfield != 0)
								 	&&((AC_WTP[m]->WTP_Radio[n]->Radio_Type != 10)&&(AC_WTP[m]->WTP_Radio[n]->Radio_Type !=12))) //fengwenchao modify 20120716 for autelan-3057
								 {
 				   					wid_radio_set_mixed_puren_switch(AC_WTP[m]->WTP_Radio[n]->Radio_G_ID);
			    					}
								/*fengwenchao add end*/
								if((check_bssid_func(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]))&&((AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->unicast_sw == 1)\
									||(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->muti_bro_cast_sw == 1)||(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->muti_rate != 10))){
									bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[m][n];
									if((1 == AC_BSS[bssindex]->muti_bro_cast_sw)&&(1 == AC_BSS[bssindex]->unicast_sw)){
										pcy = 3;
									}else if(1 == AC_BSS[bssindex]->muti_bro_cast_sw){
										pcy = 2;
									}else if(1 == AC_BSS[bssindex]->unicast_sw){
										pcy = 1;
									}else{
										pcy = 0;
									}
									if(AC_BSS[bssindex]->wifi_sw == 1){
										pcy = pcy|0x4;
									}else{
										pcy = pcy&~0x4;
									}
									setWtpUniMutiBroCastIsolation(m,n,WlanID,pcy);
									wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,m,n,WlanID,(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]));
								}
								if((check_bssid_func(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]))&&(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->noResToStaProReqSW == 1)){
									setWtpNoRespToStaProReq(m,n,WlanID,AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->noResToStaProReqSW);
									wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,m,n,WlanID,(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]));
								}
								/*fengwenchao add 20120522 for autelan-2969*/
								if((check_bssid_func(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]))&&(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->muti_rate != 10)){
									setWtpUniMutiBroCastRate(m,n,WlanID,AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n])]->muti_rate);
									wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,m,n,WlanID,(AC_WLAN[WlanID]->S_WTP_BSS_List[m][n]));
								}
								/*fengwenchao add end*/
								if((check_bssid_func(bssindex))&&(AC_BSS[bssindex]->multi_user_optimize_switch == 1))
								{
									
									char wlanid =AC_BSS[bssindex]->WlanID;
									int radioid = AC_BSS[bssindex]->Radio_G_ID;
									muti_user_optimize_switch(wlanid,radioid,1);
								}
							}
							else
							{
								continue;
							}
							//added end
						}
						//added end
					}
				}
	
			//wid_syslog_debug_debug("AC_WTP[%d]->CMD->wlanid[%d]:%d	wlanCMD:%d\n",m,WlanID, AC_WTP[m]->CMD->wlanid[WlanID],AC_WTP[m]->CMD->wlanCMD);
			}
		}
	}
	AC_WLAN[WlanID]->CMD = 0;	
	//AC_WLAN[WlanID]->Status = 0;		
	AC_WLAN[WlanID]->want_to_delete = 0;		/* Huangleilei add for ASXXZFI-1622 */
	AsdWsm_WLANOp(WlanID, WID_MODIFY, 0);

	return 0;
}

//Added by weiay 20080623
int WID_USED_WTP(unsigned int WtpID)
{
	WTPQUITREASON quitreason = WTP_INIT;	
	//msgq msg;
	//struct msgqlist *elem;	
	//unsigned int bssindex;	
	//char buf[DEFAULT_LEN];
	//struct wds_bssid *wds = NULL;
	//msgq msg2;
	//struct msgqlist *elem2;
	//msgq msg4 ;
	//struct msgqlist *elem4 = NULL;
	//char *command = NULL;
	//char *ath_str  = NULL;
	//printf("******* wtp id:%d used ********\n",WtpID);
	if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->isused == 0))
	{
		//printf("*** this wtp is used ***\n");
		//AC_WTP[WtpID]->isused = 1;
		//AC_WTP[WtpID]->quitreason = WTP_NORMAL;
	}
	else
	{
//		printf("*** this wtp before state is used ***\n");
		return 0;
	}
	// Parameter check
	if(AC_WTP[WtpID]->BindingSystemIndex == -1)
	{
//		printf("*** you must binding one interface ***\n");
		//return INTERFACE_NOT_BE_BINDED;
	}

	//if(AC_WTP[WtpID]->isBinddingWlan == 0)
	//{
		//printf("*** you must binding one wlan id ***\n");
		//return WTP_IS_NOT_BINDING_WLAN_ID;
	//}
	

////////////////////////////////////////////////////////////////////
	int j,ret,k;
	//unsigned char pcy = 0;
	char ifiname[ETH_IF_NAME_LEN-1];
	int check1=0, check2=0;
	CWThreadMutexLock(&(gWTPs[WtpID].WTPThreadControllistMutex));
	WID_CLEAN_CONTROL_LIST(WtpID);
	CWThreadMutexUnlock(&(gWTPs[WtpID].WTPThreadControllistMutex));
	for(j=0; j<AC_WTP[WtpID]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
	{
		for(k=0; k<L_BSS_NUM; k++)
			if(AC_WTP[WtpID]->WTP_Radio[j]->BSS[k] != NULL)
			{
				if(AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->BSS_IF_POLICY == NO_INTERFACE){
					check1++;
				}else{
					check2++;
				}
				//if((check1 != 0)&&(check2 != 0))
					//return IF_POLICY_CONFLICT;
				if(AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->BSS_IF_POLICY == BSS_INTERFACE)
				{
					//assemble radio1-0.1
					memset(ifiname,0,ETH_IF_NAME_LEN-1);
//					printf("radio%d-%d.%d\n",WtpID,j,AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->WlanID);
					if(local)
						snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,WtpID,j,AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->WlanID);
					else
						snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,WtpID,j,AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->WlanID);
					//memset(ifiname,0,ETH_IF_NAME_LEN);
					//snprintf(ifiname,ETH_IF_NAME_LEN,"BSS%d",AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->BSSIndex);
					ret = Check_Interface_Config(ifiname,&quitreason);
					
					if(ret != 0)
					{
						return L3_INTERFACE_ERROR;
					}
				}
				if(AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->BSS_IF_POLICY == WLAN_INTERFACE)
				{
					//assemble wlan1
					memset(ifiname,0,ETH_IF_NAME_LEN-1);
//					printf("wlan%d\n",AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->WlanID);
					if(local)
						snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlanl%d-%d-%d",slotid,vrrid,AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->WlanID);
					else
						snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlan%d-%d-%d",slotid,vrrid,AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->WlanID);
						
					ret = Check_Interface_Config(ifiname,&quitreason);
					
					if(ret != 0)
					{
						return L3_INTERFACE_ERROR;
					}
				}
			}
		#if 0
		if(AC_WTP[WtpID]->WTP_Radio[j] != NULL)
		{
			if(AC_WTP[WtpID]->WTP_Radio[j]->auto_channel != 0)
			{
				memset(buf,0,DEFAULT_LEN);
				sprintf(buf,"echo 1 > /proc/sys/dev/wifi%d/nonoverlapping",j);
				wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
				wid_radio_set_extension_command(WtpID,buf);
			}

			if(AC_WTP[WtpID]->WTP_Radio[j]->REFlag == 1)
			    wid_wtp_radio_extern_command_check(WtpID,j);
		}
		#endif
	}
	if(check1 != 0)
		AC_WTP[WtpID]->tunnel_mode = CW_LOCAL_BRIDGING;
	else if(check2 != 0)
		AC_WTP[WtpID]->tunnel_mode = CW_802_DOT_11_TUNNEL;


	AC_WTP[WtpID]->isused = 1;
	AC_WTP[WtpID]->quitreason = WTP_NORMAL;

//////////////////////////////////////////////////////////////20080625
	k = 0;
	int m = 0;
	AC_WTP[WtpID]->CMD->wlanCMD = 0;
	wid_syslog_debug_debug(WID_DEFAULT,"******wlanCMD cout :%d***\n",AC_WTP[WtpID]->CMD->wlanCMD);
	for(k = 0;k < WLAN_NUM; k++)
	{
		for(m = 0;m < AC_WTP[WtpID]->RadioCount; m++)
		{	
			AC_WTP[WtpID]->CMD->radiowlanid[m][k] = 0;
		}
	}
	//WID_CONFIG_SAVE(WtpID);
//////////////////////////////////////////////////////////////20080625
	
	return 0;
}


//zhangshu add for save traffic limit, 2011-1-7
void WID_Save_Traffic_Limit(unsigned int bssindex, unsigned int WtpID)
{
    char buf[DEFAULT_LEN];
    unsigned int value = 0;
    int flag = 0;
    
	if(AC_BSS[bssindex]->traffic_limit != 0)
	{
		value = AC_BSS[bssindex]->traffic_limit;
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"autelan traffic_limit ath.%d-%d set_vap_flag 1;autelan traffic_limit ath.%d-%d set_vap %d",AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,value);
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_value buf %s\n",buf);
		wid_radio_set_extension_command(WtpID,buf);
		flag = 1;
	}

	if(AC_BSS[bssindex]->average_rate != 0)
	{
		value = AC_BSS[bssindex]->average_rate;
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"autelan traffic_limit ath.%d-%d set_every_node_flag 1;autelan traffic_limit ath.%d-%d set_every_node %d",AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,value);
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_average_value buf %s\n",buf);
		wid_radio_set_extension_command(WtpID,buf);
		flag = 1;
	}

	if(AC_BSS[bssindex]->send_traffic_limit != 0)
	{
		value = AC_BSS[bssindex]->send_traffic_limit;
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"autelan traffic_limit ath.%d-%d set_vap_flag 1;autelan traffic_limit ath.%d-%d set_vap_send %d",AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,value);
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_send_value buf %s\n",buf);
		wid_radio_set_extension_command(WtpID,buf);
		flag = 1;
	}

	if(AC_BSS[bssindex]->send_average_rate != 0)
	{
		value = AC_BSS[bssindex]->send_average_rate;
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"autelan traffic_limit ath.%d-%d set_every_node_flag 1;autelan traffic_limit ath.%d-%d set_every_node_send %d",AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,AC_BSS[bssindex]->Radio_L_ID
																														,AC_BSS[bssindex]->WlanID
																														,value);
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_average_send_value buf %s\n",buf);
		wid_radio_set_extension_command(WtpID,buf);
		flag = 1;
	}
	if(flag != 0){
	    AC_BSS[bssindex]->traffic_limit_able = 1;
	}

    return; 
}
//fengwenchao add 20110126 for XJDEV-32  from 2.0
int wid_set_ap_eth_if_mtu(unsigned int wtpid,unsigned char eth_index)
{
//	int ret = 0;
	msgq msg;
//	struct msgqlist *elem;

	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_IF_ETH_MTU;
			msg.mqinfo.u.WtpInfo.value1 = eth_index;

			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful code
	/*else if((AC_WTP[wtpid] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = wtpid%THREAD_NUM+1;
		msg.mqinfo.WTPID = wtpid;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_IF_ETH_MTU;
		msg.mqinfo.u.WtpInfo.value1 = eth_index;

		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_info("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(wtpid, elem);
	}*/
	wid_syslog_debug_debug(WID_DEFAULT," wtpid %d,wid_set_ap_eth_if_mtu\n",wtpid);
	return 0;
}
//fengwenchao add end

int WID_UNUSED_WTP(unsigned int WtpID)
{
	//printf("******* this command no operate ********\n");
	//return 0;
	int tmp;
	int i = 0;

	//printf("******* wtp id:%d unused ********\n",WtpID);
	// Parameter check

	if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->isused == 1))
	{
		//printf("*** this wtp is unused ***\n");
		AC_WTP[WtpID]->unused_flag =1;
		wid_trap_remote_restart(WtpID);
		tmp = WID_SUSPEND_WTP(WtpID);
		//AC_WTP[WtpID]->isused = 0;
		AC_WTP[WtpID]->quitreason = WTP_UNUSED;
		//AC_WTP[WtpID]->channelsendtimes = 1;
		for(i=0;((i<AC_WTP[WtpID]->RadioCount)&&(i<L_RADIO_NUM));i++){
			if(AC_WTP[WtpID]->WTP_Radio[i]->auto_channel_cont == 0){
				AC_WTP[WtpID]->WTP_Radio[i]->channelsendtimes = 1;
			}
		}
	}
	else
	{
		//printf("*** this wtp before state is unused ***\n");
	}
		
	return 0;
}

void WID_WTP_SSID_KEY_CONFLICT(unsigned int RadioID,unsigned char WlanID)
{
	unsigned int WtpID = RadioID/L_RADIO_NUM;
	unsigned char localradio_id = RadioID%L_RADIO_NUM;
	struct wlanid *wlan_id_next = NULL;

	wid_syslog_debug_debug(WID_DEFAULT,"%s wid_dbug_trap_ssid_key_conflict\n",__func__);
	wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
	while(wlan_id_next != NULL)
	{	
		if(wlan_id_next->wlanid == WlanID){
			wlan_id_next = wlan_id_next->next;
			continue;
		}else{
			if(((AC_WLAN[wlan_id_next->wlanid])&&(AC_WLAN[wlan_id_next->wlanid]->SecurityID == AC_WLAN[WlanID]->SecurityID)
				&&(AC_WLAN[WlanID]->KeyLen))
				|| ((AC_WLAN[wlan_id_next->wlanid])&&(AC_WLAN[WlanID]->KeyLen) && (AC_WLAN[wlan_id_next->wlanid]->KeyLen == AC_WLAN[WlanID]->KeyLen)
				&& (strncmp(AC_WLAN[wlan_id_next->wlanid]->WlanKey,AC_WLAN[WlanID]->WlanKey,AC_WLAN[WlanID]->KeyLen)==0)))
				wid_dbug_trap_ssid_key_conflict(WtpID, (unsigned char)localradio_id, wlan_id_next->wlanid, WlanID);
				break;
		}
	}

	return ;
}


int Check_Interface_Config(char * ifname,WTPQUITREASON *quitreason)
{
	wid_syslog_debug_debug(WID_DEFAULT,"Check_Interface_Config:%s.\n",ifname);
	int sockfd;
	struct ifreq	ifr;
	//struct sockaddr_in	*sinptr;
	//struct sockaddr_in6	*sin6ptr;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));	
	
	if(ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
		printf("SIOCGIFINDEX error\n");
		*quitreason = IF_NOINDEX;
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is IF_NOINDEX error");
		close(sockfd);
		return APPLY_IF_FAIL;
	 }
	
	//printf("ifindex %d\n",ifr.ifr_ifindex);
	
/*	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1){
		printf("SIOCGIFFLAGS error\n");
		*quitreason = IF_NOFLAGS;
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is IF_NOFLAGS error");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	
	if(ifr.ifr_flags & IFF_UP){
		//printf("interface UP\n");
	}else{
		//printf("interface DOWN\n");
		*quitreason = IF_DOWN;
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is IF_DOWN error");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	//check interface ip addr
	
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) == -1){
		printf("SIOCGIFADDR error\n");

		*quitreason = IF_NOADDR;
		wid_syslog_notice("wtp quit reason is IF_NOADDR error");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	*/
	close(sockfd);
	wid_syslog_debug_debug(WID_DEFAULT,"Check_Interface_Config ifname:%s quitreason:%d\n",ifname,*quitreason);
	return 0;
	
}
/*fengwenchao copy from 1318 for AXSSZFI-839*/
int Get_Interface_binding_Info(char * ifname, struct ifi_info *ifi){
	int sockfd;
	struct ifreq	ifr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&ifr,0,sizeof(struct ifreq));
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));			
	
	if(ioctl(sockfd, SIOCGIFUDFFLAGS, (char *)&ifr) == -1){
		wid_syslog_err("%s,%d,get interface %s flags ioctl failed !\n",__func__,__LINE__,ifname);
	}else{
		wid_syslog_info("%s,%d,get interface %s ifr.ifr_flags:%#x",__func__,__LINE__,ifname,ifr.ifr_flags);
		ifi->ifi_bflags = ifr.ifr_flags;
	}

	close(sockfd);
	return WID_DBUS_SUCCESS;
}

int Set_Interface_binding_Info(char * ifname,char flag){/*add flag--1,clear flag--0*/
	int sockfd;
	struct ifreq	ifr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&ifr,0,sizeof(struct ifreq));
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));			

	if(ioctl(sockfd, SIOCGIFUDFFLAGS, &ifr) == -1){
		wid_syslog_err("get interface %s flags ioctl failed %s!\n",ifname);
	}else{
		wid_syslog_info("get interface %s ifr.ifr_flags:%x",ifname,ifr.ifr_flags);
	}
	if(flag == 0){
		ifr.ifr_flags &= ~(IFF_BINDING_FLAG);
	}else{
		ifr.ifr_flags |= IFF_BINDING_FLAG;
	}
	wid_syslog_info("%s,%d,set interface %s ifr.ifr_flags:%x,flag=%d.",__func__,__LINE__,ifname,ifr.ifr_flags,flag);
	if(ioctl(sockfd, SIOCSIFUDFFLAGS, (char *)&ifr) == -1){
		wid_syslog_err("%s,%d,set interface %s flags ioctl failed!\n",__func__,__LINE__,ifname);
	}else{
		wid_syslog_info("%s,%d,set interface %s ifr.ifr_flags:%x",__func__,__LINE__,ifname,ifr.ifr_flags);
	}
	close(sockfd);
	return WID_DBUS_SUCCESS;
}
/*fengwenchao copy end*/
int Get_Interface_Info(char * ifname, struct ifi_info *ifi){
	int sockfd;
	struct ifreq	ifr, ifrcopy;
	struct sockaddr_in	*sinptr;
	struct sockaddr_in6	*sin6ptr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));			
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
//		printf("SIOCGIFINDEX error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is SIOCGIFINDEX error");
		close(sockfd);
		return APPLY_IF_FAIL;
	 }
	//printf("ifindex %d\n",ifr.ifr_ifindex);
	ifi->ifi_index = ifr.ifr_ifindex;
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1){
//		printf("SIOCGIFFLAGS error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is SIOCGIFFLAGS error");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	if(ifr.ifr_flags & IFF_UP){
		//printf("interface UP\n");
		ifi->ifi_flags = ifr.ifr_flags;
	}else{
//		printf("interface DOWN\n");
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is IF_DOWN error");
		//return APPLY_IF_FAIL;
	}
	Get_Interface_binding_Info(ifname,ifi);//fengwenchao copy from 1318 for AXSSZFI-839
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) == -1){
//		printf("SIOCGIFADDR error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is SIOCGIFADDR error");
		close(sockfd);
		ifi->addr_num = 0;
		return WID_DBUS_SUCCESS;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"addr %s",inet_ntoa(((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr));
	
	ifi->addr_num = ipaddr_list(ifi->ifi_index, ifi->addr, IFI_ADDR_NUM);
	if((ifi->addr_num < 0)||(ifi->addr_num > IFI_ADDR_NUM))  //fengwenchao add for AXSSZFI-1668
	{
		wid_syslog_err("%s from kernel addr_num = %d \n",__func__,ifi->addr_num);
		close(sockfd);
		return -1;
	}
	switch (ifr.ifr_addr.sa_family) {
		case AF_INET:
			sinptr = (struct sockaddr_in *) &ifr.ifr_addr;
			ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
			memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));
	
#ifdef	SIOCGIFBRDADDR
			ifrcopy = ifr;
			if (ifi->ifi_flags & IFF_BROADCAST) {
				ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
				sinptr = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr;
				ifi->ifi_brdaddr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
				memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));				
				wid_syslog_debug_debug(WID_DEFAULT,"addr %s",inet_ntoa(((struct sockaddr_in*)(ifi->ifi_brdaddr))->sin_addr));
			}
#endif
	
			break;
			case AF_INET6:
				sin6ptr = (struct sockaddr_in6 *) &ifr.ifr_addr;
				ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
				memcpy(ifi->ifi_addr, sin6ptr, sizeof(struct sockaddr_in6));
	
				break;
	
			default:
				break;
	}
	close(sockfd);
	return WID_DBUS_SUCCESS;
}
int Get_Ipaddr_Info(struct ifi_info *ifi){
	int sockfd;
	struct ifreq	ifr, ifrcopy;
	struct sockaddr_in	*sinptr;
	struct sockaddr_in6	*sin6ptr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	int ret = 0;
	ret = ipaddr_list_v2(ifi,ifi->addr, IFI_IF_NUM);
	if(ret != 0){
		close(sockfd);
		return ret;
	}
	strncpy(ifr.ifr_name,ifi->ifi_name, sizeof(ifr.ifr_name));	
	wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,ip:%d.%d.%d.%d.,ifname=%s.",__func__,__LINE__,(ifi->addr[0])&0xFF,(ifi->addr[0])&0xFF,(ifi->addr[0])&0xFF,(ifi->addr[0])&0xFF,ifr.ifr_name);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is SIOCGIFINDEX error");
		close(sockfd);
		return APPLY_IF_FAIL;
	 }
	ifi->ifi_index = ifr.ifr_ifindex;
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1){
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is SIOCGIFFLAGS error");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	if(ifr.ifr_flags & IFF_UP){
		ifi->ifi_flags = ifr.ifr_flags;
	}else{
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is IF_DOWN error");
	}
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) == -1){
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is SIOCGIFADDR error");
		close(sockfd);
		ifi->addr_num = 0;
		return WID_DBUS_SUCCESS;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"11addr %s",inet_ntoa(((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr));
	
	switch (ifr.ifr_addr.sa_family) {
		case AF_INET:
			wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
			sinptr = (struct sockaddr_in *) &ifr.ifr_addr;
			ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
			memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));
	
#ifdef	SIOCGIFBRDADDR
			wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
			ifrcopy = ifr;
			if (ifi->ifi_flags & IFF_BROADCAST) {
				ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
				sinptr = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr;
				ifi->ifi_brdaddr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
				memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));				
				wid_syslog_debug_debug(WID_DEFAULT,"addr %s",inet_ntoa(((struct sockaddr_in*)(ifi->ifi_brdaddr))->sin_addr));
			}
#endif
	
			break;
			case AF_INET6:
				sin6ptr = (struct sockaddr_in6 *) &ifr.ifr_addr;
				ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
				memcpy(ifi->ifi_addr, sin6ptr, sizeof(struct sockaddr_in6));
	
				break;
	
			default:
				wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
				break;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
	close(sockfd);
	return WID_DBUS_SUCCESS;
}
int Delete_Bind_Interface_For_WID(struct ifi_info *ifi){
	struct CWMultiHomedInterface *tmp = gACSocket.interfaces;
	struct CWMultiHomedInterface *tmp_next = gACSocket.interfaces;
	if((gACSocket.count == 0)||(tmp == NULL))
		return 0;
	int i = 0;
	int m = 0;
	int n = 0;
	if(ifi)
		n = strlen(ifi->ifi_name);
	int gIndex = 0;
	while(tmp_next != NULL){		
		m = strlen(tmp_next->ifname);
		if((m == n)&&(strncmp(tmp_next->ifname,ifi->ifi_name, strlen(ifi->ifi_name)) == 0)){
			for(i=0;i<ifi->addr_num;i++){
				//if((memcmp(&(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr),&(ifi->addr[i]),sizeof(int))==0))
				if(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr == ifi->addr[i])
				{
					tmp = tmp_next->if_next;
					tmp_next->if_next = NULL;
					gIndex = tmp_next->gIf_Index;
					//fengwenchao modify begin 20110524
					if((gIndex < gMaxInterfacesCount)&& (gIndex >= 0))
					{	gInterfaces[gIndex].WTPCount = 0;
						gInterfaces[gIndex].enable = 0; 
					}
					else
					{
						wid_syslog_err("gIndex	%d is not legal \n",gIndex);
					}
					//fengwenchao modify end					
					wid_syslog_notice("111111111delete %d (%d, %s) index %d sock %d\n", tmp_next->gIf_Index, tmp_next->systemIndex, tmp_next->ifname,ifi->ifi_index,tmp_next->sock);
					close(tmp_next->sock);
					free(tmp_next);
					tmp_next = tmp;
					
					WIDWsm_VRRPIFOp((unsigned char*)ifi->ifi_name,ifi->addr[i],VRRP_UNREG_IF);
					gACSocket.interfaces = tmp;
					if(gACSocket.count > 0)
					{
						gACSocket.count--;
					}
					break;
				}				
			}
			if(i < ifi->addr_num){
				tmp = gACSocket.interfaces;
				tmp_next = tmp;
				continue;
			}
		}
				
		break;
	}
	if(gACSocket.interfaces == NULL)
		return 0;
	tmp = gACSocket.interfaces;
	tmp_next = tmp->if_next;
	while(tmp_next != NULL){
		m = strlen(tmp_next->ifname);
		if((m == n)&&(strncmp(tmp_next->ifname,ifi->ifi_name, strlen(ifi->ifi_name)) == 0)){
				for(i=0;i<ifi->addr_num;i++){
					//if((memcmp(&(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr).s_addr,&(ifi->addr[i]),sizeof(int))==0))
					if(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr == ifi->addr[i])
					{
						tmp->if_next = tmp_next->if_next;
						tmp_next->if_next = NULL;
						gIndex = tmp_next->gIf_Index;
						//fengwenchao modify begin 20110524
						if((gIndex < gMaxInterfacesCount)&& (gIndex >= 0))
						{	gInterfaces[gIndex].WTPCount = 0;
							gInterfaces[gIndex].enable = 0; 
						}
						else
						{
							wid_syslog_err("gIndex	%d is not legal \n",gIndex);
						}
						//fengwenchao modify end					
						wid_syslog_notice("222222222222delete %d (%d, %s) index %d sock %d\n", tmp_next->gIf_Index, tmp_next->systemIndex, tmp_next->ifname,ifi->ifi_index,tmp_next->sock);
						close(tmp_next->sock);
						free(tmp_next);
						tmp_next = tmp->if_next;						
						WIDWsm_VRRPIFOp((unsigned char*)ifi->ifi_name,ifi->addr[i],VRRP_UNREG_IF);
						if(gACSocket.count > 0)
						{
							gACSocket.count--;
						}
						break;
					}
				}
				if(i < ifi->addr_num)
					continue;			
		}
		tmp = tmp_next;
		tmp_next = tmp->if_next;
	}
	Check_gACSokcet_Poll(&gACSocket);
	return 0;
}
void Check_Current_Interface_and_delete(char * ifname, struct ifi_info *ifi){
	struct CWMultiHomedInterface *tmp = gACSocket.interfaces;
	struct CWMultiHomedInterface *tmp_next = gACSocket.interfaces;
	int i = 0;
	int m = 0;
	int n = strlen(ifname);
	int gIndex = 0;
	while(tmp_next != NULL){		
		m = strlen(tmp_next->ifname);
		if((m == n)&&(strncmp(tmp_next->ifname,ifname, strlen(ifname)) == 0)){
			if((tmp_next->systemIndex != ifi->ifi_index)){
				tmp = tmp_next->if_next;
				tmp_next->if_next = NULL;
				gIndex = tmp_next->gIf_Index;				
				//printf("delete %d (%d, %s)", tmp_next->gIf_Index, tmp_next->systemIndex, tmp_next->ifname,ifi->ifi_index);
				//fengwenchao modify begin 20110524
				if((gIndex < gMaxInterfacesCount)&& (gIndex >= 0))
				{	gInterfaces[gIndex].WTPCount = 0;
					gInterfaces[gIndex].enable = 0; 
				}
				else
				{
					wid_syslog_err("gIndex	%d is not legal \n",gIndex);
				}
				//fengwenchao modify end
				close(tmp_next->sock);
				free(tmp_next);
				tmp_next = tmp;
				gACSocket.interfaces = tmp;
				if(gACSocket.count > 0)
					gACSocket.count--;
				wid_syslog_debug_debug(WID_DEFAULT,"%s,111gACSocket.count--,count=%d\n",__func__,gACSocket.count);
				Check_WLAN_WTP_IF_Index(ifi,ifname);	
				continue;
			}else{
				for(i=0;i<ifi->addr_num;i++){
					//if((memcmp(&(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr).s_addr,&(ifi->addr[i]),sizeof(int))==0))
					if(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr == ifi->addr[i])
					{
						ifi->check_addr[i] += 1;
						if(ifi->check_addr[i] == 2)
							ifi->addr[i] = 0;
						break;
					}				
				}
				/*if((memcmp(&((struct sockaddr_in *)&(tmp_next->addr))->sin_addr,&((struct sockaddr_in *)&(ifi->ifi_brdaddr))->sin_addr,sizeof(struct in_addr))==0)){
					ifi->check_brdaddr += 1;
					break;
				}	*/
				if(i == ifi->addr_num){			
					tmp = tmp_next->if_next;
					tmp_next->if_next = NULL;
					gIndex = tmp_next->gIf_Index;
					//fengwenchao modify begin 20110524
					if((gIndex < gMaxInterfacesCount)&& (gIndex >= 0))
					{	gInterfaces[gIndex].WTPCount = 0;
						gInterfaces[gIndex].enable = 0; 
					}
					else
					{
						wid_syslog_err("gIndex	%d is not legal \n",gIndex);
					}
					//fengwenchao modify end
					//printf("delete %d (%d, %s)", tmp_next->gIf_Index, tmp_next->systemIndex, tmp_next->ifname,ifi->ifi_index);
					close(tmp_next->sock);
					free(tmp_next);
					tmp_next = tmp;
					gACSocket.interfaces = tmp;
					if(gACSocket.count > 0)
						gACSocket.count--;
					wid_syslog_debug_debug(WID_DEFAULT,"%s,222gACSocket.count--,count=%d\n",__func__,gACSocket.count);
					continue;
				}else{
					break;
				}
			}
				
		}
		break;
	}
	if(gACSocket.interfaces == NULL)
		return;
	tmp = gACSocket.interfaces;
	tmp_next = tmp->if_next;
	while(tmp_next != NULL){
		m = strlen(tmp_next->ifname);
		if((m == n)&&(strncmp(tmp_next->ifname,ifname, strlen(ifname)) == 0)){
			if((tmp_next->systemIndex != ifi->ifi_index)){
				tmp->if_next = tmp_next->if_next;
				tmp_next->if_next = NULL;
				gIndex = tmp_next->gIf_Index;
				//fengwenchao modify begin 20110524
				if((gIndex < gMaxInterfacesCount)&& (gIndex >= 0))
				{	gInterfaces[gIndex].WTPCount = 0;
					gInterfaces[gIndex].enable = 0; 
				}
				else
				{
					wid_syslog_err("gIndex	%d is not legal \n",gIndex);
				}
				//fengwenchao modify end
				close(tmp_next->sock);
				free(tmp_next);
				tmp_next = tmp->if_next;				
				if(gACSocket.count > 0)
					gACSocket.count --;
				wid_syslog_debug_debug(WID_DEFAULT,"%s,333gACSocket.count--,count=%d\n",__func__,gACSocket.count);
				Check_WLAN_WTP_IF_Index(ifi,ifname);	
				continue;
			}else{
				for(i=0;i<ifi->addr_num;i++){
					//if((memcmp(&(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr).s_addr,&(ifi->addr[i]),sizeof(struct in_addr))==0))
					if(((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr == ifi->addr[i])
					{
						ifi->check_addr[i] += 1;
						if(ifi->check_addr[i] == 2)
							ifi->addr[i] = 0;
						break;
					}
				}
				/*if((memcmp(&((struct sockaddr_in *)&(tmp_next->addr))->sin_addr,&((struct sockaddr_in *)&(ifi->ifi_brdaddr))->sin_addr,sizeof(struct in_addr))==0)){
					ifi->check_brdaddr += 1;
					tmp = tmp_next;
					tmp_next = tmp->if_next;
					continue;
				}*/
				if(i == ifi->addr_num){			
					tmp->if_next = tmp_next->if_next;
					tmp_next->if_next = NULL;
					gIndex = tmp_next->gIf_Index;
					//fengwenchao modify begin 20110524
					if((gIndex < gMaxInterfacesCount)&& (gIndex >= 0))
					{	gInterfaces[gIndex].WTPCount = 0;
						gInterfaces[gIndex].enable = 0; 
					}
					else
					{
						wid_syslog_err("gIndex	%d is not legal \n",gIndex);
					}
					//fengwenchao modify end
					close(tmp_next->sock);
					free(tmp_next);
					tmp_next = tmp->if_next;
					if(gACSocket.count > 0)
						gACSocket.count --;
					wid_syslog_debug_debug(WID_DEFAULT,"%s,444gACSocket.count--,count=%d\n",__func__,gACSocket.count);
					continue;
				}else{
					tmp = tmp_next;
					tmp_next = tmp->if_next;
					continue;
				}
			}
				
		}
		tmp = tmp_next;
		tmp_next = tmp->if_next;
	}
	return;
}
int Delete_listenning_IF(char * ifname){
//	int i = 0;
	int m = 0;
	int n = 0;
	if(ifname == NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"%s,ifname=%p\n",__func__,ifname);
		return -1;
	}
	if (WID_IF != NULL)
	{
		struct ifi *tmp2 = WID_IF;
		struct ifi *tmp3 = WID_IF;
		
		m = strlen(tmp2->ifi_name);
		n = strlen(ifname);
		wid_syslog_debug_debug(WID_DEFAULT,"%s,00delete interface from WID_IF,ifname=%s,m=%d,n=%d\n",__func__,ifname,m,n);
		if((m == n)&&(tmp2->lic_flag == DOWN_LINK_IF_TYPE)&&(memcmp(tmp2->ifi_name,ifname,m)==0)){
			WID_IF = tmp2->ifi_next;
			tmp2->ifi_next = NULL;
			free(tmp2);
			tmp2 = NULL;
			wid_syslog_debug_debug(WID_DEFAULT,"%s,11delete interface from WID_IF,ifname=%s\n",__func__,ifname);
		}else{
			tmp2 = tmp3->ifi_next;
			while(tmp2 != NULL){
				int m = strlen(tmp2->ifi_name);
				wid_syslog_debug_debug(WID_DEFAULT,"%s,22delete interface from WID_IF,tmp2->ifi_name=%s,m=%d,n=%d\n",__func__,tmp2->ifi_name,m,n);
				if((m == n)&&(tmp2->lic_flag == DOWN_LINK_IF_TYPE)&&(memcmp(tmp2->ifi_name,ifname,m)==0)){
					tmp3->ifi_next = tmp2->ifi_next;
					tmp2->ifi_next = NULL;
					free(tmp2);
					tmp2 = NULL;	
					wid_syslog_debug_debug(WID_DEFAULT,"%s,33delete interface from WID_IF,ifname=%s\n",__func__,ifname);
					break;
				}
				tmp3 = tmp2;
				tmp2 = tmp2->ifi_next;
			}
		}
	}
	else
	{
		wid_syslog_debug_debug(WID_DBUS, "%s %d WID_IF is empth\n", __func__, __LINE__);
	}
	
	if((gACSocket.interfaces!= NULL))
	{
		struct CWMultiHomedInterface *tmp = gACSocket.interfaces;
		struct CWMultiHomedInterface *tmp_next = gACSocket.interfaces;
		
		unsigned int  ip =0;
		int gIndex = 0;
		
		m = strlen(tmp->ifname);
		n = strlen(ifname);

		while(tmp != NULL){
			tmp_next = tmp->if_next;
			if((m == n)&&(strncmp(tmp->ifname,ifname,IFI_NAME) == 0)){		
				if(tmp->lic_flag == (DOWN_LINK_IF_TYPE|DOWN_LINK_IP_TYPE)){
					Set_Interface_binding_Info(ifname,0);
					tmp->lic_flag = DOWN_LINK_IP_TYPE;
					wid_syslog_debug_debug(WID_DEFAULT,"%s modify\n",__func__);
					break;
				}
				else if(tmp->lic_flag == DOWN_LINK_IF_TYPE){
					gACSocket.interfaces = tmp_next;
					tmp->if_next = NULL;
					gIndex = tmp->gIf_Index;
					if((gIndex < gMaxInterfacesCount)&& (gIndex >= 0))
					{	gInterfaces[gIndex].WTPCount = 0;
						gInterfaces[gIndex].enable = 0; 
					}
					close(tmp->sock);
					Set_Interface_binding_Info(ifname,0);			
					ip = ((struct sockaddr_in *)&(tmp->addr))->sin_addr.s_addr;
					WIDWsm_VRRPIFOp((unsigned char*)(tmp->ifname),ip,VRRP_UNREG_IF);
					free(tmp);
					tmp = NULL;
					if(gACSocket.count > 0)
						gACSocket.count--;
					wid_syslog_debug_debug(WID_DEFAULT,"%s,000gACSocket.count--,count=%d\n",__func__,gACSocket.count);
					tmp = tmp_next;
					continue;
				}
			}
			break;
		}
		while(tmp_next != NULL){		
			m = strlen(tmp_next->ifname);
			if((m == n)&&(strncmp(tmp_next->ifname,ifname,IFI_NAME) == 0)){				
				if(tmp_next->lic_flag == (DOWN_LINK_IF_TYPE|DOWN_LINK_IP_TYPE)){
					Set_Interface_binding_Info(ifname,0);
					tmp_next->lic_flag = DOWN_LINK_IP_TYPE;
					wid_syslog_debug_debug(WID_DEFAULT,"%s modify2222\n",__func__);
					break;
				}
				else if(tmp_next->lic_flag == DOWN_LINK_IF_TYPE){
					tmp->if_next = tmp_next->if_next;
					tmp_next->if_next = NULL;
					gIndex = tmp_next->gIf_Index;				
					if((gIndex >= 0)&&(gIndex < gMaxInterfacesCount)){    //fengwenchao change gIndex > 0  to  gIndex >= 0  20110525
						gInterfaces[gIndex].WTPCount = 0;
						gInterfaces[gIndex].enable = 0;
					}
					close(tmp_next->sock);
					Set_Interface_binding_Info(ifname,0);
					ip = ((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr;
					WIDWsm_VRRPIFOp((unsigned char*)(tmp_next->ifname),ip,VRRP_UNREG_IF);
					free(tmp_next);
					tmp_next = NULL;
					tmp_next = tmp->if_next;
					if(gACSocket.count > 0)
						gACSocket.count--;
					wid_syslog_debug_debug(WID_DEFAULT,"%s,222gACSocket.count--,count=%d\n",__func__,gACSocket.count);
					break;
				}else{
					tmp = tmp_next;
					tmp_next = tmp->if_next;
				}
				continue;
			}
			tmp = tmp_next;
			tmp_next = tmp->if_next;
		}
	}
	else
	{
		wid_syslog_debug_debug(WID_DBUS, "%s %d gACSocket.count = 0\n", __func__, __LINE__);
	}
	Check_gACSokcet_Poll(&gACSocket);
	if (WID_IF == NULL || gACSocket.interfaces== NULL)
	{
		wid_syslog_info("%s %d either WID_IF or gACSocket is empty!!!\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}
int Delete_listenning_IP(unsigned int ipaddr,LISTEN_FLAG flag){
	struct CWMultiHomedInterface *tmp = gACSocket.interfaces;
	struct CWMultiHomedInterface *tmp_next = gACSocket.interfaces;
	int gIndex = 0;
	struct ifi *tmp2 = WID_IF;
	struct ifi *tmp3 = WID_IF;
	char find = 0;
	if (tmp2 != NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"%s,delete ip from WID_IF,ip=%d.%d.%d.%d.\n",__func__,(ipaddr>>24)&0xFF,(ipaddr>>16)&0xFF,(ipaddr>>8)&0xFF,(ipaddr)&0xFF);
		//if((m == n)&&(memcmp(tmp2->ifi_name,ifname,m)==0))
		if((tmp2->addr == ipaddr)&&(tmp2->lic_flag == flag)){
			WID_IF = tmp2->ifi_next;
			tmp2->ifi_next = NULL;
			free(tmp2);
			tmp2 = NULL;
			wid_syslog_debug_debug(WID_DEFAULT,"1,%s,ip=%d.%d.%d.%d.\n",__func__,(ipaddr>>24)&0xFF,(ipaddr>>16)&0xFF,(ipaddr>>8)&0xFF,(ipaddr)&0xFF);
		}else{
			tmp2 = tmp3->ifi_next;
			while(tmp2 != NULL){
				//int m = strlen(tmp2->ifi_name);
				wid_syslog_debug_debug(WID_DEFAULT,"2,%s,ip=%d.%d.%d.%d.\n",__func__,(ipaddr>>24)&0xFF,(ipaddr>>16)&0xFF,(ipaddr>>8)&0xFF,(ipaddr)&0xFF);
				//if((m == n)&&(memcmp(tmp2->ifi_name,ifname,m)==0))
				if((tmp2->addr == ipaddr)&&(tmp2->lic_flag == flag)){
					tmp3->ifi_next = tmp2->ifi_next;
					tmp2->ifi_next = NULL;
					free(tmp2);
					tmp2 = NULL;	
					wid_syslog_debug_debug(WID_DEFAULT,"3,%s,ip=%d.%d.%d.%d.\n",__func__,(ipaddr>>24)&0xFF,(ipaddr>>16)&0xFF,(ipaddr>>8)&0xFF,(ipaddr)&0xFF);
					break;
				}
				tmp3 = tmp2;
				tmp2 = tmp2->ifi_next;
			}
		}
	}
	if((tmp == NULL)||(tmp_next == NULL) || gACSocket.count == 0){
		return -1;
	}
	tmp_next = tmp->if_next;
	//if((m == n)&&(strncmp(tmp->ifname,ifname,IFI_NAME) == 0))
	wid_syslog_debug_debug(WID_DEFAULT,"%s,tmp->lic_flag=%d,flag=%d.\n",__func__,tmp->lic_flag,flag);
	if((((struct sockaddr_in *)&(tmp->addr))->sin_addr.s_addr == ipaddr)){
		if(tmp->lic_flag == (DOWN_LINK_IF_TYPE|DOWN_LINK_IP_TYPE)){
			tmp->lic_flag = DOWN_LINK_IF_TYPE;
		}
		else if(tmp->lic_flag == flag){
			gACSocket.interfaces = tmp_next;
			tmp->if_next = NULL;
			close(tmp->sock);
			if(flag != LIC_TYPE)
				WIDWsm_VRRPIFOp((unsigned char*)(tmp->ifname),ipaddr,VRRP_UNREG_IF);
			free(tmp);
			tmp = NULL;
			if(gACSocket.count > 0)
				gACSocket.count--;
			wid_syslog_debug_debug(WID_DEFAULT,"%s,000gACSocket.count--,count=%d\n",__func__,gACSocket.count);
			find  = 1;
		}	
	}/*else*/
	if(find == 0){
//		wid_syslog_debug_debug(WID_DEFAULT,"22%s,tmp_next->lic_flag=%d,flag=%d.\n",__func__,tmp_next->lic_flag,flag);
		while(tmp_next != NULL){	
			wid_syslog_debug_debug(WID_DEFAULT,"33%s,tmp_next->lic_flag=%d,flag=%d.\n",__func__,tmp_next->lic_flag,flag);
			//m = strlen(tmp_next->ifname);
			//if((m == n)&&(strncmp(tmp_next->ifname,ifname,IFI_NAME) == 0))
			if((((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr == ipaddr)){
				if(tmp_next->lic_flag == (DOWN_LINK_IF_TYPE|DOWN_LINK_IP_TYPE)){
					tmp_next->lic_flag = DOWN_LINK_IF_TYPE;
				}
				else if(tmp_next->lic_flag == flag){
					tmp->if_next = tmp_next->if_next;
					tmp_next->if_next = NULL;
					gIndex = tmp_next->gIf_Index;				
					if((gIndex >= 0)&&(gIndex < gMaxInterfacesCount)){    //fengwenchao change gIndex > 0  to  gIndex >= 0  20110525
						gInterfaces[gIndex].WTPCount = 0;
						gInterfaces[gIndex].enable = 0;
					}
					close(tmp_next->sock);
					if(flag != LIC_TYPE)
						WIDWsm_VRRPIFOp((unsigned char*)(tmp_next->ifname),ipaddr,VRRP_UNREG_IF);
					free(tmp_next);
					tmp_next = NULL;
					tmp_next = tmp->if_next;
					if(gACSocket.count > 0)
						gACSocket.count--;
					wid_syslog_debug_debug(WID_DEFAULT,"%s,222gACSocket.count--,count=%d\n",__func__,gACSocket.count);
				}
				break;
			}
			tmp = tmp_next;
			tmp_next = tmp->if_next;
		}
	}
	Check_gACSokcet_Poll(&gACSocket);
	return 0;
}

void Check_Current_Interface(char * ifname, struct CWMultiHomedInterface **p,struct CWMultiHomedInterface **pbr){
	struct CWMultiHomedInterface *tmp = gACSocket.interfaces;
	if(ifname == NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"%s,ifname=%p\n",__func__,ifname);
		return;
	}
	while(tmp != NULL){
		//printf("tmp->ifname %s\n",tmp->ifname);
		int m = strlen(tmp->ifname);
		int n = strlen(ifname);
		//printf("##### m:%d n:%d####\n",m,n);
		if((m == n)&&(strncmp(tmp->ifname,ifname, strlen(ifname)) == 0)){
			if(tmp->kind == CW_PRIMARY)
				*p = tmp;
			else if(tmp->kind == CW_BROADCAST_OR_ALIAS)
				*pbr = tmp;
		}
		tmp = tmp->if_next;
	}
	return;
}

int Modify_Interface(struct CWMultiHomedInterface *p, struct CWMultiHomedInterface *pbr,  struct ifi_info *ifi, int port){
	//printf("Modify_Interface\n");
	int yes = 1;
	CWSocket sock;
	int i;
	if((sock = socket(ifi->ifi_addr->sa_family, SOCK_DGRAM, 0)) < 0) {
		CWNetworkRaiseSystemError(CW_ERROR_CREATING);
	}
	
	// reuse address
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	
	// bind address
	sock_set_port_cw(ifi->ifi_addr, htons(port));
	
	if(bind(sock, (struct sockaddr*) ifi->ifi_addr, CWNetworkGetAddressSize((CWNetworkLev4Address*)ifi->ifi_addr)) < 0) {
		close(sock);
		CWUseSockNtop(ifi->ifi_addr,wid_syslog_debug_debug(WID_DEFAULT,"failed %s", str);
		);
		//CWNetworkRaiseSystemError(CW_ERROR_CREATING);
	}

	CWUseSockNtop(ifi->ifi_addr,
		wid_syslog_debug_debug(WID_DEFAULT,"bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
	);
	
	// store socket inside multihomed socket
	wid_syslog_debug_debug(WID_DEFAULT,"p->sock %d\n",p->sock);
	close(p->sock);
	p->sock = sock;
	wid_syslog_debug_debug(WID_DEFAULT,"p->sock %d\n",p->sock);
	p->systemIndex = ifi->ifi_index;
	
	// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
	// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
	// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
	// garbage.
	p->addrIPv4.ss_family = AF_UNSPEC;
	CW_COPY_NET_ADDR_PTR(&(p->addr), ifi->ifi_addr);	
	i = p->gIf_Index;
	if((i < gMaxInterfacesCount)&&(i >= 0)&&(gInterfaces[i].enable == 1)){	//fengwenchao modify 20110525	
		CW_COPY_NET_ADDR_PTR(&(gInterfaces[i].addr), ((CWNetworkLev4Address*)&(p->addr)));
	}
/*	if (ifi->ifi_flags & IFF_BROADCAST) { // try to bind broadcast address
		if((sock = socket(ifi->ifi_addr->sa_family, SOCK_DGRAM, 0)) < 0) {
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		// reuse address
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		
	
		sock_set_port_cw(ifi->ifi_brdaddr, htons(port));
		
		if (bind(sock, (struct sockaddr*) ifi->ifi_brdaddr, CWNetworkGetAddressSize((CWNetworkLev4Address*)ifi->ifi_brdaddr)) < 0) {
			close(sock);
			if (errno == EADDRINUSE) {
				CWUseSockNtop(ifi->ifi_brdaddr,
				wid_syslog_debug_debug("EADDRINUSE: %s", str);
				);
			} else {
				CWUseSockNtop(ifi->ifi_brdaddr,
				wid_syslog_debug_debug("failed %s", str);
				);
				//CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
				//CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
		}
	
		CWUseSockNtop(ifi->ifi_brdaddr,
			wid_syslog_debug_debug("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
		);
		
		// store socket inside multihomed socket
		if(pbr == NULL){
			CW_CREATE_OBJECT_ERR(pbr, struct CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			struct CWMultiHomedInterface* inf = gACSocket.interfaces;
			while(inf->if_next != NULL)
				inf = inf->if_next;
			inf->if_next = pbr;

		}
		
		wid_syslog_debug_debug("pbr->sock %d\n",pbr->sock);
		close(pbr->sock);
		pbr->sock = sock;		
		wid_syslog_debug_debug("pbr->sock %d\n",pbr->sock);
		pbr->kind = CW_BROADCAST_OR_ALIAS;
		pbr->systemIndex = ifi->ifi_index;
		CW_COPY_NET_ADDR_PTR(&(pbr->addr), ifi->ifi_brdaddr);
		
		// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
		// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
		// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
		// garbage.
		pbr->addrIPv4.ss_family = AF_UNSPEC;
		
	}
	*/
	return 0;
}


int Match_And_Modify_Interface(struct CWMultiHomedInterface *p,struct CWMultiHomedInterface *pbr, struct ifi_info *ifi){
	if((p->systemIndex != ifi->ifi_index)||(sock_cmp_addr((struct sockaddr*)(&p->addr), (struct sockaddr*)(ifi->ifi_addr), sizeof(CWNetworkLev4Address)))){
		Modify_Interface(p,pbr,ifi,CW_CONTROL_PORT);
	//	Modify_Interface(p,pbr,ifi,CW_CONTROL_PORT_AU);
	}
	return 0;
}
int Bind_BroadAddr_For_WID(struct ifi_info *ifi, int port){
	int yes = 1;
	CWSocket sock;
	//int i;
	struct CWMultiHomedInterface *p;

	if (ifi->ifi_flags & IFF_BROADCAST) { // try to bind broadcast address
		if((sock = socket(ifi->ifi_addr->sa_family, SOCK_DGRAM, 0)) < 0) {
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		// reuse address
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		
	
		//sock_set_port_cw(ifi->ifi_brdaddr, htons(CW_CONTROL_PORT));
		sock_set_port_cw(ifi->ifi_brdaddr, htons(port));//zhanglei change
		if (bind(sock, (struct sockaddr*) ifi->ifi_brdaddr, CWNetworkGetAddressSize((CWNetworkLev4Address*)ifi->ifi_brdaddr)) < 0) {
			close(sock);
			if (errno == EADDRINUSE) {
				CWUseSockNtop(ifi->ifi_brdaddr,
					wid_syslog_debug_debug(WID_DEFAULT,"EADDRINUSE: %s", str);
				);
			} else {
	
				CWUseSockNtop(ifi->ifi_brdaddr,
					wid_syslog_debug_debug(WID_DEFAULT,"failed %s", str);
				);
				//CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
				//CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
		}
		
		CWUseSockNtop(ifi->ifi_brdaddr,
			printf("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
		);
		
		// store socket inside multihomed socket
		
		CW_CREATE_OBJECT_ERR(p, struct CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		memset(p->ifname, 0, IFI_NAME);
		strncpy(p->ifname, ifi->ifi_name, IFI_NAME);
		p->sock = sock; 		
		printf("pbr->sock %d\n",p->sock);
		p->kind = CW_BROADCAST_OR_ALIAS;
		p->systemIndex = ifi->ifi_index;
		p->systemIndexbinding = ifi->ifi_index_binding;
		CW_COPY_NET_ADDR_PTR(&(p->addr), ifi->ifi_brdaddr);
		
		// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
		// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
		// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
		// garbage.
		p->addrIPv4.ss_family = AF_UNSPEC;
		struct CWMultiHomedInterface* inf = gACSocket.interfaces;
		if(gACSocket.interfaces == NULL)
			gACSocket.interfaces = p;
		else{
			while(inf->if_next != NULL)
				inf = inf->if_next;
			inf->if_next = p;
			p->if_next = NULL;
		}
		gACSocket.count++; // we add a socket to the multihomed socket
		Check_gACSokcet_Poll(&gACSocket);

	}
	return 0;
}
int Check_Listenning_If(char* ifname,struct ifi_info *ifi,char flag){
	int m=0,n=0/*,ret=0*/;
	struct CWMultiHomedInterface /*  *p=NULL,*/*p_next = NULL;
	p_next = gListenningIF.interfaces;
	wid_syslog_debug_debug(WID_DEFAULT,"%s,gListenningIF.count=%d,gListenningIF.interfaces=%p\n",__func__,gListenningIF.count,gListenningIF.interfaces);
	if(gListenningIF.count <= 0)
	return 0;
	wid_syslog_debug_debug(WID_DEFAULT,"%s,11p_next=%p.\n",__func__,p_next);
	while(p_next !=NULL){
		if(ifname)
			m = strlen(ifname);
		if(p_next->ifname)
			n= strlen(p_next->ifname);
		wid_syslog_debug_debug(WID_DEFAULT,"%s,gListenningIF.count=%d.\n",__func__,gListenningIF.count);
		wid_syslog_debug_debug(WID_DEFAULT,"%s,11m=%d,n=%d,ifname=%s,p_next->ifname=%s\n",__func__,m,n,ifname,p_next->ifname);
		if((m==n)&&(strcmp(ifname,p_next->ifname)==0)){
			wid_syslog_debug_debug(WID_DEFAULT,"%s,22m=%d,n=%d,ifname=%s,p_next->ifname=%s\n",__func__,m,n,ifname,p_next->ifname);
			if(flag == 1){
				((struct sockaddr_in *)&(p_next->addr))->sin_addr.s_addr = ((struct sockaddr_in *) ifi->ifi_addr)->sin_addr.s_addr;
			}
			//ptr = p_next;
			return IF_HAS_BEEN_LISTENNING;
			//break;
		}
		p_next = p_next->if_next;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"%s,22p_next=%p.\n",__func__,p_next);
	return 0 ;
}
int Check_Listenning_Ip(char *ifname,unsigned int addr,LISTEN_FLAG flag,char nl_flag){
	int m=0,n=0/*,ret=0*/;
	struct CWMultiHomedInterface /*  *p=NULL,*/*p_next = NULL;
	p_next = gListenningIF.interfaces;
	char tmp_if[IFI_NAME];
	memset(tmp_if,0,IFI_NAME);
	wid_syslog_debug_debug(WID_DEFAULT,"%s,gListenningIF.count=%d,gListenningIF.interfaces=%p\n",__func__,gListenningIF.count,gListenningIF.interfaces);
	if(gListenningIF.count <= 0)
	return 0;
	wid_syslog_debug_debug(WID_DEFAULT,"%s,11p_next=%p.\n",__func__,p_next);
	while(p_next !=NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"%s,gListenningIF.count=%d.\n",__func__,gListenningIF.count);
		wid_syslog_debug_debug(WID_DEFAULT,"%s,11m=%d,n=%d,ifname=%s,p_next->ifname=%s\n",__func__,m,n,ifname,p_next->ifname);
		if((((struct sockaddr_in *)&(p_next->addr))->sin_addr.s_addr == addr)&&(flag == p_next->lic_flag))
		{
			if(memcmp(p_next->ifname,tmp_if,IFI_NAME) != 0){
				wid_syslog_debug_debug(WID_DEFAULT,"%s,p_next->ifname=%s,is NULL.\n",__func__,p_next->ifname);
			}else{
				memset(p_next->ifname,0,IFI_NAME);
				memcpy(p_next->ifname,ifname,IFI_NAME);
			}
			if(nl_flag == 1){
				memset(p_next->ifname,0,IFI_NAME);
				memcpy(p_next->ifname,ifname,IFI_NAME);
			}
			wid_syslog_debug_debug(WID_DEFAULT,"%s,22m=%d,n=%d,ifname=%s,p_next->ifname=%s,nl_flag=%d.\n",__func__,m,n,ifname,p_next->ifname,nl_flag);
			//ptr = p_next;
			return IF_HAS_BEEN_LISTENNING;
			//break;
		}
		p_next = p_next->if_next;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"%s,22p_next=%p.\n",__func__,p_next);
	return 0 ;
}
int Add_Listenning_IF(char * ifname){
	
	struct CWMultiHomedInterface *ptr;
	struct CWMultiHomedInterface* inf2 = gListenningIF.interfaces;
	CW_CREATE_OBJECT_ERR(ptr, struct CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	if(ptr == NULL){
		wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
		perror("malloc");
		return 0;
	}
	
	memset(ptr,0,sizeof(struct CWMultiHomedInterface));
	memset(ptr->ifname,0,IFI_NAME);
	memcpy(ptr->ifname,ifname,strlen(ifname));
	ptr->if_next = NULL;
	ptr->lic_flag = DOWN_LINK_IF_TYPE;
	wid_syslog_debug_debug(WID_DEFAULT,"%s,gListenningIF.count=%d,gListenningIF.interfaces=%p.\n",__func__,gListenningIF.count,gListenningIF.interfaces);
	if(gListenningIF.interfaces == NULL){
		gListenningIF.interfaces = ptr;
		ptr->if_next = NULL;
		gListenningIF.count ++;
		wid_syslog_debug_debug(WID_DEFAULT,"%s,11gListenningIF.count ++,count=%d\n",__func__,gListenningIF.count);
	}else{
		int ret = 0;
		if(ret == 0){
			while(inf2->if_next != NULL){
				inf2 = inf2->if_next;				
			}
			inf2->if_next = ptr;
			ptr->if_next = NULL;
			gListenningIF.count ++;
			wid_syslog_debug_debug(WID_DEFAULT,"%s,22gListenningIF.count ++,count=%d\n",__func__,gListenningIF.count);
		}else{
			free(ptr);
			ptr = NULL;
		}
	}
	return 0;
}
int Add_Listenning_IP(char * ifname,unsigned int addr,LISTEN_FLAG flag){
	
	struct CWMultiHomedInterface *ptr;
	struct CWMultiHomedInterface* inf2 = gListenningIF.interfaces;
	CW_CREATE_OBJECT_ERR(ptr, struct CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	if(ptr == NULL){
		wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
		perror("malloc");
		return 0;
	}
	
	memset(ptr,0,sizeof(struct CWMultiHomedInterface));
	memset(ptr->ifname,0,IFI_NAME);
	memcpy(ptr->ifname,ifname,strlen(ifname));
	ptr->lic_flag = flag;
	((struct sockaddr_in *)&(ptr->addr))->sin_addr.s_addr = addr;
	ptr->if_next = NULL;
	wid_syslog_debug_debug(WID_DEFAULT,"%s,gListenningIF.count=%d,gListenningIF.interfaces=%p.\n",__func__,gListenningIF.count,gListenningIF.interfaces);
	if(gListenningIF.interfaces == NULL){
		gListenningIF.interfaces = ptr;
		ptr->if_next = NULL;
		gListenningIF.count ++;
		wid_syslog_debug_debug(WID_DEFAULT,"%s,11gListenningIF.count ++,count=%d\n",__func__,gListenningIF.count);
	}else{
		int ret = 0;
		ret = Check_Listenning_Ip(ptr->ifname,addr,flag,0);
		if(ret == 0){
			while(inf2->if_next != NULL){
				inf2 = inf2->if_next;				
			}
			inf2->if_next = ptr;
			ptr->if_next = NULL;
			gListenningIF.count ++;
			wid_syslog_debug_debug(WID_DEFAULT,"%s,22gListenningIF.count ++,count=%d\n",__func__,gListenningIF.count);
		}else{
			free(ptr);
			ptr = NULL;
		}
	}
	return 0;
}

int check_bind_sock_info(struct ifi_info *ifi,LISTEN_FLAG lic_flag)
{
	struct CWMultiHomedInterface *tmp = gACSocket.interfaces;
	struct CWMultiHomedInterface *tmp_next = gACSocket.interfaces;
	unsigned int ipaddr = ((struct sockaddr_in *)(ifi->ifi_addr))->sin_addr.s_addr;
	if(tmp == NULL)
		return 0;
	wid_syslog_debug_debug(WID_DEFAULT,"%s,tmp->lic_flag=%d,flag=%d ip %d ip %d.\n",__func__,tmp->lic_flag,lic_flag,((struct sockaddr_in *)&(tmp->addr))->sin_addr.s_addr,ipaddr);
	/* AXSSZFI-1341: delete  (tmp->lic_flag == lic_flag) condition, before 130 version, there is no this condition check */
	/* AXSSZFI-1640: add interface's lic_flag check, and avoid 1341 bug */
	if((((struct sockaddr_in *)&(tmp->addr))->sin_addr.s_addr == ipaddr) && (tmp->lic_flag != lic_flag) && (tmp->lic_flag & (DOWN_LINK_IF_TYPE | DOWN_LINK_IP_TYPE))){		/* Huang Leilei delete last condition for AXSSZFI-1341, 2013-01-09 */
		wid_syslog_debug_debug(WID_DEFAULT,"22%s,tmp->lic_flag=%d,flag=%d.\n",__func__,tmp->lic_flag,lic_flag);
		tmp->lic_flag |= lic_flag;
		return 1;
	}
	else{
		while(tmp_next != NULL){	
			wid_syslog_debug_debug(WID_DEFAULT,"%s,tmp_next->lic_flag=%d,flag=%d ip %d ip %d.\n",__func__,tmp_next->lic_flag,lic_flag,((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr,ipaddr);
			/* AXSSZFI-1341: delete  (tmp->lic_flag == lic_flag) condition, before 130 version, there is no this condition check */
			/* AXSSZFI-1640: add interface's lic_flag check, and avoid 1341 bug */
			if((((struct sockaddr_in *)&(tmp_next->addr))->sin_addr.s_addr == ipaddr) && (tmp_next->lic_flag != lic_flag) && (tmp->lic_flag & (DOWN_LINK_IF_TYPE | DOWN_LINK_IP_TYPE))){		/* Huang Leilei delete last condition for AXSSZFI-1341, 2013-01-09 */
				wid_syslog_debug_debug(WID_DEFAULT,"33%s,tmp_next->lic_flag=%d,flag=%d.\n",__func__,tmp_next->lic_flag,lic_flag);
				tmp_next->lic_flag |= lic_flag;
				return 1;
			}
			tmp = tmp_next;
			tmp_next = tmp->if_next;
		}
	}
	return 0;
}



int Bind_Interface_For_WID(struct ifi_info *ifi, int port,LISTEN_FLAG lic_flag){
	//printf("Bind_Interface_For_WID\n");
	int yes = 1;
	CWSocket sock;
	int i;
	struct CWMultiHomedInterface *p;

	if((lic_flag != LIC_TYPE)&&(check_bind_sock_info(ifi,lic_flag)==1)){
		return 1;
	}

	
		if((sock = socket(ifi->ifi_addr->sa_family, SOCK_DGRAM, 0)) < 0) {
			CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}
		
		// reuse address
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		
		// bind address
		//sock_set_port_cw(ifi->ifi_addr, htons(CW_CONTROL_PORT));
		sock_set_port_cw(ifi->ifi_addr, htons(port));//zhanglei change
		if(bind(sock, (struct sockaddr*) ifi->ifi_addr, CWNetworkGetAddressSize((CWNetworkLev4Address*)ifi->ifi_addr)) < 0) {
			wid_syslog_info("%s,%d,errno = %d\n",__func__,__LINE__,errno);
			close(sock);
			CWUseSockNtop(ifi->ifi_addr,
			wid_syslog_debug_debug(WID_DEFAULT,"failed %s", str);
			);
			return 1;
			//CWNetworkRaiseSystemError(CW_ERROR_CREATING);
		}

		CWUseSockNtop(ifi->ifi_addr,
		wid_syslog_notice("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
		);
		
		// store socket inside multihomed socket
		
		CW_CREATE_OBJECT_ERR(p, struct CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		memset(p,0,sizeof(struct CWMultiHomedInterface));
		//memset(p->ifname, 0, IFI_NAME);
		strncpy(p->ifname, ifi->ifi_name, IFI_NAME);
		p->sock = sock;	
		p->ipv6Flag = 0;
		p->lic_flag = lic_flag;
		wid_syslog_notice("p->sock %d\n",p->sock);
		if(strncmp(ifi->ifi_name, "lo", 2)) { // don't consider loopback an interface
			wid_syslog_debug_debug(WID_DEFAULT,"Primary Address");
			p->kind = CW_PRIMARY;
		} else {
			p->kind = CW_BROADCAST_OR_ALIAS; // should be BROADCAST_OR_ALIAS_OR_MULTICAST_OR_LOOPBACK ;-)
	#ifdef CW_DEBUGGING
				if(!strncmp(ifi->ifi_name, "lo", 2)) {
					p->kind = CW_PRIMARY;
				}
	#endif
		}
		p->systemIndex = ifi->ifi_index;
		
		p->systemIndexbinding = ifi->ifi_index_binding;
		// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
		// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
		// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
		// garbage.
		p->addrIPv4.ss_family = AF_UNSPEC;
		CW_COPY_NET_ADDR_PTR(&(p->addr), ifi->ifi_addr);
		if(p->kind == CW_PRIMARY)
		for(i = 0; i < gMaxInterfacesCount; i++) {
			if(gInterfaces[i].enable == 0){
				CW_COPY_NET_ADDR_PTR(&(gInterfaces[i].addr), ((CWNetworkLev4Address*)&(p->addr)) );
				CW_COPY_NET_ADDR_PTR(&(gInterfaces[i].addrIPv4), ((struct sockaddr_in*)&(p->addr)) );
				gInterfaces[i].enable = 1;
				gInterfaces[i].WTPCount = 0;
				p->gIf_Index = i;
				break;
			}
		}
		struct CWMultiHomedInterface* inf = gACSocket.interfaces;
		if(gACSocket.interfaces == NULL){
			gACSocket.interfaces = p;
			p->if_next = NULL;
		}
		else{
			while(inf->if_next != NULL)
				inf = inf->if_next;
			inf->if_next = p;			
			p->if_next = NULL;
		}
		gACSocket.count++; // we add a socket to the multihomed socket
		Check_gACSokcet_Poll(&gACSocket);

/*		
		if (ifi->ifi_flags & IFF_BROADCAST) { // try to bind broadcast address
			if((sock = socket(ifi->ifi_addr->sa_family, SOCK_DGRAM, 0)) < 0) {
				CWNetworkRaiseSystemError(CW_ERROR_CREATING);
			}
			
			// reuse address
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
			
	
			//sock_set_port_cw(ifi->ifi_brdaddr, htons(CW_CONTROL_PORT));
			sock_set_port_cw(ifi->ifi_brdaddr, htons(port));//zhanglei change
			if (bind(sock, (struct sockaddr*) ifi->ifi_brdaddr, CWNetworkGetAddressSize((CWNetworkLev4Address*)ifi->ifi_brdaddr)) < 0) {
				close(sock);
				if (errno == EADDRINUSE) {
					CWUseSockNtop(ifi->ifi_brdaddr,
						wid_syslog_debug_debug("EADDRINUSE: %s", str);
					);
				} else {
		
					CWUseSockNtop(ifi->ifi_brdaddr,
						wid_syslog_debug_debug("failed %s", str);
					);
					//CWDeleteList(&interfaceList, CWNetworkDeleteMHInterface);
					//CWNetworkRaiseSystemError(CW_ERROR_CREATING);
				}
			}
			
			CWUseSockNtop(ifi->ifi_brdaddr,
				printf("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
			);
			
			// store socket inside multihomed socket
			
			CW_CREATE_OBJECT_ERR(p, struct CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(p->ifname, 0, IFI_NAME);
			strncpy(p->ifname, ifi->ifi_name, IFI_NAME);
			p->sock = sock;			
			printf("pbr->sock %d\n",p->sock);
			p->kind = CW_BROADCAST_OR_ALIAS;
			p->systemIndex = ifi->ifi_index;
			p->systemIndexbinding = ifi->ifi_index_binding;
			CW_COPY_NET_ADDR_PTR(&(p->addr), ifi->ifi_brdaddr);
			
			// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
			// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
			// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
			// garbage.
			p->addrIPv4.ss_family = AF_UNSPEC;
			struct CWMultiHomedInterface* inf = gACSocket.interfaces;
			if(gACSocket.interfaces == NULL)
				gACSocket.interfaces = p;
			else{
				while(inf->if_next != NULL)
					inf = inf->if_next;
				inf->if_next = p;
				p->if_next = NULL;
			}
			gACSocket.count++; // we add a socket to the multihomed socket
			
		}
*/		
	return 0;
}

int Repair_WID_Listening_Socket(struct CWMultiHomedInterface *inf){
	wid_syslog_info("$$$$$$%s,%d some socket error will repair it\n",__func__,__LINE__);
	if(inf == NULL){return -1;}
	struct CWMultiHomedInterface *tmp = gACSocket.interfaces;
	struct CWMultiHomedInterface *pre = gACSocket.interfaces;
	CWNetworkLev4Address *paddr = NULL;
	struct sockaddr *ptmp = NULL;
	int yes = 1;
	CWSocket sock = -1;
	if((gACSocket.count == 0)||(tmp == NULL))
		return -1;
	while(tmp != NULL){
		pre = tmp;
		if(memcmp(inf,tmp,sizeof(struct CWMultiHomedInterface)) == 0){
			if(close(tmp->sock)< 0)
				wid_syslog_info("%s,%d errror:%s\n",__func__,__LINE__,strerror(errno));
			paddr = &(tmp->addr);
			ptmp = (struct sockaddr*)&(tmp->addr);
			/*if happen error wid not listen this socket*/
			if((sock = socket(ptmp->sa_family, SOCK_DGRAM, 0)) < 0){
				wid_syslog_info("%s,%d errror:%s\n",__func__,__LINE__,strerror(errno));
				wid_syslog_info("%s,%d wid socket repair error\n",__func__,__LINE__);
				pre = tmp->if_next;
				free(tmp);
				tmp = NULL;
				return -1;
			}
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
			/*if happen error wid not listen this socket*/
			if(bind(sock, (struct sockaddr*) paddr, CWNetworkGetAddressSize((CWNetworkLev4Address*)paddr)) < 0) {
				close(sock);
				wid_syslog_info("%s,%d errror:%s\n",__func__,__LINE__,strerror(errno));
				wid_syslog_info("%s,%d socket repair error\n",__func__,__LINE__);
				pre = tmp->if_next;
				free(tmp);
				tmp = NULL;
				return -1;
			}
			tmp->sock = sock;
			break;
		}
		tmp = tmp->if_next;
	}
	return 0;
}
void Check_gACSokcet_Poll(CWMultiHomedSocket *ptr){

	//poll it


	int i = 0;
	if(ptr == NULL) return;
	struct CWMultiHomedInterface *inf = ptr->interfaces;
	memset(ptr->pfd,0,sizeof(ptr->pfd));
	for(i = 0;(i<ptr->count)&&(inf);i++){
		inf = inf->if_next;
	}
	ptr->count = i;
	if(i  > gMaxInterfacesCount) return;
	inf = ptr->interfaces;
	for(i = 0;(i<ptr->count)&&(inf);i++){
		ptr->pfd[i].fd = inf->sock;
		ptr->pfd[i].events = POLLIN|POLLPRI;
		inf = inf->if_next;
	}
}
void Check_WLAN_WTP_IF_Index(struct ifi_info *ifi, char *ifname){
	int i, j;
	for(i = 1;i < WTP_NUM; i++){
		if((AC_WTP[i]!=NULL)&&(strlen(AC_WTP[i]->BindingIFName) == strlen(ifname))&&(strncmp(AC_WTP[i]->BindingIFName,ifname,IFI_NAME)==0)&&(AC_WTP[i]->BindingSystemIndex != ifi->ifi_index)){
			wid_syslog_debug_debug(WID_DEFAULT,"wtpid %d\n",i);
			AC_WTP[i]->BindingSystemIndex = ifi->ifi_index;
		}
	}
	for(j = 1; j < WLAN_NUM; j++){
		if(AC_WLAN[j] != NULL){
			wid_syslog_debug_debug(WID_DEFAULT,"wlanid %d\n",j);
			struct ifi *wif = AC_WLAN[j]->Wlan_Ifi;
			while(wif != NULL){
				if((strlen(wif->ifi_name) == strlen(ifname))&&(strncmp(wif->ifi_name,ifname,IFI_NAME)==0)&&(wif->ifi_index != ifi->ifi_index)){
					wif->ifi_index = ifi->ifi_index;
					break;
				}
				wif = wif->ifi_next;
			}
		}
	}			

}

unsigned int is_local_board_interface(const char *ifname)
{
	/*********************hanhui upgrade***************
#define SIOCGIFPRIVFLAGS 0x89b0
	***************************************************/
#define SIOCGIFPRIVFLAGS 0x89b1
	/**************************end*********************/

	#define IFF_RPA 0x20
	int sock = -1;
	struct ifreq tmp;

	if (NULL == ifname) {
		return -1;
	}
	memset(&tmp, 0, sizeof(tmp));
	strcpy(tmp.ifr_name, ifname);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		return -1;
	}

	if (ioctl(sock, SIOCGIFPRIVFLAGS , &tmp) < 0) {
		close(sock);
		return -1;
	}
	close(sock);

	printf("%s\n", (IFF_RPA == tmp.ifr_flags) ? "rpa interface" : " ");
	
	return ((tmp.ifr_flags & IFF_RPA) != IFF_RPA);	
}

int Check_And_Bind_Interface_For_WID(char * ifname){
	int ret = 0;
	int retv6 = 0;
	int i = 0;
	unsigned int ip;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi_tmp->ifi_name,0,sizeof(ifi_tmp->ifi_name));
	strncpy(ifi_tmp->ifi_name,ifname,sizeof(ifi_tmp->ifi_name));
	//struct CWMultiHomedInterface *p = NULL,*pbr = NULL;	
	
	if(!is_local_board_interface(ifname)){
		if(ifi_tmp){
			free(ifi_tmp);
			ifi_tmp = NULL;
		}
		return WID_INTERFACE_NOT_BE_LOCAL_BOARD;	
	}
	ret = Get_Interface_Info(ifname,ifi_tmp);
	struct ifi * tmp;
	struct ifi * tmp2;
	//int GoOn;
	//printf("1\n");
	if(ret != 0){		
		Delete_Interface(ifname, ifi_tmp->ifi_index);
		if(ifi_tmp->ifi_addr != NULL){
			free(ifi_tmp->ifi_addr);
			ifi_tmp->ifi_addr = NULL;
		}		
		if(ifi_tmp->ifi_brdaddr != NULL){
			free(ifi_tmp->ifi_brdaddr);
			ifi_tmp->ifi_brdaddr = NULL;
		}
		free(ifi_tmp);
		ifi_tmp = NULL;
		return ret;		
	}	
	if(WID_IF == NULL){
	   /*fengwenchao copy from 1318 for AXSSZFI-839*/
	   if(0 == isNoCheck){
		   if((ifi_tmp->ifi_bflags&IFF_BINDING_FLAG)==IFF_BINDING_FLAG){
			   
			   if(ifi_tmp->ifi_addr != NULL){
				   free(ifi_tmp->ifi_addr);
				   ifi_tmp->ifi_addr = NULL;
			   }	   
			   if(ifi_tmp->ifi_brdaddr != NULL){
				   free(ifi_tmp->ifi_brdaddr);
				   ifi_tmp->ifi_brdaddr = NULL;
			   }
			   free(ifi_tmp);
			   ifi_tmp = NULL;
		   
			   wid_syslog_err("interface %s has be binded in other hansi.\n",ifname);
			   return IF_BINDING_FLAG;
		   }
		   wid_syslog_info("%s,%d,isNoCheck == 0,now is not load config,need check if binding flag.\n",__func__,__LINE__);
	   }else{
			wid_syslog_info("%s,%d,isNoCheck =%d,now is load config,not check if binding flag.\n",__func__,__LINE__,isNoCheck);
	   }
		/*fengwenchao copy end*/
		tmp = (struct ifi*)malloc(sizeof(struct ifi));
		memset(tmp,0,sizeof(struct ifi));
		memcpy(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME);
		tmp->lic_flag = DOWN_LINK_IF_TYPE;
		WID_IF = tmp;
		Set_Interface_binding_Info(ifi_tmp->ifi_name,1);//fengwenchao copy from 1318 for AXSSZFI-839
		Add_Listenning_IF(ifname);
		//printf("1111111tmp->ifname %s",tmp->ifi_name);
	}else{
		tmp = WID_IF;
		while(tmp != NULL){
			if((tmp->lic_flag == DOWN_LINK_IF_TYPE)&&(memcmp(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME)==0)){
				if(ifi_tmp->ifi_addr != NULL){
					free(ifi_tmp->ifi_addr);
					ifi_tmp->ifi_addr = NULL;
				}		
				if(ifi_tmp->ifi_brdaddr != NULL){
					free(ifi_tmp->ifi_brdaddr);
					ifi_tmp->ifi_brdaddr = NULL;
				}
				free(ifi_tmp);
				ifi_tmp = NULL;
				wid_syslog_debug_debug(WID_DEFAULT,"interface has been in WID_IF\n");
				return 0;
			}
			tmp = tmp->ifi_next;
		}
		/*fengwenchao copy from 1318 for AXSSZFI-839*/
		
		if(0 == isNoCheck){
			if((ifi_tmp->ifi_bflags&IFF_BINDING_FLAG)==IFF_BINDING_FLAG){
				
				if(ifi_tmp->ifi_addr != NULL){
					free(ifi_tmp->ifi_addr);
					ifi_tmp->ifi_addr = NULL;
				}		
				if(ifi_tmp->ifi_brdaddr != NULL){
					free(ifi_tmp->ifi_brdaddr);
					ifi_tmp->ifi_brdaddr = NULL;
				}
				free(ifi_tmp);
				ifi_tmp = NULL;
			
				wid_syslog_err("interface %s has be binded in other hansi.\n",ifname);
				return IF_BINDING_FLAG;
			}
			wid_syslog_info("%s,%d,isNoCheck == 0,now is not load config,need check if binding flag.\n",__func__,__LINE__);
		}else{
			 wid_syslog_info("%s,%d,isNoCheck =%d,now is load config,not check if binding flag.\n",__func__,__LINE__,isNoCheck);
		}
		/*fengwenchao copy end*/
		tmp = (struct ifi*)malloc(sizeof(struct ifi));
		memset(tmp,0,sizeof(struct ifi));
		memcpy(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME);
		tmp2 = WID_IF;		
		tmp->lic_flag = DOWN_LINK_IF_TYPE;
		WID_IF = tmp;
		WID_IF->ifi_next = tmp2;	
		Set_Interface_binding_Info(ifi_tmp->ifi_name,1);//fengwenchao copy from 1318 for AXSSZFI-839
		Add_Listenning_IF(ifname);
		//printf("222222222tmp->ifname %s",tmp->ifi_name);
	}
/*	printf("2\n");
	printf("ifi->addr_num %d\n",ifi->addr_num);
	Check_Current_Interface_and_delete(ifname,ifi);
	printf("ifi->addr_num %d\n",ifi->addr_num);
	if(ifi->check_brdaddr < 2){
		ret = Bind_BroadAddr_For_WID(ifi,CW_CONTROL_PORT);		
		ret = Bind_BroadAddr_For_WID(ifi,CW_CONTROL_PORT_AU);
	}*/
	for(i=0;i<ifi_tmp->addr_num;i++)
	{		
		if(ifi_tmp->addr[i] == 0)
			continue;
		memcpy(&((struct sockaddr_in *) ifi_tmp->ifi_addr)->sin_addr,&(ifi_tmp->addr[i]),sizeof(struct in_addr));
		{
			wid_syslog_debug_debug(WID_DBUS,"%s,%d,CW_CONTROL_PORT.\n",__func__,__LINE__);
			ret = Bind_Interface_For_WID(ifi_tmp,CW_CONTROL_PORT,DOWN_LINK_IF_TYPE);
			if(ret == 1){
				ret = 0;
				continue;
			}
			//ret = Bind_Interface_For_WID(ifi_tmp,CW_CONTROL_PORT_AU);
			ip = ((struct sockaddr_in *)(ifi_tmp->ifi_addr))->sin_addr.s_addr;
			WIDWsm_VRRPIFOp((unsigned char*)ifi_tmp->ifi_name,ip,VRRP_REG_IF);
			
			if(gNetworkPreferredFamily == CW_IPv6)
			{
				///retv6 = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
				//retv6 = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			}
			Check_WLAN_WTP_IF_Index(ifi_tmp,ifname);
		}

	}
	
	gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
    gInterfacesCountIpv4 = CWNetworkCountInterfaceAddressesIpv4(&gACSocket);
	gInterfacesCountIpv6 = CWNetworkCountInterfaceAddressesIpv6(&gACSocket);
	//printf("gInterfacesCount %d\n",gInterfacesCount);

	if(ifi_tmp->ifi_addr != NULL){
		free(ifi_tmp->ifi_addr);
		ifi_tmp->ifi_addr = NULL;
	}		
	if(ifi_tmp->ifi_brdaddr != NULL){
		free(ifi_tmp->ifi_brdaddr);
		ifi_tmp->ifi_brdaddr = NULL;
	}
	free(ifi_tmp);
	ifi_tmp = NULL;
	if((ret == 0)&&(retv6 == BINDING_IPV6_ADDRE_RROR))
	{
		return retv6;
	}
	return ret;
}

int Check_And_Bind_Ipaddr_For_WID(unsigned int ipaddr,LISTEN_FLAG flag){
	int ret = 0;
	int retv6 = 0;
	int i = 0;
	unsigned int ip;
	char ifname[IFI_NAME];
	memset(ifname,0,IFI_NAME);
	struct ifi * tmp;
	struct ifi * tmp2;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi_tmp,0,sizeof(ifi_tmp));
	memset(ifi_tmp->ifi_name,0,sizeof(ifi_tmp->ifi_name));
	ifi_tmp->addr[0] = ipaddr;
	ifi_tmp->addr_num = 1;
	ret = Get_Ipaddr_Info(ifi_tmp);
	wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
	strncpy(ifname,ifi_tmp->ifi_name,sizeof(ifi_tmp->ifi_name));
	if(ret != NO_IF_HAS_THIS_IP){
		
		if(!is_local_board_interface(ifname)){
			if(ifi_tmp){
				free(ifi_tmp);
				ifi_tmp= NULL;
			}
			return WID_INTERFACE_NOT_BE_LOCAL_BOARD;	
		}
		//struct listen_if_info *pnode = NULL;
		if(ret != 0){		
			Delete_Interface(ifname, ifi_tmp->ifi_index);
			if(ifi_tmp->ifi_addr != NULL){
				free(ifi_tmp->ifi_addr);
				ifi_tmp->ifi_addr = NULL;
			}		
			if(ifi_tmp->ifi_brdaddr != NULL){
				free(ifi_tmp->ifi_brdaddr);
				ifi_tmp->ifi_brdaddr = NULL;
			}
			free(ifi_tmp);
			ifi_tmp = NULL;
			return ret;		
		}	
	}else{
		ifi_tmp->addr_num = 0;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
	if(WID_IF == NULL){
		tmp = (struct ifi*)malloc(sizeof(struct ifi));
		memset(tmp,0,sizeof(struct ifi));
		memset(tmp->ifi_name,0,IFI_NAME);
		memcpy(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME);
		tmp->ifi_next = NULL;
		tmp->addr = ifi_tmp->addr[0];
		tmp->lic_flag = flag;
		WID_IF = tmp;
		wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
		Add_Listenning_IP(ifname,tmp->addr,flag);
		//printf("1111111tmp->ifname %s",tmp->ifi_name);
	}else{
		tmp = WID_IF;
		while(tmp != NULL){
			//if(memcmp(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME)==0)
			if((tmp->addr == ifi_tmp->addr[0])&&(tmp->lic_flag == flag))
			{
				if(ifi_tmp->ifi_addr != NULL){
					free(ifi_tmp->ifi_addr);
					ifi_tmp->ifi_addr = NULL;
				}		
				if(ifi_tmp->ifi_brdaddr != NULL){
					free(ifi_tmp->ifi_brdaddr);
					ifi_tmp->ifi_brdaddr = NULL;
				}
				free(ifi_tmp);
				ifi_tmp = NULL;
				wid_syslog_debug_debug(WID_DEFAULT,"interface has been in WID_IF\n");
				return 0;
			}
			tmp = tmp->ifi_next;
		}
		tmp = (struct ifi*)malloc(sizeof(struct ifi));
		memset(tmp,0,sizeof(struct ifi));
		memset(tmp->ifi_name,0,IFI_NAME);
		memcpy(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME);
		tmp->addr = ifi_tmp->addr[0];
		tmp->lic_flag = flag;
		tmp->ifi_next = NULL;
		tmp2 = WID_IF;
		WID_IF = tmp;
		WID_IF->ifi_next = tmp2;		
		wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
		Add_Listenning_IP(ifname,tmp->addr,flag);
	}
	wid_syslog_debug_debug(WID_DEFAULT,"%s,%d.",__func__,__LINE__);
		
	for(i=0;i<ifi_tmp->addr_num;i++)
	{		
		if(ifi_tmp->addr[i] == 0)
			continue;
		memcpy(&((struct sockaddr_in *) ifi_tmp->ifi_addr)->sin_addr,&(ifi_tmp->addr[i]),sizeof(struct in_addr));
		//if(Lic_ip.lic_active_ac_ip == ((struct sockaddr_in *) ifi_tmp->ifi_addr)->sin_addr.s_addr)
		if(flag == LIC_TYPE){
			ret = Bind_Interface_For_WID(ifi_tmp,WID_LIC_AC_PORT,LIC_TYPE);	 
			wid_syslog_debug_debug(WID_DBUS,"%s,%d,WID_LIC_AC_PORT.\n",__func__,__LINE__);
		}else{
			wid_syslog_debug_debug(WID_DBUS,"%s,%d,CW_CONTROL_PORT.\n",__func__,__LINE__);
			ret = Bind_Interface_For_WID(ifi_tmp,CW_CONTROL_PORT,DOWN_LINK_IP_TYPE);		
			if(ret == 1){
				ret = 0;
				continue;
			}			//ret = Bind_Interface_For_WID(ifi_tmp,CW_CONTROL_PORT_AU);
			ip = ((struct sockaddr_in *)(ifi_tmp->ifi_addr))->sin_addr.s_addr;
			WIDWsm_VRRPIFOp((unsigned char*)ifi_tmp->ifi_name,ip,VRRP_REG_IF);
			
			if(gNetworkPreferredFamily == CW_IPv6)
			{
				///retv6 = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
				//retv6 = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			}
			Check_WLAN_WTP_IF_Index(ifi_tmp,ifname);
		}

	}
	
	gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
    gInterfacesCountIpv4 = CWNetworkCountInterfaceAddressesIpv4(&gACSocket);
	gInterfacesCountIpv6 = CWNetworkCountInterfaceAddressesIpv6(&gACSocket);
	//printf("gInterfacesCount %d\n",gInterfacesCount);

	if(ifi_tmp->ifi_addr != NULL){
		free(ifi_tmp->ifi_addr);
		ifi_tmp->ifi_addr = NULL;
	}		
	if(ifi_tmp->ifi_brdaddr != NULL){
		free(ifi_tmp->ifi_brdaddr);
		ifi_tmp->ifi_brdaddr = NULL;
	}
	free(ifi_tmp);
	ifi_tmp = NULL;
	if((ret == 0)&&(retv6 == BINDING_IPV6_ADDRE_RROR))
	{
		return retv6;
	}
	return ret;
}


//added end
int WID_ADD_IF_APPLY_WLAN(unsigned char WlanID, char * ifname){
	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}	
	int sockfd;
	struct ifreq	ifr;
	struct ifi *wif;
	struct ifi *wifnext;
	int ret = Check_And_Bind_Interface_For_WID(ifname);
	if(ret != 0)
		return ret;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));
	
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	close(sockfd);
	wif = (struct ifi*)malloc(sizeof(struct ifi));
	memset(wif->ifi_name,0,ETH_IF_NAME_LEN);
	memcpy(wif->ifi_name,ifname,strlen(ifname));
	wif->ifi_index = ifr.ifr_ifindex;
	wif->nas_id_len = 0;//zhanglei add
	memset(wif->nas_id,0,NAS_IDENTIFIER_NAME);//zhanglei add
	wif->ifi_next = NULL;
	
	if(AC_WLAN[WlanID]->Wlan_Ifi == NULL){
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		AC_WLAN[WlanID]->Wlan_Ifi = wif ;
		AC_WLAN[WlanID]->Wlan_Ifi->ifi_next = NULL;
	}else{

		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext != NULL)
		{	
			if(wifnext->ifi_index == wif->ifi_index)
			{
				//printf("warnning you have binding this wlan eth ,please do not binding this again");
				free(wif);//zhanglei add
				wif = NULL;//zhanglei add
				return 0;
			}
			wifnext = wifnext->ifi_next;
		}
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext->ifi_next!= NULL)
		{	
			wifnext = wifnext->ifi_next;
		}
		
		wifnext->ifi_next= wif;
		//wifnext->ifi_next = NULL;
	}
	AC_WLAN[WlanID]->ifcount++;
/*
	//add wlan to the auto ap login if list
	if((g_auto_ap_login.ifnum != 0)||(g_auto_ap_login.auto_ap_if != NULL))
	{
		int i = 0;
		int result = 0;
		wid_auto_ap_if *iflist = NULL;
		iflist = g_auto_ap_login.auto_ap_if;
		while(iflist != NULL)
		{
			if(iflist->ifindex == ifr.ifr_ifindex)
			{
				if(iflist->wlannum >= L_BSS_NUM)
				{
					//printf("interface %s has already binded to %d wlan\n",ifname,L_BSS_NUM);
					break;
				}
				else
				{
					for(i=0;i<L_BSS_NUM;i++)
					{
						if(iflist->wlanid[i] == WlanID)
						{
							//printf("wlan %d has already in the list\n",WlanID);
							result = 1;
							break;
						}
					}
					if(result != 1)
					{
						for(i=0;i<L_BSS_NUM;i++)
						{
							if(iflist->wlanid[i] == 0)
							{
								iflist->wlanid[i] = WlanID;
								iflist->wlannum++;
								result = 1;
								//printf("add wlan %d at i %d\n",WlanID,i);
								break;
							}
						}
					}
				}
			}
			iflist = iflist->ifnext;		
		}
		if(result == 0)
		{
			//printf("interface %s is not in the auto ap login list\n",ifname);
		}
	}
*/	
	return 0;
	
}


int WID_ADD_IF_APPLY_WLAN_ipv6(unsigned char WlanID, char * ifname){
	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}	
	
	struct ifreq	ifr;
	struct ifi *wif;
	struct ifi *wifnext;	
	struct ifi * tmp;
	struct ifi * tmp2;
	int i = 0;
	int ret;
	int isystemindex;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi_tmp->ifi_name,0,sizeof(ifi_tmp->ifi_name));
	strncpy(ifi_tmp->ifi_name,ifname,sizeof(ifi_tmp->ifi_name));

	struct tag_ipv6_addr_list *ipv6list = (struct tag_ipv6_addr_list *)malloc(sizeof(struct tag_ipv6_addr_list));
	if(ipv6list == NULL)
	{
		free(ifi_tmp);
		ifi_tmp = NULL;
		return BINDING_IPV6_ADDRE_RROR;
	}
	ipv6list->ifindex = 0;
	ipv6list->ipv6list = NULL;
	ipv6list->ipv6num = 0;
	
	ret = get_if_addr_ipv6_list(ifname, ipv6list);
	if(ret != 0)
	{
		if(istryreadipv6addr == 0)
		{
			for(i=0;i<READ_IFNET_INFO_COUNT;i++)
			{
				struct timeval tval;
				tval.tv_sec = 0;
				tval.tv_usec = 100000;
				select(0,NULL,NULL,NULL,&tval); 
				
				ret = get_if_addr_ipv6_list(ifname, ipv6list);
				wid_syslog_debug_debug(WID_DEFAULT,"get ipv6 retry count %d",i);

				if(ret == 0)
				{	
					istryreadipv6addr = 1;
					break;
				}
				istryreadipv6addr = 1;
			}
		}
		if(ret != 0)
		{
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			ret = BINDING_IPV6_ADDRE_RROR;
			return ret;
		}
	}
	struct tag_ipv6_addr *ipv6addr = ipv6list->ipv6list;
		
	ifi_tmp->ifi_addr6 = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
	isystemindex = ipv6list->ifindex;
	//display_ipv6_addr_list(ipv6list);
	//free_ipv6_addr_list(ipv6list);
	//display_ipv6_addr_list(ipv6list);	
	
	
	if(WID_IF_V6== NULL)
	{
		for(i=0; i<ipv6list->ipv6num; i++)
		{
			inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
			ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
			//ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
			ipv6addr = ipv6addr->next;
		}	

		if(ret == 0)
		{
		   	tmp = (struct ifi*)malloc(sizeof(struct ifi));
			memset(tmp,0,sizeof(struct ifi));
			memcpy(tmp->ifi_name,ifname,strlen(ifname));
			
			tmp->ifi_index = isystemindex;
			printf("tmp->ifi_index = %d\n",tmp->ifi_index);
			tmp->isipv6addr = 1;
			WID_IF_V6 = tmp;
		}
		else
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return ret;
		}

		
	}
	else
	{
		tmp = WID_IF_V6;
		while(tmp != NULL)
		{
			if((strlen(ifname) == strlen(tmp->ifi_name))&&(strcmp(tmp->ifi_name,ifname)==0))
			{
				isystemindex = tmp->ifi_index;
				break;
			}
			tmp = tmp->ifi_next;
		}

		if(tmp == NULL)
		{
			for(i=0; i<ipv6list->ipv6num; i++)
			{
				inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
				ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
			//	ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
				WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
				ipv6addr = ipv6addr->next;
			}	

			if(ret == 0)
			{
				tmp = (struct ifi*)malloc(sizeof(struct ifi));
				memset(tmp,0,sizeof(struct ifi));
				memcpy(tmp->ifi_name,ifname,strlen(ifname));
				tmp->ifi_index = isystemindex;
				tmp->isipv6addr = 1;
				tmp2 = WID_IF_V6;
				WID_IF_V6 = tmp;
				WID_IF_V6->ifi_next = tmp2;	
			}
			else
			{
				free(ifi_tmp->ifi_addr6);
				ifi_tmp->ifi_addr6 = NULL;
				free(ifi_tmp);
				ifi_tmp = NULL;
				free_ipv6_addr_list(ipv6list);
				return ret;
			}
		}
	
	}	

		
	gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
	gInterfacesCountIpv4 = CWNetworkCountInterfaceAddressesIpv4(&gACSocket);
	gInterfacesCountIpv6 = CWNetworkCountInterfaceAddressesIpv6(&gACSocket);
	free(ifi_tmp->ifi_addr6);
	ifi_tmp->ifi_addr6 = NULL;
	free(ifi_tmp);
	ifi_tmp = NULL;
	free_ipv6_addr_list(ipv6list);

	if(ret != 0)
	return ret;

	ifr.ifr_ifindex = isystemindex;

	wif = (struct ifi*)malloc(sizeof(struct ifi));
	memset(wif->ifi_name,0,ETH_IF_NAME_LEN);
	memcpy(wif->ifi_name,ifname,strlen(ifname));
	wif->ifi_index = ifr.ifr_ifindex;
	printf("wif->ifi_index = %d\n",wif->ifi_index);
	wif->nas_id_len = 0;//zhanglei add
	memset(wif->nas_id,0,NAS_IDENTIFIER_NAME);//zhanglei add
	wif->ifi_next = NULL;
	wif->isipv6addr = 1;
	
	if(AC_WLAN[WlanID]->Wlan_Ifi == NULL){
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		AC_WLAN[WlanID]->Wlan_Ifi = wif ;
		AC_WLAN[WlanID]->Wlan_Ifi->ifi_next = NULL;
	}else{

		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext != NULL)
		{	
			if(wifnext->ifi_index == wif->ifi_index)
			{
				//printf("warnning you have binding this wlan eth ,please do not binding this again");
				free(wif);//zhanglei add
				wif = NULL;//zhanglei add
				return 0;
			}
			wifnext = wifnext->ifi_next;
		}
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext->ifi_next!= NULL)
		{	
			wifnext = wifnext->ifi_next;
		}
		
		wifnext->ifi_next= wif;
		//wifnext->ifi_next = NULL;
	}
	AC_WLAN[WlanID]->ifcount++;

	//add wlan to the auto ap login if list
	if((g_auto_ap_login.ifnum != 0)||(g_auto_ap_login.auto_ap_if != NULL))
	{
		int i = 0;
		int result = 0;
		wid_auto_ap_if *iflist = NULL;
		iflist = g_auto_ap_login.auto_ap_if;
		while(iflist != NULL)
		{
			if(iflist->ifindex == ifr.ifr_ifindex)
			{
				if(iflist->wlannum >= L_BSS_NUM)
				{
					//printf("interface %s has already binded to %d wlan\n",ifname,L_BSS_NUM);
					break;
				}
				else
				{
					for(i=0;i<L_BSS_NUM;i++)
					{
						if(iflist->wlanid[i] == WlanID)
						{
							//printf("wlan %d has already in the list\n",WlanID);
							result = 1;
							break;
						}
					}
					if(result != 1)
					{
						for(i=0;i<L_BSS_NUM;i++)
						{
							if(iflist->wlanid[i] == 0)
							{
								iflist->wlanid[i] = WlanID;
								iflist->wlannum++;
								result = 1;
								//printf("add wlan %d at i %d\n",WlanID,i);
								break;
							}
						}
					}
				}
			}
			iflist = iflist->ifnext;		
		}
		if(result == 0)
		{
			//printf("interface %s is not in the auto ap login list\n",ifname);
		}
	}
	return 0;
	
}

int WID_DELETE_IF_APPLY_WLAN(unsigned char WlanID, char * ifname)
{

	struct ifi *wif;
	struct ifi *wifnext;
	
	struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
	//printf("**!!!!!!!!!!!! list start !!!!!!!!!!!!!!!***\n");
	while(wlan_ifi != NULL)
	{
		//printf("**!!!!!!!!!!!! list start !!!!!!!!!!!!!!!***\n");
		//printf("**index is:%d name is:%s ***\n",wlan_ifi->ifi_index,wlan_ifi->ifi_name);
		wlan_ifi = wlan_ifi->ifi_next;
		//printf("**!!!!!!!!!!!! list end !!!!!!!!!!!!!!!***\n");
		
	}
	//printf("**!!!!!!!!!!!! list end !!!!!!!!!!!!!!!***\n");
	//added end
	wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
	
	if(AC_WLAN[WlanID]->Wlan_Ifi == NULL)
	{
		return WLAN_NOT_BINDING_IF;
	}
	//else if(wifnext->ifi_index == systemIndex)
	else if(strcmp(wifnext->ifi_name,ifname) == 0)
	{
		AC_WLAN[WlanID]->Wlan_Ifi = wifnext->ifi_next;
		free(wifnext);
		wifnext = NULL;		
		AC_WLAN[WlanID]->ifcount--;
		//delete wtp binding relationship
		wid_check_wtp_apply_wlan(WlanID,ifname);
		return 0;
			
	}
	else
	{
		while(wifnext->ifi_next != NULL)
		{	
			
			//if(wifnext->ifi_next->ifi_index == systemIndex)
			if(strcmp(wifnext->ifi_next->ifi_name,ifname) == 0)
			{
				wif = wifnext->ifi_next;
				wifnext->ifi_next = wifnext->ifi_next->ifi_next;
				free(wif);
				wif = NULL;				
				AC_WLAN[WlanID]->ifcount--;
				//delete wtp binding relationship
				wid_check_wtp_apply_wlan(WlanID,ifname);
				return 0;
			}
			wifnext = wifnext->ifi_next;
		}
	}
	
	return WLAN_NOT_BINDING_IF;

}

int WID_WLAN_HIDE_ESSID(unsigned char WlanID, unsigned char Hideessid)
{

	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}

	AC_WLAN[WlanID]->HideESSid = Hideessid;

	return 0;
}
int WID_WLAN_L3IF_POLICY(unsigned char WlanID, unsigned char wlanPolicy)
{
	wid_syslog_debug_debug(WID_DEFAULT,"way test 0000 current:%d past:%d \n",wlanPolicy,AC_WLAN[WlanID]->wlan_if_policy);
	int ret = -1;
	int i=0;
	//int j=0;
	
	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}
	//check wlan vlan policy state
	if((wlanPolicy != 0)&&(AC_WLAN[WlanID]->vlanid != 0))
	{
		return WLAN_BINDING_VLAN;
	}
	if(AC_WLAN[WlanID]->wlan_if_policy == wlanPolicy)
	{
		return 0;
	}
	else if((AC_WLAN[WlanID]->wlan_if_policy == NO_INTERFACE)&&(wlanPolicy == WLAN_INTERFACE))
	{		
		ret = Create_Wlan_L3_Interface(WlanID);

		if(ret < 0)
		{
			return WLAN_CREATE_L3_INTERFACE_FAIL;
		}
		//search all AC_BSS to keep bss policy the same with wlan policy
		for(i=0; i<BSS_NUM; i++)
		{
			if(AC_BSS[i] != NULL)
			{
				if(AC_BSS[i]->WlanID == WlanID)
				{
					if(AC_BSS[i]->BSS_IF_POLICY == AC_WLAN[WlanID]->wlan_if_policy)
					{
						AC_BSS[i]->BSS_IF_POLICY = wlanPolicy;
					}
				}
			}
		}
/*		for(i=0; i<WTP_NUM; i++)
		{
			if(AC_WTP[i] != NULL)
				for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				{
					if((AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)&&(AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]] != NULL))
					{						
						AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]]->BSS_IF_POLICY = wlanPolicy;
					}else if(AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]] == NULL)
						AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] = 0;
				}
		}*/
		
	/*	for(i=0;i<WTP_NUM;i++)
			if(AC_WTP[i] != NULL){
				for(j=0;j<AC_WTP[i]->RadioCount;j++){
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0){
						WID_DELETE_WLAN_APPLY_WTP(i,WlanID);
						break;
					}
				}
			}*///now allow split&local modes both work , so close this
		
	}
	else if((AC_WLAN[WlanID]->wlan_if_policy == NO_INTERFACE)&&(wlanPolicy == BSS_INTERFACE))
	{	
		/*for(i=0; i<WTP_NUM; i++)
		{
			if(AC_WTP[i] != NULL)
				for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				{
					if((AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)&&(AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]] != NULL))
					{
						ret = Create_BSS_L3_Interface(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]);
						
						if(ret < 0)
						{
							return WLAN_CREATE_L3_INTERFACE_FAIL;
						}
						
						AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]]->BSS_IF_POLICY = wlanPolicy;
					}else if(AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]] == NULL)
						AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] = 0;
				}
		}*/		
	/*	for(i=0;i<WTP_NUM;i++)
			if(AC_WTP[i] != NULL){
				for(j=0;j<AC_WTP[i]->RadioCount;j++){
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0){
						WID_DELETE_WLAN_APPLY_WTP(i,WlanID);
						break;
					}
				}
			}*/
	}
	else if((AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)&&(wlanPolicy == NO_INTERFACE))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"way test 0001 ");
		ret = Delete_Wlan_L3_Interface(WlanID);	
		if(ret < 0)
		{
			return WLAN_DELETE_L3_INTERFACE_FAIL;
		}
		//search all AC_BSS to keep bss policy the same with wlan policy
		for(i=0; i<BSS_NUM; i++)
		{
			if(AC_BSS[i] != NULL)
			{
				if(AC_BSS[i]->WlanID == WlanID)
				{
					if(AC_BSS[i]->BSS_IF_POLICY == AC_WLAN[WlanID]->wlan_if_policy)
					{
						AC_BSS[i]->BSS_IF_POLICY = wlanPolicy;
					}
				}
			}
		}
		/*for(i=0; i<WTP_NUM; i++)
		{
			if(AC_WTP[i] != NULL)
				for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				{
					if((AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)&&(AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]] != NULL))
					{						
						AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]]->BSS_IF_POLICY = wlanPolicy;
					}else if(AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]] == NULL)
						AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] = 0;
				}
		}*/		
/*		for(i=0;i<WTP_NUM;i++)
			if(AC_WTP[i] != NULL){
				for(j=0;j<AC_WTP[i]->RadioCount;j++){
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0){
						WID_DELETE_WLAN_APPLY_WTP(i,WlanID);
						break;
					}
				}
			}*/
	}
	else if((AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)&&(wlanPolicy == BSS_INTERFACE))
	{
		ret = Delete_Wlan_L3_Interface(WlanID);
		AC_WLAN[WlanID]->wlan_if_policy = NO_INTERFACE;

	/*	for(i=0; i<WTP_NUM; i++)
		{
			if(AC_WTP[i] != NULL)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				for(j=0; j<AC_WTP[i]->RadioCount; j++)
				{
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						ret = Create_BSS_L3_Interface(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]);
						
						if(ret < 0)
						{
							return WLAN_CREATE_L3_INTERFACE_FAIL;
						}
						
						AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]]->BSS_IF_POLICY = wlanPolicy;
					}
				}
		}*/
		
/*		for(i=0;i<WTP_NUM;i++)
			if(AC_WTP[i] != NULL){
				for(j=0;j<AC_WTP[i]->RadioCount;j++){
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0){
						WID_DELETE_WLAN_APPLY_WTP(i,WlanID);
						break;
					}
				}
			}*/
		
	}
	else if((AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE)&&(wlanPolicy == NO_INTERFACE))
	{
	/*	for(i=0; i<WTP_NUM; i++)
		{
			if(AC_WTP[i] != NULL)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				for(j=0; j<AC_WTP[i]->RadioCount; j++)
				{
					
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						
						//ret = Delete_BSS_L3_Interface(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]);
						//AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]]->BSS_IF_POLICY = wlanPolicy;		
						WID_DELETE_WLAN_APPLY_WTP(i,WlanID);
					}
				}
		}*/
		
	}
	else if((AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE)&&(wlanPolicy == WLAN_INTERFACE))
	{
	/*	for(i=0; i<WTP_NUM; i++)
		{			
			if(AC_WTP[i] != NULL)
				for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				{
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						//ret = Delete_BSS_L3_Interface(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]);
						//AC_BSS[AC_WLAN[WlanID]->S_WTP_BSS_List[i][j]]->BSS_IF_POLICY = wlanPolicy;		
						WID_DELETE_WLAN_APPLY_WTP(i,WlanID);
					}
				}
		}*/

		AC_WLAN[WlanID]->wlan_if_policy = NO_INTERFACE;

		ret = Create_Wlan_L3_Interface(WlanID);
		if(ret < 0)
		{
			return WLAN_CREATE_L3_INTERFACE_FAIL;
		}
	}

	wid_syslog_debug_debug(WID_DEFAULT,"*** wlan l3 interface policy is:%d***\n",wlanPolicy);
	
	//through ioctl create l3 interface
	
	AC_WLAN[WlanID]->wlan_if_policy = wlanPolicy;	

	return 0;
}
int WID_WLAN_L3IF_POLICY_BR(unsigned char WlanID, unsigned char wlanPolicy)
{
	wid_syslog_debug_debug(WID_DEFAULT,"WID_WLAN_L3IF_POLICY_BR policy current:%d past:%d \n",wlanPolicy,AC_WLAN[WlanID]->wlan_if_policy);
	int ret = -1;
	int i=0;
	int j=0;
	int reason = 0;
	if(AC_WLAN[WlanID]->wlan_if_policy == wlanPolicy)
	{
		return UNKNOWN_ERROR;  //fengwenchao modify "0" to "UNKNOWN_ERROR" for AXSSZFI-1587
	}
	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}
	if((wlanPolicy == WLAN_INTERFACE))
	{		
		wid_syslog_debug_debug(WID_DEFAULT,"from no to wlan\n");
		ret = Create_Wlan_L3_BR_Interface(WlanID);
		if(ret < 0)
		{
			return WLAN_CREATE_BR_FAIL;
		}
		if(AC_WLAN[WlanID]->wlan_if_policy == NO_INTERFACE)
			set_wlan_tunnel_mode(WlanID, 1);
		//search all AC_BSS to keep bss policy the same with wlan policy
		for(i=0; i<WTP_NUM; i++)
		{
			//if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))
			if(AC_WTP[i]!=NULL)
			{
				for(j=0; j<AC_WTP[i]->RadioCount; j++)
				{
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
						if(!check_bssid_func(bssindex)){
							wid_syslog_err("\n");
							//return ;							
						}else{						
							if(AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE)
							{
								ret = Create_BSS_L3_Interface(bssindex);
								if(ret < 0)
								{
									return BSS_CREATE_L3_INTERFACE_FAIL;
								}
								AC_BSS[bssindex]->BSS_IF_POLICY = BSS_INTERFACE;
								ret = ADD_BSS_L3_Interface_BR(bssindex);
								if(ret < 0)
								{
									return BSS_L3_INTERFACE_ADD_BR_FAIL;
								}
								AC_BSS[bssindex]->BSS_IF_POLICY = WLAN_INTERFACE;
								AC_BSS[bssindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
							}
							else if(AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE)
							{
								wid_syslog_debug_debug(WID_DEFAULT,"bssindex %d already bss l3 interface\n",bssindex);
								ret = ADD_BSS_L3_Interface_BR(bssindex);
								if(ret < 0)
								{
									return BSS_L3_INTERFACE_ADD_BR_FAIL;
								}
								if(WID_ADD_RADIO_IF_FAIL == ret){

								}else{
									AC_BSS[bssindex]->BSS_IF_POLICY = WLAN_INTERFACE;
									AC_BSS[bssindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
								}
							}
							else
							{
							}
						}
					}
				}
			}
		}
		char brcmd[WID_SYSTEM_CMD_LENTH];
		memset(brcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(brcmd,"ifconfig wlanl%d-%d-%d up\n",slotid,vrrid,WlanID);	
		else
			sprintf(brcmd,"ifconfig wlan%d-%d-%d up\n",slotid,vrrid,WlanID);	
			
//		printf("system cmd: %s\n",brcmd);
		ret = system(brcmd);
		
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
			return SYSTEM_CMD_ERROR;
		}
		if(ret != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error\n");
			return SYSTEM_CMD_ERROR;
		}
	}
	else if((AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)&&(wlanPolicy == NO_INTERFACE))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"from wlan to no");
		
		if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)
		ret = set_wlan_tunnel_mode(WlanID, 0);
		if(ret != 0)
		{
			return ret;
		}
		ret = Delete_Wlan_L3_BR_Interface(WlanID);	
		if(ret < 0)
		{
			return WLAN_DELETE_BR_FAIL;
		}
		AC_WLAN[WlanID]->isolation_policy = 1;
		AC_WLAN[WlanID]->multicast_isolation_policy = 1;
		AC_WLAN[WlanID]->sameportswitch = 0;
	}
	
	wid_syslog_debug_debug(WID_DEFAULT,"*** wlan l3 interface policy is:%d***\n",wlanPolicy);
	
	//through ioctl create l3 interface
	
	AC_WLAN[WlanID]->wlan_if_policy = wlanPolicy;	
	if(AC_WLAN[WlanID]->wlan_if_policy != NO_INTERFACE){
		if(AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY != g_WLAN_TUNNEL_POLICY){
			char nodeFlag = 2;
			ret = WID_RADIO_WLAN_TUNNEL_MODE(WlanID,g_WLAN_TUNNEL_POLICY,nodeFlag);
		}
	}else if(wlanPolicy == NO_INTERFACE){
		char nodeFlag = 2;
		ret = WID_RADIO_WLAN_TUNNEL_MODE(WlanID,CW_802_DOT_11_TUNNEL,nodeFlag);
	}
	return 0;
}

//bss l3 interface area
int WID_BSS_L3IF_POLICY(unsigned int WlanID,unsigned int wtpID,unsigned int radioID,unsigned char BSSID,unsigned char bssPolicy)
{
	//parse interface radio1-0.1&no interface radio1-0.1
//	printf("%d %d %d %d %d\n",WlanID,wtpID,radioID,BSSID,bssPolicy);
	unsigned int BSSindex = ((wtpID*L_RADIO_NUM+radioID)*L_BSS_NUM)+BSSID;
	int ret = -1;
	int ebr_id = 0;
	char ifiname[ETH_IF_NAME_LEN-1];
	if(AC_BSS[BSSindex] == NULL)
	{
		return BSS_NOT_EXIST;
	}
	WTPQUITREASON quitreason = WTP_INIT;
	
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_BSS[BSSindex]->BSS_IF_POLICY);
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);
	wid_syslog_debug_debug(WID_DEFAULT,"BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);

	if(AC_BSS[BSSindex]->BSS_IF_POLICY == bssPolicy)
	{
		return 0;
	}
	else
	{
		if(AC_BSS[BSSindex]->State != 0)
		{
			return BSS_BE_ENABLE;
		}
	}

	if((AC_WLAN[WlanID]->wlan_if_policy == 0) || (AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE))//no_interface,bss interface can be no_interface or bss_interface
	{	//printf("state 0 \n");
		if(bssPolicy == 1)//no wlan_interface now
		{
			return IF_POLICY_CONFLICT;
		}
		//if(AC_BSS[BSSindex]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		if(AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		{
			return 0;
		}
		//no_interface to bss_interface
		if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == NO_INTERFACE)&&(bssPolicy == BSS_INTERFACE))
		{
//			printf("state 0 f 0 t b\n");
			ret = Create_BSS_L3_Interface(BSSindex);

			if(ret < 0)
			{
				return BSS_CREATE_L3_INTERFACE_FAIL;
			}
		}
		//bss_interface to no_interface
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == BSS_INTERFACE)&&(bssPolicy == NO_INTERFACE))
		{	//printf("state 0 f b t 0\n");
			if(check_whether_in_ebr(vrrid,wtpID,radioID,WlanID,&ebr_id))
			{
				return RADIO_IN_EBR;
			}
			ret = Delete_BSS_L3_Interface(BSSindex);	

			if(ret < 0)
			{
				return BSS_DELETE_L3_INTERFACE_FAIL;
			}
		}
		//wlan_interface to bss_interface //use interface radio1-0.1
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == WLAN_INTERFACE)&&(bssPolicy == BSS_INTERFACE))
		{//	printf("state 0 f w t b\n");
			//whether to delete the wlan l3 interface???
			/*ret = Delete_Wlan_L3_Interface(WlanID);	
		
			if(ret < 0)
			{
				return WLAN_DELETE_L3_INTERFACE_FAIL;
			}*/
			ret = Create_BSS_L3_Interface(BSSindex);	
			
			if(ret < 0)
			{
				return BSS_CREATE_L3_INTERFACE_FAIL;
			}
		}
		//bss_interface to wlan_interface //use no interface radio1-0.1
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == BSS_INTERFACE)&&(bssPolicy == WLAN_INTERFACE))
		{	//printf("state 0 f b t w\n");
			//first check wlan interface
			memset(ifiname,0,ETH_IF_NAME_LEN-1);
		//	printf("wlan%d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			if(local)
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlanl%d-%d-%d",slotid,vrrid,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			else
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlan%d-%d-%d",slotid,vrrid,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
				
			ret = Check_Interface_Config(ifiname,&quitreason);
			
			if(ret != 0)
			{
				return L3_INTERFACE_ERROR;
			}

			else
			{
				
			
				//delete bss l3 interface
				ret = Delete_BSS_L3_Interface(BSSindex);	
			
				if(ret < 0)
				{
					return BSS_DELETE_L3_INTERFACE_FAIL;
				}
				

			}
		}
		//maybe not happen forever
		else
		{
			return UNKNOWN_ERROR;
		}
		
	}
	
	if(AC_WLAN[WlanID]->wlan_if_policy == 1)//wlan_interface,bss interface can be no_interface or bss_interface or wlan_interface
	{
	//	printf("state 1\n");
		//if(AC_BSS[BSSindex]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		if(AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		{
			return 0;
		}
		//no_interface to bss_interface
		if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == NO_INTERFACE)&&(bssPolicy == BSS_INTERFACE))
		{
	//		printf("state 0 f 0 t b\n");
			ret = Create_BSS_L3_Interface(BSSindex);

			if(ret < 0)
			{
				return BSS_CREATE_L3_INTERFACE_FAIL;
			}
		}
		
		//bss_interface to no_interface
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == BSS_INTERFACE)&&(bssPolicy == NO_INTERFACE))
		{//	printf("state 0 f b t 0\n");
			if(check_whether_in_ebr(vrrid,wtpID,radioID,WlanID,&ebr_id))
			{
				return RADIO_IN_EBR;
			}	
			ret = Delete_BSS_L3_Interface(BSSindex);	

			if(ret < 0)
			{
				return BSS_DELETE_L3_INTERFACE_FAIL;
			}
			/*bss to no , no return to the policy of wlan
			if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)
			{
				AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY = WLAN_INTERFACE;	

				return 0;
			}*/
		}
		//wlan_interface to bss_interface //use interface radio1-0.1//remove bss if from wlan br
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == WLAN_INTERFACE)&&(bssPolicy == BSS_INTERFACE))
		{//	printf("state 0 f w t b\n");
			// remove bss if from wlan br
			ret = Del_BSS_L3_Interface_BR(BSSindex);
			if(ret < 0)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"remove bss interface from wlan br failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
				return BSS_L3_INTERFACE_DEL_BR_FAIL;
			}
			
		}
		
		//from wlan to no
		else if ((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == WLAN_INTERFACE) && (bssPolicy == NO_INTERFACE))
		{
			// remove bss if from wlan br
			ret = Del_BSS_L3_Interface_BR(BSSindex);
			if(ret < 0)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"remove bss interface from wlan br failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
				return BSS_L3_INTERFACE_DEL_BR_FAIL;
			}
			AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY = BSS_INTERFACE;
			
			//deletele bss interface
			ret = Delete_BSS_L3_Interface(BSSindex);
			if(ret < 0)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"delete bss interface failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
				return BSS_DELETE_L3_INTERFACE_FAIL;
			}
			AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY = NO_INTERFACE;
			
		}
		//bss_interface to wlan_interface //use no interface radio1-0.1
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == BSS_INTERFACE)&&(bssPolicy == WLAN_INTERFACE))
		{//	printf("state 0 f b t w\n");
			//first check wlan interface
			memset(ifiname,0,ETH_IF_NAME_LEN-1);
		//	printf("wlan%d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			if(local)
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlanl%d-%d-%d",slotid,vrrid,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			else
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlan%d-%d-%d",slotid,vrrid,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
				
			ret = Check_Interface_Config(ifiname,&quitreason);
			
			if(ret != 0)
			{
				return L3_INTERFACE_ERROR;
			}

			else
			{
				
			
				//delete bss l3 interface
				ret = Delete_BSS_L3_Interface(BSSindex);	
			
				if(ret < 0)
				{
					return BSS_DELETE_L3_INTERFACE_FAIL;
				}
				

			}
		}
		//maybe not happen forever
		else
		{
			return UNKNOWN_ERROR;
		}
		
	}
	
//	printf("*** BSS l3 interface policy is:%d***\n",bssPolicy);
	wid_syslog_debug_debug(WID_DEFAULT,"*** BSS l3 interface policy is:%d***\n",bssPolicy);
	
	
	AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY = bssPolicy;	

	return 0;
}
int WID_RADIO_BSS_L3IF_POLICY(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy)
{
//	printf("%d %d %d %d %d\n",WlanID,wtpID,radioID,BSSID,bssPolicy);
	unsigned int BSSindex = ((wtpID*L_RADIO_NUM+radioID)*L_BSS_NUM)+BSSID;
	int ret = -1;
	char ifiname[ETH_IF_NAME_LEN-1];
	if(AC_BSS[BSSindex] == NULL)
	{
		return BSS_NOT_EXIST;
	}
	WTPQUITREASON quitreason = WTP_INIT;
	
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_BSS[BSSindex]->BSS_IF_POLICY);
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);
	wid_syslog_debug_debug(WID_DEFAULT,"BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);
	
	if(AC_BSS[BSSindex]->State != 0)
	{
		return BSS_BE_ENABLE;
	}
	/*
	if(AC_WLAN[WlanID]->wlan_if_policy == 0)//no_interface,bss interface can be no_interface or bss_interface
	{	printf("state 0 \n");
		if(bssPolicy == 1)//no wlan_interface now
		{
			return IF_POLICY_CONFLICT;
		}
		//if(AC_BSS[BSSindex]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		if(AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		{
			return 0;
		}
		//no_interface to bss_interface
		if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == NO_INTERFACE)&&(bssPolicy == BSS_INTERFACE))
		{
			printf("state 0 f 0 t b\n");
			ret = Create_BSS_L3_Interface(BSSindex);

			if(ret < 0)
			{
				return BSS_CREATE_L3_INTERFACE_FAIL;
			}
		}
		//bss_interface to no_interface
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == BSS_INTERFACE)&&(bssPolicy == NO_INTERFACE))
		{	printf("state 0 f b t 0\n");
			ret = Delete_BSS_L3_Interface(BSSindex);	

			if(ret < 0)
			{
				return BSS_DELETE_L3_INTERFACE_FAIL;
			}
		}
		//maybe not happen forever
		else
		{
			return UNKNOWN_ERROR;
		}
		
	}*/
	if(AC_WLAN[WlanID]->wlan_if_policy == 0)//no_interface,bss interface can be no_interface or bss_interface
	{
		return IF_POLICY_CONFLICT;
	}
	//only use when wlan is WLAN_INTERFACE
	if(AC_WLAN[WlanID]->wlan_if_policy == 1)//wlan_interface,bss interface can be no_interface or bss_interface or wlan_interface
	{
//		printf("state 1\n");
		//if(AC_BSS[BSSindex]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		if(AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == bssPolicy)//current & past is the same,no use to change
		{
			return 0;
		}
		//no_interface to bss_interface //use interface radio1-0.1
		if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == NO_INTERFACE)&&(bssPolicy == BSS_INTERFACE))
		{
//			printf("state 0 f 0 t b\n");
			ret = Create_BSS_L3_Interface(BSSindex);

			if(ret < 0)
			{
				return BSS_CREATE_L3_INTERFACE_FAIL;
			}
		}
		//no_interface to wlan_interface
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == NO_INTERFACE)&&(bssPolicy == WLAN_INTERFACE))
		{
//			printf("state 0 f 0 t w\n");
			//assemble wlan1
			memset(ifiname,0,ETH_IF_NAME_LEN-1);
//			printf("wlan%d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			if(local)
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlanl%d-%d-%d",slotid,vrrid,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			else
				snprintf(ifiname,ETH_IF_NAME_LEN-1,"wlan%d-%d-%d",slotid,vrrid,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
				
			ret = Check_Interface_Config(ifiname,&quitreason);
			
			if(ret != 0)
			{
				return L3_INTERFACE_ERROR;
			}

			else
			{
				
			}
		}
		//bss_interface to no_interface
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == BSS_INTERFACE)&&(bssPolicy == NO_INTERFACE))
		{	//printf("state 0 f b t 0\n");
			//first check wlan interface
			memset(ifiname,0,ETH_IF_NAME_LEN-1);
		//	printf("radio%d-%d.%d\n",wtpID,radioID,WlanID);
			if(local)
				snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,wtpID,radioID,WlanID);
			else
				snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpID,radioID,WlanID);
				
			ret = Check_Interface_Config(ifiname,&quitreason);
			
			if(ret == 0)
			{
				return BSS_IF_NEED_DELETE;
			}



		
			ret = Delete_BSS_L3_Interface(BSSindex);	

			if(ret < 0)
			{
				return BSS_DELETE_L3_INTERFACE_FAIL;
			}
		}
		//wlan_interface to no_interface
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == WLAN_INTERFACE)&&(bssPolicy == NO_INTERFACE))
		{
			//whether to delete the wlan l3 interface???
			//ret = Delete_Wlan_L3_Interface(WlanID);	
			printf("state 0 f w t 0\n");
			
		}
		//wlan_interface to bss_interface //use interface radio1-0.1
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == WLAN_INTERFACE)&&(bssPolicy == BSS_INTERFACE))
		{	//printf("state 0 f w t b\n");
			//whether to delete the wlan l3 interface???
			/*ret = Delete_Wlan_L3_Interface(WlanID);	
		
			if(ret < 0)
			{
				return WLAN_DELETE_L3_INTERFACE_FAIL;
			}*/
			ret = Create_BSS_L3_Interface(BSSindex);	
			
			if(ret < 0)
			{
				return BSS_CREATE_L3_INTERFACE_FAIL;
			}
		}
		//bss_interface to wlan_interface //use no interface radio1-0.1
		else if((AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY == BSS_INTERFACE)&&(bssPolicy == WLAN_INTERFACE))
		{	//printf("state 0 f b t w\n");
			return UNKNOWN_ERROR;
			//first check wlan interface
			/*
			memset(ifiname,0,ETH_IF_NAME_LEN);
			printf("wlan%d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			snprintf(ifiname,ETH_IF_NAME_LEN,"wlan%d",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->WlanID);
			ret = Check_Interface_Config(ifiname,&quitreason);
			
			if(ret != 0)
			{
				return L3_INTERFACE_ERROR;
			}

			else
			{
				
			
				//delete bss l3 interface
				ret = Delete_BSS_L3_Interface(BSSindex);	
			
				if(ret < 0)
				{
					return BSS_DELETE_L3_INTERFACE_FAIL;
				}
				

			}*/
		}
		//maybe not happen forever
		else
		{
			return UNKNOWN_ERROR;
		}
		
	}
	
	printf("*** BSS l3 interface policy is:%d***\n",bssPolicy);
	wid_syslog_debug_debug(WID_DEFAULT,"*** BSS l3 interface policy is:%d***\n",bssPolicy);
	
	
	AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY = bssPolicy;	

	return 0;
}
int WID_RADIO_BSS_L3IF_POLICY_BR(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy)
{
//	printf("WlanID %d wtpID %d radioID %d BSSID %d bssPolicy %d\n",WlanID,wtpID,radioID,BSSID,bssPolicy);
	unsigned int BSSindex = ((wtpID*L_RADIO_NUM+radioID)*L_BSS_NUM)+BSSID;
	int ret = -1;
	char ifname[ETH_IF_NAME_LEN];
	
	if(AC_BSS[BSSindex] == NULL)
	{
		return BSS_NOT_EXIST;
	}
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_BSS[BSSindex]->BSS_IF_POLICY);
	
	if(bssPolicy == AC_BSS[BSSindex]->BSS_IF_POLICY)
	{
		return WID_DBUS_SUCCESS;
	}
	WTPQUITREASON quitreason = WTP_INIT;
	
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);
	wid_syslog_debug_debug(WID_DEFAULT,"BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);
	
	if(AC_BSS[BSSindex]->State != 0)
	{
		return BSS_BE_ENABLE;
	}
	//from no to wlan
	if ((AC_BSS[BSSindex]->BSS_IF_POLICY == NO_INTERFACE) && (bssPolicy == WLAN_INTERFACE))
	{
		
		//check br
		memset(ifname,0,ETH_IF_NAME_LEN);
		if(local)
			sprintf(ifname,"wlanl%d-%d-%d",slotid,vrrid,WlanID);
		else
			sprintf(ifname,"wlan%d-%d-%d",slotid,vrrid,WlanID);			
		ret = Check_Interface_Config(ifname,&quitreason);
		if(ret != 0)
		{
			
			wid_syslog_debug_debug(WID_DEFAULT,"br not exist,need to be created, bssindex %d wlanid %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex,WlanID);
			return WLAN_CREATE_BR_FAIL;//no br , so return error
		}
		
		//create bss interface
		ret = Create_BSS_L3_Interface(BSSindex);
		if(ret < 0)
		{
			return BSS_CREATE_L3_INTERFACE_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = BSS_INTERFACE;
		// add bss if to wlan br
		ret = ADD_BSS_L3_Interface_BR(BSSindex);
		if(ret < 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"add bss interface to wlan br failed, bssindex %d wlanid %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex,WlanID);
	
			return BSS_L3_INTERFACE_ADD_BR_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = WLAN_INTERFACE;
		AC_BSS[BSSindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
	}

	
	//from wlan to no
	else if ((AC_BSS[BSSindex]->BSS_IF_POLICY == WLAN_INTERFACE) && (bssPolicy == NO_INTERFACE))
	{
		// remove bss if from wlan br
		ret = Del_BSS_L3_Interface_BR(BSSindex);
		if(ret < 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"remove bss interface from wlan br failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
			return BSS_L3_INTERFACE_DEL_BR_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = BSS_INTERFACE;
		
		//deletele bss interface
		ret = Delete_BSS_L3_Interface(BSSindex);
		if(ret < 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"delete bss interface failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
			return BSS_DELETE_L3_INTERFACE_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = NO_INTERFACE;
		
	}

	//from wlan to bss
	else if ((AC_BSS[BSSindex]->BSS_IF_POLICY == WLAN_INTERFACE) && (bssPolicy == BSS_INTERFACE))
	{
		// remove bss if from wlan br
		ret = Del_BSS_L3_Interface_BR(BSSindex);
		if(ret < 0)
		{	
			wid_syslog_debug_debug(WID_DEFAULT,"remove bss interface from wlan br failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
			return BSS_L3_INTERFACE_DEL_BR_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = BSS_INTERFACE;
		
	}

	//from bss to wlan
	else if ((AC_BSS[BSSindex]->BSS_IF_POLICY == BSS_INTERFACE) && (bssPolicy == WLAN_INTERFACE))
	{
		// check br
		memset(ifname,0,ETH_IF_NAME_LEN);
		if(local)
			sprintf(ifname,"wlanl%d-%d-%d",slotid,vrrid,WlanID);
		else
			sprintf(ifname,"wlan%d-%d-%d",slotid,vrrid,WlanID);			
		ret = Check_Interface_Config(ifname,&quitreason);
		if(ret != 0)
		{
			
			wid_syslog_debug_debug(WID_DEFAULT,"br not exist,need to be created, bssindex %d wlanid %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex,WlanID);
			return WLAN_CREATE_BR_FAIL;//no br , so return error
		}
		// add bss if to wlan br
		ret = ADD_BSS_L3_Interface_BR(BSSindex);
		if(ret < 0)
		{	
			wid_syslog_debug_debug(WID_DEFAULT,"add bss interface to wlan br failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
			return BSS_L3_INTERFACE_ADD_BR_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = WLAN_INTERFACE;
		AC_BSS[BSSindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
		
	}

	//from bss to no
	else if ((AC_BSS[BSSindex]->BSS_IF_POLICY == BSS_INTERFACE) && (bssPolicy == NO_INTERFACE))
	{
		//deletele bss interface
		ret = Delete_BSS_L3_Interface(BSSindex);
		if(ret < 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"delete bss interface failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
			return BSS_DELETE_L3_INTERFACE_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = NO_INTERFACE;
		
	}

	//from no to bss
	else if ((AC_BSS[BSSindex]->BSS_IF_POLICY == NO_INTERFACE) && (bssPolicy == BSS_INTERFACE))
	{
		//create bss interface
		ret = Create_BSS_L3_Interface(BSSindex);
		if(ret < 0)
		{
			return BSS_CREATE_L3_INTERFACE_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = BSS_INTERFACE;
		
	}
	
	else
	{
		return UNKNOWN_ERROR;
	}
	
	
//	printf("*** BSS l3 interface policy is:%d***0--wlan br 1--bss\n",bssPolicy);
	wid_syslog_debug_debug(WID_DEFAULT,"*** BSS l3 interface policy is:%d***\n",bssPolicy);
	
	
	return 0;
	
}

int WID_RADIO_BSS_FORWARD_MODE(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy)
{
//	printf("0999 WlanID %d wtpID %d radioID %d BSSID %d bssPolicy %d\n",WlanID,wtpID,radioID,BSSID,bssPolicy);
	unsigned int BSSindex = ((wtpID*L_RADIO_NUM+radioID)*L_BSS_NUM)+BSSID;
	int ret = -1;
	char ifname[ETH_IF_NAME_LEN];
	
	if(AC_BSS[BSSindex] == NULL)
	{
		return BSS_NOT_EXIST;
	}
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_BSS[BSSindex]->BSS_IF_POLICY);
	
	if(bssPolicy == AC_BSS[BSSindex]->BSS_IF_POLICY)
	{
		return WID_DBUS_SUCCESS;
	}
	
	WTPQUITREASON quitreason = WTP_INIT;
	
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);
	wid_syslog_debug_debug(WID_DEFAULT,"BSS current:%d past:%d \n",bssPolicy,AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSS_IF_POLICY);
	
	if(AC_BSS[BSSindex]->State != 0)
	{
		return BSS_BE_ENABLE;
	}

	
	if((AC_BSS[BSSindex]->BSS_IF_POLICY == BSS_INTERFACE) && (bssPolicy == WLAN_INTERFACE))
	{
		memset(ifname,0,ETH_IF_NAME_LEN);
		if(local)
			sprintf(ifname,"wlanl%d-%d-%d",slotid,vrrid,WlanID);
		else
			sprintf(ifname,"wlan%d-%d-%d",slotid,vrrid,WlanID);
			
		ret = Check_Interface_Config(ifname,&quitreason);
		if(ret != 0)
		{
			
			wid_syslog_debug_debug(WID_DEFAULT,"br not exist,need to be created, bssindex %d wlanid %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex,WlanID);
			return WLAN_CREATE_BR_FAIL;//no br , so return error
		}
		// add bss if to wlan br
		ret = ADD_BSS_L3_Interface_BR(BSSindex);
		if(ret < 0)
		{	
			wid_syslog_debug_debug(WID_DEFAULT,"add bss interface to wlan br failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
			return BSS_L3_INTERFACE_ADD_BR_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = WLAN_INTERFACE;
		AC_BSS[BSSindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
	}
	else if((AC_BSS[BSSindex]->BSS_IF_POLICY == WLAN_INTERFACE) && (bssPolicy == BSS_INTERFACE))
	{
		// remove bss if from wlan br
		ret = Del_BSS_L3_Interface_BR(BSSindex);
		if(ret < 0)
		{	
			wid_syslog_debug_debug(WID_DEFAULT,"remove bss interface from wlan br failed, bssindex %d\n",AC_WTP[wtpID]->WTP_Radio[radioID]->BSS[BSSID]->BSSIndex);
			return BSS_L3_INTERFACE_DEL_BR_FAIL;
		}
		AC_BSS[BSSindex]->BSS_IF_POLICY = BSS_INTERFACE;
	}
	else
	{
		//printf("code should not here\n");
		wid_syslog_debug_debug(WID_DEFAULT,"code should not here\n");
	}

	return 0;

}

int WID_RADIO_BSS_TUNNEL_MODE(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy,char nodeFlag)
{
//	printf("WlanID %d wtpID %d radioID %d BSSID %d bssPolicy %d\n",WlanID,wtpID,radioID,BSSID,bssPolicy);
	unsigned int BSSindex = ((wtpID*L_RADIO_NUM+radioID)*L_BSS_NUM)+BSSID;
	
	if(AC_BSS[BSSindex] == NULL)
	{
		return BSS_NOT_EXIST;
	}
	
//	printf("BSS current:%d past:%d \n",bssPolicy,AC_BSS[BSSindex]->BSS_TUNNEL_POLICY);

	
	if(bssPolicy == AC_BSS[BSSindex]->BSS_TUNNEL_POLICY)
	{
		return WID_DBUS_SUCCESS;
	}
	if(0 == nodeFlag){//interface node radio or wlan
		if(AC_BSS[BSSindex]->State != 0)
		{
			return BSS_BE_ENABLE;
		}
	}else{//config wlan or radio node
		if(AC_BSS[BSSindex]->BSS_IF_POLICY == NO_INTERFACE){
			return BSS_IF_NEED_CREATE;
		}
	}

	if((AC_BSS[BSSindex]->BSS_IF_POLICY == WLAN_INTERFACE) && (bssPolicy == CW_802_IPIP_TUNNEL))
	{
		return NO_SURPPORT_IPIP;
	}
	
	if(AC_BSS[BSSindex]->BSS_TUNNEL_POLICY != bssPolicy){
		msgq msg;
//		struct msgqlist *elem;
		memset((char*)&msg, 0, sizeof(msg));
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id match success**\n");
		msg.mqid = wtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = wtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_CHANGE_TUNNEL;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = radioID;
		
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[wtpID][radioID];
		
		if((AC_WTP[wtpID]->WTPStat == 5)){ 
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}//delete unuseful code
		/*else{
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpID, elem);
		}*/
	}
	AC_BSS[BSSindex]->BSS_TUNNEL_POLICY = bssPolicy;


	return WID_DBUS_SUCCESS;

}

int WID_RADIO_WLAN_TUNNEL_MODE(unsigned char WlanID,unsigned char Policy,char nodeFlag)
{
	wid_syslog_debug_debug(WID_DBUS,"WID_RADIO_WLAN_TUNNEL_MODE wlan:%d policy:%d \n",WlanID,Policy);
	int i=0;
	int j=0;
	if(0 == nodeFlag){//interface wlan node
		if((AC_WLAN[WlanID])&&(AC_WLAN[WlanID]->Status == 0))
		{
			return WLAN_BE_ENABLE;
		}
	}else{// config wlan node

	}
	if(Policy != CW_802_DOT_11_TUNNEL){
		if((AC_WLAN[WlanID]->wlan_if_policy != WLAN_INTERFACE)&&(AC_WLAN[WlanID]->wlan_if_policy != BSS_INTERFACE)){
			wid_syslog_err("%s WlanID %d not interface\n",__func__,WlanID);
			return INTERFACE_NOT_L3_IF;
		}
	}else{}
	for(i=0; i<WTP_NUM; i++)
	{
		if(AC_WTP[i]!=NULL)
		{
			for(j=0; j<AC_WTP[i]->RadioCount; j++)
			{
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
				{
					int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
					if(!check_bssid_func(bssindex)){
						wid_syslog_err("\n");
					}else{						
						if(AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)
						{
							AC_BSS[bssindex]->BSS_TUNNEL_POLICY = Policy;
							msgq msg;
//							struct msgqlist *elem;
							memset((char*)&msg, 0, sizeof(msg));
							wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id match success**\n");
							msg.mqid = i%THREAD_NUM+1;
							msg.mqinfo.WTPID = i;
							msg.mqinfo.type = CONTROL_TYPE;
							msg.mqinfo.subtype = WLAN_S_TYPE;
							msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_CHANGE_TUNNEL;
							msg.mqinfo.u.WlanInfo.WLANID = WlanID;
							msg.mqinfo.u.WlanInfo.Radio_L_ID = j;
							
							msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
							
							if((AC_WTP[i]->WTPStat == 5)){ 
								if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
									wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
									perror("msgsnd");
								}
							}
							//delete unuseful code
							/*else{
								elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
								if(elem == NULL){
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									perror("malloc");
									return 0;
								}
								memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
								elem->next = NULL;
								memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
								WID_INSERT_CONTROL_LIST(i, elem);
							}*/
						}
						else if(AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE)
						{
							AC_BSS[bssindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;//in this branch,bss must have been disable,so need't notice ap.
						}
					}
				}
			}
		}
	}
	AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY = Policy;
	wid_syslog_debug_debug(WID_DBUS,"WID_RADIO_WLAN_TUNNEL_MODE wlan:%d WLAN_TUNNEL_POLICY:%d \n",WlanID,AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY );
	return 0;
}

int WID_RADIO_SET_TYPE(unsigned int RadioID, unsigned int RadioType){
	AC_RADIO[RadioID]->Radio_Type = RadioType;
	return 0;
}

int WID_RADIO_SET_TXP(unsigned int RadioID, unsigned short RadioTxp,CWBool flag){

	if(AC_RADIO[RadioID]->ishighpower == 1)//high power hardware
	{
		if((RadioTxp > 27)&&(RadioTxp != 100))
		{
			return TXPOWER_OVER_TW_THREE;
		}
	}
	else
	{
		if((RadioTxp > 20)&&(RadioTxp != 100))    //fengwenchao modify 20110329
		{
			return TXPOWER_OVER_TW;
		}
	}
	

	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}
	
	if((AC_RADIO[RadioID]->Radio_TXP == RadioTxp)&&(!flag))
		return 0;
	if(!flag){
		if((RadioTxp != 0)&&(RadioTxp != 100)){
			AC_RADIO[RadioID]->txpowerautostate = 1;
		}else if(RadioTxp == 100){
			AC_RADIO[RadioID]->txpowerautostate = 0;
		}
	}
	int WTPIndex = AC_RADIO[RadioID]->WTPID;
	CWThreadMutexLock(&(gWTPs[WTPIndex].RRMThreadMutex));
	if(!flag){
		AC_RADIO[RadioID]->Radio_TXP = RadioTxp;
		if(AC_RADIO[RadioID]->Radio_TXP != 100)  //fengwenchao add 20111013
		{
		if(AC_RADIO[RadioID]->ishighpower == 1)
		{
			AC_RADIO[RadioID]->Radio_TXPOF = (27-AC_RADIO[RadioID]->Radio_TXP)/AC_RADIO[RadioID]->txpowerstep;
		}
		else
			{	
			AC_RADIO[RadioID]->Radio_TXPOF = (20-AC_RADIO[RadioID]->Radio_TXP)/AC_RADIO[RadioID]->txpowerstep;
			}
		}
	}
	else AC_RADIO[RadioID]->Radio_TXPOF = RadioTxp;//return WTP_NOT_IN_RUN_STATE;
	CWThreadMutexUnlock(&(gWTPs[WTPIndex].RRMThreadMutex));
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5){
	AC_RADIO[RadioID]->CMD |= 0x1;
	AC_WTP[WTPIndex]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
	AC_WTP[WTPIndex]->CMD->setCMD = 1;	
	CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
	if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN)){
		
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		if(flag==CW_FALSE){
		msg.mqinfo.u.RadioInfo.Radio_Op = Radio_TXP;
			}
		else{
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_TXPOF;
		}
		msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}
	CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}
//	while(state == 1)
//		sleep(1);
//	v(WidSemid);
	return 0;
}
int WID_RADIO_CHANNEL_OFFSET_CWMODE_CHECK(unsigned int RadioID, unsigned int check_channel,unsigned int max_chanenl,unsigned int min_channel){
	int ret2 = CHANNEL_CWMODE_SUCCESS;
	if((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2)){

			if(AC_RADIO[RadioID]->channel_offset == 1)
			{				
				if(check_channel > max_chanenl){
					ret2 = CHANNEL_CWMODE_HT40;
				}	
			}else if(AC_RADIO[RadioID]->channel_offset == -1){
				if(check_channel < min_channel){
					ret2 = CHANNEL_CWMODE_HT40;
				}	
			}else{
				ret2 = CHANNEL_CWMODE_SUCCESS;
			}
	
	}/*else if(AC_RADIO[RadioID]->cwmode == 0){   //fengwenchao comment 20110421
		if(AC_RADIO[RadioID]->channel_offset == 1)
		{				
			if(check_channel > max_chanenl+4){
				ret2 = CHANNEL_CWMODE_HT20;
			}	
		}else if(AC_RADIO[RadioID]->channel_offset == -1){
			if(check_channel < min_channel-4){
				ret2 = CHANNEL_CWMODE_HT20;
			}	
		}else{
			ret2 = CHANNEL_CWMODE_SUCCESS;
		}
	}*/  
	
	return ret2;
}
int WID_RADIO_SET_CHAN(unsigned int RadioID, unsigned char RadioChan){
//	printf("chan:%d\n",RadioChan);
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(((AC_RADIO[RadioID]->Radio_Type & 0x02) == 2)&&(RadioChan<=14)&&(RadioChan!=0))
	{
	 	return WTP_NO_SURPORT_CHANNEL;
	}


		
	if((AC_RADIO[RadioID]->Radio_Chan == RadioChan)&&(RadioChan != 0)){
/*		if(RadioChan == 0){
			AC_RADIO[RadioID]->auto_channel_cont= 0;			
		}
		else  */
			AC_RADIO[RadioID]->auto_channel_cont= 1;
		return 0;
	}
	msgq msg;
	unsigned char chan_past = 0;
	unsigned char chan_curr = 0;
	chan_past = AC_RADIO[RadioID]->Radio_Chan;
	chan_curr = RadioChan;
	///
	AC_RADIO[RadioID]->channelchangetime++;
	int WTPIndex = AC_RADIO[RadioID]->WTPID;
	CWThreadMutexLock(&(gWTPs[WTPIndex].RRMThreadMutex));
	AC_RADIO[RadioID]->Radio_Chan= RadioChan;	
	CWThreadMutexUnlock(&(gWTPs[WTPIndex].RRMThreadMutex));
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5){
	AC_RADIO[RadioID]->CMD |= 0x2;	
	AC_WTP[WTPIndex]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
	AC_WTP[WTPIndex]->CMD->setCMD = 1;	

	AsdWsm_WTP_Channelchange_Op(WTPIndex,AC_RADIO[RadioID]->Radio_L_ID,CHANNEL_CHANGE_INFO);

	
	CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
	if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Channel;
		msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}

		}
	CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

		
	if(gtrapflag>=4){
		wid_dbus_trap_wtp_channel_change(chan_past,chan_curr,RadioID);	
	}
	
	if(RadioChan == 0){
		AC_RADIO[RadioID]->auto_channel_cont= 0;			
	}
	else AC_RADIO[RadioID]->auto_channel_cont= 1;

	return 0;
}
/*wcl add for OSDEVTDPB-31*/
int WID_RADIO_SET_COUNTRYCODE(unsigned int RadioID)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x10;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Countrycode;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){					
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}


	return COUNTRY_CODE_SUCCESS;

}

/*end*/
/*wcl add for OSDEVTDPB-31*/

int WID_SET_COUNTRY_CODE_CHECK_CHAN(unsigned int RadioID)
{
	int max_channel;
	int min_channel;
	int change_channel = 0;
	if(AC_RADIO[RadioID] != NULL)
	{
		switch(AC_RADIO[RadioID]->Radio_country_code)
		{
			case COUNTRY_CHINA_CN : 	
									if ((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10))
									{
										max_channel = 159;
										min_channel = 7;
										if((AC_RADIO[RadioID]->Radio_Chan == 149)||(AC_RADIO[RadioID]->Radio_Chan == 153)||(AC_RADIO[RadioID]->Radio_Chan == 157)\
											||(AC_RADIO[RadioID]->Radio_Chan == 161)||(AC_RADIO[RadioID]->Radio_Chan == 165)){
											if(AC_RADIO[RadioID]->channel_offset == 1){
													if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 157;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 157;
													}
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){
																				
													}else{
														change_channel =149;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 149;
													}

												}
												
										}else{
											if(AC_RADIO[RadioID]->channel_offset == 1){
												change_channel = 157;
												WID_RADIO_SET_CHAN(RadioID,change_channel);
												AC_RADIO[RadioID]->Radio_Chan = 157;
											}else if(AC_RADIO[RadioID]->channel_offset == -1){
												change_channel = 149;
												WID_RADIO_SET_CHAN(RadioID,change_channel);
												AC_RADIO[RadioID]->Radio_Chan = 149;
											}else{
												change_channel = 153;
												wid_syslog_debug_debug(WID_DBUS,"**********before set channel********");
												WID_RADIO_SET_CHAN(RadioID,change_channel);
												wid_syslog_debug_debug(WID_DBUS,"**********after set channel********");
												AC_RADIO[RadioID]->Radio_Chan = 153;

											}
										}
										
									}else{
										max_channel = 9;
										min_channel = 5;
										if(AC_RADIO[RadioID]->channel_offset == 1){
											if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 8;														
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 8;
													}
										}else if(AC_RADIO[RadioID]->channel_offset == -1){
											if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){

													}else{
														change_channel = 8;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 8;
													}
										
										}
									}
									break;
									
			case COUNTRY_EUROPE_EU : 
									if ((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10))
										{
											max_channel = 134;
											min_channel = 7;
										if((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)\
											||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)||(AC_RADIO[RadioID]->Radio_Chan == 56)\
											||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)\
											||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)\
											||(AC_RADIO[RadioID]->Radio_Chan == 116)||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)\
											||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)\
											||(AC_RADIO[RadioID]->Radio_Chan == 140)) 
										{
											if(AC_RADIO[RadioID]->channel_offset == 1){
													if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 128;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 128;
													}
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){
																				
													}else{
														change_channel = 36;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 36;
													}

												}
												
											}else{
												if(AC_RADIO[RadioID]->channel_offset == 1){
													change_channel = 128;
													
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 128;
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													AC_RADIO[RadioID]->Radio_Chan = 36;
													WID_RADIO_SET_CHAN(RadioID,AC_RADIO[RadioID]->Radio_Chan);
												}else{
													change_channel = 108;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 108;

											}
										}
									}else{
										max_channel = 9;
										min_channel = 5;
										if(AC_RADIO[RadioID]->channel_offset == 1){
											if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 8;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 8;
													}
										}else if(AC_RADIO[RadioID]->channel_offset == -1){
											if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){

													}else{
														change_channel = 8;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 8;
													}
										
										}
									}	
									break;
																	
			case COUNTRY_USA_US : 
									if ((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)){
										max_channel = 159;
										min_channel = 7;
										if((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)\
											||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)||(AC_RADIO[RadioID]->Radio_Chan == 56)\
											||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)\
											||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)\
											||(AC_RADIO[RadioID]->Radio_Chan == 116)||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)\
											||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)\
											||(AC_RADIO[RadioID]->Radio_Chan == 140)||(AC_RADIO[RadioID]->Radio_Chan == 149)||(AC_RADIO[RadioID]->Radio_Chan == 153)\
											||(AC_RADIO[RadioID]->Radio_Chan == 157)||(AC_RADIO[RadioID]->Radio_Chan == 161)||(AC_RADIO[RadioID]->Radio_Chan == 165)) 
									{
										if(AC_RADIO[RadioID]->channel_offset == 1){
													if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 157;
														
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 157;
													}
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){
																				
													}else{
														change_channel = 36;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 36;
													}

												}
												
											}else{
												if(AC_RADIO[RadioID]->channel_offset == 1){
													change_channel = 157;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 157;
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													change_channel = 36;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 36;
													
												}else{
													change_channel = 108;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 108;

											}

											}
									}else{
										max_channel = 7;
										min_channel = 5;
										if(AC_RADIO[RadioID]->channel_offset == 1){
											if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 6;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 6;
													}
										}else if(AC_RADIO[RadioID]->channel_offset == -1){
											if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){

													}else{
														change_channel = 6;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 6;
													}
										
										}else if(AC_RADIO[RadioID]->Radio_Chan > 11){
												change_channel = 11;
												WID_RADIO_SET_CHAN(RadioID,change_channel);
												AC_RADIO[RadioID]->Radio_Chan = 11;
										}
										
									}
									break;
																	
			case COUNTRY_JAPAN_JP : 
									if ((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)){
										max_channel = 40;
										min_channel = 7;
										if((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)\
											||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)||(AC_RADIO[RadioID]->Radio_Chan == 56)\
											||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)\
											||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)\
											||(AC_RADIO[RadioID]->Radio_Chan == 116)||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)\
											||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)\
											||(AC_RADIO[RadioID]->Radio_Chan == 140)||(AC_RADIO[RadioID]->Radio_Chan == 184)||(AC_RADIO[RadioID]->Radio_Chan == 188)\
											||(AC_RADIO[RadioID]->Radio_Chan == 192)||(AC_RADIO[RadioID]->Radio_Chan == 196))
									{
										if(AC_RADIO[RadioID]->channel_offset == 1){
													if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 36;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 36;
													}
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){
																				
													}else{
														change_channel = 36;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 36;
													}

												}
												
											}else{
												if(AC_RADIO[RadioID]->channel_offset == 1){
													change_channel = 36;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 36;
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													change_channel = 36;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 36;
												}else{
													change_channel = 108;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 108;

												}
											}
									}else{
										max_channel = 10;
										min_channel = 5;
										if(AC_RADIO[RadioID]->channel_offset == 1){
											if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 6;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 6;
													}
										}else if(AC_RADIO[RadioID]->channel_offset == -1){
											if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){

													}else{
														change_channel = 6;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 6;
													}
											}
									}										
									break;
																	
			case COUNTRY_FRANCE_FR : 
									if ((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)){
										max_channel = 134;
										min_channel = 7;
										if((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)\
											||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)||(AC_RADIO[RadioID]->Radio_Chan == 56)\
											||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)\
											||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)\
											||(AC_RADIO[RadioID]->Radio_Chan == 116)||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)\
											||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)\
											||(AC_RADIO[RadioID]->Radio_Chan == 140))
										{
											if(AC_RADIO[RadioID]->channel_offset == 1){
													if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 132;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 132;
													}
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){
																				
													}else{
														change_channel = 36;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 36;
													}

												}
												
											}else{
												if(AC_RADIO[RadioID]->channel_offset == 1){
													change_channel = 132;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 132;
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													change_channel = 36;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 36;
												}else{
													change_channel = 108;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 108;

												}
											}
									}else{
										max_channel = 7;
										min_channel = 7;
										if(AC_RADIO[RadioID]->channel_offset == 1){
											if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 6;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 6;
													}
										}else if(AC_RADIO[RadioID]->channel_offset == -1){
											if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){

													}else{
														change_channel = 8;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 8;
													}
											}
									}	
									break;
																	
			case COUNTRY_SPAIN_ES : 
									if ((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)){
										max_channel = 134;
										min_channel = 7;
										if((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)\
											||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)||(AC_RADIO[RadioID]->Radio_Chan == 56)\
											||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)\
											||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)\
											||(AC_RADIO[RadioID]->Radio_Chan == 116)||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)\
											||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)\
											||(AC_RADIO[RadioID]->Radio_Chan == 140))
										{
											if(AC_RADIO[RadioID]->channel_offset == 1){
													if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 132;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 132;
													}
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){
																				
													}else{
														change_channel = 36;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 36;
													}

												}
												
											}else{
												if(AC_RADIO[RadioID]->channel_offset == 1){
													change_channel = 132;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 132;
												}else if(AC_RADIO[RadioID]->channel_offset == -1){
													change_channel = 36;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 36;
												}else{
													change_channel = 108;
													WID_RADIO_SET_CHAN(RadioID,change_channel);
													AC_RADIO[RadioID]->Radio_Chan = 108;

												}
											}
									}else{
										max_channel = 5;
										min_channel = 7;
										if(AC_RADIO[RadioID]->channel_offset == 1){
											if(AC_RADIO[RadioID]->Radio_Chan <= max_channel){

													}else{
														change_channel = 4;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 4;
													}
										}else if(AC_RADIO[RadioID]->channel_offset == -1){
											if(AC_RADIO[RadioID]->Radio_Chan >= min_channel){

													}else{
														change_channel = 6;
														WID_RADIO_SET_CHAN(RadioID,change_channel);
														AC_RADIO[RadioID]->Radio_Chan = 6;
													}
											}
									}	
									break;
		
			default : 
			break;
		}		
	}
	return 0;
}
	
/*end*/
int wid_set_country_code_a8()
	{
			int i = 0;
			int j = 0;
			char apcmd[WID_SYSTEM_CMD_LENTH];
			memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
			if(gCOUNTRYCODE == COUNTRY_USA_US){
				sprintf(apcmd,"set regdmn FCC");	
			}
			else if(gCOUNTRYCODE == COUNTRY_EUROPE_EU){
				sprintf(apcmd,"set regdmn ETSI");	
			}
			else {
				sprintf(apcmd,"set regdmn RoW");	
			}
			
			printf("set regdmn :%s",apcmd);
			wid_syslog_debug_debug(WID_DEFAULT,"set regdmn :%s",apcmd);
			for(i=0;i<WTP_NUM;i++){
				if(AC_WTP[i] != NULL){
					for(j=0;j<AC_WTP[i]->RadioCount;j++){
							if((AC_WTP[i]->WTP_Radio[j] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->REFlag != 0))
								wid_radio_set_extension_command(i,apcmd);
					}
				}
			}
		return 0;
	}


int WID_RADIO_SET_SUPPORT_RATE(unsigned int RadioID, int RadioRate[],int flag,int num)
{
	int i=0;
	int rate = 0;
	int drate[ACDBUSHANDLE_11BG_RATE_LIST_LEN] = {10,20,55,60,90,110,120,180,240,360,480,540};
	int wrate[ACDBUSHANDLE_11BG_RATE_LIST_LEN];
	msgq msg;
	
	struct Support_Rate_List *ptr = NULL;
	
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}
	if(AC_RADIO[RadioID]->Support_Rate_Count < 1)//rate list is empty
	{
		return RADIO_SUPPORT_RATE_EMPTY;
	}
	//printf("the list have %d rates\n",AC_RADIO[RadioID]->Support_Rate_Count);
	if(AC_RADIO[RadioID]->Radio_Rate == NULL)//rate list is empty
	{
		//printf("the list is empty\n");
		//return RADIO_SUPPORT_RATE_EMPTY;
	}

	//check with the radio type
	for(i=0;i<num;i++)
	{
		if((AC_RADIO[RadioID]->Radio_Type == 1)||(AC_RADIO[RadioID]->Radio_Type == 4)||(AC_RADIO[RadioID]->Radio_Type == 5)||(AC_RADIO[RadioID]->Radio_Type == 2)
			||(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 12)||(AC_RADIO[RadioID]->Radio_Type == 13)||(AC_RADIO[RadioID]->Radio_Type == 26)||(AC_RADIO[RadioID]->Radio_Type == 44))
		{
			if((RadioRate[i] == 10)||(RadioRate[i] == 20)||(RadioRate[i] == 55)||(RadioRate[i] == 110)||
			   (RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
			   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
			{
			}
			else
			{
				return WTP_NO_SURPORT_Rate;
			}
		}
		else
		{
			return WTP_NO_SURPORT_TYPE;
		}
			
		//added end
		switch(AC_RADIO[RadioID]->Radio_Type)
		{

			case 1 : if((RadioRate[i] == 10)||(RadioRate[i] == 20)||(RadioRate[i] == 55)||(RadioRate[i] == 110))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
		
					case 2 : if((RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
							
					case 4 : if((RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
		
					case 10 : if((RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
		
					case 12 : if((RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
		
					case 26 : if((RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
					
					case 44 : if((RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
		
					case 5 : if((RadioRate[i] == 10)||(RadioRate[i] == 20)||(RadioRate[i] == 55)||(RadioRate[i] == 110)||
							   (RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;
							
					case 13 : if((RadioRate[i] == 10)||(RadioRate[i] == 20)||(RadioRate[i] == 55)||(RadioRate[i] == 110)||
							   (RadioRate[i] == 60)||(RadioRate[i] == 90)||(RadioRate[i] == 120)||(RadioRate[i] == 180)||
							   (RadioRate[i] == 240)||(RadioRate[i] == 360)||(RadioRate[i] == 480)||(RadioRate[i] == 540))
							{
							
							}
							else
							{
								return WTP_NO_SURPORT_Rate;
							}
							break;

			default : return WTP_NO_SURPORT_TYPE;
					break;
			}
		
			
	}

	//process append to the flag
	switch(flag)
	{
		//flag 1 means add
		case 1 :
				destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				AC_RADIO[RadioID]->Support_Rate_Count = num;

				//AC_RADIO[RadioID]->Radio_Rate = (struct Support_Rate_List *)malloc(sizeof(struct Support_Rate_List));
				AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);//here add 0 first
				
				for(i=0;i<num;i++)
				{
					rate = RadioRate[i];
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				}
				
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				//printf("/////the list has %d element////\n",AC_RADIO[RadioID]->Support_Rate_Count);
				//display_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				
					
					break;
		//flag 2 means delete  not use now
		case 2 :
				
				for(i=0;i<num;i++)
					{
						rate = RadioRate[i];
						
						ptr = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,rate);
						if(ptr == NULL)//rate is in the list 
						{
							return RADIO_SUPPORT_RATE_NOT_EXIST;
						}
						else//rate is not in the list 
						{
							insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
							AC_RADIO[RadioID]->Radio_Rate = ptr;
							AC_RADIO[RadioID]->Support_Rate_Count--;

							//display_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
						}
					}
					break;

		//flag 3 means set max rate,first destroy the list,then create a new list append to the radio type
		case 3 :
				if(num > 1)
				{
					return RADIO_SUPPORT_MAX_RATE_NOT_ONE;
				}
				destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);

					if(AC_RADIO[RadioID]->Radio_Type == 1)
					{
						switch(RadioRate[0])
						{
							case 10 : wrate[0] = 10;
									  num = 1;
									  break;
							case 20 : wrate[0] = 10;
									  wrate[1] = 20;
									  num = 2;
									  break;
							case 55 : wrate[0] = 10;
									  wrate[1] = 20;
									  wrate[2] = 55;
									  num = 3;
									  break;
							case 110 : wrate[0] = 10;
									   wrate[1] = 20;
									   wrate[2] = 55;
									   wrate[3] = 110;
									   num = 4;
									   break;
							default : num = 0;
									  break;
				
						}
					}
					else if((AC_RADIO[RadioID]->Radio_Type == 4)||(AC_RADIO[RadioID]->Radio_Type == 2)||
				(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 12)||(AC_RADIO[RadioID]->Radio_Type == 26)||(AC_RADIO[RadioID]->Radio_Type == 44))
					{
						switch(RadioRate[0])
						{
				
								case 60 : wrate[0] = 60;
										  num = 1;
										  break;
								case 90 : wrate[0] = 60;
										  wrate[1] = 90;
										  num = 2;
										  break;
								case 120 : wrate[0] = 60;
										  wrate[1] = 90;
										  wrate[2] = 120;
										  num = 3;
										  break;
								case 180 : wrate[0] = 60;
										   wrate[1] = 90;
										   wrate[2] = 120;
										   wrate[3] = 180;
										   num = 4;
										   break;
								case 240 : wrate[0] = 60;
										   wrate[1] = 90;
										   wrate[2] = 120;
										   wrate[3] = 180;
										   wrate[4] = 240;
										   num = 5;
										   break;
								case 360 : wrate[0] = 60;
										   wrate[1] = 90;
										   wrate[2] = 120;
										   wrate[3] = 180;
										   wrate[4] = 240;
										   wrate[5] = 360;
										   num = 6;
										   break;
								case 480 : wrate[0] = 60;
										   wrate[1] = 90;
										   wrate[2] = 120;
										   wrate[3] = 180;
										   wrate[4] = 240;
										   wrate[5] = 360;
										   wrate[6] = 480;
										   num = 7;
										   break;
								case 540 : wrate[0] = 60;
										   wrate[1] = 90;
										   wrate[2] = 120;
										   wrate[3] = 180;
										   wrate[4] = 240;
										   wrate[5] = 360;
										   wrate[6] = 480;
										   wrate[7] = 540;
										   num = 8;
										   break;
								default : num = 0;
										  break;
		
							}

						}
						else if(AC_RADIO[RadioID]->Radio_Type == 5||AC_RADIO[RadioID]->Radio_Type == 13)
						{
							switch(RadioRate[0])
							{
								
								case 10 : num = 1;
										  break;
								case 20 : num = 2;
										  break;
								case 55 : num = 3;
										  break;
								case 60 : num = 4;
										  break;
								case 90 : num = 5;
										  break;
								case 110 : num = 6;
										   break;
								case 120 : num = 7;
										  break;
								case 180 : num = 8;
										  break;
								case 240 : num = 9;
										  break;
								case 360 : num = 10;
										  break;
								case 480 : num = 11;
										  break;
								case 540 : num = 12;
										  break;
								default : num = 0;
										  break;
								
							}
							for(i=0;i<num;i++)
							{
								wrate[i] = drate[i];
							}
						}
		
						else//add 11n or others
						{
							return RADIO_SUPPORT_RATE_CONFLICT;
						}
						//create a new list
						ptr = create_support_rate_list(1);
						AC_RADIO[RadioID]->Support_Rate_Count = num;
						for(i=0;i<num;i++)
						{
							ptr = insert_rate_into_list(ptr,wrate[i]);
						}
						AC_RADIO[RadioID]->Radio_Rate = ptr;
						AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
						AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
		
						//printf("/////the list has %d element////\n",AC_RADIO[RadioID]->Support_Rate_Count);
						
						//display_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
		
						break;
				default : break;
			}
		

	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x4;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
	
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Rates;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}

int WID_RADIO_SET_MODE(unsigned int RadioID, unsigned int RadioMode)
{
	int rate =0;
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->Radio_Type == RadioMode)
	{
		return 0;
	}
	//delete by weianying  20091117
	/*
	if(AC_RADIO[RadioID]->Radio_Type == 10)//an
	{
		if(RadioMode == 13)//bgn
		{
			//only allow to turn an to bgn
		}
		else
		{
			return RADIO_MODE_IS_11N;
		}
	}
	else if(AC_RADIO[RadioID]->Radio_Type == 13)//bgn
	{
		if(RadioMode == 10)//an
		{
			//only allow to turn bgn to an
		}
		else
		{
			return RADIO_MODE_IS_11N;
		}
	}
	
	*/
	
	//added by weiay 20080716
	if((RadioMode == 2)||(RadioMode == 1)||(RadioMode == 4)||(RadioMode == 5)||(RadioMode == 8)||(RadioMode == 10)||(RadioMode == 13)
		||(RadioMode == 12)||(RadioMode == 26)||(RadioMode == 44)/*||(RadioMode == 37)*/)  /*fengwenchao modify 20111109 for GM*/
	{
	}
	else
	{
		return WTP_NO_SURPORT_TYPE;
	}
	/*fengwenchao modify begin for GM 20111109*/
	if((RadioMode == 44)||(RadioMode == 12))
	{
		if((AC_RADIO[RadioID]->Radio_Type_Bank != 12)||(AC_RADIO[RadioID]->Radio_Type_Bank != 13)||(AC_RADIO[RadioID]->Radio_Type_Bank != 44))
		{
				
		}
		else
			return WTP_NO_SURPORT_TYPE;		
	}
	else if(RadioMode == 26)
	{
		if((AC_RADIO[RadioID]->Radio_Type_Bank != 10)||(AC_RADIO[RadioID]->Radio_Type_Bank != 26))
		{
					
		}
		else
			return WTP_NO_SURPORT_TYPE;	
	}
	else
	{
		if((AC_RADIO[RadioID]->Radio_Type_Bank | RadioMode) != AC_RADIO[RadioID]->Radio_Type_Bank)
		{
			return WTP_NO_SURPORT_TYPE;
		}		
	}
	/*fengwenchao modify end*/
	AC_RADIO[RadioID]->Radio_Type = RadioMode;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, type is %x",RadioID,RadioMode);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x8;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Mode;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	//after set mode, rate is setting to default 
	if(AC_RADIO[RadioID]->Radio_Type == 1)
	{
		AC_RADIO[RadioID]->Support_Rate_Count = 4;
		//memory leak
		
		//AC_RADIO[RadioID]->Radio_Rate = (struct Support_Rate_List *)malloc(sizeof(struct Support_Rate_List));
		destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
		AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);//here add 10 first
		
		//printf("/////the list has %d element////\n",AC_RADIO[gwtpid]->Support_Rate_Count);
		
		rate = 10;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 20;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 55;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 110;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		//printf("/////the list has %d element////\n",AC_RADIO[RadioID]->Support_Rate_Count);
		AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
		AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
		//printf("/////the list has %d element////\n",AC_RADIO[RadioID]->Support_Rate_Count);
		//display_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
	}

	if((AC_RADIO[RadioID]->Radio_Type == 4)||(AC_RADIO[RadioID]->Radio_Type == 2)||(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 12)
		||(AC_RADIO[RadioID]->Radio_Type == 26)||(AC_RADIO[RadioID]->Radio_Type == 44))
	{
		AC_RADIO[RadioID]->Support_Rate_Count = 8;
		//memory leak
		//AC_RADIO[RadioID]->Radio_Rate = (struct Support_Rate_List *)malloc(sizeof(struct Support_Rate_List));
		destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
		AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);//here add 10 first

		
		rate = 60;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 90;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 120;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 180;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 240;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 360;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 480;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		

		rate = 540;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				
		AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
		AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

	}
	//fengwenchao modify 20110411,begin
	//fengwenchao add 20101231
	if(AC_RADIO[RadioID]->Radio_Type == 13||AC_RADIO[RadioID]->Radio_Type == 5)
	{
		AC_RADIO[RadioID]->Support_Rate_Count = 12;

		destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
		AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);

		rate = 10;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 20;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 55;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 60;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 90;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 110;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

		rate = 120;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 180;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 240;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 360;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		rate = 480;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		

		rate = 540;
		AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
		AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
		AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
	}
	//fengwenchao add end
	/*fengwenchao add 20111109 for GM*/

	/*fengwenchao add end*/
	//delete by weianying  20091117
	//if((AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 13))//when mode is 11n ,set to default 12
	//{
	//	return 0;
	//}
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x4;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
	
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Rates;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}
	if((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11N) == IEEE80211_11N)
	{
		wid_radio_set_mcs_list(RadioID);
	}
	
	WID_RADIO_SET_CHAN(RadioID,0);
	
	/*fengwenchao add 20120716 for autelan-3057*/
	if((AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 12))  // 10-an, 12-gn
	{
		AC_RADIO[RadioID]->MixedGreenfield.Mixed_Greenfield = 1;
	}
	/*fengwenchao add end*/
	
	return 0;

}

//added by weiay
int WID_RADIO_SET_BEACON(unsigned int RadioID, unsigned short beaconinterval)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->BeaconPeriod == beaconinterval)
	{
		return 0;
	}
	
	AC_RADIO[RadioID]->BeaconPeriod = beaconinterval;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, beaconinterval is %d",RadioID,beaconinterval);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x10;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_BeaconPeriod;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}
int WID_RADIO_SET_FRAGMENTATION(unsigned int RadioID, unsigned short fragmentation)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->FragThreshold== fragmentation)
	{
		return 0;
	}
	
	AC_RADIO[RadioID]->FragThreshold = fragmentation;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, fragmentation is %d",RadioID,fragmentation);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_FragThreshold;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}
int WID_RADIO_SET_DTIM(unsigned int RadioID, unsigned char dtim)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->DTIMPeriod == dtim)
	{
		return 0;
	}
	
	AC_RADIO[RadioID]->DTIMPeriod = dtim;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, dtim is %d",RadioID,dtim);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x10;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_DTIMPeriod;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}
int WID_RADIO_SET_RTSTHRESHOLD(unsigned int RadioID, unsigned short rtsthreshold)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->rtsthreshold == rtsthreshold)
	{
		return 0;
	}
	
	AC_RADIO[RadioID]->rtsthreshold = rtsthreshold;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, rtsthreshold is %d",RadioID,rtsthreshold);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_rtsthreshold;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}
int WID_RADIO_SET_STATUS(unsigned int RadioID, unsigned char status)
{
	msgq msg;
	
	/*fengwenchao add 20110922 for radio disable config save*/
	if(status == 1)
	{
		AC_RADIO[RadioID]->radio_disable_flag = 0; 
	}
	else if(status == 2)
	{
		AC_RADIO[RadioID]->radio_disable_flag = 1; 	
	}
	/*fengwenchao add end*/
	if(AC_RADIO[RadioID]->OpStat == status)
	{
		return 0;
	}
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState != CW_ENTER_RUN))
	{
		return WTP_NOT_IN_RUN_STATE;
	}

	/* book add, 2011-1-25 */
	if(1 == status){
	    AC_RADIO[RadioID]->upcount++;
	}
	else if(2 == status){
	    AC_RADIO[RadioID]->downcount++;
	}
	
	AC_RADIO[RadioID]->OpStat = status;
	AC_RADIO[RadioID]->AdStat = status;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, status is %d",RadioID,status);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x80;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_STATUS;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}
int WID_RADIO_SET_PREAMBLE(unsigned int RadioID, unsigned char preamble)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->IsShortPreamble == preamble)
	{
		return 0;
	}
	
	AC_RADIO[RadioID]->IsShortPreamble = preamble;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, IsShortPreamble is %d",RadioID,preamble);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x10;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Preamble;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}
int WID_RADIO_SET_SHORTRETRY(unsigned int RadioID, unsigned char shortretry)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->ShortRetry == shortretry)
	{
		return 0;
	}
	
	AC_RADIO[RadioID]->ShortRetry = shortretry;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, ShortRetry is %d",RadioID,shortretry);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_ShortRetry;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}
int WID_RADIO_SET_LONGRETRY(unsigned int RadioID, unsigned char longretry)
{
	msgq msg;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}

	if(AC_RADIO[RadioID]->LongRetry == longretry)
	{
		return 0;
	}
	
	AC_RADIO[RadioID]->LongRetry = longretry;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, LongRetry is %d",RadioID,longretry);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_LongRetry;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}

	return 0;

}


int WID_ADD_WLAN_APPLY_RADIO(unsigned int RadioID,unsigned char WlanID){

	int ret = -1;
	int k1 = 0;	
	int i = 0;
	char nas_id[NAS_IDENTIFIER_NAME];//zhanglei add
	unsigned int nas_id_len = 0;//zhanglei add
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;
		
	if(AC_RADIO[RadioID]->BindingWlanCount >= L_BSS_NUM)
	{
		return WTP_OVER_MAX_BSS_NUM;
	}


	if(AC_WLAN[WlanID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	if (AC_WLAN[WlanID]->want_to_delete == 1)		/* Huangleilei add for ASXXZFI-1622 */
	{
		wid_syslog_err("operator want to delete wlan %d", WlanID);
		return -1;
	}

	if(AC_WLAN[WlanID]->Wlan_Ifi != NULL)
	{
		if(AC_WTP[WtpID]->BindingSystemIndex != -1)
		{
			struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
			while(wlan_ifi != NULL)
			{

				if(AC_WTP[WtpID]->BindingSystemIndex == wlan_ifi->ifi_index)
				{
					if(wlan_ifi->nas_id_len > 0)
					{
						nas_id_len = wlan_ifi->nas_id_len;//zhanglei add
						memcpy(nas_id,wlan_ifi->nas_id,NAS_IDENTIFIER_NAME);//zhanglei add
					}
					break;
				}
				wlan_ifi = wlan_ifi->ifi_next;
			}
			
			if(wlan_ifi == NULL)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding interface doesn't match with wlan binding interface **\n");
				//return WTP_WLAN_BINDING_NOT_MATCH;
			}
			
		}
		else
		{
			wid_syslog_warning("<warning>,%s,%d,WTP_IF_NOT_BE_BINDED\n",__func__,__LINE__);
			//return WTP_IF_NOT_BE_BINDED;
		}
	}
	else
	{
		wid_syslog_warning("<warning>,%s,%d,Wlan_IF_NOT_BE_BINDED\n",__func__,__LINE__);
		//return Wlan_IF_NOT_BE_BINDED;
	}

	//added end
		
	struct wlanid *wlan_id;
	struct wlanid *wlan_id_next;
	wlan_id = (struct wlanid*)malloc(sizeof(struct wlanid));
	
	wlan_id->wlanid= WlanID;
	wlan_id->next = NULL;
	wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id  is %d*\n", wlan_id->wlanid);
	
	if(AC_RADIO[RadioID]->Wlan_Id == NULL)
	{
		
		AC_RADIO[RadioID]->Wlan_Id = wlan_id ;
		
		AC_RADIO[RadioID]->isBinddingWlan = 1;
		AC_RADIO[RadioID]->BindingWlanCount++;
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding first wlan id:%d  \n",WtpID,WlanID);
	}
	else
	{
	
		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next != NULL)
		{	
			if(wlan_id_next->wlanid == WlanID)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"warnning you have binding this wlan id,please do not binding this again");
				free(wlan_id);
				return 0;
			}else{
				wid_syslog_debug_debug(WID_DEFAULT,"%s wid_dbug_trap_ssid_key_conflict\n",__func__);
				if(((AC_WLAN[wlan_id_next->wlanid])&&(AC_WLAN[wlan_id_next->wlanid]->SecurityID == AC_WLAN[WlanID]->SecurityID)
					&&(AC_WLAN[WlanID]->KeyLen))
					|| ((AC_WLAN[wlan_id_next->wlanid])&&(AC_WLAN[WlanID]->KeyLen) && (AC_WLAN[wlan_id_next->wlanid]->KeyLen == AC_WLAN[WlanID]->KeyLen)
					&& (strncmp(AC_WLAN[wlan_id_next->wlanid]->WlanKey,AC_WLAN[WlanID]->WlanKey,AC_WLAN[WlanID]->KeyLen)==0)))
					wid_dbug_trap_ssid_key_conflict(WtpID, (unsigned char)localradio_id, wlan_id_next->wlanid, WlanID);
			}
			wlan_id_next = wlan_id_next->next;
		}

		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next->next!= NULL)
		{	
			wlan_id_next = wlan_id_next->next;//insert element int tail
		}
		
		wlan_id_next->next= wlan_id;
		AC_RADIO[RadioID]->BindingWlanCount++;

		
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding more wlan id:%d  \n",WtpID,WlanID);
	}

	for(k1=0;k1<L_BSS_NUM;k1++){
			if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] == NULL){
				//printf("BSSIndex:%d\n",k1);
				if((RadioID)*L_BSS_NUM+k1 >= BSS_NUM){
					wid_syslog_err("<error>invalid bssindex:%d,%s\n",(RadioID)*L_BSS_NUM+k1,__func__);
					return BSS_ID_LARGE_THAN_MAX;
				}
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] = (WID_BSS*)malloc(sizeof(WID_BSS));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1],0,sizeof(WID_BSS));	//mahz add 2011.6.15
				 /*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，star*/
				memset(&(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_pkt_info),0,sizeof(BSSStatistics));
                /*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，end*/
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID = (unsigned char*)malloc(6);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID,0,6);
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->bss_max_allowed_sta_num= AC_WLAN[WlanID]->bss_allow_max_sta_num;//fengwenchap modify 20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WlanID = WlanID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_G_ID = RadioID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_L_ID = localradio_id;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->State = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->band_width = 25;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ath_l2_isolation = AC_WLAN[WlanID]->wlan_ath_l2_isolation; //fengwenchao modify 20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->cwmmode = 1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit_able = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit = AC_WLAN[WlanID]->wlan_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->average_rate = AC_WLAN[WlanID]->wlan_station_average_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_traffic_limit = AC_WLAN[WlanID]->wlan_send_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_average_rate = AC_WLAN[WlanID]->wlan_station_average_send_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ip_mac_binding = AC_WLAN[WlanID]->sta_ip_mac_bind; //fengwenchao modify 20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->upcount= 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->downcount = 0;				
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->limit_sta_rssi = AC_WLAN[WlanID]->wlan_limit_sta_rssi; //fengwenchao add 20120222 for RDIR-25
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex = (RadioID)*L_BSS_NUM+k1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_POLICY = AC_WLAN[WlanID]->wlan_if_policy;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->vMAC_STATE = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WDSStat = AC_WLAN[WlanID]->WDSStat;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wds_mesh = AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wblwm= AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wds_bss_list = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_TUNNEL_POLICY = AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->multi_user_optimize_switch = AC_WLAN[WlanID]->multi_user_optimize_switch;//weichao add 
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_NAME, 0, ETH_IF_NAME_LEN);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, 0, NAS_IDENTIFIER_NAME);//zhanglei add
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = 0;//zhanglei add
				if(nas_id_len > 0){
					AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = nas_id_len;//zhanglei add
					memcpy(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, nas_id, NAS_IDENTIFIER_NAME);//zhanglei add
				}
				memset(&(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_pkt_info),0,sizeof(BSSStatistics));
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf = (struct acl_config *)malloc(sizeof(struct acl_config));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf,0,sizeof(struct acl_config));
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->macaddr_acl = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->accept_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_accept_mac = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->deny_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_deny_mac = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->vlanid = 0;
				//put wlan-vlan to bss
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wlan_vlanid = AC_WLAN[WlanID]->vlanid;

				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->hotspot_id = AC_WLAN[WlanID]->hotspot_id;
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->arp_ifname,0,ETH_IF_NAME_LEN);      //fengwenchao  add				       
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->sta_static_arp_policy = AC_WLAN[WlanID]->wlan_sta_static_arp_policy;      //fengwenchao   modify 20120323				
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_port_id,0,NAS_PORT_ID_LEN);		       
				memcpy(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_port_id,AC_WLAN[WlanID]->nas_port_id,NAS_PORT_ID_LEN);

				AC_BSS[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_RADIO[RadioID]->BSS[k1] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex;
				AC_RADIO[RadioID]->BSS[k1]->muti_rate = AC_WLAN[WlanID]->wlan_muti_rate;
				AC_RADIO[RadioID]->BSS[k1]->noResToStaProReqSW = AC_WLAN[WlanID]->wlan_noResToStaProReqSW;
				AC_RADIO[RadioID]->BSS[k1]->muti_bro_cast_sw = AC_WLAN[WlanID]->wlan_muti_bro_cast_sw;
				AC_RADIO[RadioID]->BSS[k1]->unicast_sw = AC_WLAN[WlanID]->wlan_unicast_sw;
				AC_RADIO[RadioID]->BSS[k1]->wifi_sw = AC_WLAN[WlanID]->wlan_wifi_sw;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wsm_sta_info_reportswitch = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wsm_sta_info_reportinterval = 1800;
				//radio apply wep wlan
				if((AC_WLAN[WlanID]->EncryptionType == WEP)&&(AC_WLAN[WlanID]->SecurityType != IEEE8021X))
				{
					//fengwenchao change begin
					//修改前代码开始
					/*for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_WTP[WtpID]->wep_flag[i] == 0)
						{
							AC_WTP[WtpID]->wep_flag[i] = (RadioID)*L_BSS_NUM+k1;
							AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = i+1;
							break;
						}
						if(i == WTP_WEP_NUM-1)
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
						//if(AC_WTP[WtpID]-)
					}*/
					//修改前代码结束

					//修改后代码开始
					for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_RADIO[RadioID]->wep_flag[i] == 0)
						{
							int k =0;
							for(k=0;k<i;k++)
							{
								if((AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->keyindex == AC_WLAN[WlanID]->SecurityIndex)&&(AC_WLAN[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->WlanID]->EncryptionType == WEP))
								{
										WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
										return SECURITYINDEX_IS_SAME;
								}
							}
						}
						int j =0;
						j = AC_WLAN[WlanID]->SecurityIndex - 1;
						if(j<WTP_WEP_NUM)
						{
							if(AC_RADIO[RadioID]->wep_flag[j] == 0)
							{
								AC_RADIO[RadioID]->wep_flag[j] = (RadioID)*L_BSS_NUM+k1;
								AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = AC_WLAN[WlanID]->SecurityIndex;															
								break;
							}
						}	
						else
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
						//if(AC_WTP[WtpID]-)
					}
					//修改后代码结束
					//fengwenchao change end
				}
				
				if(AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE)
				{		
				
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}
	
				}
				else if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)
				{
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}

					ret = ADD_BSS_L3_Interface_BR(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return BSS_L3_INTERFACE_ADD_BR_FAIL;
					}
				}
				break;
			}
		}	

	
	if(k1 == L_BSS_NUM)
	{
		return WTP_OVER_MAX_BSS_NUM;
	}
	if(AC_WLAN[WlanID]->Status == 1)
	{
		printf("wlan is disable,so just binging this wlan,not to send add wlan msg\n");
		return 0;
	}
	msgq msg;
//	struct msgqlist *elem;
	//add to control list
	if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->WTPStat == 5))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;
		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;/* 0 asic; 1 hex*/
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;			/*Roaming (1 enable /0 disable)*/
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,strlen(AC_WLAN[WlanID]->ESSID));
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];

		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}//delete unuseful code
	/*else if((AC_WTP[WtpID] != NULL))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;
		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];

		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WtpID, elem);		
	}*/
	if(AC_WTP[WtpID]!=NULL){
		if((AC_WLAN[WlanID])&&(AC_WLAN[WlanID]->Status == 0))
			WLAN_FLOW_CHECK(WlanID);
	}

	/* send eap switch & mac to ap,zhangshu add 2010-10-22 */
	int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
	if((AC_WLAN[WlanID]->eap_mac_switch==1)&&(AC_WLAN[WlanID]->wlan_if_policy==NO_INTERFACE)&&(AC_BSS[bssindex]!=NULL)&&(AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE))
	{
	    sprintf(apcmd,"set_eap_mac ath.%d-%d %s",AC_RADIO[RadioID]->Radio_L_ID,WlanID,AC_WLAN[WlanID]->eap_mac);
	}
	else
	{
	    sprintf(apcmd,"set_eap_mac ath.%d-%d 0",AC_RADIO[RadioID]->Radio_L_ID,WlanID);
	}
	wid_syslog_debug_debug(WID_DEFAULT,"Enable Wlan: set eap mac cmd %s\n",apcmd);
	wid_radio_set_extension_command(WtpID,apcmd);
	/* end */

    /* zhangshu add , 2011-1-7 */
	if((check_bssid_func(AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id]))&&(AC_BSS[(AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id])] != NULL))
	{                                                                                        
		unsigned int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		wid_syslog_debug_debug(WID_DEFAULT,"!@#$ AC_BSS[%d]->traffic_limit_able = %d\n",bssindex,AC_BSS[bssindex]->traffic_limit_able);
		WID_Save_Traffic_Limit(bssindex, WtpID);
	}
		
	if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)&&(AC_BSS[bssindex]->BSS_TUNNEL_POLICY != CW_802_DOT_11_TUNNEL)){
		msgq msg;
//		struct msgqlist *elem;
		memset((char*)&msg, 0, sizeof(msg));
		wid_syslog_debug_debug(WID_DEFAULT,"*** %s,%d.**\n",__func__,__LINE__);
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_CHANGE_TUNNEL;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;
		
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		if((AC_WTP[WtpID]->WTPStat == 5)){ 
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}//delete unuseful code
		/*else{
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WtpID, elem);
		}*/
	}	
	if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->multi_user_optimize_switch == 1))
	{
		char wlanid =AC_BSS[bssindex]->WlanID;
		int radioid = AC_BSS[bssindex]->Radio_G_ID;
		muti_user_optimize_switch(wlanid,radioid,1);
		
	}	
	return 0;

}
int WID_ADD_WLAN_APPLY_RADIO_BASE_VLANID(unsigned int RadioID,unsigned char WlanID,unsigned int vlan_id){

	int ret = -1;
	int k1 = 0; 
	int i = 0;
	char nas_id[NAS_IDENTIFIER_NAME];//zhanglei add
	unsigned int nas_id_len = 0;//zhanglei add
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;
		
	if(AC_RADIO[RadioID]->BindingWlanCount >= L_BSS_NUM)
	{
		return WTP_OVER_MAX_BSS_NUM;
	}


	if(AC_WLAN[WlanID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	if (AC_WLAN[WlanID]->want_to_delete == 1)		/* Huangleilei add for ASXXZFI-1622 */
	{
		wid_syslog_err("operator want to delete wlan %d", WlanID);
		return -1;
	}
	if(AC_WLAN[WlanID]->Wlan_Ifi != NULL)
	{
		if(AC_WTP[WtpID]->BindingSystemIndex != -1)
		{
			struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
			while(wlan_ifi != NULL)
			{
				if(AC_WTP[WtpID]->BindingSystemIndex == wlan_ifi->ifi_index)
				{
					if(wlan_ifi->nas_id_len > 0)
					{
						nas_id_len = wlan_ifi->nas_id_len;//zhanglei add
						memcpy(nas_id,wlan_ifi->nas_id,NAS_IDENTIFIER_NAME);//zhanglei add
					}
					break;
				}
				wlan_ifi = wlan_ifi->ifi_next;
			}
			
			if(wlan_ifi == NULL)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding interface doesn't match with wlan binding interface **\n");
				//return WTP_WLAN_BINDING_NOT_MATCH;
			}
			
		}
		else
		{
			wid_syslog_warning("<warning>,%s,%d,WTP_IF_NOT_BE_BINDED\n",__func__,__LINE__);
			//return WTP_IF_NOT_BE_BINDED;
		}
	}
	else
	{
		wid_syslog_warning("<warning>,%s,%d,Wlan_IF_NOT_BE_BINDED\n",__func__,__LINE__);
		//return Wlan_IF_NOT_BE_BINDED;
	}

	//added end
		
	struct wlanid *wlan_id;
	struct wlanid *wlan_id_next;
	wlan_id = (struct wlanid*)malloc(sizeof(struct wlanid));
	
	wlan_id->wlanid= WlanID;
	wlan_id->next = NULL;
	wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id  is %d*\n", wlan_id->wlanid);
	
	if(AC_RADIO[RadioID]->Wlan_Id == NULL)
	{
		
		AC_RADIO[RadioID]->Wlan_Id = wlan_id ;
		
		AC_RADIO[RadioID]->isBinddingWlan = 1;
		AC_RADIO[RadioID]->BindingWlanCount++;
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding first wlan id:%d	\n",WtpID,WlanID);
	}
	else
	{
	
		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next != NULL)
		{	
			if(wlan_id_next->wlanid == WlanID)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"warnning you have binding this wlan id,please do not binding this again");
				//wtp have already binding this wlan ,only to add vlanid
				int i = 0;	
				for(i=0;i<L_BSS_NUM;i++)
				{
					if(AC_RADIO[RadioID]->BSS[i] != NULL)
					{
						if(AC_RADIO[RadioID]->BSS[i]->WlanID == WlanID)
						{
							AC_RADIO[RadioID]->BSS[i]->vlanid = vlan_id;
							break;
						}
					}
				}
				free(wlan_id);
				return 0;
			}
			wlan_id_next = wlan_id_next->next;
		}

		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next->next!= NULL)
		{	
			wlan_id_next = wlan_id_next->next;//insert element int tail
		}
		
		wlan_id_next->next= wlan_id;
		AC_RADIO[RadioID]->BindingWlanCount++;
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding more wlan id:%d  \n",WtpID,WlanID);
	}

	for(k1=0;k1<L_BSS_NUM;k1++){
			if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] == NULL){
				//printf("BSSIndex:%d\n",k1);
				if((RadioID)*L_BSS_NUM+k1 >= BSS_NUM){
					wid_syslog_err("<error>invalid bssindex:%d,%s\n",(RadioID)*L_BSS_NUM+k1,__func__);
					return BSS_ID_LARGE_THAN_MAX;
				}
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] = (WID_BSS*)malloc(sizeof(WID_BSS));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1],0,sizeof(WID_BSS));	//mahz add 2011.6.15
				/*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，star*/
				memset(&(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_pkt_info),0,sizeof(BSSStatistics));
                /*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，end*/
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID = (unsigned char*)malloc(6);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID,0,6);
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->bss_max_allowed_sta_num= AC_WLAN[WlanID]->bss_allow_max_sta_num;//fengwenchap modify 20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WlanID = WlanID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_G_ID = RadioID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_L_ID = localradio_id;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->State = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->band_width = 25;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WDSStat = AC_WLAN[WlanID]->WDSStat;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wblwm= AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wds_mesh = AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ath_l2_isolation = AC_WLAN[WlanID]->wlan_ath_l2_isolation; //fengwenchao modify 20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->cwmmode = 1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit_able = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit = AC_WLAN[WlanID]->wlan_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->average_rate = AC_WLAN[WlanID]->wlan_station_average_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_traffic_limit = AC_WLAN[WlanID]->wlan_send_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_average_rate = AC_WLAN[WlanID]->wlan_station_average_send_traffic_limit;;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ip_mac_binding = AC_WLAN[WlanID]->sta_ip_mac_bind; //fengwenchao modify 20120323
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->arp_ifname,0,ETH_IF_NAME_LEN);      //fengwenchao  add	20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->sta_static_arp_policy = AC_WLAN[WlanID]->wlan_sta_static_arp_policy;      //fengwenchao   add 20120323				
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->upcount = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->downcount = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex = (RadioID)*L_BSS_NUM+k1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_POLICY = AC_WLAN[WlanID]->wlan_if_policy;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_TUNNEL_POLICY = AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY;
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_NAME, 0, ETH_IF_NAME_LEN);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, 0, NAS_IDENTIFIER_NAME);//zhanglei add
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = 0;//zhanglei add
				if(nas_id_len > 0){
					AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = nas_id_len;//zhanglei add
					memcpy(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, nas_id, NAS_IDENTIFIER_NAME);//zhanglei add
				}
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->hotspot_id = AC_WLAN[WlanID]->hotspot_id;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->multi_user_optimize_switch = AC_WLAN[WlanID]->multi_user_optimize_switch;//weichao add 
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf = (struct acl_config *)malloc(sizeof(struct acl_config));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf,0,sizeof(struct acl_config));
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->macaddr_acl = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->accept_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_accept_mac = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->deny_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_deny_mac = 0;

				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wlan_vlanid = AC_WLAN[WlanID]->vlanid;
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_port_id,0,NAS_PORT_ID_LEN);		       
				memcpy(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_port_id,AC_WLAN[WlanID]->nas_port_id,NAS_PORT_ID_LEN);
				//put vlan to bss
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->vlanid = vlan_id;
				AC_BSS[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_RADIO[RadioID]->BSS[k1] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex;

				AC_RADIO[RadioID]->BSS[k1]->muti_rate = AC_WLAN[WlanID]->wlan_muti_rate;
				AC_RADIO[RadioID]->BSS[k1]->noResToStaProReqSW = AC_WLAN[WlanID]->wlan_noResToStaProReqSW;
				AC_RADIO[RadioID]->BSS[k1]->muti_bro_cast_sw = AC_WLAN[WlanID]->wlan_muti_bro_cast_sw;
				AC_RADIO[RadioID]->BSS[k1]->unicast_sw = AC_WLAN[WlanID]->wlan_unicast_sw;
				AC_RADIO[RadioID]->BSS[k1]->wifi_sw = AC_WLAN[WlanID]->wlan_wifi_sw;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wsm_sta_info_reportswitch = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wsm_sta_info_reportinterval = 1800;
				//radio apply wep wlan
				if((AC_WLAN[WlanID]->EncryptionType == WEP)&&(AC_WLAN[WlanID]->SecurityType != IEEE8021X))
				{
					//fengwenchao change begin
					//修改前代码开始
					/*for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_WTP[WtpID]->wep_flag[i] == 0)
						{
							AC_WTP[WtpID]->wep_flag[i] = (RadioID)*L_BSS_NUM+k1;
							AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = i+1;
							break;
						}
						if(i == WTP_WEP_NUM-1)
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
					}*/
					//修改前代码结束
					
					//修改后代码开始
					for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_RADIO[RadioID]->wep_flag[i] == 0)
						{
							int k =0;
							for(k=0;k<i;k++)
							{
								if((AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->keyindex == AC_WLAN[WlanID]->SecurityIndex)&&(AC_WLAN[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->WlanID]->EncryptionType == WEP))
								{
										WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
										return SECURITYINDEX_IS_SAME;
								}
							}
						}
						int j =0;
						j = AC_WLAN[WlanID]->SecurityIndex - 1;
						if(j<WTP_WEP_NUM)
						{
							if(AC_RADIO[RadioID]->wep_flag[j] == 0)
							{
								AC_RADIO[RadioID]->wep_flag[j] = (RadioID)*L_BSS_NUM+k1;
								AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = AC_WLAN[WlanID]->SecurityIndex;															
								break;
							}
						}	
						else
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
						//if(AC_WTP[WtpID]-)
					}
					//修改后代码结束
					//fengwenchao change end
				}
				if(AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE)
				{		
				
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}
	
				}
				else if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)
				{
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}

					ret = ADD_BSS_L3_Interface_BR(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return BSS_L3_INTERFACE_ADD_BR_FAIL;
					}
				}
				break;
			}
		}	

	
	if(k1 == L_BSS_NUM)
	{
		return WTP_OVER_MAX_BSS_NUM;
	}	
	if(AC_WLAN[WlanID]->Status == 1)
	{
		printf("wlan is disable,so just binging this wlan,not to send add wlan msg\n");
		return 0;
	}
	msgq msg;
//	struct msgqlist *elem;
	//add to control list
	if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->WTPStat == 5))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;

		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;/* 0 asic; 1 hex*/
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;			/*Roaming (1 enable /0 disable)*/
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,strlen(AC_WLAN[WlanID]->ESSID));
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}//delete unuseful code
	/*else if((AC_WTP[WtpID] != NULL))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;

		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;	
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WtpID, elem);		
	}*/
	int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
	if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)&&(AC_BSS[bssindex]->BSS_TUNNEL_POLICY != CW_802_DOT_11_TUNNEL)){
		msgq msg;
//		struct msgqlist *elem;
		memset((char*)&msg, 0, sizeof(msg));
		wid_syslog_debug_debug(WID_DEFAULT,"*** %s,%d.**\n",__func__,__LINE__);
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_CHANGE_TUNNEL;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;
		
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		if((AC_WTP[WtpID]->WTPStat == 5)){ 
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}//delete unuseful cod
		/*else{
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WtpID, elem);
		}*/
	}
	if(AC_WTP[WtpID]!=NULL){
		if((AC_WLAN[WlanID])&&(AC_WLAN[WlanID]->Status == 0))
			WLAN_FLOW_CHECK(WlanID);
	}
	if((AC_RADIO[RadioID]->BSS[k1])&&(AC_RADIO[RadioID]->BSS[k1]->multi_user_optimize_switch == 1))
	{
		char wlanid =WlanID;
		int radioid = AC_RADIO[RadioID]->BSS[k1]->Radio_G_ID;
		muti_user_optimize_switch(wlanid,radioid,1);
		
	}
	return 0;

}

//mahz add 2011.5.30
int WID_ADD_WLAN_APPLY_RADIO_BASE_NAS_PORT_ID(unsigned int RadioID,unsigned char WlanID,char* nas_port_id){

	int ret = -1;
	int k1 = 0; 
	int i = 0;
	char nas_id[NAS_IDENTIFIER_NAME];//zhanglei add
	unsigned int nas_id_len = 0;//zhanglei add
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;
		
	if(AC_RADIO[RadioID]->BindingWlanCount >= L_BSS_NUM){
		return WTP_OVER_MAX_BSS_NUM;
	}

	if(AC_WLAN[WlanID] == NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	if (AC_WLAN[WlanID]->want_to_delete == 1)		/* Huangleilei add for ASXXZFI-1622 */
	{
		wid_syslog_err("operator want to delete wlan %d", WlanID);
		return -1;
	}
	if(AC_WLAN[WlanID]->Wlan_Ifi != NULL)
	{
		if(AC_WTP[WtpID]->BindingSystemIndex != -1)
		{
			struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
			while(wlan_ifi != NULL)
			{
				if(AC_WTP[WtpID]->BindingSystemIndex == wlan_ifi->ifi_index)
				{
					if(wlan_ifi->nas_id_len > 0)
					{
						nas_id_len = wlan_ifi->nas_id_len;//zhanglei add
						memcpy(nas_id,wlan_ifi->nas_id,NAS_IDENTIFIER_NAME);//zhanglei add
					}
					break;
				}
				wlan_ifi = wlan_ifi->ifi_next;
			}
			
			if(wlan_ifi == NULL)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding interface doesn't match with wlan binding interface **\n");
				//return WTP_WLAN_BINDING_NOT_MATCH;
			}
			
		}
		else
		{
			wid_syslog_warning("<warning>,%s,%d,WTP_IF_NOT_BE_BINDED\n",__func__,__LINE__);
			//return WTP_IF_NOT_BE_BINDED;
		}
	}
	else
	{
		wid_syslog_warning("<warning>,%s,%d,Wlan_IF_NOT_BE_BINDED\n",__func__,__LINE__);
		//return Wlan_IF_NOT_BE_BINDED;
	}

	//added end
		
	struct wlanid *wlan_id;
	struct wlanid *wlan_id_next;
	wlan_id = (struct wlanid*)malloc(sizeof(struct wlanid));
	
	wlan_id->wlanid= WlanID;
	wlan_id->next = NULL;
	wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id  is %d*\n", wlan_id->wlanid);
	
	if(AC_RADIO[RadioID]->Wlan_Id == NULL){
		AC_RADIO[RadioID]->Wlan_Id = wlan_id ;
		AC_RADIO[RadioID]->isBinddingWlan = 1;
		AC_RADIO[RadioID]->BindingWlanCount++;
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding first wlan id:%d	\n",WtpID,WlanID);
	}
	else{
		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next != NULL)
		{	
			if(wlan_id_next->wlanid == WlanID){
				wid_syslog_debug_debug(WID_DEFAULT,"warnning you have binding this wlan id,please do not binding this again");
				//wtp have already binding this wlan ,only to add nas_port_id
				int i = 0;	
				for(i=0;i<L_BSS_NUM;i++)
				{
					if(AC_RADIO[RadioID]->BSS[i] != NULL)
					{
						if(AC_RADIO[RadioID]->BSS[i]->WlanID == WlanID)
						{
							memset(AC_RADIO[RadioID]->BSS[i]->nas_port_id,0,NAS_PORT_ID_LEN);
							memcpy(AC_RADIO[RadioID]->BSS[i]->nas_port_id,nas_port_id,strlen(nas_port_id));
							break;
						}
					}
				}
				free(wlan_id);
				return 0;
			}
			wlan_id_next = wlan_id_next->next;
		}

		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next->next!= NULL){	
			wlan_id_next = wlan_id_next->next;//insert element int tail
		}
		
		wlan_id_next->next= wlan_id;
		AC_RADIO[RadioID]->BindingWlanCount++;
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding more wlan id:%d  \n",WtpID,WlanID);
	}

	for(k1=0;k1<L_BSS_NUM;k1++){
			if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] == NULL){
				//printf("BSSIndex:%d\n",k1);
				if((RadioID)*L_BSS_NUM+k1 >= BSS_NUM){
					wid_syslog_err("<error>invalid bssindex:%d,%s\n",(RadioID)*L_BSS_NUM+k1,__func__);
					return BSS_ID_LARGE_THAN_MAX;
				}
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] = (WID_BSS*)malloc(sizeof(WID_BSS));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1],0,sizeof(WID_BSS));	//mahz add 2011.6.15
				/*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，star*/
				memset(&(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_pkt_info),0,sizeof(BSSStatistics));
                /*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，end*/
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID = (unsigned char*)malloc(6);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID,0,6);
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->bss_max_allowed_sta_num= AC_WLAN[WlanID]->bss_allow_max_sta_num;//fengwenchap modify 20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WlanID = WlanID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_G_ID = RadioID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_L_ID = localradio_id;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->State = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->band_width = 25;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WDSStat = AC_WLAN[WlanID]->WDSStat;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wblwm= AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wds_mesh = AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ath_l2_isolation = AC_WLAN[WlanID]->wlan_ath_l2_isolation; //fengwenchao modify 20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->cwmmode = 1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit_able = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit = AC_WLAN[WlanID]->wlan_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->average_rate = AC_WLAN[WlanID]->wlan_station_average_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_traffic_limit = AC_WLAN[WlanID]->wlan_send_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_average_rate = AC_WLAN[WlanID]->wlan_station_average_send_traffic_limit;;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ip_mac_binding = AC_WLAN[WlanID]->sta_ip_mac_bind; //fengwenchao modify 20120323
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->arp_ifname,0,ETH_IF_NAME_LEN);      //fengwenchao  add	20120323
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->sta_static_arp_policy = AC_WLAN[WlanID]->wlan_sta_static_arp_policy;      //fengwenchao   add 20120323				
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->upcount = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->downcount = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex = (RadioID)*L_BSS_NUM+k1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_POLICY = AC_WLAN[WlanID]->wlan_if_policy;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_TUNNEL_POLICY = AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY;
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_NAME, 0, ETH_IF_NAME_LEN);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, 0, NAS_IDENTIFIER_NAME);//zhanglei add
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = 0;//zhanglei add
				if(nas_id_len > 0){
					AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = nas_id_len;//zhanglei add
					memcpy(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, nas_id, NAS_IDENTIFIER_NAME);//zhanglei add
				}
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf = (struct acl_config *)malloc(sizeof(struct acl_config));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf,0,sizeof(struct acl_config));
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->macaddr_acl = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->accept_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_accept_mac = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->deny_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_deny_mac = 0;

				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->multi_user_optimize_switch = AC_WLAN[WlanID]->multi_user_optimize_switch;//weichao add 
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wlan_vlanid = AC_WLAN[WlanID]->vlanid;
				//put vlan to bss
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->vlanid = 0;
				AC_BSS[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_RADIO[RadioID]->BSS[k1] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->hotspot_id = AC_WLAN[WlanID]->hotspot_id;

				AC_RADIO[RadioID]->BSS[k1]->muti_rate = AC_WLAN[WlanID]->wlan_muti_rate;
				AC_RADIO[RadioID]->BSS[k1]->noResToStaProReqSW = AC_WLAN[WlanID]->wlan_noResToStaProReqSW;
				AC_RADIO[RadioID]->BSS[k1]->muti_bro_cast_sw = AC_WLAN[WlanID]->wlan_muti_bro_cast_sw;
				AC_RADIO[RadioID]->BSS[k1]->unicast_sw = AC_WLAN[WlanID]->wlan_unicast_sw;
				AC_RADIO[RadioID]->BSS[k1]->wifi_sw = AC_WLAN[WlanID]->wlan_wifi_sw;
				//mahz add 2011.5.26
				unsigned int BSSIndex = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex;
				memset(AC_BSS[BSSIndex]->nas_port_id,0,NAS_PORT_ID_LEN);
				memcpy(AC_BSS[BSSIndex]->nas_port_id,nas_port_id,strlen(nas_port_id));
				wid_syslog_debug_info("AC_BSS->nas_port_id : %s\n",AC_BSS[BSSIndex]->nas_port_id); 	//for test

				//radio apply wep wlan
				if((AC_WLAN[WlanID]->EncryptionType == WEP)&&(AC_WLAN[WlanID]->SecurityType != IEEE8021X))
				{
					//fengwenchao change begin
					//修改前代码开始
					/*for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_WTP[WtpID]->wep_flag[i] == 0)
						{
							AC_WTP[WtpID]->wep_flag[i] = (RadioID)*L_BSS_NUM+k1;
							AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = i+1;
							break;
						}
						if(i == WTP_WEP_NUM-1)
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
					}*/
					//修改前代码结束
					
					//修改后代码开始
					for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_RADIO[RadioID]->wep_flag[i] == 0)
						{
							int k =0;
							for(k=0;k<i;k++)
							{
								if((AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->keyindex == AC_WLAN[WlanID]->SecurityIndex)&&(AC_WLAN[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->WlanID]->EncryptionType == WEP))
								{
										WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
										return SECURITYINDEX_IS_SAME;
								}
							}
						}
						int j =0;
						j = AC_WLAN[WlanID]->SecurityIndex - 1;
						if(j<WTP_WEP_NUM)
						{
							if(AC_RADIO[RadioID]->wep_flag[j] == 0)
							{
								AC_RADIO[RadioID]->wep_flag[j] = (RadioID)*L_BSS_NUM+k1;
								AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = AC_WLAN[WlanID]->SecurityIndex;															
								break;
							}
						}	
						else
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
						//if(AC_WTP[WtpID]-)
					}
					//修改后代码结束
					//fengwenchao change end
				}
				if(AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE)
				{		
				
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}
	
				}
				else if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)
				{
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}

					ret = ADD_BSS_L3_Interface_BR(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return BSS_L3_INTERFACE_ADD_BR_FAIL;
					}
				}
				break;
			}
		}	

	
	if(k1 == L_BSS_NUM)
	{
		return WTP_OVER_MAX_BSS_NUM;
	}
	if(AC_WLAN[WlanID]->Status == 1)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"wlan is disable,so just binging this wlan,not to send add wlan msg\n");
		return 0;
	}
	msgq msg;
//	struct msgqlist *elem;
	//add to control list
	if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->WTPStat == 5))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;

		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;/* 0 asic; 1 hex*/
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;			/*Roaming (1 enable /0 disable)*/
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,strlen(AC_WLAN[WlanID]->ESSID));
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}//delete unuseful cod
	/*else if((AC_WTP[WtpID] != NULL))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;

		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;	
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WtpID, elem);		
	}*/
	if(AC_WTP[WtpID]!=NULL){
		if((AC_WLAN[WlanID])&&(AC_WLAN[WlanID]->Status == 0))
			WLAN_FLOW_CHECK(WlanID);
	}
	int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
	if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)&&(AC_BSS[bssindex]->BSS_TUNNEL_POLICY != CW_802_DOT_11_TUNNEL)){
		msgq msg;
//		struct msgqlist *elem;
		memset((char*)&msg, 0, sizeof(msg));
		wid_syslog_debug_debug(WID_DEFAULT,"*** %s,%d.**\n",__func__,__LINE__);
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_CHANGE_TUNNEL;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;
		
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		if((AC_WTP[WtpID]->WTPStat == 5)){ 
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}//delete unuseful cod
		/*else{
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WtpID, elem);
		}*/
	}
	if((AC_RADIO[RadioID]->BSS[k1])&&(AC_RADIO[RadioID]->BSS[k1]->multi_user_optimize_switch == 1))
	{
		char wlanid =WlanID;
		int radioid = AC_RADIO[RadioID]->BSS[k1]->Radio_G_ID;
		muti_user_optimize_switch(wlanid,radioid,1);
		
	}
	return 0;

}

int WID_ADD_WLAN_APPLY_RADIO_CLEAN_NAS_PORT_ID(unsigned int RadioID,unsigned char WlanID){

	int isfind = 0; 
	int i = 0;
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;
	if(AC_WTP[WtpID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WTP_ID_NOT_EXIST;
	}
	if(AC_WLAN[WlanID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i] != NULL)
		{
			if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->WlanID == WlanID)
			{
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->nas_port_id, 0, NAS_PORT_ID_LEN);
				isfind = 1;
				break;
			}
		}
	}
	if(isfind != 1)
	{
		return WTP_WLAN_BINDING_NOT_MATCH;
	}
	return 0;
}

//weichao copy from vlan 
int WID_ADD_WLAN_APPLY_RADIO_BASE_HOTSPOT_ID(unsigned int RadioID,unsigned char WlanID,unsigned int  hotspot_id){

	int ret = -1;
	int k1 = 0; 
	int i = 0;
	char nas_id[NAS_IDENTIFIER_NAME];//zhanglei add
	unsigned int nas_id_len = 0;//zhanglei add
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;
		
	if(AC_RADIO[RadioID]->BindingWlanCount >= L_BSS_NUM){
		return WTP_OVER_MAX_BSS_NUM;
	}

	if(AC_WLAN[WlanID] == NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	if (AC_WLAN[WlanID]->want_to_delete == 1)		/* Huangleilei add for ASXXZFI-1622 */
	{
		wid_syslog_err("operator want to delete wlan %d", WlanID);
		return -1;
	}
	if(AC_WLAN[WlanID]->Wlan_Ifi != NULL)
	{
		if(AC_WTP[WtpID]->BindingSystemIndex != -1)
		{
			struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
			while(wlan_ifi != NULL)
			{
				if(AC_WTP[WtpID]->BindingSystemIndex == wlan_ifi->ifi_index)
				{
					if(wlan_ifi->nas_id_len > 0)
					{
						nas_id_len = wlan_ifi->nas_id_len;//zhanglei add
						memcpy(nas_id,wlan_ifi->nas_id,NAS_IDENTIFIER_NAME);//zhanglei add
					}
					break;
				}
				wlan_ifi = wlan_ifi->ifi_next;
			}
			
			if(wlan_ifi == NULL)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding interface doesn't match with wlan binding interface **\n");
				//return WTP_WLAN_BINDING_NOT_MATCH;
			}
			
		}
		else
		{
			return WTP_IF_NOT_BE_BINDED;
		}
	}
	else
	{
		//return Wlan_IF_NOT_BE_BINDED;
	}
	
	//added end
		
	struct wlanid *wlan_id;
	struct wlanid *wlan_id_next;
	wlan_id = (struct wlanid*)malloc(sizeof(struct wlanid));
	
	wlan_id->wlanid= WlanID;
	wlan_id->next = NULL;
	wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id  is %d*\n", wlan_id->wlanid);
	
	if(AC_RADIO[RadioID]->Wlan_Id == NULL){
		AC_RADIO[RadioID]->Wlan_Id = wlan_id ;
		AC_RADIO[RadioID]->isBinddingWlan = 1;
		AC_RADIO[RadioID]->BindingWlanCount++;
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding first wlan id:%d	\n",WtpID,WlanID);
	}
	else{
		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next != NULL)
		{	
			if(wlan_id_next->wlanid == WlanID){
				wid_syslog_debug_debug(WID_DEFAULT,"warnning you have binding this wlan id,please do not binding this again");
				int i = 0;	
				for(i=0;i<L_BSS_NUM;i++)
				{
					if(AC_RADIO[RadioID]->BSS[i] != NULL)
					{
						if(AC_RADIO[RadioID]->BSS[i]->WlanID == WlanID)
						{
							AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->hotspot_id = hotspot_id;
							break;
						}
					}
				}
				free(wlan_id);
				return 0;
			}
			wlan_id_next = wlan_id_next->next;
		}

		wlan_id_next = AC_RADIO[RadioID]->Wlan_Id;
		while(wlan_id_next->next!= NULL){	
			wlan_id_next = wlan_id_next->next;//insert element int tail
		}
		
		wlan_id_next->next= wlan_id;
		AC_RADIO[RadioID]->BindingWlanCount++;
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** wtp id:%d binding more wlan id:%d  \n",WtpID,WlanID);
	}

	for(k1=0;k1<L_BSS_NUM;k1++){
			if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] == NULL){
				//printf("BSSIndex:%d\n",k1);
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1] = (WID_BSS*)malloc(sizeof(WID_BSS));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1],0,sizeof(WID_BSS));	//mahz add 2011.6.14
				/*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，star*/
				memset(&(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_pkt_info),0,sizeof(BSSStatistics));
                /*zhaoruijia,初始化BSS_pkt_info数据防止随机数产生，end*/
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID = (unsigned char*)malloc(6);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSID,0,6);
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->bss_max_allowed_sta_num=128;//xm add
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WlanID = WlanID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_G_ID = RadioID;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->Radio_L_ID = localradio_id;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->State = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->band_width = 25;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->WDSStat = AC_WLAN[WlanID]->WDSStat;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wblwm= AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wds_mesh = AC_WLAN[WlanID]->wds_mesh;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ath_l2_isolation = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->cwmmode = 1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit_able = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->traffic_limit = AC_WLAN[WlanID]->wlan_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->average_rate = AC_WLAN[WlanID]->wlan_station_average_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_traffic_limit = AC_WLAN[WlanID]->wlan_send_traffic_limit;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->send_average_rate = AC_WLAN[WlanID]->wlan_station_average_send_traffic_limit;;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->ip_mac_binding = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->upcount = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->downcount = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex = (RadioID)*L_BSS_NUM+k1;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_POLICY = AC_WLAN[WlanID]->wlan_if_policy;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_TUNNEL_POLICY = AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY;
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSS_IF_NAME, 0, ETH_IF_NAME_LEN);
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, 0, NAS_IDENTIFIER_NAME);//zhanglei add
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = 0;//zhanglei add
				if(nas_id_len > 0){
					AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len = nas_id_len;//zhanglei add
					memcpy(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id, nas_id, NAS_IDENTIFIER_NAME);//zhanglei add
				}
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf = (struct acl_config *)malloc(sizeof(struct acl_config));
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf,0,sizeof(struct acl_config));
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->macaddr_acl = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->accept_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_accept_mac = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->deny_mac = NULL;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->acl_conf->num_deny_mac = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wlan_vlanid = AC_WLAN[WlanID]->vlanid;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->hotspot_id =  hotspot_id;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->multi_user_optimize_switch = AC_WLAN[WlanID]->multi_user_optimize_switch;//weichao add 
				//put vlan to bss
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->vlanid = 0;
				AC_BSS[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_RADIO[RadioID]->BSS[k1] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1];
				AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id] = AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex;

				
				AC_RADIO[RadioID]->BSS[k1]->muti_rate = AC_WLAN[WlanID]->wlan_muti_rate;
				AC_RADIO[RadioID]->BSS[k1]->noResToStaProReqSW = AC_WLAN[WlanID]->wlan_noResToStaProReqSW;
				AC_RADIO[RadioID]->BSS[k1]->muti_bro_cast_sw = AC_WLAN[WlanID]->wlan_muti_bro_cast_sw;
				AC_RADIO[RadioID]->BSS[k1]->unicast_sw = AC_WLAN[WlanID]->wlan_unicast_sw;
				AC_RADIO[RadioID]->BSS[k1]->wifi_sw = AC_WLAN[WlanID]->wlan_wifi_sw;
				memset(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_port_id,0,NAS_PORT_ID_LEN);		       
				memcpy(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->nas_port_id,AC_WLAN[WlanID]->nas_port_id,NAS_PORT_ID_LEN);

				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wsm_sta_info_reportswitch = 0;
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->wsm_sta_info_reportinterval = 1800;
				if((AC_WLAN[WlanID]->EncryptionType == WEP)&&(AC_WLAN[WlanID]->SecurityType != IEEE8021X))
				{
					//fengwenchao change begin
					//修改前代码开始
					/*for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_WTP[WtpID]->wep_flag[i] == 0)
						{
							AC_WTP[WtpID]->wep_flag[i] = (RadioID)*L_BSS_NUM+k1;
							AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = i+1;
							break;
						}
						if(i == WTP_WEP_NUM-1)
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
					}*/
					//修改前代码结束
					
					//修改后代码开始
					for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_RADIO[RadioID]->wep_flag[i] == 0)
						{
							int k =0;
							for(k=0;k<i;k++)
							{
								if((AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->keyindex == AC_WLAN[WlanID]->SecurityIndex)&&(AC_WLAN[AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k]->WlanID]->EncryptionType == WEP))
								{
										WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
										return SECURITYINDEX_IS_SAME;
								}
							}
						}
						int j =0;
						j = AC_WLAN[WlanID]->SecurityIndex - 1;
						if(j<WTP_WEP_NUM)
						{
							if(AC_RADIO[RadioID]->wep_flag[j] == 0)
							{
								AC_RADIO[RadioID]->wep_flag[j] = (RadioID)*L_BSS_NUM+k1;
								AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->keyindex = AC_WLAN[WlanID]->SecurityIndex;															
								break;
							}
						}	
						else
						{
							wid_syslog_debug_debug(WID_DEFAULT,"radio apply wep wlan over 4\n");
							WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
							return WTP_WEP_NUM_OVER;
						}
						//if(AC_WTP[WtpID]-)
					}
					//修改后代码结束
					//fengwenchao change end
				}
				if(AC_WLAN[WlanID]->wlan_if_policy == BSS_INTERFACE)
				{		
				
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}
	
				}
				else if(AC_WLAN[WlanID]->wlan_if_policy == WLAN_INTERFACE)
				{
					ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return WLAN_CREATE_L3_INTERFACE_FAIL;
					}

					ret = ADD_BSS_L3_Interface_BR(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[k1]->BSSIndex);
					if(ret < 0)
					{
						//WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanID);
						WID_DELETE_WLAN_APPLY_RADIO(RadioID,WlanID);
						return BSS_L3_INTERFACE_ADD_BR_FAIL;
					}
				}
				break;
			}
		}	

	
	if(k1 == L_BSS_NUM)
	{
		return WTP_OVER_MAX_BSS_NUM;
	}	
	if(AC_WLAN[WlanID]->Status == 1)
	{
		printf("wlan is disable,so just binging this wlan,not to send add wlan msg\n");
		return 0;
	}
	msgq msg;
//	struct msgqlist *elem;
	//add to control list
	if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->WTPStat == 5))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;

		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;/* 0 asic; 1 hex*/
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;			/*Roaming (1 enable /0 disable)*/
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,strlen(AC_WLAN[WlanID]->ESSID));
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}//delete unuseful cod
	/*else if((AC_WTP[WtpID] != NULL))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;

		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanID]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanID]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanID]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanID]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanID]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanID]->asic_hex;
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanID]->Roaming_Policy;	
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanID]->ESSID,ESSID_LENGTH);
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WtpID, elem);		
	}*/
	if(AC_WTP[WtpID]!=NULL){
		if((AC_WLAN[WlanID])&&(AC_WLAN[WlanID]->Status == 0))
			WLAN_FLOW_CHECK(WlanID);
	}
	int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
	if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)&&(AC_BSS[bssindex]->BSS_TUNNEL_POLICY != CW_802_DOT_11_TUNNEL)){
		msgq msg;
//		struct msgqlist *elem;
		memset((char*)&msg, 0, sizeof(msg));
		wid_syslog_debug_debug(WID_DEFAULT,"*** %s,%d.**\n",__func__,__LINE__);
		msg.mqid = WtpID%THREAD_NUM+1;
		msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_CHANGE_TUNNEL;
		msg.mqinfo.u.WlanInfo.WLANID = WlanID;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = localradio_id;
		
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[WtpID][localradio_id];
		
		if((AC_WTP[WtpID]->WTPStat == 5)){ 
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}//delete unuseful cod
		/*else{
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WtpID, elem);
		}*/
	}	
	if((AC_RADIO[RadioID]->BSS[k1])&&(AC_RADIO[RadioID]->BSS[k1]->multi_user_optimize_switch == 1))
	{
		char wlanid =AC_RADIO[RadioID]->BSS[k1]->WlanID;
		int radioid = AC_RADIO[RadioID]->BSS[k1]->Radio_G_ID;
		muti_user_optimize_switch(wlanid,radioid,1);
		
	}
	return 0;

}

int WID_ADD_WLAN_APPLY_RADIO_CLEAN_HOTSPOT_ID(unsigned int RadioID,unsigned char WlanID){
	wid_syslog_debug_debug(WID_DEFAULT,"*** in func %s**\n",__func__);

	int isfind = 0; 
	int i = 0;
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;
	if(AC_WTP[WtpID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WTP_ID_NOT_EXIST;
	}
	if(AC_WLAN[WlanID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i] != NULL)
		{
			if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->WlanID == WlanID)
			{
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->hotspot_id = AC_WLAN[WlanID]->hotspot_id;
				wid_syslog_debug_debug(WID_DEFAULT,"hotspot id is %d\n",AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->hotspot_id);
				isfind = 1;
				break;
			}
		}
	}
	if(isfind != 1)
	{
		return WTP_WLAN_BINDING_NOT_MATCH;
	}
	return 0;
}


int WID_ADD_WLAN_APPLY_RADIO_CLEAN_VLANID(unsigned int RadioID,unsigned char WlanID){

	int isfind = 0; 
	int i = 0;
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;
	if(AC_WTP[WtpID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WTP_ID_NOT_EXIST;
	}
	if(AC_WLAN[WlanID] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i] != NULL)
		{
			if(AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->WlanID == WlanID)
			{
				AC_WTP[WtpID]->WTP_Radio[localradio_id]->BSS[i]->vlanid = 0;
				isfind = 1;
				break;
			}
		}
	}
	if(isfind != 1)
	{
		return WTP_WLAN_BINDING_NOT_MATCH;
	}
	return 0;
}

int WID_BINDING_IF_APPLY_WTP(unsigned int WtpID, char * ifname)
{
	
	if(AC_WTP[WtpID]->isused == 1)
	{
	//	printf("*** error this WTP is used and active, you can not binding interface ***\n");
		wid_syslog_debug_debug(WID_DEFAULT,"*** error this WTP is used and active, you can not binding interface ***\n");
		return WTP_BE_USING;
	}
	int sockfd = -1;
	int isystemindex = -1;
	//int sockdes = -1;
	//CWBool bretflag = CW_FALSE;
	struct ifreq	ifr;
	int ret = Check_And_Bind_Interface_For_WID(ifname);
	if(ret != 0)
		return ret;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** can't create socket connection ***\n");
		return INTERFACE_NOT_EXIST;
	}
	
	//we should check ifname error
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));
	
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//Retrieve  the interface index  
		wid_syslog_debug_debug(WID_DEFAULT,"*** can't retrieve the interface index ***\n");
		close(sockfd);
		return INTERFACE_NOT_EXIST;
	}
	close(sockfd);
	isystemindex = ifr.ifr_ifindex;
	wid_syslog_debug_debug(WID_DEFAULT,"*** binding iterface name isystemindex is %d ***\n",isystemindex);

	//bretflag = get_sock_descriper(isystemindex, &sockdes);
	//printf("*** binding iterface name isystemindex is %d socket is %d flag is %d ***\n",isystemindex,sockdes,bretflag);
	
	/*
	***** Input wtp id is 1
	***** Input wtp id binding interface name is eth1
	*** binding iterface name isystemindex is 3 ***
	*** binding iterface name isystemindex is 3 socket is -1 flag is 0 ***
	*** can't binding iterface name, please make sure input correct interface name ***
	*/
	
	if(AC_WTP[WtpID] != NULL)
	{
		//we should add a variables into WID_WTP struct
		//AC_WTP[WTPID]->sock = sockdes;

		//delete bingding wlan id 
		//if(AC_WTP[WtpID]->Wlan_Id != NULL)
		//{
			//return WID_BINDING_WLAN;
		//}
		
		AC_WTP[WtpID]->BindingSystemIndex= isystemindex;
		memset(AC_WTP[WtpID]->BindingIFName, 0, ETH_IF_NAME_LEN);
		memcpy(AC_WTP[WtpID]->BindingIFName,ifname, strlen(ifname));
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** binding iterface name to wtp success ***\n");
		return 0;
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** can't binding iterface name, please make sure you have create wtp ***\n");
		return WTP_ID_NOT_EXIST;
	}
		
}


int WID_BINDING_IF_APPLY_WTP_ipv6(unsigned int WtpID, char * ifname)
{
	
	if(AC_WTP[WtpID]->isused == 1)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** error this WTP is used and active, you can not binding interface ***\n");
		return WTP_BE_USING;
	}
//	int sockfd = -1;
	int isystemindex = 1;
	int ret;
	int i = 0;
	struct ifi * tmp;
	struct ifi * tmp2;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi_tmp->ifi_name,0,sizeof(ifi_tmp->ifi_name));
	strncpy(ifi_tmp->ifi_name,ifname,sizeof(ifi_tmp->ifi_name));

	struct tag_ipv6_addr_list *ipv6list = (struct tag_ipv6_addr_list *)malloc(sizeof(struct tag_ipv6_addr_list));
	if(ipv6list == NULL)
	{
		free(ifi_tmp);
		ifi_tmp = NULL;
		return BINDING_IPV6_ADDRE_RROR;
	}
	ipv6list->ifindex = 0;
	ipv6list->ipv6list = NULL;
	ipv6list->ipv6num = 0;
	
	ret = get_if_addr_ipv6_list(ifname, ipv6list);
	if(ret != 0)
	{
		free(ifi_tmp);
		ifi_tmp = NULL;
		free_ipv6_addr_list(ipv6list);
		ret = BINDING_IPV6_ADDRE_RROR;
		return ret;
	}
	struct tag_ipv6_addr *ipv6addr = ipv6list->ipv6list;
		
	ifi_tmp->ifi_addr6 = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
	isystemindex = ipv6list->ifindex;
	
	if(WID_IF_V6 == NULL)
	{
		for(i=0; i<ipv6list->ipv6num; i++)
		{
			inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
			ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
		//	ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
			ipv6addr = ipv6addr->next;
		}	

		if(ret == 0)
		{
		   	tmp = (struct ifi*)malloc(sizeof(struct ifi));
			memset(tmp,0,sizeof(struct ifi));
			memcpy(tmp->ifi_name,ifname,strlen(ifname));
			tmp->ifi_index = isystemindex;
			WID_IF_V6 = tmp;
		}
		else
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return ret;
		}
		
	}
	else
	{
		tmp = WID_IF_V6;
		while(tmp != NULL)
		{
			printf("ifname = %s name = %s\n",tmp->ifi_name,ifname);
			if(( strlen(ifname) ==  strlen(tmp->ifi_name))&&(strcmp(tmp->ifi_name,ifname)==0))
			{	
				free(ifi_tmp->ifi_addr6);
				ifi_tmp->ifi_addr6 = NULL;
				free(ifi_tmp);
				ifi_tmp = NULL;
				free_ipv6_addr_list(ipv6list);

				if(AC_WTP[WtpID] != NULL)
				{
					
					AC_WTP[WtpID]->BindingSystemIndex= tmp->ifi_index;
					printf("AC_WTP[WtpID]->BindingSystemIndex = %d\n",AC_WTP[WtpID]->BindingSystemIndex);
					AC_WTP[WtpID]->isipv6addr = 1;
					memset(AC_WTP[WtpID]->BindingIFName, 0, ETH_IF_NAME_LEN);
					memcpy(AC_WTP[WtpID]->BindingIFName,ifname, strlen(ifname));
					
					wid_syslog_debug_debug(WID_DEFAULT,"*** binding iterface name to wtp success ***\n");
					return 0;
				}
				else
				{
					wid_syslog_debug_debug(WID_DEFAULT,"*** can't binding iterface name, please make sure you have create wtp ***\n");
					return WTP_ID_NOT_EXIST;
				}
				
				return 0;
			}
			tmp = tmp->ifi_next;
		}

		for(i=0; i<ipv6list->ipv6num; i++)
		{
			inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
			ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
			//ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
			ipv6addr = ipv6addr->next;
		}	

		if(ret == 0)
		{
			tmp = (struct ifi*)malloc(sizeof(struct ifi));
			memset(tmp,0,sizeof(struct ifi));
			memcpy(tmp->ifi_name,ifname,strlen(ifname));
			tmp->ifi_index = isystemindex;
			tmp2 = WID_IF_V6;
			WID_IF_V6 = tmp;
			WID_IF_V6->ifi_next = tmp2;	
		}
		else
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return ret;
		}
	
	}	
		
	gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
	gInterfacesCountIpv4 = CWNetworkCountInterfaceAddressesIpv4(&gACSocket);
	gInterfacesCountIpv6 = CWNetworkCountInterfaceAddressesIpv6(&gACSocket);
	free(ifi_tmp->ifi_addr6);
	ifi_tmp->ifi_addr6 = NULL;
	free(ifi_tmp);
	ifi_tmp = NULL;
	free_ipv6_addr_list(ipv6list);

	if(ret != 0)
	return ret;

	
	if(AC_WTP[WtpID] != NULL)
	{
		
		AC_WTP[WtpID]->BindingSystemIndex= isystemindex;
		AC_WTP[WtpID]->isipv6addr = 1;
		memset(AC_WTP[WtpID]->BindingIFName, 0, ETH_IF_NAME_LEN);
		memcpy(AC_WTP[WtpID]->BindingIFName,ifname, strlen(ifname));
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** binding iterface name to wtp success ***\n");
		return 0;
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** can't binding iterface name, please make sure you have create wtp ***\n");
		return WTP_ID_NOT_EXIST;
	}
		
}


//add wtp binding special wlan weiay 20080604
int WID_BINDING_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId)
{
	int radioid = WtpID*L_RADIO_NUM;
	
	int ret = WID_ADD_WLAN_APPLY_RADIO(radioid, WlanId);

	return ret;
	
	/* delete by weiay for multi radio
	
	int ret = -1;
	char nas_id[NAS_IDENTIFIER_NAME];//zhanglei add
	unsigned int nas_id_len = 0;//zhanglei add
	if(AC_WTP[WtpID]->BindingWlanCount >= L_BSS_NUM)
	{
		return WTP_OVER_MAX_BSS_NUM;
	}


	if(AC_WLAN[WlanId] == NULL)
	{
		wid_syslog_debug_debug("*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	if(AC_WLAN[WlanId]->Status == 0)
	{
		//return WLAN_BE_ENABLE;
	}
	//added by weiay 20080630 match result
	if(AC_WLAN[WlanId]->Wlan_Ifi != NULL)
	{
		if(AC_WTP[WtpID]->BindingSystemIndex != -1)
		{
			//Match operate
			struct ifi * wlan_ifi = AC_WLAN[WlanId]->Wlan_Ifi;
			while(wlan_ifi != NULL)
			{
				//printf("*** wlan index is %d\n",wlan_ifi->ifi_index);
				if(AC_WTP[WtpID]->BindingSystemIndex == wlan_ifi->ifi_index)
				{
					if(wlan_ifi->nas_id_len > 0){
						nas_id_len = wlan_ifi->nas_id_len;//zhanglei add
						memcpy(nas_id,wlan_ifi->nas_id,NAS_IDENTIFIER_NAME);//zhanglei add
					}
					break;
				}
				wlan_ifi = wlan_ifi->ifi_next;
			}
			
			if(wlan_ifi == NULL)
			{
				wid_syslog_debug_debug("*** wtp binding interface doesn't match with wlan binding interface **\n");
				return WTP_WLAN_BINDING_NOT_MATCH;
			}
			//match operate end
		}
		else
		{
			return WTP_IF_NOT_BE_BINDED;
		}
	}
	else
	{
		return Wlan_IF_NOT_BE_BINDED;
	}
	
	//added end
		
	struct wlanid *wlan_id;
	struct wlanid *wlan_id_next;
	wlan_id = (struct wlanid*)malloc(sizeof(struct wlanid));
	
	wlan_id->wlanid= WlanId;
	wlan_id->next = NULL;
	wid_syslog_debug_debug("*** wtp binding wlan id  is %d*\n", wlan_id->wlanid);
	
	if(AC_WTP[WtpID]->Wlan_Id== NULL){
		
		AC_WTP[WtpID]->Wlan_Id = wlan_id ;
		//AC_WTP[WtpID]->Wlan_Id->next= NULL;
		AC_WTP[WtpID]->isBinddingWlan = 1;
		AC_WTP[WtpID]->BindingWlanCount++;
		wid_syslog_debug_debug("*** wtp id:%d binding first wlan id:%d  \n",WtpID,WlanId);
	}else{
	
		wlan_id_next = AC_WTP[WtpID]->Wlan_Id;
		while(wlan_id_next != NULL)
		{	
			if(wlan_id_next->wlanid == WlanId)
			{
				wid_syslog_debug_debug("warnning you have binding this wlan id,please do not binding this again");
				free(wlan_id);
				return 0;
			}
			wlan_id_next = wlan_id_next->next;
		}

		wlan_id_next = AC_WTP[WtpID]->Wlan_Id;
		while(wlan_id_next->next!= NULL)
		{	
			wlan_id_next = wlan_id_next->next;//insert element int tail
		}
		
		wlan_id_next->next= wlan_id;
		AC_WTP[WtpID]->BindingWlanCount++;
		//wlan_id_next->next= NULL;
		wid_syslog_debug_debug("*** wtp id:%d binding more wlan id:%d  \n",WtpID,WlanId);
	}
	
	int k1 = 0, RadioID = 0, GID = 0;	
	int RCount = RadioNumCheck(WtpID);	
	for(RadioID = 0;RadioID < RCount; RadioID++)
	for(k1=0;k1<L_BSS_NUM;k1++){
		if(AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1] == NULL){
			//printf("BSSIndex:%d\n",k1);
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1] = (WID_BSS*)malloc(sizeof(WID_BSS));
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSSID = (unsigned char*)malloc(6);
			memset(AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSSID,0,6);
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->bss_max_allowed_sta_num=64;//xm add
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->WlanID = WlanId;
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->Radio_G_ID = AC_WTP[WtpID]->WTP_Radio[RadioID]->Radio_G_ID;
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->Radio_L_ID = RadioID;
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->State = 0;
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->band_width = 25;
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSSIndex = (AC_WTP[WtpID]->WTP_Radio[RadioID]->Radio_G_ID)*L_BSS_NUM+k1;
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSS_IF_POLICY = AC_WLAN[WlanId]->wlan_if_policy;
			memset(AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSS_IF_NAME, 0, ETH_IF_NAME_LEN);
			memset(AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->nas_id, 0, NAS_IDENTIFIER_NAME);//zhanglei add
			AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->nas_id_len = 0;//zhanglei add
			if(nas_id_len > 0){
				AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->nas_id_len = nas_id_len;//zhanglei add
				memcpy(AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->nas_id, nas_id, NAS_IDENTIFIER_NAME);//zhanglei add
			}
			AC_BSS[AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSSIndex] = AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1];
			GID = AC_WTP[WtpID]->WTP_Radio[RadioID]->Radio_G_ID;
			AC_RADIO[GID]->BSS[k1] = AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1];
			AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][RadioID] = AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSSIndex;

			if(AC_WLAN[WlanId]->wlan_if_policy == BSS_INTERFACE)
			{		
			
				ret = Create_BSS_L3_Interface(AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[k1]->BSSIndex);
				
				if(ret < 0)
				{
					WID_DELETE_WLAN_APPLY_WTP(WtpID, WlanId);
					return WLAN_CREATE_L3_INTERFACE_FAIL;
				}

			}
						
			break;
		}
	}	
		if(k1 == L_BSS_NUM){
			return WTP_OVER_MAX_BSS_NUM;
	}
		
	return 0;

	*/
		
}
int WID_DELETE_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId)
{
	int radioid = WtpID*L_RADIO_NUM;
	
	int ret = WID_DELETE_WLAN_APPLY_RADIO(radioid, WlanId);

	return ret;


	/*
	int ret = 0;
	if(AC_WLAN[WlanId] == NULL)
	{
		wid_syslog_debug_debug("*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}

	struct wlanid *wlan_id;
	struct wlanid *wlan_id_next;
	//print info
	wlan_id = AC_WTP[WtpID]->Wlan_Id;
	wid_syslog_debug_debug("**!!!!!!!!!!!! list start !!!!!!!!!!!!!!!***\n");
	while(wlan_id != NULL)
	{
		//printf("**!!!!!!!!!!!! list start !!!!!!!!!!!!!!!***\n");
		wid_syslog_debug_debug("**wlan id is:%d ***\n",wlan_id->wlanid);
		wlan_id = wlan_id->next;
		//printf("**!!!!!!!!!!!! list end !!!!!!!!!!!!!!!***\n");
		
	}
	wid_syslog_debug_debug("**!!!!!!!!!!!! list end !!!!!!!!!!!!!!!***\n");

	wlan_id_next = AC_WTP[WtpID]->Wlan_Id;

	if(AC_WTP[WtpID]->isBinddingWlan == 0)
	{
		return INTERFACE_NOT_BE_BINDED;
	}
	else if(wlan_id_next->wlanid == WlanId)
	{
		ret = delete_wlan_bss(WtpID,WlanId);
		if(ret == -1)
		{
			return BSS_BE_ENABLE;
		}
		AC_WTP[WtpID]->Wlan_Id = wlan_id_next->next;
		free(wlan_id_next);
		wlan_id_next = NULL;
		AC_WTP[WtpID]->BindingWlanCount--;
		if(AC_WTP[WtpID]->Wlan_Id == NULL)
		{
			AC_WTP[WtpID]->isBinddingWlan = 0;
			AC_WTP[WtpID]->BindingWlanCount = 0;
		}
		return 0;
	}

	else
	{
		while(wlan_id_next->next != NULL)
		{	
			if(wlan_id_next->next->wlanid == WlanId)
			{
				ret = delete_wlan_bss(WtpID,WlanId);
				if(ret == -1)
				{
					return BSS_BE_ENABLE;
				}
				wlan_id = wlan_id_next->next;
				wlan_id_next->next = wlan_id_next->next->next;
				free(wlan_id);
				wlan_id = NULL;
				AC_WTP[WtpID]->BindingWlanCount--;
				return 0;
			}
			wlan_id_next = wlan_id_next->next;
		}
	}

	return INTERFACE_NOT_BE_BINDED;
	
	*/
}
/*fengwenchao add 20120509 for onlinebug-271*/
int DELETE_WLAN_CHECK_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId)
{
	int ebr_id = 0;
	int WtpID = RadioId/L_RADIO_NUM;
	int local_radioid = RadioId%L_RADIO_NUM;
	struct wlanid *wlan_id;

	if(AC_WLAN[WlanId] == NULL)
	{
		wid_syslog_warning("  %s you binding wlan %d does not exist **\n",__func__,WlanId);
		return WLAN_ID_NOT_EXIST;
	}
	
	wlan_id = AC_RADIO[RadioId]->Wlan_Id;	

	while(wlan_id != NULL)
	{
		if(wlan_id->wlanid == WlanId)
		{
			if(check_whether_in_ebr(vrrid,WtpID,local_radioid,wlan_id->wlanid,&ebr_id))
			{
				wid_syslog_warning("<error> %s check interface in ebr \n",__func__);
				return RADIO_IN_EBR;
			}
		}
		wlan_id = wlan_id->next;	
	}	
	return 0;
}
/*fengwenchao add end*/
int WID_DELETE_WLAN_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId)
{
	int ret = 0;
	int ebr_id = 0;
	int WtpID = RadioId/L_RADIO_NUM;
	int local_radioid = RadioId%L_RADIO_NUM;
	//msgq msg;
	//struct msgqlist *elem;
	if(AC_WLAN[WlanId] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}

	struct wlanid *wlan_id;
	struct wlanid *wlan_id_next;
	wlan_id = AC_RADIO[RadioId]->Wlan_Id;
	
	#if 0
	if((AC_WLAN[WlanId])&&(AC_RADIO[RadioId])&&(!strncasecmp(AC_RADIO[RadioId]->br_ifname[WlanId],"ebr",3)))
	{	
		memset(ebrname,0,ETH_IF_NAME_LEN);
		memcpy(ebrname,AC_RADIO[RadioId]->br_ifname[WlanId],strlen(AC_RADIO[RadioId]->br_ifname[WlanId]));
		wid_syslog_debug_debug(WID_DEFAULT,"ebrname =  %s \n",ebrname);
		 ret = Check_Interface_Exist(ebrname,&quitreason);
		 if(ret == 0)
		 	return RADIO_IN_EBR;			
	}
	#endif

	/*fengwenchao add end*/
	if(check_whether_in_ebr(vrrid,WtpID,local_radioid,WlanId,&ebr_id))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"<error> %s check interface in ebr \n",__func__);
		return RADIO_IN_EBR;
	}	
	wid_syslog_debug_debug(WID_DEFAULT,"**!!!!!!!!!!!! list start !!!!!!!!!!!!!!!***\n");
//	printf("**!!!!!!!!!!!! list start !!!!!!!!!!!!!!!***\n");
	while(wlan_id != NULL)
	{
		//printf("**!!!!!!!!!!!! list start !!!!!!!!!!!!!!!***\n");

		wid_syslog_debug_debug(WID_DEFAULT,"**wlan id is:%d ***\n",wlan_id->wlanid);
		wid_syslog_debug_debug(WID_DEFAULT,"vrrid = %d WtpID = %d ,  local_radioid   = %d  wlan_id->wlanid = %d \n",vrrid,WtpID,local_radioid,wlan_id->wlanid);


		wlan_id = wlan_id->next;
		//printf("**!!!!!!!!!!!! list end !!!!!!!!!!!!!!!***\n");
		
	}
	wid_syslog_debug_debug(WID_DEFAULT,"**!!!!!!!!!!!! list end !!!!!!!!!!!!!!!***\n");
//	printf("**!!!!!!!!!!!! list end !!!!!!!!!!!!!!!***\n");

	wlan_id_next = AC_RADIO[RadioId]->Wlan_Id;

	if(AC_RADIO[RadioId]->isBinddingWlan == 0)
	{
		return INTERFACE_NOT_BE_BINDED;
	}
	else if(wlan_id_next->wlanid == WlanId)
	{
			if((AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid] != 0)&&(AC_BSS[AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]] != NULL))
			{		
				int BSSIndex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
				
				
				if((AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->State == 1))
				{
					return BSS_BE_ENABLE;
				}
				#if 0
				if(AC_WTP[WtpID]->WTPStat != 5){
					AsdWsm_BSSOp(BSSIndex, WID_DEL, 1);
					memset((char*)&msg, 0, sizeof(msg));
					msg.mqid = WtpID%THREAD_NUM+1;
					msg.mqinfo.WTPID = WtpID;
					msg.mqinfo.type = CONTROL_TYPE;
					msg.mqinfo.subtype = WLAN_S_TYPE;
					msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_DEL;
					msg.mqinfo.u.WlanInfo.WLANID = WlanId;
					msg.mqinfo.u.WlanInfo.Radio_L_ID = local_radioid;

					msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
					elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
					if(elem == NULL){
						wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
						perror("malloc");
						return 0;
					}
					memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
					elem->next = NULL;
					memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
					WID_INSERT_CONTROL_LIST(WtpID, elem);
				}
				#endif
			}
		ret = delete_wlan_bss_by_radioId(RadioId,WlanId);
		if(ret == -1)
		{
			return BSS_BE_ENABLE;
		}
		else if(ret == BSS_NOT_EXIST)  //fengwenchao add 20120131 for TESTBED-17
		{
			return BSS_NOT_EXIST;
		}
		AC_RADIO[RadioId]->Wlan_Id = wlan_id_next->next;
		free(wlan_id_next);
		wlan_id_next = NULL;
		AC_RADIO[RadioId]->BindingWlanCount--;
		if(AC_RADIO[RadioId]->Wlan_Id == NULL)
		{
			AC_RADIO[RadioId]->isBinddingWlan = 0;
			AC_RADIO[RadioId]->BindingWlanCount = 0;
		}
		return 0;
	}

	else
	{
		while(wlan_id_next->next != NULL)
		{	
			if(wlan_id_next->next->wlanid == WlanId)
			{
				ret = delete_wlan_bss_by_radioId(RadioId,WlanId);
				if(ret == -1)
				{
					return BSS_BE_ENABLE;
				}
				else if(ret == BSS_NOT_EXIST)  //fengwenchao add 20120131 for TESTBED-17
				{
					return BSS_NOT_EXIST;
				}				
				wlan_id = wlan_id_next->next;
				wlan_id_next->next = wlan_id_next->next->next;
				free(wlan_id);
				wlan_id = NULL;
				AC_RADIO[RadioId]->BindingWlanCount--;
				return 0;
			}
			wlan_id_next = wlan_id_next->next;
		}
	}

	return INTERFACE_NOT_BE_BINDED;

}
int WID_DISABLE_WLAN_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId)
{
	int ifind = 0;
			//wid_dbus_trap_wtp_wireless_interface_down(WtpID);
	int j = 0;
	int ret = 0;
	int WtpID = RadioId/L_RADIO_NUM;
	int local_radioid = RadioId%L_RADIO_NUM;
	msgq msg;
	
	if(gtrapflag>=4){
			wid_dbus_trap_wtp_wireless_interface_down(WtpID);
		}
	if(AC_WLAN[WlanId] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	struct wlanid *wlan_id_next;

	wlan_id_next = AC_RADIO[RadioId]->Wlan_Id;

	if(AC_RADIO[RadioId]->isBinddingWlan == 0)
	{
		return INTERFACE_NOT_BE_BINDED;
	}
	else if(wlan_id_next->wlanid == WlanId)
	{
		ifind = 1;

		for(j=0;j<L_BSS_NUM;j++)
		{
			if(AC_RADIO[RadioId]->BSS[j] != NULL)
			{
				if(AC_RADIO[RadioId]->BSS[j]->WlanID == WlanId)
				{
					if(AC_RADIO[RadioId]->BSS[j]->State != 0)
					{
						ret = 1;
					}
				}
			}

		}
	
		if(ret == 0)
		{
			return 0;
		}
	}
	else
	{
		while(wlan_id_next->next != NULL)
		{	
			if(wlan_id_next->next->wlanid == WlanId)
			{
				ifind = 1;
		
				ret =0;
				
				for(j=0;j<L_BSS_NUM;j++)
				{
					if(AC_RADIO[RadioId]->BSS[j] != NULL)
					{
						if(AC_RADIO[RadioId]->BSS[j]->WlanID == WlanId)
						{
							if(AC_RADIO[RadioId]->BSS[j]->State != 0)
							{
								ret = 1;
							}
						}
					}

				}
				
				if(ret == 0)
				{
					return 0;
				}

			}
			wlan_id_next = wlan_id_next->next;
		}
	}

	if(gtrapflag>=4){
			wid_dbus_trap_wtp_wireless_interface_down(WtpID);
		}
	if(ifind == 0)
	{
		return WTP_WLAN_BINDING_NOT_MATCH;
	}
	printf("*** ///////////// **\n");

	if((AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid] != 0)&&(check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]))&&(AC_BSS[AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]] != NULL))
	{		
		printf("*** 2222222222222 **\n");
		int BSSIndex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
		if(!check_bssid_func(BSSIndex)){
			wid_syslog_err("<error>%s\n",__func__);
			return BSS_NOT_EXIST;
		}else{}
		if((AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->State == 0)){
			return 0;
		}
		if((AC_BSS[BSSIndex] != NULL)&&(AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->CMD->radiowlanid[local_radioid][WlanId] == 2)&&(AC_BSS[BSSIndex]->WlanID == WlanId))
		{
			AsdWsm_BSSOp(BSSIndex, WID_DEL, 1);
			wid_update_bss_to_wifi(BSSIndex,WtpID,0);/*resolve send capwap message,before ath be created*/	
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WtpID%THREAD_NUM+1;
			msg.mqinfo.WTPID = WtpID;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WLAN_S_TYPE;
			msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_DEL;
			msg.mqinfo.u.WlanInfo.WLANID = WlanId;
			msg.mqinfo.u.WlanInfo.Radio_L_ID = local_radioid;//zhanglei wait for M-radio
			msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

			if(gtrapflag>=4){
				wid_dbus_trap_wtp_wireless_interface_down(WtpID);
				}

			int type = 1;//manual
			int flag = 0;//disable
			if(gtrapflag>=4){
				wid_dbus_trap_ap_ath_error(WtpID,local_radioid,WlanId,type,flag);
				}

			/*fengwenchao add 20121213 for onlinebug-767*/
			AC_WTP[WtpID]->CMD->wlanCMD -= 1;
			AC_WTP[WtpID]->CMD->radiowlanid[local_radioid][WlanId] = 0;
			if(AC_BSS[BSSIndex] != NULL){
				AC_BSS[BSSIndex]->State = 0;
				if(AC_BSS[BSSIndex]->BSSID != NULL){
					memset(AC_BSS[BSSIndex]->BSSID, 0, 6);
				}
				if(AC_BSS[BSSIndex]->BSS_TUNNEL_POLICY == CW_802_IPIP_TUNNEL)
				{
					delete_ipip_tunnel(BSSIndex);
				}
			}			
			/*fengwenchao add end*/	
			//continue;
		}
		else if((AC_BSS[BSSIndex] == NULL)||(AC_WTP[WtpID] == NULL))
		{
			AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid] = 0;
			//continue;
		}
		else if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->CMD->radiowlanid[local_radioid][WlanId] == 1))
		{
			AC_WTP[WtpID]->CMD->wlanCMD -= 1;
			AC_WTP[WtpID]->CMD->radiowlanid[local_radioid][WlanId] = 0;
		}	
	}
	
	if((check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]))&&(AC_BSS[AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]] != NULL))
	{
  		AC_BSS[AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]]->downcount++;
	}
	//AC_RADIO[RadioId]->downcount++;
	return 0;


}
int WID_ENABLE_WLAN_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId)
{
	msgq msg2;
	msgq msg4 ;
//	struct msgqlist *elem4 = NULL; 
	char *command = NULL;
	char *ath_str = NULL;
	struct msgqlist *elem2;
	WTPQUITREASON quitreason = WTP_INIT;
	int j=0;
	int ret2 = 0;
	int WtpID = RadioId/L_RADIO_NUM;
	msgq msg;
	int local_radioid = RadioId%L_RADIO_NUM;
	WID_BSS *BSS;
	char buf[DEFAULT_LEN];
	unsigned char pcy = 0;
	struct wds_bssid *wds = NULL;
	if(gtrapflag>=4){
			wid_dbus_trap_wtp_wireless_interface_down_clear(WtpID);
		}
	if(AC_WTP[WtpID]->WTPStat != 5)
	{
		return WTP_NOT_IN_RUN_STATE;
	}

	if(AC_WLAN[WlanId]->Status == 1)
	{
		return WLAN_BE_DISABLE;
	}
	
	if (AC_WLAN[WlanId]->want_to_delete == 1)		/* Huangleilei add for ASXXZFI-1622 */
	{
		wid_syslog_err("operator want to delete wlan %d", WlanId);
		return 0;
	}
	int ifind = 0;
	
	struct wlanid *wlan_id_next;
	
	wlan_id_next = AC_RADIO[RadioId]->Wlan_Id;

	if(AC_RADIO[RadioId]->isBinddingWlan == 0)
	{
		return INTERFACE_NOT_BE_BINDED;
	}
	else if(wlan_id_next->wlanid == WlanId)
	{
		//printf("*** 001**\n");
		ifind = 1;

		for(j=0;j<L_BSS_NUM;j++)
		{
			if(AC_RADIO[RadioId]->BSS[j] != NULL)
			{
				if(AC_RADIO[RadioId]->BSS[j]->WlanID == WlanId)
				{
					if(AC_RADIO[RadioId]->BSS[j]->State != 1)
					{
						ret2 = 1;
					}
					BSS = AC_RADIO[RadioId]->BSS[j]; 
				}
			}

		}
	
		if(ret2 == 0)
		{
			return VALUE_IS_NONEED_TO_CHANGE;
		}
	}
	else
	{
		//printf("*** 002 **\n");
		while(wlan_id_next->next != NULL)
		{	
			if(wlan_id_next->next->wlanid == WlanId)
			{
				ifind = 1;
		
				ret2 =0;
				
				for(j=0;j<L_BSS_NUM;j++)
				{
					if(AC_RADIO[RadioId]->BSS[j] != NULL)
					{
						if(AC_RADIO[RadioId]->BSS[j]->WlanID == WlanId)
						{
							if(AC_RADIO[RadioId]->BSS[j]->State != 1)
							{
								ret2 = 1;
							}
							BSS = AC_RADIO[RadioId]->BSS[j];
						}
					}

				}
				
				if(ret2 == 0)
				{
					return VALUE_IS_NONEED_TO_CHANGE;
				}

			}
			wlan_id_next = wlan_id_next->next;
		}
	}
	//printf("*** 003 **\n");

	if(ifind == 0)
	{
		return WTP_IS_NOT_BINDING_WLAN_ID;
	}
	/*check wtp wlan binding interface match*/
	int if_index = 0;
	if_index = AC_WTP[WtpID]->BindingSystemIndex; 
	struct ifi *wlan_interface = NULL;
	wlan_interface = AC_WLAN[WlanId]->Wlan_Ifi;
	ifind = 0;
	while(wlan_interface != NULL)
	{
		if(wlan_interface->ifi_index == if_index)
		{
//			printf("wtp wlan binding interface match\n");
			ifind = 1;
			break;
		}
		wlan_interface = wlan_interface->ifi_next;
	}
	if(ifind == 0)
	{
		wid_syslog_warning("<warning>%s,%d.WTP_WLAN_BINDING_NOT_MATCH.\n",__func__,__LINE__);
		//return WTP_WLAN_BINDING_NOT_MATCH;		
	}

	int ret;
	char ifiname[ETH_IF_NAME_LEN-1];
	//printf("*** ############# **\n");

	if((AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid] != 0)&&(check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]))&&(AC_BSS[AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]] != NULL))
	{		
		int bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];

		if((check_bssid_func(bssindex))&&(AC_BSS[bssindex]->State == 1))
		{
			return VALUE_IS_NONEED_TO_CHANGE;
		}

		if((check_bssid_func(bssindex))&&(AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE))
		{
			memset(ifiname,0,ETH_IF_NAME_LEN-1);
			if(local)
				snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,WtpID,local_radioid,WlanId);
			else
				snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,WtpID,local_radioid,WlanId);
			ret = Check_Interface_Config(ifiname,&quitreason);
			
			if(ret != 0)
			{
				return L3_INTERFACE_ERROR;
			}
		}
#if 0
		if(AC_WLAN[WlanId]->EncryptionType == WEP)
		{
			if(AC_BSS[bssindex]->keyindex == 0)//means should be wep bss but not now
			{
				int i = 0;
				int result = 0;
				for(i=0;i<WTP_WEP_NUM;i++)
				{
					if(AC_WTP[WtpID]->wep_flag[i] == bssindex)
					{
						result = 1;// bssindex is in the wep flag
						break;
					}
				}

				if(result == 0)// bssindex is not in the wep flag
				{
					for(i=0;i<WTP_WEP_NUM;i++)
					{
						if(AC_WTP[WtpID]->wep_flag[i] == 0)
						{
							AC_WTP[WtpID]->wep_flag[i] = bssindex;
							AC_BSS[bssindex]->keyindex = i+1;
							result = 1;//put bssindex in the wep flag
							break;
						}
					}

					if(result == 0)//no room to put the wep wlan 
					{
						return WTP_WEP_NUM_OVER;
					}
				}
			}
			else
			{
				
			}
		}
#endif
		AC_WTP[WtpID]->CMD->wlanCMD += 1;
		AC_WTP[WtpID]->CMD->radiowlanid[local_radioid][WlanId] = 1;
	}
		

	memset((char*)&msg, 0, sizeof(msg));
	msg.mqid = WtpID%THREAD_NUM+1;
	msg.mqinfo.WTPID = WtpID;
	msg.mqinfo.type = CONTROL_TYPE;
	msg.mqinfo.subtype = WLAN_S_TYPE;
	msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
	msg.mqinfo.u.WlanInfo.WLANID = WlanId;
	msg.mqinfo.u.WlanInfo.Radio_L_ID = local_radioid;//zhanglei wait for M-radio

	msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanId]->HideESSid;
	memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
	memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanId]->WlanKey,DEFAULT_LEN);
	msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanId]->KeyLen;
	msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanId]->SecurityType;
	msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanId]->SecurityIndex;
	msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanId]->asic_hex;/* 0 asic; 1 hex*/
	msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanId]->Roaming_Policy; 		/*Roaming (1 enable /0 disable)*/
	memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
	//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanId]->ESSID,ESSID_LENGTH);
	memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanId]->ESSID,strlen(AC_WLAN[WlanId]->ESSID));
	msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
	
	if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
		wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
		perror("msgsnd");
	}

	
	//wds state
	if((check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]))&&(AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->WDSStat == WDS_ANY))
	{
		msg2.mqid = WtpID%THREAD_NUM +1;
		msg2.mqinfo.WTPID = WtpID;;
		msg2.mqinfo.type = CONTROL_TYPE;
		msg2.mqinfo.subtype = WDS_S_TYPE;
		msg2.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_ENABLE;
		msg2.mqinfo.u.WlanInfo.WLANID = WlanId;
		msg2.mqinfo.u.WlanInfo.Radio_L_ID = local_radioid;
		
		elem2 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem2 == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem2->mqinfo), 0, sizeof(msgqdetail));
		elem2->next = NULL;
		memcpy((char*)&(elem2->mqinfo),(char*)&(msg2.mqinfo),sizeof(msg2.mqinfo));
		WID_INSERT_CONTROL_LIST(WtpID, elem2);
	}	
	//wds state end
	else{
		if(BSS->WDSStat == WDS_SOME){
			wds = BSS->wds_bss_list;
			while(wds != NULL){
				memset(buf,0,DEFAULT_LEN);
				sprintf(buf,"/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",BSS->Radio_L_ID,WlanId,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
//				printf("/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",BSS->Radio_L_ID,WlanId,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
				wid_radio_set_extension_command(WtpID,buf);
				wds = wds->next;
			}
		}
	}
		if((AC_WLAN[WlanId]->SecurityType == IEEE8021X)||(AC_WLAN[WlanId]->SecurityType ==WPA_E)||(AC_WLAN[WlanId]->SecurityType ==WPA2_E)||(AC_WLAN[WlanId]->SecurityType ==MD5))
		{
			msgq msg3;
//			struct msgqlist *elem3;
			msg3.mqid = WtpID%THREAD_NUM +1;
			msg3.mqinfo.WTPID = WtpID;
			msg3.mqinfo.type = CONTROL_TYPE;
			msg3.mqinfo.subtype = WTP_S_TYPE;
			msg3.mqinfo.u.WtpInfo.Wtp_Op = WTP_FLOW_CHECK;
			msg3.mqinfo.u.WlanInfo.WLANID = WlanId;
			msg3.mqinfo.u.WlanInfo.Radio_L_ID = local_radioid;
			msg3.mqinfo.u.WlanInfo.flow_check = AC_WLAN[WlanId]->flow_check;
			msg3.mqinfo.u.WlanInfo.no_flow_time = AC_WLAN[WlanId]->no_flow_time;
			msg3.mqinfo.u.WlanInfo.limit_flow = AC_WLAN[WlanId]->limit_flow;
			if(AC_WTP[WtpID]->WTPStat == 5){	
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg3, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}//delete unuseful cod
		/*else{
				elem3 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem3 == NULL){			
					wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
			
				memset((char*)&(elem3->mqinfo), 0, sizeof(msgqdetail));
				elem3->next = NULL;
				memcpy((char*)&(elem3->mqinfo),(char*)&(msg3.mqinfo),sizeof(msg3.mqinfo));
				WID_INSERT_CONTROL_LIST(WtpID, elem3);
			}*/
		}
		//weichao add 
			if(AC_WLAN[WlanId]->Status == 0){
			command = (char *)malloc(sizeof(char)*100);
			if(NULL == command)
			{							
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				return 0;
			}
			ath_str = (char *)malloc(sizeof(char)*20);
			if(NULL == ath_str)
			{
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				CW_FREE_OBJECT(command);
				return 0;
			}
			memset(command,0,100);
			memset(ath_str,0,20);
			memset(&msg4,0,sizeof(msg4));
			msg4.mqid = WtpID%THREAD_NUM +1;
			msg4.mqinfo.WTPID = WtpID;
			msg4.mqinfo.type = CONTROL_TYPE;
			msg4.mqinfo.subtype = WTP_S_TYPE;
			msg4.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;
			
			sprintf(ath_str,"ath.%d-%d",local_radioid,WlanId);
			sprintf(command,"ifconfig %s down;iwpriv %s inact %u;ifconfig %s up",ath_str,ath_str,AC_WLAN[WlanId]->ap_max_inactivity,ath_str);
			memcpy(msg4.mqinfo.u.WtpInfo.value, command, strlen(command));
			if(AC_WTP[WtpID]->WTPStat == 5){	
					if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg4, sizeof(msg.mqinfo), 0) == -1){
						wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
						perror("msgsnd");
					}
				}//delete unuseful cod
			/*else{
				elem4 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem4 == NULL){			
					wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				
				memset((char*)&(elem4->mqinfo), 0, sizeof(msgqdetail));
				elem4->next = NULL;
				memcpy((char*)&(elem4->mqinfo),(char*)&(msg4.mqinfo),sizeof(msg4.mqinfo));
				WID_INSERT_CONTROL_LIST(WtpID, elem4);
			}*/
			free(command);
			command = NULL;
			free(ath_str);
			ath_str = NULL;
			}
	if(gtrapflag>=4){
			wid_dbus_trap_wtp_wireless_interface_down_clear(WtpID);
		}
	int type = 1;//manual
	int flag = 1;//enable
	if(gtrapflag>=4){
		wid_dbus_trap_ap_ath_error(WtpID,local_radioid,WlanId,type,flag);
	}
	unsigned int bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
	if((check_bssid_func(bssindex))&&(AC_BSS[bssindex] != NULL))
	{
		AC_BSS[bssindex]->upcount++;

		/* zhangshu add , 2011-1-7 */
		//unsigned int bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
		wid_syslog_debug_debug(WID_DEFAULT,"!@#$ AC_BSS[%d]->traffic_limit_able = %d\n",bssindex,AC_BSS[bssindex]->traffic_limit_able);
		if(AC_BSS[bssindex]->traffic_limit_able == 1){
		    WID_Save_Traffic_Limit(bssindex, WtpID);
		}
	}
	//AC_RADIO[RadioId]->upcount++;
	if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->multi_user_optimize_switch == 1))
	{
		char wlanid =AC_BSS[bssindex]->WlanID;
		int radioid = AC_BSS[bssindex]->Radio_G_ID;
		muti_user_optimize_switch(wlanid,radioid,1);
		
	}

	/* send eap switch & mac to ap ,zhangshu add 2010-10-28 */
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if((AC_WLAN[WlanId]->eap_mac_switch==1)&&(AC_WLAN[WlanId]->wlan_if_policy==NO_INTERFACE)&&(AC_BSS[bssindex]!=NULL)&&(AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE))
	{
	    sprintf(apcmd,"set_eap_mac ath.%d-%d %s",AC_RADIO[RadioId]->Radio_L_ID,WlanId,AC_WLAN[WlanId]->eap_mac);
	}
	else
	{
	    sprintf(apcmd,"set_eap_mac ath.%d-%d 0",AC_RADIO[RadioId]->Radio_L_ID,WlanId);
	}
	wid_syslog_debug_debug(WID_DEFAULT,"Radio enable Wlan: set eap mac cmd %s\n",apcmd);
	wid_radio_set_extension_command(WtpID,apcmd);
	/* end */

	if((check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]))&&((AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->unicast_sw == 1)\
		||(AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->muti_bro_cast_sw == 1)||(AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->muti_rate != 10))){
		int bssindex1 = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid];
		if((1 == AC_BSS[bssindex1]->muti_bro_cast_sw)&&(1 == AC_BSS[bssindex1]->unicast_sw)){
			pcy = 3;
		}else if(1 == AC_BSS[bssindex1]->muti_bro_cast_sw){
			pcy = 2;
		}else if(1 == AC_BSS[bssindex1]->unicast_sw){
			pcy = 1;
		}else{
			pcy = 0;
		}
		if(AC_BSS[bssindex1]->wifi_sw == 1){
			pcy = pcy|0x4;
		}else{
			pcy = pcy&~0x4;
		}
		setWtpUniMutiBroCastIsolation(WtpID,local_radioid,WlanId,pcy);
		wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,WtpID,local_radioid,WlanId,(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]));
	}
	if((check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]))&&(AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->noResToStaProReqSW == 1)){
		setWtpNoRespToStaProReq(WtpID,local_radioid,WlanId,AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->noResToStaProReqSW);
		wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,WtpID,local_radioid,WlanId,(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]));
	}
	if((check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]))&&(AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->muti_rate != 10)){
		setWtpUniMutiBroCastRate(WtpID,local_radioid,WlanId,AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid])]->muti_rate);
		wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,WtpID,local_radioid,WlanId,(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][local_radioid]));
	}
	
	return 0;		


}

int WID_DISABLE_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId)
{
	int ifind = 0;
	int j = 0;
	int ret = 0;	
	msgq msg;
	if(AC_WLAN[WlanId] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}
	
	struct wlanid *wlan_id_next;

	wlan_id_next = AC_WTP[WtpID]->WTP_Radio[0]->Wlan_Id;

	if(AC_WTP[WtpID]->WTP_Radio[0]->isBinddingWlan == 0)
	{
		return INTERFACE_NOT_BE_BINDED;
	}
	else if(wlan_id_next->wlanid == WlanId)
	{
		ifind = 1;
		
		for(j=0;j<L_BSS_NUM;j++)
		{
			if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[j] != NULL)
			{
				if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[j]->WlanID == WlanId)
				{
					if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[j]->State != 0)
					{
						ret = 1;
					}
				}
			}
		}
	
		if(ret == 0)
		{
			return 0;
		}
	}
	else
	{
		while(wlan_id_next->next != NULL)
		{	
			if(wlan_id_next->next->wlanid == WlanId)
			{
				ifind = 1;
		
				ret =0;

				for(j=0;j<L_BSS_NUM;j++)
				{
					if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[j] != NULL)
					{
						if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[j]->WlanID == WlanId)
						{
							if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[j]->State != 0)
							{
								ret = 1;
							}
						}
					}
				}					
		
				if(ret == 0)
				{
					return 0;
				}

			}
			wlan_id_next = wlan_id_next->next;
		}
	}

	if(ifind == 0)
	{
		return WTP_WLAN_BINDING_NOT_MATCH;
	}
	
	//added 
	int i=0;
	if(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0] != 0)
	{
		unsigned int BSSIndex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0];
		if(!check_bssid_func(BSSIndex)){
			wid_syslog_err("<error>%s\n",__func__);
			return BSS_NOT_EXIST;
		}else{
		}
		if((AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->State == 0)){
			//continue;
		}
		if((AC_BSS[BSSIndex] != NULL)&&(AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->CMD->radiowlanid[0][WlanId] == 2)&&(AC_BSS[BSSIndex]->WlanID == WlanId))
		{
			AsdWsm_BSSOp(BSSIndex, WID_DEL, 1);
			wid_update_bss_to_wifi(BSSIndex,WtpID,0);/*resolve send capwap message,before ath be created*/	
			memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WtpID%THREAD_NUM+1;
				msg.mqinfo.WTPID = WtpID;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WLAN_S_TYPE;
			msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_DEL;
			msg.mqinfo.u.WlanInfo.WLANID = WlanId;
			msg.mqinfo.u.WlanInfo.Radio_L_ID = 0;//zhanglei wait for M-radio
			msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0];
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
			
			int type = 1;//manual
			int flag = 0;//disable
			if(gtrapflag>=4){
				wid_dbus_trap_ap_ath_error(WtpID,i,WlanId,type,flag);
				}
			/*fengwenchao add 20121213 for onlinebug-767*/
			AC_WTP[WtpID]->CMD->wlanCMD -= 1;
			AC_WTP[WtpID]->CMD->radiowlanid[0][WlanId] = 0;
			if(AC_BSS[BSSIndex] != NULL){
				AC_BSS[BSSIndex]->State = 0;
				if(AC_BSS[BSSIndex]->BSSID != NULL){
					memset(AC_BSS[BSSIndex]->BSSID, 0, 6);
				}
				if(AC_BSS[BSSIndex]->BSS_TUNNEL_POLICY == CW_802_IPIP_TUNNEL)
				{
					delete_ipip_tunnel(BSSIndex);
				}
			}			
			/*fengwenchao add end*/			
			//AC_WTP[WtpID]->CMD->wlanCMD += 1;
			//AC_WTP[WtpID]->CMD->wlanid[WlanId] = 3;

		}
		else if((AC_BSS[BSSIndex] == NULL)||(AC_WTP[WtpID] == NULL))
		{
			AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0] = 0;

		}
		else if((AC_WTP[WtpID] != NULL)&&(AC_WTP[WtpID]->CMD->radiowlanid[0][WlanId] == 1))
		{
			AC_WTP[WtpID]->CMD->wlanCMD -= 1;
			AC_WTP[WtpID]->CMD->radiowlanid[0][WlanId] = 0;
		}
	}
	

	return 0;
}

int WID_WTP_TUNNEL_MODE_CHECK(int WtpID){
	int j,k;
	int check1=0, check2=0;
	for(j=0; j<AC_WTP[WtpID]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
	{
		for(k=0; k<L_BSS_NUM; k++)
			if(AC_WTP[WtpID]->WTP_Radio[j]->BSS[k] != NULL)
			{
				if(AC_WTP[WtpID]->WTP_Radio[j]->BSS[k]->BSS_IF_POLICY == NO_INTERFACE){
					check1++;
				}else{
					check2++;
				}
				if((check1 != 0)&&(check2 != 0))
					return IF_POLICY_CONFLICT;
			}
	}
	if(check1 != 0)
		AC_WTP[WtpID]->tunnel_mode = CW_LOCAL_BRIDGING;
	else if(check2 != 0)
		AC_WTP[WtpID]->tunnel_mode = CW_802_DOT_11_TUNNEL;
	return 0;
}


int WID_ENABLE_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId)
{
	msgq msg2;
	struct msgqlist *elem2;
	WTPQUITREASON quitreason = WTP_INIT;
	//int i=0;
	int k=0;
	int ret2 = 0;
	//int ret1 = 0;	
	msgq msg;	
	char buf[DEFAULT_LEN];
	struct wds_bssid *wds = NULL;
	int ifind = 0;
	
	if(AC_WLAN[WlanId] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** you binding wlan does not exist **\n");
		return WLAN_ID_NOT_EXIST;
	}

	if(AC_WTP[WtpID]->WTPStat != 5)
	{
		return WTP_NOT_IN_RUN_STATE;
	}

	if(AC_WLAN[WlanId]->Status == 1)
	{
		return WLAN_BE_DISABLE;
	}
	
	//ret1 = WID_WTP_TUNNEL_MODE_CHECK(WtpID);
	//if(ret1 > 0)
	//	return IF_POLICY_CONFLICT;
	struct wlanid *wlan_id_next;

	wlan_id_next = AC_WTP[WtpID]->WTP_Radio[0]->Wlan_Id;

	if(AC_WTP[WtpID]->WTP_Radio[0]->isBinddingWlan == 0)
	{
		return INTERFACE_NOT_BE_BINDED;
	}
	else if(wlan_id_next->wlanid == WlanId)
	{
		ifind = 1;

		for(k=0;k<L_BSS_NUM;k++)
		{
			if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[k] != NULL)
			{
				if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[k]->WlanID == WlanId)
				{
					if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[k]->State != 1)
					{
						ret2 = 1;
					}
				}
			}
		}


	
		if(ret2 == 0)
		{
			return 0;
		}
	}
	else
	{
		while(wlan_id_next->next != NULL)
		{	
			if(wlan_id_next->next->wlanid == WlanId)
			{
				ifind = 1;				

				for(k=0;k<L_BSS_NUM;k++)
				{
					if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[k] != NULL)
					{
						if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[k]->WlanID == WlanId)
						{
							if(AC_WTP[WtpID]->WTP_Radio[0]->BSS[k]->State != 1)
							{
								ret2 = 1;
							}
						}
					}
				}
			
				if(ret2 == 0)
				{
					return 0;
				}

			}
			wlan_id_next = wlan_id_next->next;
		}
	}

	if(ifind == 0)
	{
		return WTP_WLAN_BINDING_NOT_MATCH;
	}
/////////////////////////////////////////////
	int j=0,ret;
	char ifiname[ETH_IF_NAME_LEN-1];
	if(!check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0])){
		wid_syslog_err("<error>%s\n",__func__);
		return BSS_NOT_EXIST;
	}else{
	}
	if((AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0] != 0)&&(AC_BSS[AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0]] != NULL))
	{		
		int bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][0];
		if(AC_BSS[bssindex]->State == 1)
			//continue;
		//if(((AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)&&(AC_WTP[WtpID]->tunnel_mode == CW_LOCAL_BRIDGING))||((AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE)&&(AC_WTP[WtpID]->tunnel_mode != CW_LOCAL_BRIDGING)))
		//	return IF_POLICY_CONFLICT;
		if(AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE){
			memset(ifiname,0,ETH_IF_NAME_LEN-1);
			//snprintf(ifiname,ETH_IF_NAME_LEN,"BSS%d",AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][j]);
			if(local)
				snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,WtpID,j,WlanId);
			else
				snprintf(ifiname,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,WtpID,j,WlanId);
			ret = Check_Interface_Config(ifiname,&quitreason);

			if(ret != 0)
			{
				return L3_INTERFACE_ERROR;
			}

		}		
		
		memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WtpID%THREAD_NUM+1;
			msg.mqinfo.WTPID = WtpID;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WLAN_S_TYPE;
		msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
		msg.mqinfo.u.WlanInfo.WLANID = WlanId;
		msg.mqinfo.u.WlanInfo.Radio_L_ID = j;//zhanglei wait for M-radio

		msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[WlanId]->HideESSid;
		memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
		memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[WlanId]->WlanKey,DEFAULT_LEN);
		msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[WlanId]->KeyLen;
		msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[WlanId]->SecurityType;
		msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[WlanId]->SecurityIndex;
		msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[WlanId]->asic_hex;/* 0 asic; 1 hex*/
		msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[WlanId]->Roaming_Policy; 		/*Roaming (1 enable /0 disable)*/
		memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
		//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanId]->ESSID,ESSID_LENGTH);
		memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[WlanId]->ESSID,strlen(AC_WLAN[WlanId]->ESSID));
		msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][j];
		
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
		if(!check_bssid_func(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][j])){
			wid_syslog_err("<error>%s\n",__func__);
			return BSS_NOT_EXIST;
		}else{
		    /* zhangshu add , 2011-1-7 */
        	if(AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][j])] != NULL) {
        		bssindex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][j];
        		wid_syslog_debug_debug(WID_DEFAULT,"!@#$ AC_BSS[%d]->traffic_limit_able = %d\n",bssindex,AC_BSS[bssindex]->traffic_limit_able);
        		if(AC_BSS[bssindex]->traffic_limit_able == 1){
        		    WID_Save_Traffic_Limit(bssindex, WtpID);
        		}
				
		if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->multi_user_optimize_switch == 1))
		{
			char wlanid =AC_BSS[bssindex]->WlanID;
			int radioid = AC_BSS[bssindex]->Radio_G_ID;
			muti_user_optimize_switch(wlanid,radioid,1);
		}
        	}
		}
			if(AC_BSS[(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][j])]->WDSStat == WDS_ANY)
			{
				msg2.mqid = WtpID%THREAD_NUM +1;
				msg2.mqinfo.WTPID = WtpID;
				msg2.mqinfo.type = CONTROL_TYPE;
				msg2.mqinfo.subtype = WDS_S_TYPE;
				msg2.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_ENABLE;
				msg2.mqinfo.u.WlanInfo.WLANID = WlanId;
				msg2.mqinfo.u.WlanInfo.Radio_L_ID = j;
				
				elem2 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem2 == NULL){
					wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				memset((char*)&(elem2->mqinfo), 0, sizeof(msgqdetail));
				elem2->next = NULL;
				memcpy((char*)&(elem2->mqinfo),(char*)&(msg2.mqinfo),sizeof(msg2.mqinfo));
				WID_INSERT_CONTROL_LIST(WtpID, elem2);
			}

			//wds state end
			else{
				if(AC_BSS[bssindex]->WDSStat == WDS_SOME){
					wds = AC_BSS[bssindex]->wds_bss_list;
					while(wds != NULL){
						memset(buf,0,DEFAULT_LEN);
						sprintf(buf,"/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",AC_BSS[bssindex]->Radio_L_ID,WlanId,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
//						printf("/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",AC_BSS[bssindex]->Radio_L_ID,WlanId,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
						wid_radio_set_extension_command(WtpID,buf);
						wds = wds->next;
					}
				}
			}
		int type = 1;//manual
		int flag = 1;//enable
		if(gtrapflag>=4){
			wid_dbus_trap_ap_ath_error(WtpID,j,WlanId,type,flag);
			}
	}
	

	return 0;		

}

int delete_wlan_bss(unsigned int WtpID, unsigned char WlanId)
{
	wid_syslog_debug_debug(WID_DEFAULT,"delete_wlan_bss::wtp:%d delete wlan:%d\n",WtpID,WlanId);

	int	RadioID = 0;
	int RCount = AC_WTP[WtpID]->RadioCount;
	struct wds_bssid *wds=NULL;
	struct wds_bssid *wds1=NULL;
	for(RadioID=0; RadioID<RCount; RadioID++)
	{
		if(AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][RadioID] != 0)
		{
			unsigned int BSSIndex = AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][RadioID];
			if(!check_bssid_func(BSSIndex)){
				wid_syslog_err("<error>%s\n",__func__);
				return BSS_NOT_EXIST;
			}else{
			}
			if((AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->State == 0)&&(AC_BSS[BSSIndex]->WlanID == WlanId))
			{
				wid_syslog_debug_debug(WID_DEFAULT,"delete wlan success\n");
				unsigned char L_ID = BSSIndex%L_BSS_NUM;
				
				AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][RadioID] = 0;
				AC_WTP[WtpID]->CMD->radiowlanid[0][WlanId] = 0;

				if(AC_BSS[BSSIndex]->BSS_IF_POLICY == BSS_INTERFACE)
				{
					//should added delete l3 interface
					Delete_BSS_L3_Interface(BSSIndex);
				}
				
				AC_BSS[BSSIndex]->WlanID = 0;
				AC_BSS[BSSIndex]->Radio_G_ID = 0;
				AC_BSS[BSSIndex]->Radio_L_ID = 0;
				AC_BSS[BSSIndex]->State = 0;
				AC_BSS[BSSIndex]->BSSIndex = 0;
				//weichao add 
				if( AC_BSS[BSSIndex]->acl_conf != NULL){	
					if( AC_BSS[BSSIndex]->acl_conf->accept_mac != NULL) {
						free_maclist(AC_BSS[BSSIndex]->acl_conf,AC_BSS[BSSIndex]->acl_conf->accept_mac);
						AC_BSS[BSSIndex]->acl_conf->accept_mac = NULL;
						}
					if( AC_BSS[BSSIndex]->acl_conf->deny_mac != NULL) {
						free_maclist(AC_BSS[BSSIndex]->acl_conf,AC_BSS[BSSIndex]->acl_conf->deny_mac);
						AC_BSS[BSSIndex]->acl_conf->deny_mac = NULL;
						}
					free(AC_BSS[BSSIndex]->acl_conf);
					AC_BSS[BSSIndex]->acl_conf = NULL;
				}
				memset(AC_BSS[BSSIndex]->BSSID, 0, 6);
				free(AC_BSS[BSSIndex]->BSSID);
				AC_BSS[BSSIndex]->BSSID = NULL;
				wds = AC_BSS[BSSIndex]->wds_bss_list;
				while(wds){
					wds1 = wds;
					wds = wds->next;
					free(wds1);
					wds1 = NULL;
				}
				AC_BSS[BSSIndex]->wds_bss_list = NULL;
				free(AC_BSS[BSSIndex]);
				AC_BSS[BSSIndex] = NULL;
				
				AC_WTP[WtpID]->WTP_Radio[RadioID]->BSS[L_ID] = NULL;

				//return 0;
			}
			else if((AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->State == 1))
			{
				//printf("delete wlan error bss is enable\n");
				wid_syslog_debug_debug(WID_DEFAULT,"delete wlan error bss is enable");
				return -1;
			}

		}

				
	}

	//printf("delete wlan fail\n");
	return 0;
	
}
int delete_wlan_bss_by_radioId(unsigned int RadioId, unsigned char WlanId)
{
	wid_syslog_debug_debug(WID_DEFAULT,"delete_wlan_bss::radioid:%d delete wlan:%d\n",RadioId,WlanId);

	int	WtpID = RadioId/L_RADIO_NUM;
	int L_RadioId = RadioId%L_RADIO_NUM;
	int i = 0;

	for(i=0; i<L_BSS_NUM; i++)
	{
		if((AC_RADIO[RadioId]->BSS[i] != NULL)&&(AC_RADIO[RadioId]->BSS[i]->WlanID == WlanId))
		{
			break;
		}
	}

	if(i == L_BSS_NUM)
	{
		return -1;
	}
	
	unsigned int BSSIndex = AC_RADIO[RadioId]->BSS[i]->BSSIndex;
	if(!check_bssid_func(BSSIndex)){
		wid_syslog_err("<error>%s\n",__func__);
		return BSS_NOT_EXIST;
	}else{
	}
	if((AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->State == 0)&&(AC_BSS[BSSIndex]->WlanID == WlanId))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"delete wlan success\n");
		unsigned char L_ID = BSSIndex%L_BSS_NUM;
		
		AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][L_RadioId] = 0;
		AC_WTP[WtpID]->CMD->radiowlanid[L_RadioId][WlanId] = 0;
		
		//AC_RADIO[RadioId]->CMD_radio->wlanid[WlanId] = 0;

		if(AC_BSS[BSSIndex]->BSS_IF_POLICY == BSS_INTERFACE)
		{
			//should added delete l3 interface
			Delete_BSS_L3_Interface(BSSIndex);
		}
		else if(AC_BSS[BSSIndex]->BSS_IF_POLICY == WLAN_INTERFACE)
		{
			//remove bss if from wlan br
			Del_BSS_L3_Interface_BR(BSSIndex);
			//should added delete l3 interface
			Delete_BSS_L3_Interface(BSSIndex);
		}
		//wep wlan area
		if(AC_BSS[BSSIndex]->keyindex != 0)
		{
			int keyindex = AC_BSS[BSSIndex]->keyindex-1;
			//AC_WTP[WtpID]->wep_flag[keyindex] = 0;
			AC_RADIO[RadioId]->wep_flag[keyindex] = 0;
			AC_BSS[BSSIndex]->keyindex = 0;
		}
		AC_BSS[BSSIndex]->WlanID = 0;
		AC_BSS[BSSIndex]->Radio_G_ID = 0;
		AC_BSS[BSSIndex]->Radio_L_ID = 0;
		AC_BSS[BSSIndex]->State = 0;
		AC_BSS[BSSIndex]->BSSIndex = 0;
		//weichao add 
		if( AC_BSS[BSSIndex]->acl_conf != NULL){	
			if( AC_BSS[BSSIndex]->acl_conf->accept_mac != NULL) {
				free_maclist(AC_BSS[BSSIndex]->acl_conf,AC_BSS[BSSIndex]->acl_conf->accept_mac);
				AC_BSS[BSSIndex]->acl_conf->accept_mac = NULL;
				}
			if( AC_BSS[BSSIndex]->acl_conf->deny_mac != NULL) {
				free_maclist(AC_BSS[BSSIndex]->acl_conf,AC_BSS[BSSIndex]->acl_conf->deny_mac);
				AC_BSS[BSSIndex]->acl_conf->deny_mac = NULL;
				}
			free(AC_BSS[BSSIndex]->acl_conf);
			AC_BSS[BSSIndex]->acl_conf = NULL;
		}
		memset(AC_BSS[BSSIndex]->BSSID, 0, 6);
		free(AC_BSS[BSSIndex]->BSSID);
		AC_BSS[BSSIndex]->BSSID = NULL;
		free(AC_BSS[BSSIndex]);
		AC_BSS[BSSIndex] = NULL;
		
		AC_WTP[WtpID]->WTP_Radio[L_RadioId]->BSS[L_ID] = NULL;

		//return 0;
	}
	else if((AC_BSS[BSSIndex] != NULL)&&(AC_BSS[BSSIndex]->State == 1))
	{
		//printf("delete wlan error bss is enable\n");
		wid_syslog_debug_debug(WID_DEFAULT,"delete wlan error bss is enable");
		return -1;
	}
				
	

	return 0;
	
}

int delete_wlan_all_bss(unsigned int WtpID)
{
	int num = AC_WTP[WtpID]->RadioCount;
	int i;
	int k = 0;
	int bssindex = 0;
	int ret = 0;
	unsigned int RID = AC_WTP[WtpID]->WFR_Index;
	for(i=0; i<num; i++)
	{

		for(k=0; k<L_BSS_NUM; k++)
		{
			if(AC_RADIO[RID]->BSS[k] != NULL)
			{				
				bssindex = AC_RADIO[RID]->BSS[k]->BSSIndex;
				if((AC_BSS[bssindex] != NULL)&&(AC_BSS[bssindex]->State == 1))
				{
					ret = 1;
				}
			}

		}
		
		RID++;
	}
	
	if(ret == 1)
	{
		//printf("delete wlan error bss is enable\n");
		wid_syslog_debug_debug(WID_DEFAULT,"delete wlan error bss is enable");
		return BSS_BE_ENABLE;
	}

	//delete all bss
	RID = AC_WTP[WtpID]->WFR_Index;
	for(i=0; i<num; i++)
	{
		for(k=0; k<L_BSS_NUM; k++)
		{
			if(AC_RADIO[RID]->BSS[k] != NULL)
			{
				if(AC_RADIO[RID]->BSS[k]->BSS_IF_POLICY == BSS_INTERFACE)
				{
					//should added delete l3 interface
					Delete_BSS_L3_Interface(AC_RADIO[RID]->BSS[k]->BSSIndex);
				}
				
				bssindex = AC_RADIO[RID]->BSS[k]->BSSIndex;
				//weichao add 
				if( AC_BSS[bssindex]->acl_conf != NULL){	
					if( AC_BSS[bssindex]->acl_conf->accept_mac != NULL) {
						free_maclist(AC_BSS[bssindex]->acl_conf,AC_BSS[bssindex]->acl_conf->accept_mac);
						AC_BSS[bssindex]->acl_conf->accept_mac = NULL;
						}
					if( AC_BSS[bssindex]->acl_conf->deny_mac != NULL) {
						free_maclist(AC_BSS[bssindex]->acl_conf,AC_BSS[bssindex]->acl_conf->deny_mac);
						AC_BSS[bssindex]->acl_conf->deny_mac = NULL;
						}
					free(AC_BSS[bssindex]->acl_conf);
					AC_BSS[bssindex]->acl_conf = NULL;
				}
				unsigned char WlanId = AC_BSS[bssindex]->WlanID;
				AC_RADIO[RID]->BSS[k]->WlanID = 0;
				AC_RADIO[RID]->BSS[k]->Radio_G_ID = 0;
				AC_RADIO[RID]->BSS[k]->Radio_L_ID = 0;
				AC_RADIO[RID]->BSS[k]->State = 0;
				AC_RADIO[RID]->BSS[k]->BSSIndex = 0;
				memset(AC_RADIO[RID]->BSS[k]->BSSID, 0, 6);
				free(AC_RADIO[RID]->BSS[k]->BSSID);
				AC_RADIO[RID]->BSS[k]->BSSID = NULL;
				free(AC_RADIO[RID]->BSS[k]);
				AC_RADIO[RID]->BSS[k] = NULL;
				AC_WTP[WtpID]->WTP_Radio[i]->BSS[k]=NULL;	
				if(AC_WLAN[WlanId])
				{
					AC_WLAN[WlanId]->S_WTP_BSS_List[WtpID][i] = 0;
				}
				AC_BSS[bssindex] = NULL;
			}

		}

		RID++;
	}
	
	struct wlanid *wlan_id = AC_WTP[WtpID]->WTP_Radio[0]->Wlan_Id;
	while(wlan_id != NULL)
	{
		AC_WTP[WtpID]->WTP_Radio[0]->Wlan_Id = wlan_id->next;
		free(wlan_id);
		wlan_id = NULL;
		wlan_id = AC_WTP[WtpID]->WTP_Radio[0]->Wlan_Id;
		
	}

	AC_WTP[WtpID]->WTP_Radio[0]->isBinddingWlan = 0;
	AC_WTP[WtpID]->WTP_Radio[0]->BindingWlanCount = 0;

	return WTP_CLEAR_BINDING_WLAN_SUCCESS;

}

int wid_set_ap_scanning(APScanningSetting scansetting)
{
	int i = 0;
	msgq msg;
	for(i=0; i<WTP_NUM; i++)
	{
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
//			printf("##002 wtp id= %d##\n",i);
			
			AC_WTP[i]->WTP_Radio[0]->CMD |= 0x40;
			AC_WTP[i]->CMD->radioid[0] += 1;
			AC_WTP[i]->CMD->setCMD = 1;	
			int WTPIndex = i;
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SCANNING_OP;
				msg.mqinfo.u.WtpInfo.value1 = scansetting.opstate;
				msg.mqinfo.u.WtpInfo.value2 = scansetting.reportinterval;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gtrapflag>=4){
				wid_dbus_trap_ap_rrm_state_change(WTPIndex,scansetting.opstate);
				}
		}
	}

	return 0;

}

int Create_BSS_L3_Interface(unsigned int BSSIndex)
{
	
	if_basic_info ifinfo;
	ifinfo.wlanID = 0;
	ifinfo.BSSIndex = BSSIndex;
	ifinfo.vrid = local*MAX_INSTANCE+vrrid;
	int wtpid = 0;
	int l_radioid = 0;
	int wlanid = 0;
	int reason = 0;
	memset(ifinfo.if_name,0,ETH_IF_NAME_LEN-1);
	//assemble radio1-1.1
	wtpid = BSSIndex/(L_BSS_NUM*L_RADIO_NUM);
	l_radioid = AC_BSS[BSSIndex]->Radio_L_ID;
	wlanid = AC_BSS[BSSIndex]->WlanID;
//	printf("radio%d-%d.%d\n",wtpid,l_radioid,wlanid);
	//snprintf(ifinfo.if_name,ETH_IF_NAME_LEN-1,"radio%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	if(local)
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	else
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpid,l_radioid,wlanid);
	//snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"BSS%d",BSSIndex);
	memcpy(AC_BSS[BSSIndex]->BSS_IF_NAME, ifinfo.if_name, ETH_IF_NAME_LEN-1);
//	printf("*** Create_BSS ifname:%s ***\n",ifinfo.if_name);
//	printf("Create_BSS ifname:%s\n",AC_BSS[BSSIndex]->BSS_IF_NAME);
	wid_syslog_debug_debug(WID_DEFAULT,"*** Create_BSS ifname:%s ***\n",ifinfo.if_name);
	wid_syslog_debug_debug(WID_DEFAULT,"Create_BSS ifname:%s\n",AC_BSS[BSSIndex]->BSS_IF_NAME);
	
	int ret = -1;
	
	int fd = open("/dev/wifi0", O_RDWR);
	wid_syslog_debug_debug(WID_DEFAULT,"*** fd:%d ***\n",fd);

	if(fd < 0)
	{
		return -1;//create failure
	}

	
	ret = ioctl(fd, WIFI_IOC_IF_CREATE, &ifinfo);
	
	wid_syslog_debug_debug(WID_DEFAULT,"*** Create_BSS_L3_Interface ret:%d ***\n",ret);
	close(fd);
	if(ret < 0)
	{
		return -1;
	}
	if((is_secondary != 2)&&(set_vmac_state == 1)){
		hwaddr_set(AC_BSS[BSSIndex]->BSS_IF_NAME,v_mac,MAC_LEN);
		AC_BSS[BSSIndex]->vMAC_STATE = 1;
	}
	
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	//sprintf(systemcmd,"ifconfig radio%d-%d-%d.%d up",vrrid,wtpid,l_radioid,wlanid);
	if(local)
		sprintf(systemcmd,"ifconfig r%d-%d-%d.%d up",vrrid,wtpid,l_radioid,wlanid);
	else	
		sprintf(systemcmd,"ifconfig r%d-%d-%d-%d.%d up",slotid,vrrid,wtpid,l_radioid,wlanid);
//	printf("systemcmd:%s\n",systemcmd);

	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	return 0;

}
int Delete_BSS_L3_Interface(unsigned int BSSIndex)
{
	if_basic_info ifinfo;
	ifinfo.wlanID = 0;
	ifinfo.BSSIndex = BSSIndex;
	ifinfo.vrid = local*MAX_INSTANCE+vrrid;
	int wtpid = 0;
	int l_radioid = 0;
	int wlanid = 0;
	memset(ifinfo.if_name,0,ETH_IF_NAME_LEN-1);
	//snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"BSS%d",BSSIndex);
	//assemble radio1-0.1
	wtpid = BSSIndex/(L_BSS_NUM*L_RADIO_NUM);
	l_radioid = AC_BSS[BSSIndex]->Radio_L_ID;
	wlanid = AC_BSS[BSSIndex]->WlanID;
//	printf("radio%d-%d.%d\n",wtpid,l_radioid,wlanid);
	//snprintf(ifinfo.if_name,ETH_IF_NAME_LEN-1,"radio%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	if(local)
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	else
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpid,l_radioid,wlanid);
		
	memset(AC_BSS[BSSIndex]->BSS_IF_NAME, 0, ETH_IF_NAME_LEN-1);
	wid_syslog_debug_debug(WID_DEFAULT,"*** Delete_BSS ifname:%s ***\n",ifinfo.if_name);
	
	int ret = -1;
	
	int fd = open("/dev/wifi0", O_RDWR);
	wid_syslog_debug_debug(WID_DEFAULT,"*** fd:%d ***\n",fd);

	if(fd < 0)
	{
		return -1;//create failure
	}
	wid_syslog_debug_debug(WID_DEFAULT,"*** macro:%x ***\n",WIFI_IOC_IF_DELETE);
	
	ret = ioctl(fd, WIFI_IOC_IF_DELETE, &ifinfo);
	
	wid_syslog_debug_debug(WID_DEFAULT,"*** Delete_BSS_L3_Interface ret:%d ***\n",ret);
	close(fd);
	if(ret < 0)
	{
		return -1;
	}


	return 0;


}
int ADD_BSS_L3_Interface_BR(unsigned int BSSIndex)
{
	int ret = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char addifcmd[WID_SYSTEM_CMD_LENTH];
	memset(addifcmd,0,WID_SYSTEM_CMD_LENTH);
	
	int wtpid = 0;
	int l_radioid = 0;
	int wlanid = 0;
	int reason = 0;
	wtpid = BSSIndex/(L_BSS_NUM*L_RADIO_NUM);
	l_radioid = AC_BSS[BSSIndex]->Radio_L_ID;
	wlanid = AC_BSS[BSSIndex]->WlanID;
	int G_radio_id = 0;
	char bssifname[ETH_IF_NAME_LEN];
	char ifcheck[WID_SYSTEM_CMD_LENTH];
	memset(bssifname,0,ETH_IF_NAME_LEN);
	memset(ifcheck,0,WID_SYSTEM_CMD_LENTH);
//	sprintf(bssifname,"radio%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	if(local)
		sprintf(bssifname,"r%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	else
		sprintf(bssifname,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpid,l_radioid,wlanid);

	if(local)
		sprintf(ifcheck,"/sys/class/net/wlanl%d-%d-%d/brif/%s/port_id",slotid,vrrid,wlanid,bssifname);
	else
		sprintf(ifcheck,"/sys/class/net/wlan%d-%d-%d/brif/%s/port_id",slotid,vrrid,wlanid,bssifname);

	G_radio_id = wtpid*L_RADIO_NUM+l_radioid;
//	printf("radio%d-%d.%d\n",wtpid,l_radioid,wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"ADD_BSS_L3_Interface_BR ifname:%s\n",bssifname);

	//check bss if validity
	ret = Check_Interface_Config(bssifname,&quitreason);
	if(ret != 0)
	{
		return -1;
	}
	ret = file_check(ifcheck);
	if(ret == 1){
		ret = 0;
	}else{
		//add if to br
		if(local)
			sprintf(addifcmd,"brctl addif wlanl%d-%d-%d %s\n",slotid,vrrid,wlanid,bssifname);
		else
			sprintf(addifcmd,"brctl addif wlan%d-%d-%d %s\n",slotid,vrrid,wlanid,bssifname);
	//	printf("system cmd: %s\n",addifcmd);
		wid_syslog_debug_debug(WID_DEFAULT,"ADD_BSS_L3_Interface_BR addifcmd:%s\n",addifcmd);
		
		ret = system(addifcmd);

		wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_warning("<warnning>system cmd error,error code %d\n",reason);
			wid_syslog_warning("<warnning>interface %s may have been in other br.\n",bssifname);
			time_t now = 0;
			time(&now);
			syslog(LOG_INFO|LOG_LOCAL7, "br wlan%d add %s fail at Time:%s.(it may have been in other br,if we need add it to br wlan,please add it in manual,when peel it off other br)\n",wlanid,bssifname,ctime(&now));

			return WID_ADD_RADIO_IF_FAIL;
		}
	}
	if(ret != 0)
	{
		//return -1;//create failure
		return WID_ADD_RADIO_IF_FAIL;//wu wl change 2010-12-28
	}
	else
	{	
		if(AC_RADIO[G_radio_id] != NULL){
			if(local)
				sprintf(AC_RADIO[G_radio_id]->br_ifname[wlanid],"wlanl%d-%d-%d",slotid,vrrid,wlanid);
			else
				sprintf(AC_RADIO[G_radio_id]->br_ifname[wlanid],"wlan%d-%d-%d",slotid,vrrid,wlanid);
			wid_syslog_debug_debug(WID_DEFAULT,"interface wlan br_ifname:%s.\n",AC_RADIO[G_radio_id]->br_ifname[wlanid]);
		}
		return 0;//create success
	}
	
}
int Del_BSS_L3_Interface_BR(unsigned int BSSIndex)
{
	int ret = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char delifcmd[WID_SYSTEM_CMD_LENTH];
	memset(delifcmd,0,WID_SYSTEM_CMD_LENTH);
	
	int wtpid = 0;
	int l_radioid = 0;
	int wlanid = 0;
	int reason = 0;
	wtpid = BSSIndex/(L_BSS_NUM*L_RADIO_NUM);
	l_radioid = AC_BSS[BSSIndex]->Radio_L_ID;
	wlanid = AC_BSS[BSSIndex]->WlanID;
	
	int G_radio_id = 0;
	G_radio_id = wtpid*L_RADIO_NUM+l_radioid;

	char bssifname[ETH_IF_NAME_LEN];
	memset(bssifname,0,ETH_IF_NAME_LEN);
//	sprintf(bssifname,"radio%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	if(local)
		sprintf(bssifname,"r%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	else
		sprintf(bssifname,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpid,l_radioid,wlanid);
//	printf("radio%d-%d.%d\n",wtpid,l_radioid,wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"Del_BSS_L3_Interface_BR ifname:%s\n",bssifname);

	//check bss if validity
	ret = Check_Interface_Config(bssifname,&quitreason);
	if(ret != 0)
	{
		return -1;
	}

	//remove if from br
	if(local)
		sprintf(delifcmd,"brctl delif wlanl%d-%d-%d %s\n",slotid,vrrid,wlanid,bssifname);
	else
		sprintf(delifcmd,"brctl delif wlan%d-%d-%d %s\n",slotid,vrrid,wlanid,bssifname);		
//	printf("system cmd: %s\n",delifcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"Del_BSS_L3_Interface_BR delifcmd:%s\n",delifcmd);
	
	ret = system(delifcmd);

	wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);

	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return -1;
	}
	if(ret != 0)
	{
		return -1;//delete failure
	}
	else
	{
		if(AC_RADIO[G_radio_id] != NULL){
			memset(AC_RADIO[G_radio_id]->br_ifname[wlanid],0,IF_NAME_MAX);
			wid_syslog_debug_debug(WID_DEFAULT,"interface wlan br_ifname:%s\n",AC_RADIO[G_radio_id]->br_ifname[wlanid]);
		}
		return 0;//delete success
	}
	
}

int Create_Wlan_L3_Interface(unsigned 	char WlanID)
{
	if_basic_info ifinfo;
	ifinfo.BSSIndex = 0;
	ifinfo.wlanID = WlanID;
	
	memset(ifinfo.if_name,0,ETH_IF_NAME_LEN-1);
	//snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"WLAN%d",WlanID);	
	if(local)
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN-1,"wlanl%d-%d-%d",slotid,vrrid,WlanID);	
	else
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN-1,"wlan%d-%d-%d",slotid,vrrid,WlanID);	
		
	memcpy(AC_WLAN[WlanID]->WlanL3IFName,ifinfo.if_name,ETH_IF_NAME_LEN-1);
	wid_syslog_debug_debug(WID_DEFAULT,"*** Create_Wlan ifname:%s ***\n",ifinfo.if_name);
	
	int ret = -1;

	int fd = open("/dev/wifi0", O_RDWR);
	
	wid_syslog_debug_debug(WID_DEFAULT,"*** fd:%d ***\n",fd);

	if(fd < 0)
	{
		return -1;//create failure
	}
	wid_syslog_debug_debug(WID_DEFAULT,"*** macro:%x ***\n",WIFI_IOC_IF_CREATE);

	ret = ioctl(fd, WIFI_IOC_IF_CREATE, &ifinfo);

	wid_syslog_debug_debug(WID_DEFAULT,"*** Create_Wlan_L3_Interface ret:%d ***\n",ret);
	close(fd);
	if(ret < 0)
	{
		return -1;//create failure
	}	
	


	return 0;//create success
	
}
int Delete_Wlan_L3_Interface(unsigned char WlanID)
{
	if_basic_info ifinfo;
	ifinfo.BSSIndex = 0;
	ifinfo.wlanID = WlanID;
	ifinfo.vrid = local*MAX_INSTANCE +vrrid;
	wid_syslog_debug_debug(WID_DEFAULT,"way test 0002 \n");
	memset(ifinfo.if_name,0,ETH_IF_NAME_LEN-1);
	//snprintf(ifinfo.if_name,ETH_IF_NAME_LEN,"WLAN%d",WlanID);
	if(local)
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN-1,"wlanl%d-%d-%d",slotid,vrrid,WlanID);	
	else
		snprintf(ifinfo.if_name,ETH_IF_NAME_LEN-1,"wlan%d-%d-%d",slotid,vrrid,WlanID);	
		
	memset(AC_WLAN[WlanID]->WlanL3IFName,0,ETH_IF_NAME_LEN-1);
	wid_syslog_debug_debug(WID_DEFAULT,"*** Delete_Wlan ifname:%s ***\n",ifinfo.if_name);
	
	int ret = -1;


	int fd = open("/dev/wifi0", O_RDWR);

	wid_syslog_debug_debug(WID_DEFAULT,"*** fd:%d ***\n",fd);

	if(fd < 0)
	{
		return -1;//create failure
	}
	wid_syslog_debug_debug(WID_DEFAULT,"*** macro:%x ***\n",WIFI_IOC_IF_DELETE);

	ret = ioctl(fd, WIFI_IOC_IF_DELETE, &ifinfo);

	wid_syslog_debug_debug(WID_DEFAULT,"*** Delete_Wlan_L3_Interface ret:%d ***\n",ret);
	close(fd);
	if(ret < 0)
	{
		return -1;//create failure
	}	

	return 0;//create success

}
int Create_Wlan_L3_BR_Interface(unsigned 	char WlanID)
{
	int ret = -1;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	memset(brname,0,ETH_IF_NAME_LEN);
	
	char addbrcmd[WID_SYSTEM_CMD_LENTH];
	memset(addbrcmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(brname,"wlanl%d-%d-%d",slotid,vrrid,WlanID);
	else
		sprintf(brname,"wlan%d-%d-%d",slotid,vrrid,WlanID);
		
	ret = Check_Interface_Config(brname,&quitreason);

	wid_syslog_debug_debug(WID_DEFAULT,"Check_Interface_Config ifname:%s quitreason:%d\n",brname,quitreason);
	if(ret == 0){
#if 0
		memset(addbrcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(addbrcmd,"ifconfig wlanl%d-%d-%d down\n",slotid,vrrid,WlanID); 
		else
			sprintf(addbrcmd,"ifconfig wlan%d-%d-%d down\n",slotid,vrrid,WlanID);	
			
//		printf("system cmd: %s\n",addbrcmd);
		ret = system(addbrcmd);

		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
			return -1;
		}
		if(ret != 0)
		{
			return -1;
		}
	
		memset(addbrcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(addbrcmd,"brctl delbr wlanl%d-%d-%d\n",slotid,vrrid,WlanID);
		else
			sprintf(addbrcmd,"brctl delbr wlan%d-%d-%d\n",slotid,vrrid,WlanID);
			
//		printf("system cmd: %s\n",addbrcmd);
		wid_syslog_debug_debug(WID_DEFAULT,"Delete_Wlan_L3_BR_Interface addbrcmd:%s\n",addbrcmd);
		
		ret = system(addbrcmd);

		wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
			return -1;
		}
		if(ret != 0)
		{
			return -1;
		}
#endif
	}
	else{
		memset(addbrcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(addbrcmd,"brctl addbr wlanl%d-%d-%d\n",slotid,vrrid,WlanID);	
		else
			sprintf(addbrcmd,"brctl addbr wlan%d-%d-%d\n",slotid,vrrid,WlanID);	
			
//		printf("system cmd: %s\n",addbrcmd);
		wid_syslog_debug_debug(WID_DEFAULT,"Create_Wlan_L3_BR_Interface addbrcmd:%s\n",addbrcmd);
		
		ret = system(addbrcmd);
		wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
			return -1;
		}
		if(ret != 0)
		{
			return -1;//create failure
		}
	}
	//set the br if up
	
	memset(addbrcmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(addbrcmd,"ifconfig wlanl%d-%d-%d up\n",slotid,vrrid,WlanID);	
	else
		sprintf(addbrcmd,"ifconfig wlan%d-%d-%d up\n",slotid,vrrid,WlanID);			
	//printf("system cmd: %s\n",addbrcmd);
	//ret = system(addbrcmd);

	if(ret != 0)
	{
		return -1;//create failure
	}
	
	else
	{
		wid_set_wlan_br_isolation(WlanID,1);
		wid_set_wlan_br_multicast_isolation(WlanID,1);
		wid_set_wlan_br_sameportswitch(WlanID,0);
		return 0;//create success
	}
}
int Delete_Wlan_L3_BR_Interface(unsigned char WlanID)
{
	int ret = -1;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	memset(brname,0,ETH_IF_NAME_LEN);
	
	char addbrcmd[WID_SYSTEM_CMD_LENTH];
	memset(addbrcmd,0,WID_SYSTEM_CMD_LENTH);

	if(local)
		sprintf(brname,"wlanl%d-%d-%d",slotid,vrrid,WlanID);
	else
		sprintf(brname,"wlan%d-%d-%d",slotid,vrrid,WlanID);
		
	ret = Check_Interface_Config(brname,&quitreason);

	if(ret == 0)
	{
		if(local)
			sprintf(addbrcmd,"ifconfig wlanl%d-%d-%d down\n",slotid,vrrid,WlanID);	
		else
			sprintf(addbrcmd,"ifconfig wlan%d-%d-%d down\n",slotid,vrrid,WlanID);	
			
//		printf("system cmd: %s\n",addbrcmd);
		ret = system(addbrcmd);

		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
			return -1;
		}
		if(ret != 0)
		{
			return -1;
		}
	
		memset(addbrcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(addbrcmd,"brctl delbr wlanl%d-%d-%d\n",slotid,vrrid,WlanID);
		else
			sprintf(addbrcmd,"brctl delbr wlan%d-%d-%d\n",slotid,vrrid,WlanID);
			
//		printf("system cmd: %s\n",addbrcmd);
		wid_syslog_debug_debug(WID_DEFAULT,"Delete_Wlan_L3_BR_Interface addbrcmd:%s\n",addbrcmd);
		
		ret = system(addbrcmd);

		wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
			return -1;
		}
		if(ret != 0)
		{
			return -1;//delete failure
		}
		else
		{
			return 0;//delete success
		}
	}

	return -1;
	
}


void display_ap_info_list(Neighbor_AP_INFOS *paplist)
{
	//printf("#####0007777777777777####\n");
	if((paplist == NULL)||(paplist->neighborapInfos == NULL)||(paplist->neighborapInfosCount == 0))
	{
		printf("display_ap_info_list parameter error\n");
		return;
	}
	
	int i = 0;
	int j = 0;
	//printf("######0000888888888####\n");
	struct Neighbor_AP_ELE *phead = paplist->neighborapInfos;
		
	printf("## display The ap info list total count is = %d: ##\n",paplist->neighborapInfosCount);
	for(i=0; i<paplist->neighborapInfosCount; i++)
	{
		printf("## the count i = %d##\n",i);
		printf("mac = ");
		for(j=0; j<6; j++)
		{
			printf("%02x", phead->BSSID[j]);
		}
		printf("\n");

		printf("rate = %d\n", phead->Rate);
		printf("Channel = %d\n", phead->Channel);
		printf("RSSI = %d\n", phead->RSSI);
		printf("NOISE = %d\n", phead->NOISE);
		
		printf("BEACON_INT = %d\n", phead->BEACON_INT);
		printf("status = %d\n", phead->status);
		printf("opstatus = %d\n", phead->opstatus);
		printf("capabilityinfo = %d\n", phead->capabilityinfo);
		
		printf("ESSID = %s\n", phead->ESSID);
		printf("IEs_INFO = %s\n", phead->IEs_INFO);

		phead = phead->next;
		printf("info #################################\n");
	}
}

void destroy_ap_info_list(Neighbor_AP_INFOS **paplist)
{
	if(((*paplist) == NULL)||((*paplist)->neighborapInfosCount == 0))
	{
		//printf("destroy_ap_info_list parameter error\n");
		CW_FREE_OBJECT(*paplist);
		return;
	}
	
//	int i = 0;
	struct Neighbor_AP_ELE *phead = NULL;
	struct Neighbor_AP_ELE *pnext = NULL;
	phead = (*paplist)->neighborapInfos;
	(*paplist)->neighborapInfos = NULL;
		
	//printf("## destroy The ap info list total count is = %d: ##\n",(*paplist)->neighborapInfosCount);
	while(phead != NULL)
	{	
		//printf("## the count i = %d##\n",i++);
		
		pnext = phead->next;
		
		//CW_FREE_OBJECT(phead->ESSID);
		CW_FREE_OBJECT(phead->IEs_INFO);

		CW_FREE_OBJECT(phead);

		phead = pnext;
		
		//printf("###########################################\n");
	}

	(*paplist)->neighborapInfosCount = 0;
	CW_FREE_OBJECT(*paplist);

}

Neighbor_AP_INFOS * create_ap_info_list(int count)
{
	Neighbor_AP_INFOS *create_ap_info;
	int i = 0;
	CW_CREATE_OBJECT_ERR(create_ap_info, Neighbor_AP_INFOS, return NULL;);	
				
	create_ap_info->neighborapInfosCount = count;
	create_ap_info->neighborapInfos = NULL;
	
	struct Neighbor_AP_ELE *neighborapelem = NULL;
	struct Neighbor_AP_ELE *phead = NULL;

	for(i=0; i<create_ap_info->neighborapInfosCount; i++)
	{
		CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct Neighbor_AP_ELE), return NULL;);		
		
		char str[3][6] = {{"111111"},{"222222"},{"333333"}};
		memcpy(neighborapelem->BSSID ,str[i], 6);
		
		neighborapelem->Rate = 110;

		neighborapelem->Channel = 1;
		neighborapelem->RSSI = 1;
		neighborapelem->NOISE = 1;
		neighborapelem->BEACON_INT = 10;
		
		neighborapelem->status = 0;
		neighborapelem->opstatus = 0;
		neighborapelem->capabilityinfo = 0;

		char *essid = (char *)malloc(5);
		memset(essid,0,5);
		memcpy(essid,"way",3);

		memset(neighborapelem->ESSID,0,strlen(essid)+1);
		memcpy(neighborapelem->ESSID,essid,strlen(essid));
		//neighborapelem->ESSID = essid;

		char *ie = (char *)malloc(7);
		memset(ie,0,7);
		memcpy(ie,"capwap",6);
		neighborapelem->IEs_INFO = ie;

		neighborapelem->next = NULL;

		if(i== 0)
		{
			//printf("parse first ap info\n");
			create_ap_info->neighborapInfos = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		else
		{
			//printf("parse more ap info\n");
			phead->next = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		//printf("######002####\n");
		if(essid){
			free(essid);
			essid = NULL;
		}
	}	

	return create_ap_info;
	//display_ap_info_list(create_ap_info);


}
Neighbor_AP_INFOS * wid_check_rogue_ap_all()
{
	Neighbor_AP_INFOS * rouge_ap_info = wid_get_neighbor_ap_list();	
	if(rouge_ap_info == NULL)
	{
		return NULL;
	}

	//printf("######wid_check_rogue_ap_all display_ap_info_list####\n");	
	//display_ap_info_list(rouge_ap_info);
	
	white_mac_list * maclist = wid_check_white_mac();
	
	//printf("######wid_check_rogue_ap_all wid_check_white_mac####\n");	
	//display_mac_info_list(maclist);

	struct white_mac *pmacnode = NULL;
	if(maclist != NULL)
	{
		pmacnode = maclist->list_mac;
	}

	while(pmacnode != NULL)
	{
		delete_elem_from_ap_list_bymac(&rouge_ap_info, pmacnode);
		pmacnode = pmacnode->next;
	}

	if(maclist != NULL)
	{
		wid_destroy_white_mac(&maclist);
	}
	
	return rouge_ap_info;

	

}

Neighbor_AP_INFOS * wid_check_rogue_ap_mac(int wtpid)
{	
	if((AC_WTP[wtpid] == NULL)||(AC_WTP[wtpid]->NeighborAPInfos== NULL)||(AC_WTP[wtpid]->NeighborAPInfos->neighborapInfosCount == 0))
	{
		return NULL;
	}

	//printf("wid_check_rogue_ap_mac\n");
	Neighbor_AP_INFOS * rouge_ap_info=NULL;
	struct Neighbor_AP_ELE *neighborapelem = NULL;
	struct Neighbor_AP_ELE *prougehead = NULL;
	struct Neighbor_AP_ELE *phead = AC_WTP[wtpid]->NeighborAPInfos->neighborapInfos;
	int i = 0;
	int j = 0;
	int k = 0;
	int charlen = 0;
	int flag = 0;

	CW_CREATE_OBJECT_ERR(rouge_ap_info, Neighbor_AP_INFOS, return NULL;);					
	rouge_ap_info->neighborapInfosCount = 0;
	rouge_ap_info->neighborapInfos = NULL;
	memset(rouge_ap_info,0,sizeof(*rouge_ap_info));

	while(phead != NULL)
	{
		for(i=0; i<WTP_NUM; i++)
		{
			
			if ((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
			{
				for(j=0;j<AC_WTP[i]->RadioCount;j++)
				{				
					if(AC_WTP[i]->WTP_Radio[j] != NULL)
					{
						for(k=0;k<L_BSS_NUM;k++)
						{
							if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSID != NULL))
							{
								if (memcmp((AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSID),phead->BSSID,6) == 0)
								{
									flag = 1;
									break;
								}
							}
						}
					}
					if(flag == 1)
						break;
						
				}

			}
			if(flag == 1)
				break;

		}

		if(flag == 0)
		{
			CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct Neighbor_AP_ELE), return NULL;);		
			memset(neighborapelem, 0,sizeof(struct Neighbor_AP_ELE));
			/////////////////copy information//////////////////////
			memcpy(neighborapelem->BSSID, phead->BSSID, 6);
			neighborapelem->Rate = phead->Rate;
			
			neighborapelem->Channel = phead->Channel;
			neighborapelem->RSSI = phead->RSSI;
			neighborapelem->NOISE = phead->NOISE;
			neighborapelem->BEACON_INT = phead->BEACON_INT;
			
			neighborapelem->status = phead->status;
			neighborapelem->opstatus = phead->opstatus;
			neighborapelem->capabilityinfo = phead->capabilityinfo;
			
			charlen = strlen((char*)phead->ESSID);
			if(charlen > 32)
				charlen = 32;
			//neighborapelem->ESSID = (char *)malloc(charlen+1);
			memset(neighborapelem->ESSID, 0, charlen+1);
			memcpy(neighborapelem->ESSID, phead->ESSID, charlen);


			charlen = strlen(phead->IEs_INFO);
			neighborapelem->IEs_INFO = (char *)malloc(charlen+1);
			memset(neighborapelem->IEs_INFO, 0, charlen+1);
			memcpy(neighborapelem->IEs_INFO, phead->IEs_INFO, charlen);
				
			neighborapelem->next = NULL;

			

			if(rouge_ap_info->neighborapInfos == NULL)
			{
				rouge_ap_info->neighborapInfos = neighborapelem;
				prougehead = neighborapelem;
				neighborapelem = NULL;
			}
			else
			{
				prougehead->next = neighborapelem;
				prougehead = neighborapelem;
				neighborapelem = NULL;
			}

			rouge_ap_info->neighborapInfosCount++;
		}
		else
		{
			flag = 0;
		}

		phead = phead->next;
	}

	//printf("wid_check_rogue_ap_mac display_ap_info_list    \n");

	//display_ap_info_list(rouge_ap_info);

	
	
	if(rouge_ap_info->neighborapInfosCount != 0)
	{
		return rouge_ap_info;
	}
	else
	{
		CW_FREE_OBJECT(rouge_ap_info);
		return NULL;
	}

}		

void wid_mark_rogue_ap(Neighbor_AP_INFOS *paplist)
{
	if((paplist == NULL)||(paplist->neighborapInfos == NULL)||(paplist->neighborapInfosCount== 0))
	{
		//printf("wid_mark_rogue_ap parameter error\n");
		return ;
	}	

	struct Neighbor_AP_ELE *phead = paplist->neighborapInfos;
	int i = 0;
	int j = 0;
	int k = 0;
	int breakflag = 0;

	while(phead != NULL)
	{
		for(i=0; i<WTP_NUM; i++)
		{
		///////////////////
			if ((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
			{
				for(j=0;j<AC_WTP[i]->RadioCount;j++)
				{				
					if(AC_WTP[i]->WTP_Radio[j] != NULL)
					{
						for(k=0;k<L_BSS_NUM;k++)
						{
							if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSID != NULL))
							{
								if (memcmp((AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSID),phead->BSSID,6) == 0)
								{

									phead->wtpid = i;
									phead->status = 2;
									if(AC_WTP[i]->WTP_Radio[j]->Radio_Chan == 0)
										AC_WTP[i]->WTP_Radio[j]->Radio_Chan = phead->Channel;
									breakflag = 1;
									break;
								}
							}
						}
					}
					if(breakflag ==1)
						break;
				}

			}		
			if(breakflag ==1)
				break;
	
			
		}
		if(breakflag == 1)
			breakflag = 0;
		//phead->wtpid = 0;
		//phead->status = 1;
		phead = phead->next;
	}

}
int wid_count_rogue_ap(Neighbor_AP_INFOS *paplist,int wtpid)
{
	//printf("here is in wid_count_rogue_ap\n");
	unsigned int count=0;
	unsigned int m = 0;
	unsigned int interval = 0;
	unsigned int times = 0;
	//printf("here is in wid_count_rogue_ap 0000000000000\n");
	
	if((paplist == NULL)||(paplist->neighborapInfos == NULL)||(paplist->neighborapInfosCount== 0))
	{
		//printf("wid_mark_rogue_ap parameter error\n");
		return -1;
	}	
	struct Neighbor_AP_ELE *phead = paplist->neighborapInfos;
	
	int i=0;
	//printf("here is in wid_count_rogue_ap 11111111111111\n");
	while(phead!=NULL){
	//	printf("here is in wid_count_rogue_ap 222222222222222\n");
		for(i=0;i<WTP_NUM;i++){
	//		printf("here is in wid_count_rogue_ap 33333333333333333\n");
			if(phead->status == 1){
				
	//			printf("here is in wid_count_rogue_ap 4444444444444\n");
				count ++;
				break;
			}
		}
		phead = phead->next;
	}
	//printf("here is in wid_count_rogue_ap 5555555555555\n");
	//printf("count is %d \n",count);
	wid_syslog_debug_debug(WID_DEFAULT,"***this is in wid_count_rogue_ap***cur rogue ap count is %d \n",count);
	printf("***cur rogue ap count is %d \n",count);
	if(count>=neighborrogueapcount){
		wid_syslog_debug_debug(WID_DEFAULT,"***wid_count_rogue_ap***gtrapflag is %d \n",gtrapflag);
		//printf("count>neighborrogueapcount ,and neighborrogueapcount is %d \n",neighborrogueapcount);
		m = AC_WTP[wtpid]->trap_collect_time%AC_WTP[wtpid]->wifi_extension_reportinterval;
		interval = gapscanset.reportinterval;
		if(interval != 0){
			if(m == 0){
				times = AC_WTP[wtpid]->trap_collect_time/interval;
			}else{
				times = AC_WTP[wtpid]->trap_collect_time/interval + 1;
			}
		}
		AC_WTP[wtpid]->rogueaptrap_resend_times ++;
		if(gtrapflag>=4){
			if(AC_WTP[wtpid]->ap_rogue_threshold_flag == 1)    //fengwenchao add 20110221
			{
				char flag=0;
				if((gtrap_rogue_ap_threshold_switch  == 1)&&(AC_WTP[wtpid]->wtp_rogue_ap_threshold <= count))
					{
						if(AC_WTP[wtpid]->rogueaptrap_resend_times >= times)
							{
								AC_WTP[wtpid]->rogueaptrap_resend_times = 0;
								wid_dbus_trap_ap_rogue_threshold(wtpid,count,flag);
								AC_WTP[wtpid]->ap_rogue_threshold_flag = 0;    //fengwenchao add 20110221
							}
					}
			}
		}	
	}
	//printf("count<neighborrogueapcount ,and neighborrogueapcount is %d \n",neighborrogueapcount);
	return CW_TRUE;
}
CWBool insert_elem_into_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem)
{
	if((elem == NULL)||(paplist == NULL))
	{
		//printf("insert_elem_into_ap_list parameter error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"insert_elem_into_ap_list parameter error\n");
		return CW_FALSE;
	}	

	if(paplist->neighborapInfosCount == 0)
	{
		paplist->neighborapInfos = elem;
		elem->next = NULL;
		paplist->neighborapInfosCount++;	
		return CW_TRUE;
	}
	/*
	struct Neighbor_AP_ELE *pnode = paplist->neighborapInfos;

	paplist->neighborapInfos = elem;
	elem->next = pnode;
	paplist->neighborapInfosCount++;	
	*/

	elem->next = paplist->neighborapInfos;
	paplist->neighborapInfos = elem;
	paplist->neighborapInfosCount++;	
	
	//printf("insert_elem_into_ap_list insert success\n");

	return CW_TRUE;//insert success
	
}
CWBool modify_elem_into_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem)
{
	if((elem == NULL)||((paplist) == NULL)||((paplist)->neighborapInfos == NULL)||((paplist)->neighborapInfosCount== 0))
	{
		//printf("modify_elem_into_ap_list parameter error\n");
		return CW_FALSE;
	}	


	
	struct Neighbor_AP_ELE *pnode = paplist->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;

	if(memcmp((pnode->BSSID),elem->BSSID,6) == 0)
	{
		pnode->Channel = elem->Channel;
		pnode->RSSI = elem->RSSI;
		pnode->NOISE = elem->NOISE;
		
		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->BSSID),elem->BSSID,6) == 0)
		{
			pnext->Channel = elem->Channel;
			pnext->RSSI = elem->RSSI;
			pnext->NOISE = elem->NOISE;

			return CW_TRUE;

		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}

	//printf("modify_elem_into_ap_list modify success\n");

	return CW_FALSE;//insert success

}

CWBool delete_elem_from_ap_list_wtpid(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem,int wtpid)
{
	if((elem == NULL)||((paplist) == NULL)||((paplist)->neighborapInfos == NULL)||((paplist)->neighborapInfosCount== 0))
	{
		//printf("delete_elem_from_ap_list parameter error\n");
		return CW_FALSE;
	}	

	char currentchannel = AC_WTP[wtpid]->WTP_Radio[0]->Radio_Chan;
	
	struct Neighbor_AP_ELE *pnode = (paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;

	if(memcmp((pnode->BSSID),elem->BSSID,6) == 0)
	{
		(paplist)->neighborapInfos = (paplist)->neighborapInfos->next;


		if(gtrapflag>=4){
			if(AC_WTP[wtpid]->channel_device_interference_flag == 1)    //fengwenchao add 20110221
				{
			wid_dbus_trap_wtp_channel_device_interference_clear(wtpid,currentchannel);
					AC_WTP[wtpid]->channel_device_interference_flag = 0;   //fengwenchao add 20110221
				}
			}
					
		if((pnode->IEs_INFO != NULL)&&(strncmp(pnode->IEs_INFO,"E",1)==0))
		{
							
			if(gtrapflag>=4){
				if(AC_WTP[wtpid]->samechannel_trap_flag == 1)     //fengwenchao add 20110221
					{
				wid_dbus_trap_wtp_channel_ap_interference_clear(wtpid,currentchannel);
						AC_WTP[wtpid]->samechannel_trap_flag = 0;   //fengwenchao add 20110221
					}
			}
			
		}
		else if((pnode->IEs_INFO != NULL)&&(strncmp(pnode->IEs_INFO,"I",1)==0))
		{
			if(gtrapflag>=4){
				if(AC_WTP[wtpid]->wid_trap.rogue_terminal_trap_flag == 1)   //fengwenchao add 20110221
					{
				wid_dbus_trap_wtp_channel_terminal_interference_clear(wtpid,0,currentchannel,NULL);	
						AC_WTP[wtpid]->wid_trap.rogue_terminal_trap_flag = 0;  //fengwenchao add 20110221
					}
			}
		}
		else
		{
		}

		//CW_FREE_OBJECT(pnode->ESSID);
		CW_FREE_OBJECT(pnode->IEs_INFO);

		CW_FREE_OBJECT(pnode);

		(paplist)->neighborapInfosCount--;


		//printf("delete_elem_from_ap_list delete node success\n");
		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->BSSID),elem->BSSID,6) == 0)
		{		
			if(gtrapflag>=4){
				if(AC_WTP[wtpid]->channel_device_interference_flag == 1)     //fengwenchao add 20110221
					{
				wid_dbus_trap_wtp_channel_device_interference_clear(wtpid,currentchannel);
						AC_WTP[wtpid]->channel_device_interference_flag = 0;   //fengwenchao add 20110221
					}
				}
						
			if((pnext->IEs_INFO != NULL)&&(strncmp(pnext->IEs_INFO,"E",1)==0))
			{
								
				if(gtrapflag>=4){
					if(AC_WTP[wtpid]->samechannel_trap_flag == 1)     //fengwenchao add 20110221
						{
					wid_dbus_trap_wtp_channel_ap_interference_clear(wtpid,currentchannel);
							AC_WTP[wtpid]->samechannel_trap_flag = 0;   //fengwenchao add 20110221
						}
				}
				
			}
			else if((pnext->IEs_INFO != NULL)&&(strncmp(pnext->IEs_INFO,"I",1)==0))
			{
			if(gtrapflag>=4){
				if(AC_WTP[wtpid]->wid_trap.rogue_terminal_trap_flag == 1)   //fengwenchao add 20110221
					{
					wid_dbus_trap_wtp_channel_terminal_interference_clear(wtpid,0,currentchannel,NULL);
						AC_WTP[wtpid]->wid_trap.rogue_terminal_trap_flag = 0;  //fengwenchao add 20110221
					}
				}
			}
			else
			{
			}
			
			pnode->next = pnext->next;

			//CW_FREE_OBJECT(pnext->ESSID);
			CW_FREE_OBJECT(pnext->IEs_INFO);

			CW_FREE_OBJECT(pnext);	
			(paplist)->neighborapInfosCount--;
			
			//printf("delete_elem_from_ap_list delete node success\n");

			return CW_TRUE;

		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}
	
	//printf("delete_elem_from_ap_list delete node fail\n");

	return CW_FALSE;
}

CWBool delete_elem_from_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem)
{
	if((elem == NULL)||((paplist) == NULL)||((paplist)->neighborapInfos == NULL)||((paplist)->neighborapInfosCount== 0))
	{
		//printf("delete_elem_from_ap_list parameter error\n");
		return CW_FALSE;
	}	

	
	struct Neighbor_AP_ELE *pnode = (paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;

	if(memcmp((pnode->BSSID),elem->BSSID,6) == 0)
	{
		(paplist)->neighborapInfos = (paplist)->neighborapInfos->next;

		//CW_FREE_OBJECT(pnode->ESSID);
		CW_FREE_OBJECT(pnode->IEs_INFO);

		CW_FREE_OBJECT(pnode);

		(paplist)->neighborapInfosCount--;


		//printf("delete_elem_from_ap_list delete node success\n");
		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->BSSID),elem->BSSID,6) == 0)
		{
			pnode->next = pnext->next;

			//CW_FREE_OBJECT(pnext->ESSID);
			CW_FREE_OBJECT(pnext->IEs_INFO);

			CW_FREE_OBJECT(pnext);

			
			(paplist)->neighborapInfosCount--;
			
			//printf("delete_elem_from_ap_list delete node success\n");

			return CW_TRUE;

		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}
	
	//printf("delete_elem_from_ap_list delete node fail\n");

	return CW_FALSE;
}
void merge_ap_list(Neighbor_AP_INFOS *pdestlist,Neighbor_AP_INFOS **psrclist,int wtpid)
{
	if((pdestlist == NULL)||(pdestlist->neighborapInfos == NULL)||(pdestlist->neighborapInfosCount== 0))
	{
		//printf("merge_ap_list dest parameter error\n");
		return ;
	}	
	
	if(((*psrclist) == NULL)||((*psrclist)->neighborapInfos == NULL)||((*psrclist)->neighborapInfosCount== 0))
	{
		//printf("merge_ap_list src parameter error\n");
		CW_FREE_OBJECT(*psrclist);
		return ;
	}	

	struct Neighbor_AP_ELE *pnode = (*psrclist)->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;

	if(pnode->opstatus == 1) //add element
	{
		insert_elem_into_ap_list(pdestlist,pnode);
		//printf("insert_elem_into_ap_list src modify node success\n");
	}
	else if(pnode->opstatus == 2)//delete element
	{
		delete_elem_from_ap_list_wtpid(pdestlist,pnode,wtpid);

		//CW_FREE_OBJECT(pnode->ESSID);
		CW_FREE_OBJECT(pnode->IEs_INFO);

		CW_FREE_OBJECT(pnode);
		//printf("delete_elem_from_ap_list src modify node success\n");
	}
	else //change element
	{
		modify_elem_into_ap_list(pdestlist,pnode);

		//CW_FREE_OBJECT(pnode->ESSID);
		CW_FREE_OBJECT(pnode->IEs_INFO);

		CW_FREE_OBJECT(pnode);

		//printf("modify_elem_into_ap_list src modify node success\n");

	}

	while(pnext != NULL)
	{
		pnode = pnext ->next;
		
		if(pnext->opstatus == 1) //add element
		{
			insert_elem_into_ap_list(pdestlist,pnext);
			//printf("insert_elem_into_ap_list src modify node success\n");
		}
		else if(pnext->opstatus == 2)//delete element
		{
			delete_elem_from_ap_list(pdestlist,pnext);
		
			//CW_FREE_OBJECT(pnext->ESSID);
			CW_FREE_OBJECT(pnext->IEs_INFO);
		
			CW_FREE_OBJECT(pnext);
			//sprintf("delete_elem_from_ap_list src modify node success\n");
		}
		else //change element
		{
			modify_elem_into_ap_list(pdestlist,pnext);
		
			//CW_FREE_OBJECT(pnext->ESSID);
			CW_FREE_OBJECT(pnext->IEs_INFO);
		
			CW_FREE_OBJECT(pnext);
		
			//printf("modify_elem_into_ap_list src modify node success\n");
		
		}

		pnext = pnode;
		
	}

	CW_FREE_OBJECT(*psrclist);
	
	//printf("merge_ap_list to end\n");
	
	
	return ;
}

void delete_rouge_ap_list_by_ouilist(Neighbor_AP_INFOS **paplist)
{
	if(((*paplist) == NULL)||((*paplist)->neighborapInfos == NULL)||((*paplist)->neighborapInfosCount== 0))

	{
		//printf("delete_rouge_ap_list_by_ouilist parameter error\n");
		return ;
	}	

	struct oui_node *node_oui =NULL;           //filter   oui
	if(g_oui_list.oui_list!=NULL){
		node_oui = g_oui_list.oui_list;
	}

	while(node_oui!=NULL){
		delete_elem_from_ap_list_byoui(paplist,node_oui);
		node_oui=node_oui->next;
	}

}

void delete_rouge_ap_list_by_essidlist(Neighbor_AP_INFOS **paplist)
{
	if(((*paplist) == NULL)||((*paplist)->neighborapInfos == NULL)||((*paplist)->neighborapInfosCount== 0)){
		//printf("delete_rouge_ap_list_by_essid_list parameter error\n");
		return ;
	}

	
	struct essid_node *node_essid =NULL;           //filter   essid
	if(g_essid_list.essid_list!=NULL){
		node_essid= g_essid_list.essid_list;
	}

	while(node_essid!=NULL){
		delete_elem_from_ap_list_byessid(paplist,node_essid);
		node_essid=node_essid->next;
	}
	

}

void delete_rouge_ap_list_by_whitelist(Neighbor_AP_INFOS **paplist)
{
	if(((*paplist) == NULL)||((*paplist)->neighborapInfos == NULL)||((*paplist)->neighborapInfosCount== 0))

	{
		//printf("delete_rouge_ap_list_by_whitelist parameter error\n");
		return ;
	}	

	struct white_mac *pmacnode = NULL;
	
	if(pwhite_mac_list != NULL)
	{
		pmacnode = pwhite_mac_list->list_mac;
	}

	while(pmacnode != NULL)
	{
		delete_elem_from_ap_list_bymac(paplist, pmacnode);
		pmacnode = pmacnode->next;
	}


}

struct Neighbor_AP_ELE * create_mac_elem(char *mac)
{
	struct Neighbor_AP_ELE *neighborapelem = NULL;
	CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct Neighbor_AP_ELE), return NULL;);		

	memcpy(neighborapelem->BSSID ,mac, 6);
	
	neighborapelem->Rate = 110;
	
	neighborapelem->Channel = 1;
	neighborapelem->RSSI = 1;
	neighborapelem->NOISE = 1;
	neighborapelem->BEACON_INT = 10;
	
	neighborapelem->status = 0;
	neighborapelem->opstatus = 0;
	neighborapelem->capabilityinfo = 0;
	
	char *essid = (char *)malloc(5);
	memset(essid,0,5);
	memcpy(essid,"way",3);

	memset(neighborapelem->ESSID,0,strlen(essid)+1);
	memcpy(neighborapelem->ESSID,essid,strlen(essid));
	//neighborapelem->ESSID = essid;
	
	char *ie = (char *)malloc(7);
	memset(ie,0,7);
	memcpy(ie,"capwap",6);
	neighborapelem->IEs_INFO = ie;
	
	neighborapelem->next = NULL;
	if(essid){
		free(essid);
		essid = NULL;
	}
	return neighborapelem;
}

Neighbor_AP_INFOS * create_ap_info_list_test(int count)//just for test will delete later
{
	Neighbor_AP_INFOS *create_ap_info;
	int i = 0;
	CW_CREATE_OBJECT_ERR(create_ap_info, Neighbor_AP_INFOS, return NULL;);	
				
	create_ap_info->neighborapInfosCount = count;
	create_ap_info->neighborapInfos = NULL;
	
	struct Neighbor_AP_ELE *neighborapelem = NULL;
	struct Neighbor_AP_ELE *phead = NULL;

	for(i=0; i<create_ap_info->neighborapInfosCount; i++)
	{
		CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct Neighbor_AP_ELE), return NULL;);		
		
		char str[3][6] = {{"444444"},{"222222"},{"333333"}};
		memcpy(neighborapelem->BSSID ,str[i], 6);
		
		neighborapelem->Rate = 110;

		neighborapelem->Channel = 1;
		neighborapelem->RSSI = 1;
		neighborapelem->NOISE = 1;
		neighborapelem->BEACON_INT = 10;
		
		neighborapelem->status = 0;
		neighborapelem->opstatus = 0;
		neighborapelem->capabilityinfo = 0;

		char *essid = (char *)malloc(5);
		memset(essid,0,5);
		memcpy(essid,"way",3);
		//neighborapelem->ESSID = essid;
		memset(neighborapelem->ESSID,0,strlen(essid)+1);
		memcpy(neighborapelem->ESSID,essid,strlen(essid));


		char *ie = (char *)malloc(7);
		memset(ie,0,7);
		memcpy(ie,"capwap",6);
		neighborapelem->IEs_INFO = ie;

		neighborapelem->next = NULL;

		if(i== 0)
		{
			//printf("parse first ap info\n");
			create_ap_info->neighborapInfos = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		else
		{
			//printf("parse more ap info\n");
			phead->next = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		//printf("######002####\n");
		if(essid){
			free(essid);
			essid = NULL;
		}
	}	

	return create_ap_info;
	//display_ap_info_list(create_ap_info);


}
white_mac_list * wid_check_white_mac()
{
	int i = 0;
	int j = 0;
	int k = 0;
	struct white_mac *pmac = NULL;
	struct white_mac *phead = NULL;
	white_mac_list *maclist = NULL;
	CW_CREATE_OBJECT_ERR(maclist, white_mac_list, return NULL;);
	
	maclist->imaccount = 0;
	maclist->list_mac = NULL;

	for(i=0; i<WTP_NUM; i++)
	{		
		if ((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
			for(j=0;j<AC_WTP[i]->RadioCount;j++)
			{				
				if(AC_WTP[i]->WTP_Radio[j] != NULL)
				{
					for(k=0;k<L_BSS_NUM;k++)
					{
						if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSID != NULL))
						{
							CW_CREATE_OBJECT_SIZE_ERR(pmac,sizeof(struct white_mac),return NULL;);

							memcpy(pmac->elem_mac, AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSID, 6);
							pmac->next = NULL;

							if(maclist->list_mac == NULL)
							{
								maclist->list_mac = pmac;
								maclist->imaccount++;
							}
							else
							{
								phead = maclist->list_mac;
								maclist->list_mac = pmac;
								pmac->next = phead;
								maclist->imaccount++;
							}
						}
					}
				}
			}

		}
	}

	
	return maclist;
	
}
Neighbor_AP_INFOS * wid_get_neighbor_ap_list()
{
	int i = 0;
	CWBool ret = CW_FALSE;
	struct Neighbor_AP_ELE *phead = NULL;
	struct Neighbor_AP_ELE *Pnode = NULL;
	
	Neighbor_AP_INFOS *create_ap_info;
	CW_CREATE_OBJECT_ERR(create_ap_info, Neighbor_AP_INFOS, return NULL;);		
	create_ap_info->neighborapInfosCount = 0;
	create_ap_info->neighborapInfos = NULL;

	for(i=0; i<WTP_NUM; i++)
	{
		
		if ((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
			CWThreadMutexLock(&(gWTPs[i].RRMThreadMutex));
			if(AC_WTP[i]->NeighborAPInfos != NULL){
				phead = AC_WTP[i]->NeighborAPInfos->neighborapInfos;

				while(phead != NULL)
				{
					ret = check_elem_in_ap_list(create_ap_info,phead);
					if(ret == CW_FALSE)
					{
						Pnode = create_ap_elem(phead);
						if(Pnode != NULL)
						{
							if(insert_elem_into_ap_list_head(create_ap_info,Pnode) == CW_FALSE)
							{
								//CW_FREE_OBJECT(Pnode->ESSID);
								CW_FREE_OBJECT(Pnode->IEs_INFO);

								CW_FREE_OBJECT(Pnode);
							}
						}
						
					}
					phead = phead->next;
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[i].RRMThreadMutex));
		}		
	}


	if((create_ap_info->neighborapInfos == NULL)||(create_ap_info->neighborapInfosCount == 0))
	{
		CW_FREE_OBJECT(create_ap_info);
		return NULL;
	}

	
	return create_ap_info;

}
//==================================================================
CWBool delete_elem_from_ap_list_byoui(Neighbor_AP_INFOS **paplist,struct oui_node *oui)
{
	if((oui == NULL)||((*paplist) == NULL)||((*paplist)->neighborapInfos == NULL)||((*paplist)->neighborapInfosCount== 0))
	{
		//printf("delete_elem_from_ap_list_byoui parameter error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"delete_elem_from_ap_list_byoui parameter error\n");
		return CW_FALSE;
	}	
	/*
	struct Neighbor_AP_ELE *pnode = (*paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pn = (*paplist)->neighborapInfos;

	while(pnode != NULL)
	{
		if(memcmp((pnode->BSSID),oui->oui,3) == 0){	
			if(pnode==pn){
				(*paplist)->neighborapInfos=pnode->next;
			}else{
				pn->next = pnode->next;
			}
			
			CW_FREE_OBJECT(pnode->ESSID);
			CW_FREE_OBJECT(pnode->IEs_INFO);

			CW_FREE_OBJECT(pnode);

			(*paplist)->neighborapInfosCount--;
			
			printf("delete_elem_from_ap_list_byoui delete node success\n");

			pnode=pn->next;
		}else{
			pn=pnode;
			pnode=pnode->next;
				printf("2\n");
		}
	}

	printf("1\n");
	*/

	struct Neighbor_AP_ELE *pnode = (*paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;



	if(memcmp((pnode->BSSID),oui->oui,3) == 0)
	{
		(*paplist) ->neighborapInfos = (*paplist)->neighborapInfos->next;

		//CW_FREE_OBJECT(pnode->ESSID);
		CW_FREE_OBJECT(pnode->IEs_INFO);

		CW_FREE_OBJECT(pnode);

		(*paplist)->neighborapInfosCount--;

		if((*paplist)->neighborapInfosCount == 0)
		{
			CW_FREE_OBJECT((*paplist));
		}
//		printf("delete_elem_from_ap_list_byoui delete node success\n");
		//return CW_TRUE;
		pnode=pnext;
		if(pnext!=NULL){
			pnext=pnext->next;
		}else{
			return CW_TRUE;
		}

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->BSSID),oui->oui,3) == 0)
		{
			pnode->next = pnext->next;

			//CW_FREE_OBJECT(pnext->ESSID);
			CW_FREE_OBJECT(pnext->IEs_INFO);

			CW_FREE_OBJECT(pnext);

			
			(*paplist)->neighborapInfosCount--;
			
//			printf("delete_elem_from_ap_list_byoui delete node success\n");

			//return CW_TRUE;
			pnext = pnode->next;
		}else{

			pnode = pnode->next;
			pnext = pnext->next;
		}
	}
	
//	printf("delete_elem_from_ap_list_byoui delete no this node \n");
	wid_syslog_debug_debug(WID_DEFAULT,"delete_elem_from_ap_list_byoui delete no this node \n");

	return CW_FALSE;	
}

CWBool delete_elem_from_ap_list_byessid(Neighbor_AP_INFOS **paplist,struct essid_node *essid)
{
	if((essid == NULL)||((*paplist) == NULL)||((*paplist)->neighborapInfos == NULL)||((*paplist)->neighborapInfosCount== 0))
	{
		//printf("delete_elem_from_ap_list_byessid parameter error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"delete_elem_from_ap_list_byessid parameter error\n");
		return CW_FALSE;
	}	
/*
	struct Neighbor_AP_ELE *pnode = (*paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pn = (*paplist)->neighborapInfos;

	while(pnode != NULL)
	{
		if(strcmp((pnode->ESSID),essid->essid) == 0){	
			if(pnode==pn){
				(*paplist)->neighborapInfos=pnode->next;
			}else{
				pn->next = pnode->next;
			}
			
			CW_FREE_OBJECT(pnode->ESSID);
			CW_FREE_OBJECT(pnode->IEs_INFO);

			CW_FREE_OBJECT(pnode);

			(*paplist)->neighborapInfosCount--;
			
			printf("delete_elem_from_ap_list_byessid delete node success\n");

			pnode=pn->next;
		}else{
			pn=pnode;
			pnode=pnode->next;
		}
	}
*/

	
	struct Neighbor_AP_ELE *pnode = (*paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;

	if(memcmp((pnode->ESSID),essid->essid,essid->len) == 0)
	{
		(*paplist) ->neighborapInfos = (*paplist)->neighborapInfos->next;

		//CW_FREE_OBJECT(pnode->ESSID);
		CW_FREE_OBJECT(pnode->IEs_INFO);

		CW_FREE_OBJECT(pnode);

		(*paplist)->neighborapInfosCount--;

		if((*paplist)->neighborapInfosCount == 0)
		{
			CW_FREE_OBJECT((*paplist));
		}
//		printf("delete_elem_from_ap_list_byessid delete node success\n");
		//return CW_TRUE;

		pnode=pnext;
		if(pnext!=NULL){
			pnext=pnext->next;
		}else{
			return CW_TRUE;
		}
	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->ESSID),essid->essid,essid->len) == 0)
		{
			pnode->next = pnext->next;

			//CW_FREE_OBJECT(pnext->ESSID);
			CW_FREE_OBJECT(pnext->IEs_INFO);

			CW_FREE_OBJECT(pnext);

			
			(*paplist)->neighborapInfosCount--;
			
//			printf("delete_elem_from_ap_list_byessid delete node success\n");

			//return CW_TRUE;
			pnext = pnode->next;
		}else{

			pnode = pnode->next;
			pnext = pnext->next;
		}
	}
	
//	printf("delete_elem_from_ap_list_byessid delete no this node \n");
	wid_syslog_debug_debug(WID_DEFAULT,"delete_elem_from_ap_list_byessid delete no this node \n");
	return CW_FALSE;	
}

//==================================================================

CWBool delete_elem_from_ap_list_bymac(Neighbor_AP_INFOS **paplist,struct white_mac *pmac)
{
	if((pmac == NULL)||((*paplist) == NULL)||((*paplist)->neighborapInfos == NULL)||((*paplist)->neighborapInfosCount== 0))
	{
		//printf("delete_elem_from_ap_list_bymac parameter error\n");
		return CW_FALSE;
	}	

	
	struct Neighbor_AP_ELE *pnode = (*paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;

	if(memcmp((pnode->BSSID),pmac->elem_mac,6) == 0)
	{
		(*paplist) ->neighborapInfos = (*paplist)->neighborapInfos->next;

		//CW_FREE_OBJECT(pnode->ESSID);
		CW_FREE_OBJECT(pnode->IEs_INFO);

		CW_FREE_OBJECT(pnode);

		(*paplist)->neighborapInfosCount--;

		if((*paplist)->neighborapInfosCount == 0)
		{
			CW_FREE_OBJECT((*paplist));
		}
		//printf("delete_elem_from_ap_list_bymac delete node success\n");
		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->BSSID),pmac->elem_mac,6) == 0)
		{
			pnode->next = pnext->next;

			//CW_FREE_OBJECT(pnext->ESSID);
			CW_FREE_OBJECT(pnext->IEs_INFO);

			CW_FREE_OBJECT(pnext);

			
			(*paplist)->neighborapInfosCount--;
			
			//printf("delete_elem_from_ap_list_bymac delete node success\n");

			return CW_TRUE;

		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}
	
	//printf("delete_elem_from_ap_list_bymac delete no this node \n");

	return CW_FALSE;	
}
void wid_destroy_white_mac(white_mac_list **pmaclist)
{
	//int i = 0;
	if(((*pmaclist) == NULL)||((*pmaclist)->list_mac == NULL))
	{
		//printf("wid_destroy_white_mac parameter error\n");
		return;
	}

	struct white_mac *phead = NULL;
	struct white_mac *pnext = NULL;
	phead = (*pmaclist)->list_mac;
	(*pmaclist)->list_mac = NULL;
		
	//printf("## wid_destroy_white_mac total count is = %d: ##\n",(*pmaclist)->imaccount);
	while(phead != NULL)
	{	
		//printf("## the count i = %d##\n",i++);
		
		pnext = phead->next;

		CW_FREE_OBJECT(phead);

		phead = pnext;
	}

	(*pmaclist)->imaccount= 0;
	CW_FREE_OBJECT(*pmaclist);

}
void display_mac_info_list(white_mac_list *pmaclist)
{
	if((pmaclist == NULL)||(pmaclist->list_mac == NULL)||(pmaclist->imaccount == 0))
	{
		printf("display_mac_info_list parameter error\n");
		return;
	}
	
	int i = 0;
	int j = 0;
	
	struct white_mac *phead = pmaclist->list_mac;
		
	printf("## display The mac info list total count is = %d: ##\n",pmaclist->imaccount);
	for(i=0; i<pmaclist->imaccount; i++)
	{
		printf("## the count i = %d##\n",i);
		printf("mac = ");
		for(j=0; j<6; j++)
		{
			printf("%02x", phead->elem_mac[j]);
		}
		printf("\n");

		phead = phead->next;
	}

}

CWBool check_elem_in_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem)
{
	if((elem == NULL)||((paplist) == NULL)||((paplist)->neighborapInfos == NULL)||((paplist)->neighborapInfosCount== 0))
	{
		//printf("check_elem_in_ap_list parameter error\n");
		return CW_FALSE;
	}	

	
	struct Neighbor_AP_ELE *pnode = (paplist)->neighborapInfos;
	struct Neighbor_AP_ELE *pnext = pnode->next;

	if(memcmp((pnode->BSSID),elem->BSSID,6) == 0)
	{

		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->BSSID),elem->BSSID,6) == 0)
		{
			return CW_TRUE;
		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}

	return CW_FALSE;

}
CWBool insert_elem_into_ap_list_head(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem)
{
	if((elem == NULL)||(paplist == NULL))
	{
		//printf("insert_elem_into_ap_list_heads parameter error\n");
		return CW_FALSE;
	}	

	if(paplist->neighborapInfosCount == 0)
	{
		paplist->neighborapInfos = elem;
		paplist->neighborapInfosCount++;	
		return CW_TRUE;
	}
	
	struct Neighbor_AP_ELE *pnode = paplist->neighborapInfos;

	paplist->neighborapInfos = elem;
	elem->next = pnode;
	paplist->neighborapInfosCount++;	

	return CW_TRUE;


}
struct Neighbor_AP_ELE * create_ap_elem(struct Neighbor_AP_ELE *apelem)
{
	struct Neighbor_AP_ELE *neighborapelem = NULL;
	CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct Neighbor_AP_ELE), return NULL;);
	//CW_CREATE_OBJECT_SIZE_ERR(neighborapelem->ESSID,(strlen(apelem->ESSID)+1),return NULL;);
	CW_CREATE_OBJECT_SIZE_ERR(neighborapelem->IEs_INFO,(strlen(apelem->IEs_INFO)+1),return NULL;);


	
	memcpy(neighborapelem->BSSID ,apelem->BSSID, 6);
	
	neighborapelem->Rate = apelem->Rate;

	neighborapelem->Channel = apelem->Channel;
	neighborapelem->RSSI = apelem->RSSI;
	neighborapelem->NOISE = apelem->NOISE;
	neighborapelem->BEACON_INT = apelem->BEACON_INT;
	
	neighborapelem->status = apelem->status;
	neighborapelem->opstatus = apelem->opstatus;
	neighborapelem->capabilityinfo = apelem->capabilityinfo;

	memset(neighborapelem->ESSID, 0, strlen((char*)apelem->ESSID)+1);
	memcpy(neighborapelem->ESSID, apelem->ESSID, strlen((char*)apelem->ESSID));

	memset(neighborapelem->IEs_INFO, 0, strlen(apelem->IEs_INFO)+1);
	memcpy(neighborapelem->IEs_INFO, apelem->IEs_INFO, strlen(apelem->IEs_INFO));

	neighborapelem->fst_dtc_tm = apelem->fst_dtc_tm;
	time(&neighborapelem->lst_dtc_tm);
	neighborapelem->encrp_type = apelem->encrp_type;
	neighborapelem->polcy = apelem->polcy;

	neighborapelem->next = NULL;	

	return neighborapelem;
	
}

void wid_check_wtp_apply_wlan(unsigned char WlanID, char * ifname)
{
	int i = 0;

	for(i=0; i<WTP_NUM; i++)
	{
		
		if ((AC_WTP[i] != NULL)&&(strlen(AC_WTP[i]->BindingIFName) == strlen(ifname))&&(strncmp(AC_WTP[i]->BindingIFName,ifname,strlen(ifname)) == 0))
		{
			WID_DELETE_WLAN_APPLY_WTP(i,WlanID);
		}
	}

}

void display_support_rate_list(struct Support_Rate_List *ratelist)  
{
	if(ratelist == NULL)
	{
		printf("the list is empty\n");
		return;
	}

	
	 while(ratelist != NULL) 
	 {
		  printf("support rate: %d\n",ratelist->Rate);
		  ratelist = ratelist->next;
	 }
	 
	 
}

void destroy_support_rate_list(struct Support_Rate_List *ratelist)
{

	if(ratelist == NULL)
	{
		//printf("the list is empty\n");
		return;
	}
	//int i = 0;
	struct Support_Rate_List *phead = NULL;
	struct Support_Rate_List *pnext = NULL;
	phead = ratelist;
	ratelist = NULL;
		
	//printf("## destroy The list ##\n");
	
	while(phead != NULL)
	{	
		//printf("## the count i = %d##\n",i++);
		
		pnext = phead->next;
	
		CW_FREE_OBJECT(phead);

		phead = pnext;
		
		//printf("###########################################\n");
	}

	

}

struct Support_Rate_List *create_support_rate_list(int count)
{
	struct Support_Rate_List *create_rate_list = NULL;
	int i = 0;
	//CW_CREATE_OBJECT_ERR(create_rate_list,struct Support_Rate_List, return NULL;);	
	
	struct Support_Rate_List *supportrate = NULL;
	struct Support_Rate_List *phead = NULL;

	for(i=0; i<count; i++)
	{
		CW_CREATE_OBJECT_SIZE_ERR(supportrate, sizeof(struct Support_Rate_List), return NULL;);		
		
		
		supportrate->Rate = 0;

		supportrate->next = NULL;

		if(i== 0)
		{
			
			create_rate_list = supportrate;
			phead = supportrate;
			supportrate = NULL;
		}
		else
		{
			
			phead->next = supportrate;
			phead = supportrate;
			supportrate = NULL;
		}

	}	

	return create_rate_list;



}


struct Support_Rate_List *find_rate_from_list(struct Support_Rate_List *ratelist,int rate)     
{
	struct Support_Rate_List *ptr = NULL;       
	ptr = ratelist;  
		 
	while(ptr != NULL)              
	{
		if(ptr->Rate == rate)     
		{
			//printf("find it\n");
			return ptr;
		}
		else
		{
			ptr = ptr->next;       
		}
	}
	//printf("no find\n");
	return ptr;//here return null
}

struct Support_Rate_List *insert_rate_into_list(struct Support_Rate_List *ratelist,int rate)
{
	//find first
	struct Support_Rate_List *p = NULL;
	p = find_rate_from_list(ratelist,rate);

	if (p != NULL)//rate is in the list
	{
		//printf("the rate is in the list,no insert\n");
		return ratelist;
	}

	else//insert
	{
		//create new element
		struct Support_Rate_List *newnode = NULL;
		newnode=(struct Support_Rate_List *)malloc(sizeof(struct Support_Rate_List));
		if(!newnode)  
		{
			//printf("malloc error\n");
			wid_syslog_info("malloc error\n");
			return NULL;
		}

		newnode->Rate = rate;          
		newnode->next = NULL;


		//compare the new rate with the first rate,if bigger,add at head
		
		if(rate > ratelist->Rate)    
		{
			newnode->next = ratelist;         //head
			return newnode;
		}
		else//find the smaller one,then add in front of it
		{
			struct Support_Rate_List *ptr = NULL;
			struct Support_Rate_List *temp = NULL;
			ptr = ratelist;
			while(ptr != NULL)              
			{
				if(rate > ptr->Rate)     
				{
					temp->next = newnode;  //newnode catch front

					newnode->next = ptr;         //front
					return ratelist;
				}
				else
				{
					temp = ptr;
					ptr = ptr->next;       
			
			if(ptr->next == NULL)            //end
			{
				ptr->next = newnode;
			}
				}
			}
		}
		return ratelist;

	}
}
struct Support_Rate_List *delete_rate_from_list(struct Support_Rate_List *ratelist,int rate)
{
	//find first
	struct Support_Rate_List *ptr = NULL;
	struct Support_Rate_List *temp = NULL;
	ptr = ratelist;
	temp = ratelist;  //fengwenchao add 20110411
	if(ratelist->Rate == rate)
	{
		ratelist = ratelist->next;
	}

	while(ptr != NULL)              
	{
		if(ptr->Rate == rate)     
		{
			//printf("find it\n");
			//del
			temp->next = ptr->next;
			ptr->next = NULL;
			CW_FREE_OBJECT(ptr);
			
			return ratelist;
		}
		else
		{
			temp = ptr;
			ptr = ptr->next;       
		}
	}
	//printf("not find\n");
	
	return ratelist;

}

int   length_of_rate_list(struct Support_Rate_List *ratelist)   
  {   
          struct Support_Rate_List *p = NULL;
		  p = ratelist;   
          int   k = 0;   
          while(p !=NULL)   
          {   
                k++;   
                p=p->next;   
          }   
          return   k;   
  } 

 CWBool wid_add_mac_blacklist(unsigned char * pmac)
{
	if(pblack_mac_list == NULL)
	{
		CW_CREATE_OBJECT_ERR(pblack_mac_list, white_mac_list, return CW_FALSE;);
		pblack_mac_list->imaccount = 0;
	 	pblack_mac_list->list_mac = NULL;
	} 

	struct white_mac *phead = pblack_mac_list->list_mac;		
	struct white_mac *pnode = phead;
	
	while(phead != NULL)
	{
		if(memcmp((phead->elem_mac),pmac,6) == 0)
		{
			break;
		}
		pnode = phead;
		phead = phead->next;
	}

	if(phead == NULL)
	{
		struct white_mac *pnewnode = NULL;
		CW_CREATE_OBJECT_SIZE_ERR(pnewnode,sizeof(struct white_mac),return CW_FALSE;);

	 	memcpy(pnewnode->elem_mac,pmac, 6);
		pnewnode->next = NULL;

		if(pnode == NULL)
		{
			pblack_mac_list->imaccount++;
	 		pblack_mac_list->list_mac = pnewnode;	
		}
		else
		{
			pblack_mac_list->imaccount++;
	 		pnode->next = pnewnode;				
		}
	}

	return CW_TRUE;

}
CWBool wid_add_mac_whitelist(unsigned char * pmac)
{
	if(pwhite_mac_list == NULL)
	{
		CW_CREATE_OBJECT_ERR(pwhite_mac_list, white_mac_list, return CW_FALSE;);
		pwhite_mac_list->imaccount = 0;
	 	pwhite_mac_list->list_mac = NULL;
	} 

	struct white_mac *phead = pwhite_mac_list->list_mac;		
	struct white_mac *pnode = phead;
	
	while(phead != NULL)
	{
		if(memcmp((phead->elem_mac),pmac,6) == 0)
		{
			break;
		}
		pnode = phead;
		phead = phead->next;
	}

	if(phead == NULL)
	{
		struct white_mac *pnewnode = NULL;
		CW_CREATE_OBJECT_SIZE_ERR(pnewnode,sizeof(struct white_mac),return CW_FALSE;);

	 	memcpy(pnewnode->elem_mac,pmac, 6);
		pnewnode->next = NULL;

		if(pnode == NULL)
		{
			pwhite_mac_list->imaccount++;
	 		pwhite_mac_list->list_mac = pnewnode;	
		}
		else
		{
			pwhite_mac_list->imaccount++;
	 		pnode->next = pnewnode;				
		}
	}

	return CW_TRUE;

}
CWBool wid_delete_mac_blacklist(unsigned char * pmac)
{
	if(pblack_mac_list != NULL)
	{
		struct white_mac *phead = pblack_mac_list->list_mac;		
		struct white_mac *pnode = phead;
		
		while(phead != NULL)
		{
			if(memcmp((phead->elem_mac),pmac,6) == 0)
			{
				//delete node
				if(phead == pnode)
				{
					pblack_mac_list->list_mac = phead->next;
					pblack_mac_list->imaccount--;
					CW_FREE_OBJECT(phead);
					if(pblack_mac_list->imaccount == 0)
					{
						CW_FREE_OBJECT(pblack_mac_list);
					}
					break;
				}
				else
				{
					pnode->next = phead->next;
					CW_FREE_OBJECT(phead);
					pblack_mac_list->imaccount--;
					break;
					
				}
			}
			pnode = phead;
			phead = phead->next;
		}
	}
	
	return CW_TRUE;

}
CWBool wid_delete_mac_whitelist(unsigned char * pmac)
{
	if(pwhite_mac_list != NULL)
	{
		struct white_mac *phead = pwhite_mac_list->list_mac;		
		struct white_mac *pnode = phead;
		
		while(phead != NULL)
		{
			if(memcmp((phead->elem_mac),pmac,6) == 0)
			{
				//delete node
				if(phead == pnode)
				{
					pwhite_mac_list->list_mac = phead->next;
					pwhite_mac_list->imaccount--;
					CW_FREE_OBJECT(phead);

					if(pwhite_mac_list->imaccount == 0)
					{
						CW_FREE_OBJECT(pwhite_mac_list);
					}
					break;
				}
				else
				{
					pnode->next = phead->next;
					CW_FREE_OBJECT(phead);
					pwhite_mac_list->imaccount--;
					break;
					
				}
			}
			pnode = phead;
			phead = phead->next;
		}
	}
	
	return CW_TRUE;


}

CWBool wid_change_mac_whitelist(unsigned char * pmac,unsigned char *pmacdest)
{
	if(pwhite_mac_list == NULL)
	{
		return MAC_DOESNOT_EXIT;
	}

	int ihavesrc = 0;
	int inodest = 0;

	struct white_mac *phead = pwhite_mac_list->list_mac;		
	struct white_mac *pnode = phead;
	
	while(phead != NULL)
	{
		if(memcmp((phead->elem_mac),pmac,6) == 0)
		{
			ihavesrc = 1;
		}
		
		if(memcmp((phead->elem_mac),pmacdest,6) == 0)
		{
			inodest = 1;
		}		
		pnode = phead;
		phead = phead->next;
	}

	if(ihavesrc == 0)
	{
		return MAC_DOESNOT_EXIT;
	}
	if(inodest == 1)
	{
		return MAC_ALREADY_EXIT;
	}
	phead = pwhite_mac_list->list_mac;		
	pnode = phead;
	
	while(phead != NULL)
	{
		if(memcmp((phead->elem_mac),pmac,6) == 0)
		{
			memcpy(phead->elem_mac,pmacdest, 6);
			break;
		}
		
		pnode = phead;
		phead = phead->next;
	}

	return 0;	

}

CWBool wid_change_mac_blacklist(unsigned char * pmac,unsigned char *pmacdest)
{
	if(pblack_mac_list == NULL)
	{
		return MAC_DOESNOT_EXIT;
	}

	int ihavesrc = 0;
	int inodest = 0;

	struct white_mac *phead = pblack_mac_list->list_mac;		
	struct white_mac *pnode = phead;
	
	while(phead != NULL)
	{
		if(memcmp((phead->elem_mac),pmac,6) == 0)
		{
			ihavesrc = 1;
		}
		
		if(memcmp((phead->elem_mac),pmacdest,6) == 0)
		{
			inodest = 1;
		}		
		pnode = phead;
		phead = phead->next;
	}

	if(ihavesrc == 0)
	{
		return MAC_DOESNOT_EXIT;
	}
	if(inodest == 1)
	{
		return MAC_ALREADY_EXIT;
	}
	phead = pblack_mac_list->list_mac;		
	pnode = phead;
	
	while(phead != NULL)
	{
		if(memcmp((phead->elem_mac),pmac,6) == 0)
		{
			memcpy(phead->elem_mac,pmacdest, 6);
			break;
		}
		
		pnode = phead;
		phead = phead->next;
	}

	return 0;	

}

void get_power_control_info(transmit_power_control *txpoer_control_info, Neighbor_AP_INFOS *paplist)
{
	if((txpoer_control_info == NULL)||(paplist == NULL))
	{
		return ;
	}
		
	struct Neighbor_AP_ELE *pnode = paplist->neighborapInfos;
	
	while(pnode != NULL)
	{
		if((pnode->wtpid != 0)||(control_scope == 1)) //should change for later
		{
			if((pnode->RSSI) > (txpoer_control_info->neighbor_rssi[0]))
			{
				txpoer_control_info->neighbor_rssi[3] = txpoer_control_info->neighbor_rssi[2];
				txpoer_control_info->neighbor_rssi[2] = txpoer_control_info->neighbor_rssi[1];
				txpoer_control_info->neighbor_rssi[1] = txpoer_control_info->neighbor_rssi[0];
				txpoer_control_info->neighbor_rssi[0] = pnode->RSSI;
			}
			else if((pnode->RSSI) > (txpoer_control_info->neighbor_rssi[1]))
			{
				txpoer_control_info->neighbor_rssi[3] = txpoer_control_info->neighbor_rssi[2];
				txpoer_control_info->neighbor_rssi[2] = txpoer_control_info->neighbor_rssi[1];
				txpoer_control_info->neighbor_rssi[1] = pnode->RSSI;
			}
			else if((pnode->RSSI) > (txpoer_control_info->neighbor_rssi[2]))
			{
				txpoer_control_info->neighbor_rssi[3] = txpoer_control_info->neighbor_rssi[2];
				txpoer_control_info->neighbor_rssi[2] = pnode->RSSI;
			}
			else if((pnode->RSSI) > (txpoer_control_info->neighbor_rssi[3]))
			{
				txpoer_control_info->neighbor_rssi[3] = pnode->RSSI;
			}
			else
			{
			}

			txpoer_control_info->wtp_cnt++;

			
		}
		
		pnode = pnode->next;
	}
}

void calc_transmit_power_control(unsigned int wtpid, Neighbor_AP_INFOS *paplist,int *setflag)
{
	int calret = 0;
	int ret = 0;
		
	transmit_power_control power_control_info = {0};
	power_control_info.wtpid = wtpid;
	power_control_info.txpower= AC_WTP[wtpid]->WTP_Radio[0]->Radio_TXP;
	power_control_info.pre_txpower =power_control_info.txpower;

	get_power_control_info(&power_control_info,paplist);

	//display information

	//display_power_control_info(power_control_info);

	//calc
	if(power_control_info.wtp_cnt == 0)
	{
		if((power_control_info.txpower)+6 <= tx_power_max)
		{
			*setflag = 2;
			if(ret == 0)
			{
				power_control_info.txpower = power_control_info.txpower + 6;
			}
		}
	}
	else if(power_control_info.wtp_cnt == 1)
	{

		if((power_control_info.txpower)+6 <= tx_power_max)
		{
			*setflag = 2;
			if(ret == 0)
			{
				power_control_info.txpower = power_control_info.txpower + 6;
			}
		}else if(power_control_info.neighbor_rssi[0] > tx_power_threshold){
		
			calret = tx_power_max + tx_power_threshold - power_control_info.neighbor_rssi[0];
			if(power_control_info.txpower > calret)
			{
				if((power_control_info.txpower)-3 >= 1)
				{
					*setflag = 1;
					if(ret == 0)
					{
						power_control_info.txpower = power_control_info.txpower - 3;
					}
				}
			}
		}

	}
	else if(power_control_info.wtp_cnt == 2)
	{
		if(power_control_info.neighbor_rssi[1] > tx_power_threshold)
		{
			calret = tx_power_max + tx_power_threshold - power_control_info.neighbor_rssi[1];

			if(power_control_info.txpower > calret)
			{
				if((power_control_info.txpower)-3 >= 1)
				{
					//set radio value
					//ret = WID_RADIO_SET_TXP(radioid,((power_control_info.txpower)-3));
					*setflag = 1;
					if(ret == 0)
					{
						power_control_info.txpower = power_control_info.txpower - 3;
					}
				}
			}
		}
		else if(power_control_info.neighbor_rssi[0] < coverage_threshold)
		{
			calret = power_control_info.txpower - power_constant - coverage_threshold;
			if(calret < 0)
			{
				calret = -calret;
			}

			if(calret > power_control_info.txpower)
			{
				if((power_control_info.txpower)+6 <= tx_power_max)
				{
					//set radio value
					//ret = WID_RADIO_SET_TXP(radioid,((power_control_info.txpower)+6));
					*setflag = 2;
					if(ret == 0)
					{
						power_control_info.txpower = power_control_info.txpower + 6;
					}
				}
			}
		}
	}
	else
	{
		if(power_control_info.neighbor_rssi[2] > tx_power_threshold)
		{
			calret = tx_power_max + tx_power_threshold - power_control_info.neighbor_rssi[2];

			if(power_control_info.txpower > calret)
			{
				if((power_control_info.txpower)-3 >= 1)
				{
					//set radio value
					//ret = WID_RADIO_SET_TXP(radioid,((power_control_info.txpower)-3));
					*setflag = 1;
					if(ret == 0)
					{
						power_control_info.txpower = power_control_info.txpower - 3;
					}
				}
			}
		}
		else if(power_control_info.neighbor_rssi[0] < coverage_threshold)
		{
			calret = power_control_info.txpower - power_constant - coverage_threshold;
			if(calret < 0)
			{
				calret = -calret;
			}

			if(calret > power_control_info.txpower)
			{
				if((power_control_info.txpower)+6 <= tx_power_max)
				{
					//set radio value
					//ret = WID_RADIO_SET_TXP(radioid,((power_control_info.txpower)+6));
					*setflag = 2;
					if(ret == 0)
					{
						power_control_info.txpower = power_control_info.txpower + 6;
					}
				}
			}
		}
	}

	
	//display information

	//display_power_control_info(power_control_info);
	
}

void display_power_control_info(transmit_power_control power_control_info)
{
	printf("### display power control information start ###\n");

	//printf("$$$ wtpid = %d $$$\n",power_control_info.wtpid);

	int i = 0;
	printf("$$$ wtp cnt = %d $$$\n",power_control_info.wtp_cnt);
	for(i=0; i<4; i++)
	{
		printf("$$$ RSSI[%d] = %d $$$\n",i,power_control_info.neighbor_rssi[i]);
	}
	
	printf("### display power control information end ###\n");
		
}

unsigned int Wid_Find_Wtp(WID_WTP **WTP){
	int i = 0;
	unsigned int wtp_num = 0;
	while(i<WTP_NUM){
		CWThreadMutexLock(&(gWTPs[i].WTPThreadMutex));//fengwenchao add 20121123 for AXSSZFI-1050
		if((AC_WTP[i]!= NULL)
			&&(AC_WTP[i]->WTPID < WTP_NUM)
			&&(AC_WTP[i]->WTPID > 0)){
			WTP[wtp_num]  = AC_WTP[i];
			wtp_num ++;
		}
		CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));
		i++;
	}
	return wtp_num;
}

unsigned int Wid_Find_Running_Wtp(WID_WTP **WTP){
	int i = 0;
	unsigned int wtp_num = 0;
	while(i<WTP_NUM){
		CWThreadMutexLock(&(gWTPs[i].WTPThreadMutex));//fengwenchao add 20121123 for AXSSZFI-1050
		if((AC_WTP[i]!= NULL)
			&&(AC_WTP[i]->WTPID < WTP_NUM)
			&&(AC_WTP[i]->WTPStat ==5 )
			&&(AC_WTP[i]->WTPID > 0)){
			WTP[wtp_num]  = AC_WTP[i];
			wtp_num ++;
		}
		CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));
		i++;
	}
	return wtp_num;
}


unsigned int Wid_Find_ROUGE_Wtp(WID_WTP **WTP,unsigned int *wtp_num){
	int i = 0;
	*wtp_num = 0;
	unsigned int rouge_num = 0;
	
	while(i<WTP_NUM){
		if(AC_WTP[i]!= NULL){
			(*wtp_num) ++;
			if((AC_WTP[i]->NeighborAPInfos != NULL)&&(AC_WTP[i]->NeighborAPInfos->neighborapInfosCount != 0)){
				
				CWThreadMutexLock(&(gWTPs[i].RRMThreadMutex));

				if(AC_WTP[i]->rouge_ap_infos == NULL){
					AC_WTP[i]->rouge_ap_infos = wid_check_rogue_ap_mac(i);
					delete_rouge_ap_list_by_whitelist(&(AC_WTP[i]->rouge_ap_infos));
				}
				
				else{
					if((AC_WTP[i]->rouge_ap_infos) != NULL){
						destroy_ap_info_list(&(AC_WTP[i]->rouge_ap_infos));
					}
					AC_WTP[i]->rouge_ap_infos = wid_check_rogue_ap_mac(i);
					delete_rouge_ap_list_by_whitelist(&(AC_WTP[i]->rouge_ap_infos));
				}

				if((AC_WTP[i]->rouge_ap_infos != NULL)&&(AC_WTP[i]->rouge_ap_infos->neighborapInfosCount != 0)){
					WTP[rouge_num]  = AC_WTP[i];
					rouge_num ++;
				}
				CWThreadMutexUnlock(&(gWTPs[i].RRMThreadMutex));
			}
		}
		i++;
	}
	return rouge_num;
}

/*fengwenchao add 20110401 for dot11WlanDataPktsTable*/
int WID_CHECK_SAME_ATH_OF_ALL_WTP(unsigned char wlanid,unsigned int *tx_pkt,unsigned long long *rx_bts,unsigned long long *tx_bts)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int n = 0;   //fengwenchao add 20110426
	unsigned int wtp_num = 0;

	WID_WTP **WTP;
	WTP = malloc(WTP_NUM*(sizeof(WID_WTP *)));
	wtp_num = Wid_Find_Wtp(WTP);
	//WID_WTP_RADIO	**AC_RADIO_FOR_SEARCH;
	//AC_RADIO_FOR_SEARCH = malloc(G_RADIO_NUM*(sizeof(WID_WTP_RADIO *)));
	if(wtp_num != 0)
	{	for(i = 0; i < wtp_num; i++)
		{
			 for(j=0;j<L_RADIO_NUM;j++)
			 {
			 	if(WTP[i]->WTP_Radio[j] != NULL)
			 	{
			 		 for(k=0;k<L_BSS_NUM;k++)
			 		 {
			 		 	 if((WTP[i]->WTP_Radio[j]->BSS[k] != NULL)
									&&(WTP[i]->WTP_Radio[j]->BSS[k]->WlanID==wlanid)
									&&(WTP[i]->WTP_Radio[j]->BSS[k]->BSSID != NULL))
			 		 	 {
							for(n = 0; n < TOTAL_AP_IF_NUM;n++)     //fengwenchao add 20110426
				 		 	{ 	
								if((WTP[i]->apstatsinfo[n].radioId < TOTAL_AP_IF_NUM+1) && (wlanid == WTP[i]->apstatsinfo[n].wlanId)&&(WTP[i]->apstatsinfo[n].type == 0))   //fengwenchao modify 20110426
				 		 	 	{
				 		 	 		*tx_pkt += WTP[i]->apstatsinfo[n].tx_packets;
									*rx_bts += WTP[i]->apstatsinfo[n].rx_bytes;
									*tx_bts += WTP[i]->apstatsinfo[n].tx_bytes;
				 		 	 	}
							}
			 		 	 }
			 		 }
			 	}
				else
				{
					wid_syslog_debug_debug(WID_WTPINFO,"WID_CHECK_SAME_ATH_OF_ALL_WTP : WTP[%d]->WTP_Radio[%d] = NULL\n",i,j);
				}
			 }
		}
	}
	else
	{
		wid_syslog_debug_debug(WID_WTPINFO,"WID_CHECK_SAME_ATH_OF_ALL_WTP : wtp_num = 0\n");
	}
	if(WTP)
	{
		free(WTP);
		WTP = NULL;
	}
	return 0;
}
/*fengwenchao add end*/

unsigned char WID_WTP_FIND_RADIO(WID_WTP_RADIO	**AC_RADIO_FOR_SEARCH,WID_WTP *WTP){
	int i = 0;
	unsigned int g_radio_id = 0;
	unsigned char num_of_radio = 0;
	for(i = 0;i<L_RADIO_NUM;i++){
		if(WTP->WTP_Radio[i] != NULL){
			g_radio_id = WTP->WTP_Radio[i]->Radio_G_ID;
			AC_RADIO_FOR_SEARCH[num_of_radio] = AC_RADIO[g_radio_id];
			num_of_radio ++;
		}
	}
	return num_of_radio;
}

int WID_CHECK_WTP_ID(u_int32_t Id){
	int ret = 0;
	if(Id >= WTP_NUM)
		ret =  WTP_ID_LARGE_THAN_MAX;
	else if(AC_WTP[Id] == NULL)
		ret =  WTP_ID_NOT_EXIST;
	
	return ret ;
	
}

int WID_CHECK_WLAN_ID(u_int32_t Id){
	int ret = 0;
	if(Id >= WLAN_NUM)
		ret = WLAN_ID_LARGE_THAN_MAX;
	else if(AC_WLAN[Id] == NULL)
		ret = WLAN_ID_NOT_EXIST;
	else if (AC_WLAN[Id]->want_to_delete == 1)		/* Huangleilei add for ASXXZFI-1622 */
		ret = WID_WANT_TO_DELETE_WLAN;
		
	return ret ;
	
}

int WID_CHECK_G_RADIO_ID(u_int32_t Id){
	int ret = 0;
	if(Id >= G_RADIO_NUM)
		ret = RADIO_ID_LARGE_THAN_MAX;
	else if(AC_RADIO[Id]== NULL)
		ret = RADIO_ID_NOT_EXIST;
	
	return ret ;
}

int WID_CHECK_ID(unsigned int TYPE,u_int32_t Id){
	int ret = 0;
	
	if(WID_WTP_CHECK == TYPE)
		ret = WID_CHECK_WTP_ID(Id);
	
	if(WID_WLAN_CHECK == TYPE)
		ret = WID_CHECK_WLAN_ID(Id);/*remember changing type from char to int*/

	if(WID_RADIO_CHECK == TYPE)
		ret = WID_CHECK_G_RADIO_ID(Id);

	return ret ;

}



CWBool check_ascii_32_to126(const char * str)
{
	if(str == NULL)
	{
		return CW_TRUE;
	}

	CWBool ret = CW_TRUE;
	const char *p = str;

	while(*p != '\0')
	{
		//printf("*p = %d\n",*p);
		if((*p < 32 )||(*p > 126))
		{
			ret = CW_FALSE;
			break;
		}
		p++;
	}
	//printf("ret = %d\n",ret);
	return ret;
}
void WID_CHECK_WLAN_APPLY_WTP_BSS_NAS_ID(unsigned char WlanID,unsigned short ifindex,unsigned int nas_id_len,char *nas_id){
	int i,j,BSSIndex;	
	for(i=0;i<WTP_NUM;i++)
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->BindingSystemIndex == ifindex)){
			for(j=0;j<AC_WTP[i]->RadioCount;j++){
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0){
					BSSIndex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
					if((check_bssid_func(BSSIndex))&&(AC_BSS[BSSIndex]!=NULL)){
						memset(AC_BSS[BSSIndex]->nas_id, 0, NAS_IDENTIFIER_NAME);
						AC_BSS[BSSIndex]->nas_id_len = nas_id_len;
						memcpy(AC_BSS[BSSIndex]->nas_id, nas_id, NAS_IDENTIFIER_NAME);
					}
				}
			}
		}
}
void wid_check_remove_wlan_bss_if_nasid(unsigned char WlanID,unsigned short ifindex)
{	
	int i,j,BSSIndex;	
	for(i=0;i<WTP_NUM;i++)
	{
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->BindingSystemIndex == ifindex))
		{
			for(j=0;j<AC_WTP[i]->RadioCount;j++)
			{
				if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
				{
					BSSIndex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
					if(AC_BSS[BSSIndex]!=NULL)
					{
						memset(AC_BSS[BSSIndex]->nas_id, 0, NAS_IDENTIFIER_NAME);
						AC_BSS[BSSIndex]->nas_id_len = 0;
					}
				}
			}
		}
	}
}

int WID_INTERFACE_SET_NASID(unsigned char WlanID,char* ifname,char* nas_id)
{
	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}
//	if(AC_WLAN[WlanID]->SecurityID == 0)
//	{
//		return WLAN_APPLY_SECURITY_FIRST;
//	}
//	if((AC_WLAN[WlanID]->SecurityType != IEEE8021X)&&(AC_WLAN[WlanID]->SecurityType != WPA_E)&&(AC_WLAN[WlanID]->SecurityType != WPA2_E)){
//		return WLAN_NOT_NEED_NAS;
//	}
	int sockfd;
	struct ifreq	ifr;
	struct ifi *wif;
	struct ifi *wifnext;
	int ret = Check_And_Bind_Interface_For_WID(ifname);
	if(ret != 0)
		return ret;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));
	
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	close(sockfd);
	wif = (struct ifi*)malloc(sizeof(struct ifi));
	memset(wif->ifi_name,0,ETH_IF_NAME_LEN);
	memcpy(wif->ifi_name,ifname,strlen(ifname));
	wif->ifi_index = ifr.ifr_ifindex;
	wif->nas_id_len = strlen(nas_id);
	memset(wif->nas_id,0,NAS_IDENTIFIER_NAME);
	memcpy(wif->nas_id,nas_id,strlen(nas_id));
	wif->ifi_next = NULL;
	
	if(AC_WLAN[WlanID]->Wlan_Ifi == NULL){
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		AC_WLAN[WlanID]->Wlan_Ifi = wif ;
		AC_WLAN[WlanID]->Wlan_Ifi->ifi_next = NULL;
	}else{

		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext != NULL)
		{	
			if(wifnext->ifi_index == wif->ifi_index)
			{
				wifnext->nas_id_len = strlen(nas_id);
				memset(wifnext->nas_id,0,NAS_IDENTIFIER_NAME);
				memcpy(wifnext->nas_id,nas_id,strlen(nas_id));
				//printf("warnning you have binding this wlan eth ,please do not binding this again");
				wid_syslog_debug_debug(WID_DEFAULT,"warnning you have binding this wlan eth ,please do not binding this again");
				WID_CHECK_WLAN_APPLY_WTP_BSS_NAS_ID(WlanID, wif->ifi_index, wifnext->nas_id_len, nas_id);
				free(wif);
				wif = NULL;
				return 0;
			}
			wifnext = wifnext->ifi_next;
		}
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext->ifi_next!= NULL)
		{	
			wifnext = wifnext->ifi_next;
		}
		
		wifnext->ifi_next= wif;		
		WID_CHECK_WLAN_APPLY_WTP_BSS_NAS_ID(WlanID, wif->ifi_index, wifnext->nas_id_len, nas_id);
		//wifnext->ifi_next = NULL;
	}
	AC_WLAN[WlanID]->ifcount++;
	return 0;
}
int wid_remove_wlan_interface_nasid(unsigned char WlanID,char* ifname)
{
	
	int sockfd;
	struct ifreq	ifr;
	int ifindex = 0;
	int isfind = 0;
	struct ifi *wif;
	
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(0 > sockfd){
		return -1;
	}
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));
	
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	close(sockfd);
	ifindex = ifr.ifr_ifindex;

	
	wif = AC_WLAN[WlanID]->Wlan_Ifi;
	while(wif != NULL)
	{
		if(wif->ifi_index == ifindex)/*this is the interface*/
		{
			isfind = 1;
			memset(wif->nas_id,0,NAS_IDENTIFIER_NAME);
			wif->nas_id_len = 0;
			wid_check_remove_wlan_bss_if_nasid(WlanID,ifindex);
			break;
		}

		wif = wif->ifi_next;
	}

	if(isfind == 0)
	{
		return WLAN_NOT_BINDING_IF;
	}
	return 0;
}
int wid_set_ap_statistics(int apstatics)
{
	int i = 0;
	msgq msg;
	for(i=0; i<WTP_NUM; i++)
	{
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
//			printf("## wid_set_ap_statistics wtp id= %d##\n",i);
			
			AC_WTP[i]->WTP_Radio[0]->CMD |= 0x0100;
			AC_WTP[i]->CMD->radioid[0] += 1;
			AC_WTP[i]->CMD->setCMD = 1;	
			int WTPIndex = i;
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
			{
				
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STATISTICS_REPORT;
				msg.mqinfo.u.WtpInfo.value2 = apstatics;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
		}
	}

	return 0;

}


int wid_set_ap_statistics_interval(unsigned int wtpid,unsigned int apstatics_interval)
{
	int i = 0;
	msgq msg;
//	struct msgqlist *elem;
	if(wtpid == 0){
		for(i=0; i<WTP_NUM; i++)
		{
			if((AC_WTP[i] != NULL)&&(AC_WTP[i]->apstatisticsinterval != apstatics_interval)){
				AC_WTP[i]->apstatisticsinterval = apstatics_interval;
			}
			if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
			{
				AC_WTP[i]->WTP_Radio[0]->CMD |= 0x0100;
				AC_WTP[i]->CMD->radioid[0] += 1;
				AC_WTP[i]->CMD->setCMD = 1;	
				int WTPIndex = i;
				CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
				if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
				{
					
					memset((char*)&msg, 0, sizeof(msg));
					msg.mqid = WTPIndex%THREAD_NUM+1;
					msg.mqinfo.WTPID = WTPIndex;
					msg.mqinfo.type = CONTROL_TYPE;
					msg.mqinfo.subtype = WTP_S_TYPE;
					msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STATISTICS_REPORT_INTERVAL;
					msg.mqinfo.u.WtpInfo.value2 = apstatics_interval;
					if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
						wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
						perror("msgsnd");
					}
				}		
				CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
			}
		}
	}
	else{
		if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
		{
			AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x0100;
			AC_WTP[wtpid]->CMD->radioid[0] += 1;
			AC_WTP[wtpid]->CMD->setCMD = 1; 
			int WTPIndex = wtpid;
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
			{
				
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STATISTICS_REPORT_INTERVAL;
				msg.mqinfo.u.WtpInfo.value2 = apstatics_interval;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
		}//delete unuseful cod
		/*else if(AC_WTP[wtpid] != NULL){
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = wtpid%THREAD_NUM+1;
				msg.mqinfo.WTPID = wtpid;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STATISTICS_REPORT_INTERVAL;
				msg.mqinfo.u.WtpInfo.value2 = apstatics_interval;

				elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem == NULL){
					wid_syslog_info("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
				elem->next = NULL;
				memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
				WID_INSERT_CONTROL_LIST(wtpid, elem);
			}*/
	}
	return 0;

}


//qos area
int WID_ADD_QOS_PROFILE(char *name,int ID){

	int i = 0;
	

	WID_QOS[ID] = (AC_QOS*)malloc(sizeof(AC_QOS));
	WID_QOS[ID]->QosID = ID;
	WID_QOS[ID]->name = (char *)malloc(strlen(name)+1);
	memset(WID_QOS[ID]->name, 0, strlen(name)+1);
	memcpy(WID_QOS[ID]->name, name, strlen(name));

	//set wireless qos info for mib
	WID_QOS[ID]->qos_total_bandwidth = 25;
	WID_QOS[ID]->qos_res_scale = 20;
	WID_QOS[ID]->qos_share_bandwidth = 10;
	WID_QOS[ID]->qos_res_share_scale = 20;
	WID_QOS[ID]->qos_use_res_grab = 0;
	WID_QOS[ID]->qos_use_res_shove = 0;
	
	memset(WID_QOS[ID]->qos_manage_arithmetic, 0, WID_QOS_ARITHMETIC_NAME_LEN);
	memcpy(WID_QOS[ID]->qos_manage_arithmetic, "ManagePolicy",12);
	memset(WID_QOS[ID]->qos_res_grab_arithmetic, 0, WID_QOS_ARITHMETIC_NAME_LEN);
	memcpy(WID_QOS[ID]->qos_res_grab_arithmetic, "GrabPolicy",10);
	memset(WID_QOS[ID]->qos_res_shove_arithmetic, 0, WID_QOS_ARITHMETIC_NAME_LEN);
	memcpy(WID_QOS[ID]->qos_res_shove_arithmetic, "ShovePolicy",11);
	for(i=0;i<4;i++)
	{
		WID_QOS[ID]->radio_qos[i] = (qos_profile*)malloc(sizeof(qos_profile));

		WID_QOS[ID]->radio_qos[i]->qos_average_rate = 5;
		WID_QOS[ID]->radio_qos[i]->qos_max_degree = 100;
		WID_QOS[ID]->radio_qos[i]->qos_policy_pri = 0;
		WID_QOS[ID]->radio_qos[i]->qos_res_shove_pri = 0;
		WID_QOS[ID]->radio_qos[i]->qos_res_grab_pri = 0;
		WID_QOS[ID]->radio_qos[i]->qos_max_parallel = 20;
		WID_QOS[ID]->radio_qos[i]->qos_bandwidth = 5;
		WID_QOS[ID]->radio_qos[i]->qos_bandwidth_scale = 20;
		WID_QOS[ID]->radio_qos[i]->qos_use_wred = 0;
		WID_QOS[ID]->radio_qos[i]->qos_use_traffic_shaping = 0;
		WID_QOS[ID]->radio_qos[i]->qos_use_flow_eq_queue = 0;
		WID_QOS[ID]->radio_qos[i]->qos_flow_max_queuedepth = 199;
		WID_QOS[ID]->radio_qos[i]->qos_flow_max_degree = 20;
		WID_QOS[ID]->radio_qos[i]->qos_flow_average_rate = 5;
	}
	//set wireless qos info for mib end
	for(i=0;i<4;i++)
	{
		//radio type
		
			
		WID_QOS[ID]->radio_qos[i]->QueueDepth = 199;
		/*
		WID_QOS[ID]->radio_qos[i]->CWMin = WID_QOS_CWMIN_DEFAULT;
		WID_QOS[ID]->radio_qos[i]->CWMax = WID_QOS_CWMAX_DEFAULT;
		WID_QOS[ID]->radio_qos[i]->AIFS = WID_QOS_AIFS_DEFAULT;
		
		WID_QOS[ID]->radio_qos[i]->TXOPlimit = 0;

		*/
		if(i == 0)//be
		{
			WID_QOS[ID]->radio_qos[i]->CWMin = 4;
			WID_QOS[ID]->radio_qos[i]->CWMax = 10;
			WID_QOS[ID]->radio_qos[i]->AIFS = 2;
			WID_QOS[ID]->radio_qos[i]->TXOPlimit = 2048;
		}
		else if(i == 1)//ba
		{
			WID_QOS[ID]->radio_qos[i]->CWMin = 4;
			WID_QOS[ID]->radio_qos[i]->CWMax = 10;
			WID_QOS[ID]->radio_qos[i]->AIFS = 7;
			WID_QOS[ID]->radio_qos[i]->TXOPlimit = 0;
		}
		else if(i == 2)//vi
		{
			WID_QOS[ID]->radio_qos[i]->CWMin = 3;
			WID_QOS[ID]->radio_qos[i]->CWMax = 4;
			WID_QOS[ID]->radio_qos[i]->AIFS = 1;
			WID_QOS[ID]->radio_qos[i]->TXOPlimit = 3008;
		}
		else if(i == 3)//v0
		{
			WID_QOS[ID]->radio_qos[i]->CWMin = 2;
			WID_QOS[ID]->radio_qos[i]->CWMax = 3;
			WID_QOS[ID]->radio_qos[i]->AIFS = 1;
			WID_QOS[ID]->radio_qos[i]->TXOPlimit = 1504;
		}
		
		//WID_QOS[ID]->radio_qos[i]->Dot1PTag = 0;
		//WID_QOS[ID]->radio_qos[i]->DSCPTag = 0;
		WID_QOS[ID]->radio_qos[i]->ACK = 1;//default value is ack

		WID_QOS[ID]->radio_qos[i]->mapstate = 0;
		WID_QOS[ID]->radio_qos[i]->wmm_map_dot1p = 0;
		WID_QOS[ID]->radio_qos[i]->dot1p_map_wmm_num = 0;
		memset(WID_QOS[ID]->radio_qos[i]->dot1p_map_wmm,0,8);
		
		

		//client type
		WID_QOS[ID]->client_qos[i] = (qos_profile*)malloc(sizeof(qos_profile));
		WID_QOS[ID]->client_qos[i]->QueueDepth = 199;
		/*
		WID_QOS[ID]->client_qos[i]->CWMin = WID_QOS_CWMIN_DEFAULT;
		WID_QOS[ID]->client_qos[i]->CWMax = WID_QOS_CWMAX_DEFAULT;
		WID_QOS[ID]->client_qos[i]->AIFS = WID_QOS_AIFS_DEFAULT;
		
		WID_QOS[ID]->client_qos[i]->TXOPlimit = 0;

		*/
		if(i == 0)//be
		{
			WID_QOS[ID]->client_qos[i]->CWMin = 4;
			WID_QOS[ID]->client_qos[i]->CWMax = 10;
			WID_QOS[ID]->client_qos[i]->AIFS = 2;
			WID_QOS[ID]->client_qos[i]->TXOPlimit = 2048;
		}
		else if(i == 1)//ba
		{
			WID_QOS[ID]->client_qos[i]->CWMin = 4;
			WID_QOS[ID]->client_qos[i]->CWMax = 10;
			WID_QOS[ID]->client_qos[i]->AIFS = 7;
			WID_QOS[ID]->client_qos[i]->TXOPlimit = 0;
		}
		else if(i == 2)//vi
		{
			WID_QOS[ID]->client_qos[i]->CWMin = 3;
			WID_QOS[ID]->client_qos[i]->CWMax = 4;
			WID_QOS[ID]->client_qos[i]->AIFS = 2;
			WID_QOS[ID]->client_qos[i]->TXOPlimit = 3008;
		}
		else if(i == 3)//v0
		{
			WID_QOS[ID]->client_qos[i]->CWMin = 2;
			WID_QOS[ID]->client_qos[i]->CWMax = 3;
			WID_QOS[ID]->client_qos[i]->AIFS = 2;
			WID_QOS[ID]->client_qos[i]->TXOPlimit = 1504;
		}
		//WID_QOS[ID]->client_qos[i]->Dot1PTag = 0;
		//WID_QOS[ID]->client_qos[i]->DSCPTag = 0;
		//WID_QOS[ID]->client_qos[i]->ACK = 0;//default value is noack
	}
	
	return WID_DBUS_SUCCESS;
}
int WID_DELETE_QOS_PROFILE(int ID){
	int i = 0;

	for(i=0;i<4;i++)
	{
		WID_QOS[ID]->radio_qos[i]->QueueDepth = UNUSED_QOS_VALUE;
		WID_QOS[ID]->radio_qos[i]->CWMin = UNUSED_QOS_VALUE;
		WID_QOS[ID]->radio_qos[i]->CWMax = UNUSED_QOS_VALUE;
		WID_QOS[ID]->radio_qos[i]->AIFS = UNUSED_QOS_VALUE;
		WID_QOS[ID]->radio_qos[i]->TXOPlimit = UNUSED_QOS_VALUE;
		WID_QOS[ID]->radio_qos[i]->Dot1PTag = UNUSED_QOS_VALUE;
		WID_QOS[ID]->radio_qos[i]->DSCPTag = UNUSED_QOS_VALUE;
		WID_QOS[ID]->radio_qos[i]->ACK = UNUSED_QOS_VALUE;


		WID_QOS[ID]->client_qos[i]->QueueDepth = UNUSED_QOS_VALUE;
		WID_QOS[ID]->client_qos[i]->CWMin = UNUSED_QOS_VALUE;
		WID_QOS[ID]->client_qos[i]->CWMax = UNUSED_QOS_VALUE;
		WID_QOS[ID]->client_qos[i]->AIFS = UNUSED_QOS_VALUE;
		WID_QOS[ID]->client_qos[i]->TXOPlimit = UNUSED_QOS_VALUE;
		WID_QOS[ID]->client_qos[i]->Dot1PTag = UNUSED_QOS_VALUE;
		WID_QOS[ID]->client_qos[i]->DSCPTag = UNUSED_QOS_VALUE;
		WID_QOS[ID]->client_qos[i]->ACK = UNUSED_QOS_VALUE;

		CW_FREE_OBJECT(WID_QOS[ID]->radio_qos[i]);
		CW_FREE_OBJECT(WID_QOS[ID]->client_qos[i]);
	}

	CW_FREE_OBJECT(WID_QOS[ID]->name);
	
	CW_FREE_OBJECT(WID_QOS[ID]);
	
	return WID_DBUS_SUCCESS;
}
int WID_QOS_SET_QOS_INFO(int ID,int qos_stream_id,unsigned short cwmin,unsigned short cwmax,unsigned char aifs,unsigned char ack,unsigned short txoplimit)
{
	

	WID_QOS[ID]->radio_qos[qos_stream_id]->CWMin = cwmin;
	WID_QOS[ID]->radio_qos[qos_stream_id]->CWMax = cwmax;
	WID_QOS[ID]->radio_qos[qos_stream_id]->AIFS = aifs;
	WID_QOS[ID]->radio_qos[qos_stream_id]->ACK = ack;
	WID_QOS[ID]->radio_qos[qos_stream_id]->TXOPlimit = txoplimit;
	return 0;
}
int WID_QOS_SET_QOS_INFO_CLIENT(int ID,int qos_stream_id,unsigned short cwmin,unsigned short cwmax,unsigned char aifs,unsigned short txoplimit)
{
	

	WID_QOS[ID]->client_qos[qos_stream_id]->CWMin = cwmin;
	WID_QOS[ID]->client_qos[qos_stream_id]->CWMax = cwmax;
	WID_QOS[ID]->client_qos[qos_stream_id]->AIFS = aifs;
	WID_QOS[ID]->client_qos[qos_stream_id]->TXOPlimit = txoplimit;
	return 0;
}
int WID_QOS_SET_QOS_WMM_MAP(int ID,int isadd)
{
	int i,j;

	if(isadd)
	{
		for(i=0;i<4;i++)
		{
			WID_QOS[ID]->radio_qos[i]->mapstate = 1;

			WID_QOS[ID]->radio_qos[i]->wmm_map_dot1p = 0;
			WID_QOS[ID]->radio_qos[i]->dot1p_map_wmm_num = 0;
			for(j=0;j<8;j++)
			{
				WID_QOS[ID]->radio_qos[i]->dot1p_map_wmm[j] = 0;
			}
		}
	}
	else
	{
		for(i=0;i<4;i++)
		{
			WID_QOS[ID]->radio_qos[i]->mapstate = 0;
			WID_QOS[ID]->radio_qos[i]->wmm_map_dot1p = 0;
			WID_QOS[ID]->radio_qos[i]->dot1p_map_wmm_num = 0;
			for(j=0;j<8;j++)
			{
				WID_QOS[ID]->radio_qos[i]->dot1p_map_wmm[j] = 0;
			}
		}
	}

	
	return 0;
}
int WID_QOS_SET_QOS_WMM_MAP_DOT1P(int ID,int wmm_order,unsigned char dot1p)
{
	

	WID_QOS[ID]->radio_qos[wmm_order]->wmm_map_dot1p = dot1p;

	return 0;
}
int WID_QOS_SET_QOS_DOT1P_MAP_WMM(int ID,int wmm_order,unsigned char num,unsigned char dot1p[])
{
	int i;
	//printf("num %d\n",num);
	WID_QOS[ID]->radio_qos[wmm_order]->dot1p_map_wmm_num = num;

	
	for(i=0;i<num;i++)
	{	//printf("i %d num %d\n",i,dot1p[i]);
		WID_QOS[ID]->radio_qos[wmm_order]->dot1p_map_wmm[i] = dot1p[i];

	}
	
	
	return 0;
}


int WID_ADD_RADIO_APPLY_QOS(unsigned int RadioID,int qosID,int qosstate){
	msgq msg;
//	struct msgqlist *elem;
	if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		return RADIO_IS_DISABLE;
	}
	
	int WTPIndex = AC_RADIO[RadioID]->WTPID;
	int l_radio_id = AC_RADIO[RadioID]->Radio_L_ID;
	AC_RADIO[RadioID]->QOSID = qosID;
	AC_RADIO[RadioID]->QOSstate = qosstate;
	
	//add to control list
	if((AC_WTP[WTPIndex] != NULL)&&(AC_WTP[WTPIndex]->WTPStat == 5))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Qos;
		msg.mqinfo.u.RadioInfo.Radio_L_ID = l_radio_id;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
		msg.mqinfo.u.RadioInfo.id1 = qosID;
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}
	
	return 0;
}
int wid_set_qos_flow_parameter_value(unsigned int qosid,unsigned char streamid,unsigned int type,unsigned int value)
{
	unsigned int stream_index = 4;
	wid_syslog_debug_debug(WID_DEFAULT,"qosid %d streamid %d type %d value %d \n",qosid,streamid,type,value);
	if((qosid < 1)||(qosid >15))
	{
		return WID_QOS_NOT_EXIST;
	}
	if(streamid >3)
	{
		return WID_QOS_STREAM_ERROR;
	}
	
	switch(streamid)
	{
		case WID_BESTEFFORT : stream_index= 0;
							break;
		case WID_BACKGTOUND: stream_index= 1;
							break;
		case WID_VIDEO: stream_index= 2;
						break;
		case WID_VOICE: stream_index= 3;
						break;
		default : break;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"streamid %d stream_index %d value %d \n",streamid,stream_index,value);
	
	switch(type)
	{
		case averagerate_type : 
								WID_QOS[qosid]->radio_qos[stream_index]->qos_average_rate = value;
								break;

		case max_burstiness_type: 
			
								WID_QOS[qosid]->radio_qos[stream_index]->qos_max_degree = value;
								break;

		case manage_priority_type: 
	
								WID_QOS[qosid]->radio_qos[stream_index]->qos_policy_pri = value;
								break;

		case shove_priority_type: 
	
								WID_QOS[qosid]->radio_qos[stream_index]->qos_res_shove_pri = value;
								break;

		case grab_priority_type: 
	
								WID_QOS[qosid]->radio_qos[stream_index]->qos_res_grab_pri = value;
								break;

		case max_parallel_type:
			
								WID_QOS[qosid]->radio_qos[stream_index]->qos_max_parallel = value;
								break;
		case bandwidth_type:
			
								WID_QOS[qosid]->radio_qos[stream_index]->qos_bandwidth = value;
								break;
		case bandwidth_percentage_type:
			
								WID_QOS[qosid]->radio_qos[stream_index]->qos_bandwidth_scale = value;
								break;
		case flowqueuelenth_type:
			
								WID_QOS[qosid]->radio_qos[stream_index]->qos_flow_max_queuedepth= value;
								break;
		case flowaveragerate_type:
			
								WID_QOS[qosid]->radio_qos[stream_index]->qos_flow_average_rate = value;
								break;
		case flowmaxburstiness_type:
			
								WID_QOS[qosid]->radio_qos[stream_index]->qos_flow_max_degree= value;
								break;
		case unkonwn_type: break;

		default : break;
	}
	return WID_DBUS_SUCCESS;
}
int wid_set_qos_flow_able_value(unsigned int qosid,unsigned char streamid,unsigned int able_type,unsigned int flag)
{
	unsigned int stream_index = 4;
	wid_syslog_debug_debug(WID_DEFAULT,"qosid %d streamid %d abletype %d \n",qosid,streamid,able_type);
	if((qosid < 1)||(qosid >15))
	{
		return WID_QOS_NOT_EXIST;
	}
	if(streamid >3)
	{
		return WID_QOS_STREAM_ERROR;
	}
	
	switch(streamid)
	{
		case WID_BESTEFFORT : stream_index= 0;
							break;
		case WID_BACKGTOUND: stream_index= 1;
							break;
		case WID_VIDEO: stream_index= 2;
						break;
		case WID_VOICE: stream_index= 3;
						break;
		default : break;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"streamid %d stream_index %d abletype %d \n",streamid,stream_index,able_type);

	if(flag == 1){
		WID_QOS[qosid]->radio_qos[stream_index]->qos_use_traffic_shaping = able_type;
	}
	else if(flag == 2){
		WID_QOS[qosid]->radio_qos[stream_index]->qos_use_flow_eq_queue = able_type;
	}else if(flag == 3){
		WID_QOS[qosid]->radio_qos[stream_index]->qos_use_wred = able_type;
	}else{}
	return WID_DBUS_SUCCESS;
}

int wid_set_qos_parameter_value(unsigned int qosid,unsigned int type,unsigned int value)
{
	
	wid_syslog_debug_debug(WID_DEFAULT,"qosid %d type %d value %d \n",qosid,type,value);
	if((qosid < 1)||(qosid >15))
	{
		return WID_QOS_NOT_EXIST;
	}
	
	switch(type)
	{
		case totalbandwidth_type : 
								WID_QOS[qosid]->qos_total_bandwidth = value;
								break;

		case resourcescale_type : 
			
								WID_QOS[qosid]->qos_res_scale = value;
								break;

		case sharebandwidth_type : 
	
								WID_QOS[qosid]->qos_share_bandwidth = value;
								break;

		case resourcesharescale_type : 
	
								WID_QOS[qosid]->qos_res_share_scale= value;
								break;

		case unkonwn_type: break;

		default : break;
	}
	return WID_DBUS_SUCCESS;
}

int wid_radio_set_throughout(int wtpid, unsigned char bandwidth)
{
	msgq msg;
	AC_WTP[wtpid]->WTP_Radio[0]->bandwidth = bandwidth;
	
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x0800;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		int WTPIndex = wtpid;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Throughput;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = 0;// waiting M-Radio
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}
	
	return 0;	

}
int wid_mul_radio_set_throughout(unsigned int radioid, unsigned char bandwidth)
{
	msgq msg;
	unsigned int wtpid = 0;
	unsigned int l_radioid = 0;
	wtpid = radioid/L_RADIO_NUM;
	l_radioid = radioid%L_RADIO_NUM;
	if(AC_WTP[wtpid] == NULL)
	{
		return WTP_ID_NOT_EXIST;
	}
	if(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL)
	{
		return RADIO_ID_NOT_EXIST;
	}
	if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->bandwidth == bandwidth)
	{
		return 0;
	}
	AC_WTP[wtpid]->WTP_Radio[l_radioid]->bandwidth = bandwidth;
	
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->CMD |= 0x0800;
		AC_WTP[wtpid]->CMD->radioid[l_radioid] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		int WTPIndex = wtpid;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Throughput;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = l_radioid;//M-Radio
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}
	
	return 0;	

}


int wid_radio_set_ip_gateway(int wtpid,unsigned int ip,unsigned int gateway,unsigned char mask)
{
	msgq msg;
	AC_WTP[wtpid]->ap_ipadd = ip;
	AC_WTP[wtpid]->ap_gateway = gateway;
	AC_WTP[wtpid]->resetflag = 1;
	AC_WTP[wtpid]->ap_mask = mask;
	
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x1000;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		int WTPIndex = wtpid;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SET_IP;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}
	
	if(gtrapflag>=4){
		wid_dbus_trap_wtp_ip_change_alarm(wtpid);
		}

	return 0;	

}
int wid_update_ap_config(int wtpid,char *ip)
{
	char *command;
	command = (char *)malloc(sizeof(char)*50);
	memset(command,0,50);
	sprintf(command,"cd /jffs && tftp -g -r config.wtp %s",ip);
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	//printf("command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	time(&AC_WTP[wtpid]->config_update_time);
	return 0;	

}

int wid_set_ap_timestamp(int timestamp)
{

	int i = 0;
	msgq msg;
	for(i=0; i<WTP_NUM; i++)
	{
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
	//		printf("## wid_set_ap_timestamp wtp id= %d##\n",i);
			
			AC_WTP[i]->WTP_Radio[0]->CMD |= 0x2000;
			AC_WTP[i]->CMD->radioid[0] += 1;
			AC_WTP[i]->CMD->setCMD = 1;	
			int WTPIndex = i;
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_TIMESTAMP;
				
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));			
								
			//if(gtrapflag>=4){
			//	wid_dbus_trap_wtp_ap_ACTimeSynchroFailure(i);
			//}
		}
	
	}
	return 0;

}

int wid_radio_set_extension_command(int wtpid, char * command)
{
	msgq msg;
//	struct msgqlist *elem;
	
//	int WTPIndex = wtpid;
	free(AC_WTP[wtpid]->WTP_Radio[0]->excommand);
	AC_WTP[wtpid]->WTP_Radio[0]->excommand = NULL;
	
	AC_WTP[wtpid]->WTP_Radio[0]->excommand = (char*)malloc(strlen(command)+1);
	memset(AC_WTP[wtpid]->WTP_Radio[0]->excommand, 0, strlen(command)+1);
	memcpy(AC_WTP[wtpid]->WTP_Radio[0]->excommand, command, strlen(command));

	if((strlen(command) == 8)&&(strcmp(command, "poweroff") == 0))
	{
			
		if(gtrapflag>=1){
			wid_dbus_trap_wtp_ap_power_off(wtpid);
		}
	}
	if((strlen(command) == 9)&&(strcmp(command, "sysreboot") == 0))
	{
		if(gtrapflag>=1){
			wid_dbus_trap_wtp_ap_reboot(wtpid);
		}
	}
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x0400;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		int WTPIndex = wtpid;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;
			memcpy(msg.mqinfo.u.WtpInfo.value, command, strlen(command));
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;
		memcpy(msg.mqinfo.u.WtpInfo.value, command, strlen(command));

		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(wtpid, elem);
	}*/	
	return 0;	
}
int wid_radio_set_option60_parameter(int wtpid, char * parameter)
{
	msgq msg;
//	struct msgqlist *elem;
	
	int WTPIndex = wtpid;
	if(AC_WTP[WTPIndex]->option60_param != NULL){
		free(AC_WTP[WTPIndex]->option60_param);
		AC_WTP[WTPIndex]->option60_param = NULL;
	}
	AC_WTP[WTPIndex]->option60_param = (char *)malloc(strlen(parameter)+1);
	memset(AC_WTP[WTPIndex]->option60_param,0,(strlen(parameter)+1));
	memcpy(AC_WTP[WTPIndex]->option60_param,parameter,strlen(parameter));
	
	if((AC_WTP[WTPIndex] != NULL)&&(AC_WTP[WTPIndex]->WTPStat == 5))
	{
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_OPTION60_PARAM;
			memcpy(msg.mqinfo.u.WtpInfo.value, parameter, strlen(parameter));
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_OPTION60_PARAM;
		memcpy(msg.mqinfo.u.WtpInfo.value, parameter, strlen(parameter));

		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(wtpid, elem);
	}*/	
	return 0;	
}

int wid_trap_remote_restart(unsigned int wtpid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	char *command;
	command = (char *)malloc(sizeof(char)*10);
	memset(command,0,10);
	strncpy(command,"sysreboot",9);
	wid_syslog_debug_debug(WID_DEFAULT,"sysreboot %s\n",command);

	if(gtrapflag>=4){
		wid_dbus_trap_set_wtp_remote_restart(wtpid);
		}
	wid_radio_set_extension_command(wtpid,command);
	free(command);	
	return 0;
}
int wid_trap_channel_disturb_enable(unsigned int wtpid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	char *command;
	command = (char *)malloc(sizeof(char)*27);
	memset(command,0,27);
	strncpy(command,"touch /tmp/disturb_warning",26);

	char *command2;
	command2 = (char *)malloc(sizeof(char)*30);
	memset(command2,0,30);
	strncpy(command2,"echo 1 > /tmp/disturb_warning",29);
	
	//wid_radio_set_extension_command(wtpid,command);
	wid_radio_set_extension_command(wtpid,command2);
	CW_FREE_OBJECT(command);
	CW_FREE_OBJECT(command2);
	return 0;
}
int wid_trap_channel_disturb_disable(unsigned int wtpid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	char *command;
	command = (char *)malloc(sizeof(char)*30);
	memset(command,0,30);
	strncpy(command,"echo 0 > /tmp/disturb_warning",29);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_ap_l2_isolation_enable(unsigned int wtpid,unsigned char wlanid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int i = 0;
	int ret = 0;
	for (i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->mib_info.wlan_l2isolation[i].wlanid == wlanid)
		{
			return -2;
		}
		
	}
	for (i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->mib_info.wlan_l2isolation[i].wlanid == 0)
		{
			AC_WTP[wtpid]->mib_info.wlan_l2isolation[i].wlanid = wlanid;
			AC_WTP[wtpid]->mib_info.wlan_l2isolation[i].l2_isolation_switch = 1;
			ret = 1;
			break;
		}
		
	}
	if(ret == 0)
	{
		return -1;
	}
	char *command;
	command = (char *)malloc(sizeof(char)*25);
	memset(command,0,25);
	sprintf(command,"iwpriv ath%d ap_bridge 0",wlanid);//0 means open isolation,sta can not ping
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_ap_l2_isolation_disable(unsigned int wtpid,unsigned char wlanid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int i = 0;
	int ret = 0;
	
	for (i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->mib_info.wlan_l2isolation[i].wlanid == wlanid)
		{
			AC_WTP[wtpid]->mib_info.wlan_l2isolation[i].wlanid = 0;
			AC_WTP[wtpid]->mib_info.wlan_l2isolation[i].l2_isolation_switch = 0;
			ret = 1;
			break;
		}
		
	}
	if(ret == 0)
	{
		return -2;									/*xiaodawei modify for l2 isolation disable already, 20101207*/
	}
	
	char *command;
	command = (char *)malloc(sizeof(char)*25);
	memset(command,0,25);
	sprintf(command,"iwpriv ath%d ap_bridge 1",wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_radio_l2_isolation_enable(unsigned int wtpid,unsigned int radioid,unsigned char wlanid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int bssindex = 0;
	bssindex = AC_WLAN[wlanid]->S_WTP_BSS_List[wtpid][radioid];
	if((bssindex != 0)&&(check_bssid_func(bssindex)))
	{
		if(AC_BSS[bssindex]->ath_l2_isolation == 1)
		{
			return 0;
		}
		else
		{
			AC_BSS[bssindex]->ath_l2_isolation = 1;
		}
	}
	
	char *command;
	command = (char *)malloc(sizeof(char)*28);
	memset(command,0,28);
	sprintf(command,"iwpriv ath.%d-%d ap_bridge 0",radioid,wlanid);//0 means open isolation,sta can not ping
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_radio_l2_isolation_disable(unsigned int wtpid,unsigned int radioid,unsigned char wlanid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int bssindex = 0;
	bssindex = AC_WLAN[wlanid]->S_WTP_BSS_List[wtpid][radioid];
	if((bssindex != 0)&&(check_bssid_func(bssindex)))
	{
		if(AC_BSS[bssindex]->ath_l2_isolation == 0)
		{
			return 0;
		}
		else
		{
			AC_BSS[bssindex]->ath_l2_isolation = 0;
		}
	}
	
	char *command;
	command = (char *)malloc(sizeof(char)*28);
	memset(command,0,28);
	sprintf(command,"iwpriv ath.%d-%d ap_bridge 1",radioid,wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_radio_11n_cwmmode(unsigned int wtpid,unsigned int radioid,unsigned char wlanid,unsigned char policy)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int bssindex = 0;
	bssindex = AC_WLAN[wlanid]->S_WTP_BSS_List[wtpid][radioid];
	if(!check_bssid_func(bssindex)){
		wid_syslog_err("<error>%s\n",__func__);
		return BSS_NOT_EXIST;
	}else{}
	if(AC_BSS[bssindex]->cwmmode == policy)
	{
		return 0;
	}
	char *command;
	command = (char *)malloc(sizeof(char)*32);
	memset(command,0,32);
	sprintf(command,"iwpriv ath.%d-%d cwmmode %d",radioid,wlanid,policy);//0 means 20 mode;1 means 20/40 mode
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);

	AC_BSS[bssindex]->cwmmode = policy;
	return 0;
}

int wid_set_ap_dos_def_enable(unsigned int wtpid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	AC_WTP[wtpid]->mib_info.dos_def_switch = 1;
	char *command;
	command = (char *)malloc(sizeof(char)*13);
	memset(command,0,13);
	strncpy(command,"dosdef start",12);
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_ap_dos_def_disable(unsigned int wtpid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	AC_WTP[wtpid]->mib_info.dos_def_switch = 0;
	char *command;
	command = (char *)malloc(sizeof(char)*12);
	memset(command,0,12);
	strncpy(command,"dosdef stop",11);

	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_ap_igmp_snoop_enable(unsigned int wtpid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	AC_WTP[wtpid]->mib_info.igmp_snoop_switch = 1;
	char *command;
	command = (char *)malloc(sizeof(char)*17);
	memset(command,0,17);
	strncpy(command,"igmp_snoop start",16);
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}
int wid_set_ap_igmp_snoop_disable(unsigned int wtpid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	AC_WTP[wtpid]->mib_info.igmp_snoop_switch = 0;
	char *command;
	command = (char *)malloc(sizeof(char)*16);
	memset(command,0,16);
	strncpy(command,"igmp_snoop stop",15);
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	wid_radio_set_extension_command(wtpid,command);
	CW_FREE_OBJECT(command);
	return 0;
}

void channel_interference_detected(int wtpid)
	{
		unsigned char currentchannel = AC_WTP[wtpid]->WTP_Radio[0]->Radio_Chan;
		int i = 0;
		int channel_1 =0,channel_6 =0,channel_11 =0;
		int findrougeap = 0;
		unsigned char same_channel_flag = 0;
		unsigned char neighbor_channel_flag = 0;
		unsigned char mac[MAC_LEN];
		
		Neighbor_AP_INFOS *p_aplist =  AC_WTP[wtpid]->NeighborAPInfos;
		struct Neighbor_AP_ELE *phead = p_aplist->neighborapInfos;
	
		for(i=0; ((phead != NULL)||(i<p_aplist->neighborapInfosCount)); i++)
		{
	
			if(phead->Channel == currentchannel)
			{
				//trap出现干扰当前信道的设备 APInterferenceDetected
				memset(mac,0,MAC_LEN);
				mac[0] = phead->BSSID[0];
				mac[1] = phead->BSSID[1];
				mac[2] = phead->BSSID[2];
				mac[3] = phead->BSSID[3];
				mac[4] = phead->BSSID[4];
				mac[5] = phead->BSSID[5];
				wid_syslog_debug_debug(WID_DEFAULT,"%02X-%02X-%02X-%02X-%02X-%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				if(gtrap_channel_device_interference_switch == 1)
				if(gtrapflag>=24){
					if(AC_WTP[wtpid]->channel_device_interference_flag == 0)    //fengwenchao add 20110221
						{
							wid_dbus_trap_wtp_channel_device_interference(wtpid,currentchannel,mac);
							AC_WTP[wtpid]->channel_device_interference_flag = 1;   //fengwenchao add 20110221
						}
					}
				
				if((phead->IEs_INFO != NULL)&&(strncmp(phead->IEs_INFO,"E",1)==0))
				{
					//trap出出现干扰当前信道的AP  APInterferenceDetected
					 /*zhaoruijia,20100825,当改变临频阀值时也能发送同频 干扰告警清除,start*/
					if(gtrap_channel_device_ap_switch == 1){
						if(((phead->RSSI-95) - AC_WTP[wtpid]->samechannelrssithold)>0){
						if(gtrapflag>=24){
							if(AC_WTP[wtpid]->samechannel_trap_flag == 0)     //fengwenchao add 20110221
								{
								wid_dbus_trap_wtp_channel_ap_interference(wtpid,currentchannel,mac);
								AC_WTP[wtpid]->samechannel_trap_flag = 1;
								}
							}
							same_channel_flag = 1;
						}
					  /*zhaoruijia,20100825,当改变临频阀值时也能发送临频干扰告警清除,end*/
					}
					/*get neighbor ap sta info from asd*/
					if(phead->wtpid > 0){
						wid_syslog_debug_debug(WID_DEFAULT,"rogue_terminal,phead->wtpid=%d,%s\n",phead->wtpid,__func__);
						Asd_neighbor_ap_sta_check_op(wtpid,phead->wtpid,mac,WTP_STA_CHECK);
					}
				}
				else if((phead->IEs_INFO != NULL)&&(strncmp(phead->IEs_INFO,"I",1)==0))
				{
					//trap 出现干扰当前信道的终端 StaInterferenceDetected
					if(gtrapflag>=24){
						if((gtrap_channel_terminal_interference_switch == 1)&&(AC_WTP[wtpid]->wtp_rogue_terminal_threshold>=1))
							if(AC_WTP[wtpid]->wid_trap.rogue_terminal_trap_flag == 0)  //fengwenchao add 20110221
								{
									wid_dbus_trap_wtp_channel_terminal_interference(wtpid,0,currentchannel,mac);
									AC_WTP[wtpid]->wid_trap.rogue_terminal_trap_flag = 1;   //fengwenchao add 20110221
								}
					}
				}
				else
				{
				}
				
			}
			else if((phead->Channel == (currentchannel-1))||(phead->Channel == (currentchannel+1))){
				/*zhaoruijia,20100825,当改变临频阀值时也能发送临频干扰告警清除,start*/
				if(((phead->RSSI-95) - AC_WTP[wtpid]->neighborchannelrssithold )>0){
					if(gtrapflag>=24){
						if(AC_WTP[wtpid]->neighborchannel_trap_flag == 0)  //fengwenchao add 20110221
							{
								wid_dbus_trap_wtp_neighbor_channel_ap_interference(wtpid,currentchannel,mac,0);
								AC_WTP[wtpid]->neighborchannel_trap_flag = 1;
							}
					}
					neighbor_channel_flag = 1;
				}
				/*zhaoruijia,20100825,当改变临频阀值时也能发送临频干扰告警清除,end*/
			}
	
			if(phead->wtpid == 0)
			{
				findrougeap = 1;
			}
			
			if(phead->Channel == 1)
			{
				channel_1 = 1;
			}
			else if(phead->Channel == 6)
			{
				channel_6 = 1;
			}
			else if(phead->Channel == 11)
			{
				channel_11= 1;
			}
			else
			{
			}
	
			
			phead = phead->next;
			
		}
	
		//printf("same_channel_flag %d\n",same_channel_flag);
		//printf("AC_WTP[%d]->samechannel_trap_flag %d\n",wtpid,AC_WTP[wtpid]->samechannel_trap_flag);
		//printf("neighbor_channel_flag %d\n",neighbor_channel_flag);
		//printf("AC_WTP[%d]->neighborchannel_trap_flag %d\n",wtpid,AC_WTP[wtpid]->neighborchannel_trap_flag);
	
		
		if(same_channel_flag == 0){
			if(AC_WTP[wtpid]->samechannel_trap_flag == 1){
				wid_dbus_trap_wtp_channel_ap_interference_clear(wtpid,currentchannel);
				AC_WTP[wtpid]->samechannel_trap_flag = 0;
			}
		}
		if(neighbor_channel_flag == 0){
			if(AC_WTP[wtpid]->neighborchannel_trap_flag == 1){
				wid_dbus_trap_wtp_neighbor_channel_ap_interference(wtpid,currentchannel,mac,1);
				AC_WTP[wtpid]->neighborchannel_trap_flag = 0;
			}
		}
	
		if((channel_1 == 1)&&(channel_6 == 1)&&(channel_11 == 1))
		{
			// 可供使用的信道数过低 DFSFreeCountBelowThreshold 
			//trap
			if(gtrap_channel_count_minor_switch == 1)
			if(gtrapflag>=4){
				if(AC_WTP[wtpid]->channel_count_minor_flag == 0)    //fengwenchao add 20110221
					{
						wid_dbus_trap_wtp_channel_count_minor(wtpid);
						AC_WTP[wtpid]->channel_count_minor_flag = 1;  //fengwenchao add 20110221
					}
				}
		}
		else
		{
		if(gtrapflag>=4){
			if(AC_WTP[wtpid]->channel_count_minor_flag == 1)    //fengwenchao add 20110221
				{
					wid_dbus_trap_wtp_channel_count_minor_clear(wtpid);
					AC_WTP[wtpid]->channel_count_minor_flag = 0;  //fengwenchao add 20110221
				}
			}
		}
		unsigned char now_rssi = 0;           //fengwenchao add 20110507
		struct Neighbor_AP_ELE *p_head = p_aplist->neighborapInfos; //fengwenchao add 20110507
		struct Neighbor_AP_ELE *p_rssi = p_aplist->neighborapInfos;       //fengwenchao add 20110507
		if(findrougeap == 1)
		{
			if(gtrapflag>=4){
				if(AC_WTP[wtpid]->ac_discovery_danger_ap_flag == 1)   //fengwenchao add 20110221
					{
						/*fengwenchao add 20110507*/
						for(i=0; ((p_head != NULL)&&(i<p_aplist->neighborapInfosCount)); i++)
						{
							if(p_head->wtpid == 0)
							{
								if(p_head->RSSI   >  now_rssi)
								{
									now_rssi = p_head->RSSI;
									p_rssi = p_head;
								}
							}
							p_head = p_head->next;
						}
						/*fengwenchao add end*/
						wid_dbus_trap_wtp_ac_discovery_danger_ap(wtpid,p_rssi);   //fengwenchao modify 20110509
						AC_WTP[wtpid]->ac_discovery_danger_ap_flag = 0;    //fengwenchao add 20110221
					}
				}
		}
	}

int wid_send_to_ap_extension_infomation(unsigned int wtpid)
{
	msgq msg;
//	struct msgqlist *elem;
	//if(gWTPs[wtpid].currentState != CW_ENTER_RUN)
	{
		//return WTP_NOT_IN_RUN_STATE;
	}

		
	if(AC_WTP[wtpid]->WTPStat == 5)
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x4000;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_INFO_GET;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_INFO_GET;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/

	return 0;
	
}

int wid_radio_wsm_sta_info_report(unsigned int wtpid,unsigned int l_radioid,unsigned int radioid,unsigned int bssindex)
{	
	msgq msg;
	if(AC_WTP[wtpid]->WTPStat == 5)
	{
		AC_RADIO[radioid]->CMD |= 0x20;
		AC_WTP[AC_RADIO[radioid]->WTPID]->CMD->radioid[l_radioid] += 1;
		AC_WTP[AC_RADIO[radioid]->WTPID]->CMD->setCMD = 1;	
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)& msg, 0, sizeof(msg));
			msg.mqid = wtpid % THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = RADIO_WSM_STA_INFO_REPORT;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = radioid%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = radioid;
			msg.mqinfo.u.RadioInfo.id1 = bssindex;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = RADIO_WSM_STA_INFO_REPORT;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = radioid%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = radioid;
			msg.mqinfo.u.RadioInfo.id1 = bssindex;
			struct msgqlist *elem;
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
			elem = NULL;
		}*/
	return 0;
}
int wid_send_to_ap_sta_deauth_report(unsigned int wtpid)
{
	wid_syslog_debug_debug(WID_DEFAULT,"in func %s :  set %d wtp deauth %s",__func__,wtpid,AC_WTP[wtpid]->sta_deauth_message_reportswitch?"enable":"disable");
	msgq msg;
//	struct msgqlist *elem;	
	if(AC_WTP[wtpid]->WTPStat == 5)
	{		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_DEAUTH_SWITCH;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_DEAUTH_SWITCH;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/

	return 0;
	
}
//
int wid_send_to_ap_sta_flow_information_report(unsigned int wtpid)
{
	wid_syslog_debug_debug(WID_DEFAULT,"in func %s :  set %d wtp flow information %s",__func__,wtpid,AC_WTP[wtpid]->sta_flow_information_reportswitch?"enable":"disable");
	msgq msg;
//	struct msgqlist *elem;	
	if(AC_WTP[wtpid]->WTPStat == 5)
	{		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_FLOW_INFORMATION_SWITCH;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_FLOW_INFORMATION_SWITCH;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/

	return 0;
	
}
/* zhangshu add for Terminal Disturb Info Report, 2010-10-08 */
int wid_send_to_ap_Terminal_Disturb_info(unsigned int wtpid)
{
	msgq msg;
//	struct msgqlist *elem;
		
	if(AC_WTP[wtpid]->WTPStat == 5)
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x4000;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_TERMINAL_DISTRUB_INFO;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_TERMINAL_DISTRUB_INFO;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/

	return 0;
	
}


int setWtpNoRespToStaProReq(unsigned int wtpid,unsigned char l_radioid,unsigned char wlanid,unsigned int policy)
{
	msgq msg;
//	struct msgqlist *elem;
	wid_syslog_debug_debug(WID_DBUS,"%s,%d,wtp %d l_radio %d wlan:%d,policy=%d.\n",__func__,__LINE__,wtpid,l_radioid,wlanid,policy);
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x4000;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_NO_RESP_STA_PRO_REQ;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = l_radioid;
			msg.mqinfo.u.RadioInfo.id1 = policy;
			msg.mqinfo.u.RadioInfo.wlanid = wlanid;

			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_NO_RESP_STA_PRO_REQ;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = l_radioid;
			msg.mqinfo.u.RadioInfo.id1 = policy;
			msg.mqinfo.u.RadioInfo.wlanid = wlanid;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/

	return 0;
	
}


int setWtpUniMutiBroCastIsolation(unsigned int wtpid,unsigned char radioid,unsigned char wlanid,unsigned char policy)
{
	msgq msg;
//	struct msgqlist *elem;
	wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d wlan:%d,policy=%d.\n",__func__,__LINE__,radioid,wlanid,policy);
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x4000;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_UNI_MUTIBRO_CAST_ISO_SW;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = radioid;
			msg.mqinfo.u.RadioInfo.wlanid = wlanid;
			msg.mqinfo.u.RadioInfo.id_char = policy;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}
	//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_UNI_MUTIBRO_CAST_ISO_SW;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = radioid;
			msg.mqinfo.u.RadioInfo.wlanid = wlanid;
			msg.mqinfo.u.RadioInfo.id_char = policy;
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/

	return 0;
	
}

int setWtpUniMutiBroCastRate(unsigned int wtpid,unsigned char radioid,unsigned char wlanid,unsigned int rate)
{
	msgq msg;
//	struct msgqlist *elem;
	wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d wlan:%d,rate=%d.\n",__func__,__LINE__,radioid,wlanid,rate);
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x4000;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_UNI_MUTIBRO_CAST_TATE;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = radioid;
			msg.mqinfo.u.RadioInfo.wlanid = wlanid;
			msg.mqinfo.u.RadioInfo.id1 = rate;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	
	/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_UNI_MUTIBRO_CAST_TATE;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = radioid;
			msg.mqinfo.u.RadioInfo.wlanid = wlanid;
			msg.mqinfo.u.RadioInfo.id1 = rate;
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/

	return 0;
	
}

int uni_muti_bro_cast_rate_check(unsigned int wtpindex,unsigned int radioid,unsigned int rate){
	/*
	11b rate set：1000 2000 5500 11000
	11g rate set：1000 2000 5500 11000 6000 9000 12000 18000 24000 36000
						48000 54000
	11nght20 rate set：1000 2000 5500 11000 6000 9000 12000 18000 24000 36000
							48000 54000
	11nght40/11nght40plus/11nght40minus 
	rate set：1000 2000 5500 11000 6000 9000 12000 18000 24000 36000 48000 54000
	
	11a rate set：	  6000 9000 12000 18000 24000 36000 48000 54000
	11naht20 rate set：6000 9000 12000 18000 24000 36000 48000 54000
	
	11naht40/11naht40plus/11naht40minus 
	rate set：6000 9000 12000 18000 24000 36000 48000 54000
	*/
	int ret = 0;
	if(AC_WTP[wtpindex]->WTP_Radio[radioid]->Radio_Type == IEEE80211_11B){
		wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d ,check rate:%d.\n",__func__,__LINE__,radioid,rate);
		if((10 == rate)||(20 == rate )||(55 == rate)||(110 == rate)){
			ret = 0;
		}else{
			ret = RADIO_SUPPORT_RATE_EXIST;
		}
	}else if(AC_WTP[wtpindex]->WTP_Radio[radioid]->Radio_Type & IEEE80211_11G){
		wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d ,check rate:%d.\n",__func__,__LINE__,radioid,rate);
		if((10 == rate)||(20 == rate )||(55 == rate)||(110 == rate)||(60 == rate)||(90 == rate )||(120 == rate)||(180 == rate)\
			||(240 == rate)||(360 == rate)||(480 == rate)||(540 == rate)){
			ret = 0;
		}else{
			ret = RADIO_SUPPORT_RATE_EXIST;
		}
	}else if(AC_WTP[wtpindex]->WTP_Radio[radioid]->Radio_Type & IEEE80211_11A){
		wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d ,check rate:%d.\n",__func__,__LINE__,radioid,rate);
		if((60 == rate)||(90 == rate )||(120 == rate)||(180 == rate)\
			||(240 == rate)||(360 == rate)||(480 == rate)||(540 == rate)){
			ret = 0;
		}else{
			ret = RADIO_SUPPORT_RATE_EXIST;
		}
	}else{
		wid_syslog_warning("%s,%d,unknow radio type:%d.\n",__func__,__LINE__,AC_WTP[wtpindex]->WTP_Radio[radioid]->Radio_Type);
		ret = RADIO_SUPPORT_RATE_EXIST;
	}
	wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d ,check rate:%d,ret=%d.\n",__func__,__LINE__,radioid,rate,ret);
	return ret;
}
int set_wid_sample_enable()
{
	int i = 0;
	msgq msg;
	for(i=0; i<WTP_NUM; i++)
	{
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
			wid_syslog_debug_debug(WID_DEFAULT,"## set_wid_sample_enable wtp id= %d##\n",i);
			
			AC_WTP[i]->WTP_Radio[0]->CMD |= 0x8000;
			AC_WTP[i]->CMD->radioid[0] += 1;
			AC_WTP[i]->CMD->setCMD = 1;	
			int WTPIndex = i;
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SAMPLE_INFO_SET;
				
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}

			}		
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
		}
	}
	
	return 0;
}
int wid_radio_bss_set_max_throughput(unsigned int wtp_id,unsigned int l_radio_id,unsigned int l_bss_id,unsigned int throughput)
{
	msgq msg;
	if(AC_WTP[wtp_id]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->band_width == throughput)
	{
		return 0;
	}
	else
	{
		AC_WTP[wtp_id]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->band_width = throughput;
	}
	if((AC_WTP[wtp_id] != NULL)&&(AC_WTP[wtp_id]->WTPStat == 5))
	{
		AC_WTP[wtp_id]->WTP_Radio[0]->CMD |= 0x0800;
		AC_WTP[wtp_id]->CMD->radioid[0] += 1;
		AC_WTP[wtp_id]->CMD->setCMD = 1;	
		int WTPIndex = wtp_id;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_BSS_Throughput;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = l_radio_id;
			msg.mqinfo.u.RadioInfo.BSS_L_ID = l_bss_id;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}
	
	return 0;	
}
int wid_set_wlan_vlanid(unsigned char Wlanid,unsigned int vlanid)
{
	if (AC_WLAN[Wlanid]->vlanid == vlanid)
	{
		return WID_DBUS_SUCCESS;
	}
	
	AC_WLAN[Wlanid]->vlanid = vlanid;
	
	{
		int i,j;
		for(i=0; i<WTP_NUM; i++)
		{
			/*if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))*/
			if(AC_WTP[i]!=NULL)
			{
				for(j=0; j<AC_WTP[i]->RadioCount; j++)//zhanglei change L_RADIO_NUM to AC_WTP[i]->RadioCount
				{
					if(AC_WLAN[Wlanid]->S_WTP_BSS_List[i][j] != 0)
					{
						int bssindex = AC_WLAN[Wlanid]->S_WTP_BSS_List[i][j];
						if(!check_bssid_func(bssindex)){
							wid_syslog_err("<error>%s\n",__func__);
							//return BSS_NOT_EXIST;
						}else{						
							AC_BSS[bssindex]->wlan_vlanid = vlanid;
						}
					}
				}
			}
		}
	}
	return WID_DBUS_SUCCESS;
}
int wid_set_wlan_vlan_priority(unsigned char Wlanid,unsigned int priority)
{
	if (AC_WLAN[Wlanid]->wlan_1p_priority == priority)
	{
		return WID_DBUS_SUCCESS;
	}
	
	AC_WLAN[Wlanid]->wlan_1p_priority = priority;
	
	return WID_DBUS_SUCCESS;
}

int CHECK_AND_UPDATE_IF_POLICY(char* ifname,int *wtpid1,int *radioid1,int *wlanid1,int isadd)
{
//	int ret = 0;
	unsigned int vrrid_t = 0;
	int wtpid = 0,radioid = 0, wlanid = 0;
	
	if(!strncasecmp(ifname,"r",1)){
		char *id = (char *)malloc(sizeof(char)*25);
		memset(id,0,25);
		if(id == NULL)
		{
			wid_syslog_err("malloc error,%s",__func__);
			return MALLOC_ERROR;
		}
		memcpy(id,ifname+1,(strlen(ifname)-1));
		if(parse_radio_ifname(id,&wtpid,&radioid,&wlanid)==0)
		{
			wid_syslog_debug_debug(WID_DBUS,"ifname %s,wtpid %d,radioid %d wlanid2 %d",ifname,wtpid,radioid,wlanid);
		}else if(parse_radio_ifname_v2(id, &wtpid,&radioid,&wlanid,&vrrid_t)==0){

		}else{
			wid_syslog_warning("parse radio interface %s error",ifname);;
			if(id != NULL){
				free(id);
				id = NULL;
			}
			return -1;
		}
		if(id != NULL){
			free(id);
			id = NULL;
		}
	}else{
		wid_syslog_debug_debug(WID_DBUS,"%s,%s",__func__,ifname);
	}
	
	*wtpid1 = wtpid;
	*radioid1 = radioid;
	*wlanid1 = wlanid;
	if((AC_WLAN[wlanid])&&(AC_WLAN[wlanid]->S_WTP_BSS_List[wtpid][radioid] != 0))
	{
		int bssindex = AC_WLAN[wlanid]->S_WTP_BSS_List[wtpid][radioid];
		if(!check_bssid_func(bssindex)){
			wid_syslog_err("\n");
			//return ;							
		}else if(isadd){						
			if(AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE)
			{
				wid_syslog_warning("<warning>bss %d policy is NO_INTERFACE->WLAN_INTERFACE\n",bssindex);
				AC_BSS[bssindex]->BSS_IF_POLICY = WLAN_INTERFACE;
				AC_BSS[bssindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
			}
			else if(AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"bss %d policy is BSS_INTERFACE->WLAN_INTERFACE\n",bssindex);
				AC_BSS[bssindex]->BSS_IF_POLICY = WLAN_INTERFACE;
				AC_BSS[bssindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
			}
			else
			{
			}
		}else{						
			if(AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE)
			{
				wid_syslog_warning("<warning>bss %d policy is NO_INTERFACE->BSS_INTERFACE\n",bssindex);
				AC_BSS[bssindex]->BSS_IF_POLICY = BSS_INTERFACE;
				AC_BSS[bssindex]->BSS_TUNNEL_POLICY = 0;
			}
			else if(AC_BSS[bssindex]->BSS_IF_POLICY == WLAN_INTERFACE)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"bss %d policy is WLAN_INTERFACE->BSS_INTERFACE\n",bssindex);
				AC_BSS[bssindex]->BSS_IF_POLICY = BSS_INTERFACE;
				AC_BSS[bssindex]->BSS_TUNNEL_POLICY = 0;
			}
			else
			{
			}
		}
	}
	return 0;
}

int wid_set_tunnel_wlan_vlan(unsigned char wlanid,char * ifname)
{
//	printf("input wlanid %d ifname %s\n",wlanid,ifname);

	int ret = 0;
	int reason = 0;
	char brname[ETH_IF_NAME_LEN];
	char syscmd[WID_SYSTEM_CMD_LENTH];
	WTPQUITREASON quitreason = WTP_INIT;
	int wtpid = 0;
	int wlanid2 = 0;
	int radioid = 0;
		
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(syscmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(brname,"wlanl%d-%d-%d",slotid,vrrid,wlanid);
	else
		sprintf(brname,"wlan%d-%d-%d",slotid,vrrid,wlanid);
		
	ret = Check_Interface_Config(brname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return ret;
	}
	sprintf(syscmd,"brctl addif %s %s",brname,ifname);
//	printf("syscmd %s\n",syscmd);

	ret = system(syscmd);
	reason = WEXITSTATUS(ret);
	//if(ret == WID_DBUS_SUCCESS)
	if(reason == WID_DBUS_SUCCESS)
	{
		if(CHECK_AND_UPDATE_IF_POLICY(ifname,&wtpid,&radioid,&wlanid2,1) == 0){
			wid_syslog_debug_debug(WID_DBUS,"ifname %s CHECK_AND_UPDATE_IF_POLICY successfull",ifname);
		}else{
			wid_syslog_debug_debug(WID_DBUS,"ifname %s,line %d",ifname,__LINE__);
		}
		struct WID_TUNNEL_WLAN_VLAN *wif;
		struct WID_TUNNEL_WLAN_VLAN *wifnext;
		
		wif = (struct WID_TUNNEL_WLAN_VLAN*)malloc(sizeof(struct WID_TUNNEL_WLAN_VLAN));
		memset(wif->ifname,0,ETH_IF_NAME_LEN);
		memcpy(wif->ifname,ifname,strlen(ifname));
		wif->ifnext = NULL;
		
		if(AC_WLAN[wlanid]->tunnel_wlan_vlan == NULL)
		{
//			printf("wlan id:%d ifname :%s\n",wlanid,ifname);
			AC_WLAN[wlanid]->tunnel_wlan_vlan = wif ;
			AC_WLAN[wlanid]->tunnel_wlan_vlan->ifnext = NULL;
		}
		else
		{
			wifnext = AC_WLAN[wlanid]->tunnel_wlan_vlan;
			while(wifnext != NULL)
			{	
				if(strncmp(wifnext->ifname,ifname,strlen(ifname)) == 0)
				{
//					printf("already in the list\n");
					free(wif);
					wif = NULL;
					return 0;
				}
				wifnext = wifnext->ifnext;
			}
			
//			printf("wlan id:%d ifname :%s\n",wlanid,ifname);
			wifnext = AC_WLAN[wlanid]->tunnel_wlan_vlan;
			while(wifnext->ifnext != NULL)
			{	
				wifnext = wifnext->ifnext;
			}
			
			wifnext->ifnext = wif;
			
		}
		return 0;
	}
	else
	{
		return SYSTEM_CMD_ERROR;
	}
	
}
int wid_undo_tunnel_wlan_vlan(unsigned char wlanid,char * ifname)
{
//	printf("input wlanid %d ifname %s\n",wlanid,ifname);

	int ret = 0;
	int reason = 0;
	int wtpid = 0;
	int wlanid2 = 0;
	int radioid = 0;
	char brname[ETH_IF_NAME_LEN];
	char syscmd[WID_SYSTEM_CMD_LENTH];
	WTPQUITREASON quitreason = WTP_INIT;
		
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(syscmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(brname,"wlanl%d-%d-%d",slotid,vrrid,wlanid);
	else
		sprintf(brname,"wlan%d-%d-%d",slotid,vrrid,wlanid);
	ret = Check_Interface_Config(brname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return ret;
	}

	sprintf(syscmd,"brctl delif %s %s",brname,ifname);
//	printf("syscmd %s\n",syscmd);

	ret = system(syscmd);
	reason = WEXITSTATUS(ret);
	//if(ret == WID_DBUS_SUCCESS)
	if(reason == WID_DBUS_SUCCESS)
	{

		if(CHECK_AND_UPDATE_IF_POLICY(ifname,&wtpid,&radioid,&wlanid2,0) == 0){
			wid_syslog_debug_debug(WID_DBUS,"ifname %s,wtpid %d,radioid %d wlanid2 %d",ifname,wtpid,radioid,wlanid2);
		}else{
			wid_syslog_debug_debug(WID_DBUS,"ifname %s,line %d",ifname,__LINE__);
		}
	
		struct WID_TUNNEL_WLAN_VLAN *wif;
		struct WID_TUNNEL_WLAN_VLAN *wifnext;

		wifnext = AC_WLAN[wlanid]->tunnel_wlan_vlan;
		
		if(AC_WLAN[wlanid]->tunnel_wlan_vlan != NULL)
		{
			if(strncmp(wifnext->ifname,ifname,strlen(ifname)) == 0)
			{
				AC_WLAN[wlanid]->tunnel_wlan_vlan = wifnext->ifnext;
				free(wifnext);
				wifnext = NULL;		
//				printf("delete ifname %s from list\n",ifname);
			}
			else
			{
				while(wifnext->ifnext != NULL)
				{	
					if(strncmp(wifnext->ifnext->ifname,ifname,strlen(ifname)) == 0)
					{
						wif = wifnext->ifnext;
						wifnext->ifnext = wifnext->ifnext->ifnext;
						free(wif);
						wif = NULL;				
//						printf("delete ifname %s from list\n",ifname);
						return 0;
					}
					wifnext = wifnext->ifnext;
				}
			}
		}
		return 0;
	}
	else
	{
		return SYSTEM_CMD_ERROR;
	}
}

//xm add
static int get_oui_node(unsigned char oui[],struct oui_node **od,struct oui_node **cr){

	if(0==g_oui_list.list_len)
		return -1;

	if(od!=NULL&&cr!=NULL){
		*od = *cr = g_oui_list.oui_list;
		
		while(*cr!=NULL){
			if(0!=memcmp((*cr)->oui,oui,3)){
				*od=*cr;
				*cr=(*cr)->next;
			}else
				return 0;
		}
		
		return -1;
	}else if(od==NULL&&cr==NULL){
	
		struct oui_node *crt;
		
		crt = g_oui_list.oui_list;
		while(crt!=NULL){
			if(0!=memcmp(crt->oui,oui,3)){
				crt=crt->next;
			}else
				return 0;
		}
		
		return -1;		
	}
	return -1;	
}


static int get_essid_node(char *essid,struct essid_node **od,struct essid_node **cr){
	
		if(0==g_essid_list.list_len)
			return ESSID_LIST_IS_NULL;
	
		if(od!=NULL&&cr!=NULL){
			*od = *cr = g_essid_list.essid_list;
			
			while(*cr!=NULL){
				if(0!=strcmp((*cr)->essid,essid)){
					*od=*cr;
					*cr=(*cr)->next;
				}else
					return 0;
			}
			
			return ESSID_NOT_EXIT;
		}else if(od==NULL&&cr==NULL){
		
			struct essid_node *crt;
			crt = g_essid_list.essid_list;
			while(crt!=NULL){
				if(0!=strcmp(crt->essid,essid)){
					crt=crt->next;
				}else
					return 0;
			}
			
			return ESSID_NOT_EXIT;		
		}
		return -1;
	}

static int get_mac_node(unsigned char mac[],struct attack_mac_node **od, struct attack_mac_node **cr){
	
		if(0==g_attack_mac_list.list_len)
			return -1;
	
		if(od!=NULL&&cr!=NULL){
			*od = *cr = g_attack_mac_list.attack_mac_list;
			
			while(*cr!=NULL){
				if(0!=memcmp((*cr)->mac,mac,6)){
					*od=*cr;
					*cr=(*cr)->next;
				}else
					return 0;
			}
			
			return -1;
		}else if(od==NULL&&cr==NULL){
			struct attack_mac_node *crt;
			crt = g_attack_mac_list.attack_mac_list;
			while(crt!=NULL){
				if(0!=memcmp(crt->mac,mac,6)){
					crt=crt->next;
				}else
					return 0;
			}
			
			return -1;		
		}
		return -1;
}

int wid_add_manufacturer_oui(unsigned char oui[]){

	int ret;
	struct oui_node *node=NULL;

	if(oui==NULL)
		return -1;

	if(0==g_oui_list.list_len){
		CW_CREATE_OBJECT_ERR(node, struct oui_node, return -1;);
		node->oui[0]=oui[0];
		node->oui[1]=oui[1];
		node->oui[2]=oui[2];
		node->next=NULL;

		g_oui_list.list_len++;
		g_oui_list.oui_list=node;

	}else{
		ret=get_oui_node(oui,NULL,NULL);
		if(0==ret)
			return 0;
		else{
			CW_CREATE_OBJECT_ERR(node, struct oui_node, return -1;);
			node->oui[0]=oui[0];
			node->oui[1]=oui[1];
			node->oui[2]=oui[2];

			node->next=g_oui_list.oui_list;
			g_oui_list.oui_list=node;
			
			g_oui_list.list_len++;
			
		}

	}

	return 0;
}
int wid_add_legal_essid(char *essid){

	int ret;
	struct essid_node *node=NULL;

	if(essid==NULL)
		return -1;

	if(0==g_essid_list.list_len){
		CW_CREATE_OBJECT_ERR(node, struct essid_node, return -1;);

		node->essid=(char *)malloc(strlen(essid)+1);
		memset(node->essid,0,strlen(essid)+1);
		memcpy(node->essid,essid,strlen(essid));

		node->len=strlen(essid);
		node->next=NULL;

		g_essid_list.list_len++;
		g_essid_list.essid_list=node;

	}else{
		ret=get_essid_node(essid,NULL,NULL);
		if(0==ret)
			return 0;
		else{
			CW_CREATE_OBJECT_ERR(node, struct essid_node, return -1;);
			
			node->essid=(char *)malloc(strlen(essid)+1);
			memset(node->essid,0,strlen(essid)+1);
			memcpy(node->essid,essid,strlen(essid));
			
			node->len=strlen(essid);

			node->next=g_essid_list.essid_list;
			g_essid_list.essid_list=node;
			
			g_essid_list.list_len++;
			
		}

	}

	return 0;
}

int wid_modify_legal_essid(char *essid,char *essid_new){
		struct essid_node *od, *cr;
		int ret;
		ret=get_essid_node(essid,&od,&cr);
	
		if(0==ret){
				memset(cr->essid,0,strlen(essid_new)+1);
				memcpy(cr->essid,essid_new,strlen(essid_new));
		}else
			return ret;
		return 0;
	}

int wid_add_attack_ap_mac(unsigned char mac[]){

	int ret;
	struct attack_mac_node *node=NULL;

	if(mac==NULL)
		return -1;

	if(0==g_attack_mac_list.list_len){
		CW_CREATE_OBJECT_ERR(node, struct attack_mac_node, return -1;);

		memset(node->mac,0,6);
		memcpy(node->mac,mac,6);
		
		node->next=NULL;

		g_attack_mac_list.list_len++;
		g_attack_mac_list.attack_mac_list=node;

	}else{
		ret=get_mac_node(mac,NULL,NULL);
		if(0==ret)
			return 0;
		else{
			CW_CREATE_OBJECT_ERR(node, struct attack_mac_node, return -1;);
			
			memset(node->mac,0,6);
			memcpy(node->mac,mac,6);

			node->next=g_attack_mac_list.attack_mac_list;
			g_attack_mac_list.attack_mac_list=node;
			
			g_attack_mac_list.list_len++;
		}

	}

	return 0;
}


int wid_del_manufacturer_oui(unsigned char oui[]){
	struct oui_node *od, *cr;
	int ret;
	ret=get_oui_node(oui,&od,&cr);

	if(0==ret){
		if(od==cr){
			g_oui_list.oui_list=cr->next;

		}else{
			od->next=cr->next;
		}
		cr->next=NULL;
		free(cr);
		cr=NULL;
		g_oui_list.list_len--;
	}else
		return 0;
	return 0;
}


int wid_del_legal_essid(char *essid){
	struct essid_node *od, *cr;
	int ret;
	ret=get_essid_node(essid,&od,&cr);

	if(0==ret){
		if(od==cr){
			g_essid_list.essid_list=cr->next;

		}else{
			od->next=cr->next;
		}
		cr->next=NULL;

		free(cr->essid);
		cr->essid=NULL;
		free(cr);
		cr=NULL;
		g_essid_list.list_len--;
	}else
		return 0;
	return 0;
}

int wid_del_attack_ap_mac(unsigned char mac[]){
	struct attack_mac_node *od, *cr;
	int ret;
	ret=get_mac_node(mac,&od,&cr);
	
	if(0==ret){
		if(od==cr){
			g_attack_mac_list.attack_mac_list=cr->next;
	
		}else{
			od->next=cr->next;
		}
		
		cr->next=NULL;
		free(cr);
		cr=NULL;
		g_attack_mac_list.list_len--;
	}else
		return 0;
	return 0;
}
int wid_set_wlan_br_isolation(unsigned char wlanid,unsigned char state)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brcmd[WID_SYSTEM_CMD_LENTH];
	memset(brcmd,0,WID_SYSTEM_CMD_LENTH);
	
	
	char brname[ETH_IF_NAME_LEN];
	memset(brname,0,ETH_IF_NAME_LEN);
	if(local)
		sprintf(brname,"wlanl%d-%d-%d",slotid,vrrid,wlanid);
	else
		sprintf(brname,"wlan%d-%d-%d",slotid,vrrid,wlanid);		
//	printf("brname:wlan%d\n",wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_wlan_br_isolation ifname:%s\n",brname);

	//check br if validity
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WLAN_CREATE_BR_FAIL;
	}

	//set br isolation
	if(state == 0)
	{
		sprintf(brcmd,"brctl isolation %s off\n",brname);
	}
	else if(state == 1)
	{
		sprintf(brcmd,"brctl isolation %s on\n",brname);
	}
	wid_syslog_debug_debug(WID_DEFAULT,"set bridge-isolation cmd:%s\n",brcmd);
	
	ret = system(brcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);

	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;//set failure
	}
	else
	{
		AC_WLAN[wlanid]->isolation_policy = state;
		return 0;//set success
	}
	
}
int wid_set_wlan_br_multicast_isolation(unsigned char wlanid,unsigned char state)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brcmd[WID_SYSTEM_CMD_LENTH];
	memset(brcmd,0,WID_SYSTEM_CMD_LENTH);
	
	
	char brname[ETH_IF_NAME_LEN];
	memset(brname,0,ETH_IF_NAME_LEN);
	if(local)
		sprintf(brname,"wlanl%d-%d-%d",slotid,vrrid,wlanid);
	else
		sprintf(brname,"wlan%d-%d-%d",slotid,vrrid,wlanid);		
//	printf("brname:wlan%d\n",wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_wlan_br_multicast_isolation ifname:%s\n",brname);

	//check br if validity
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WLAN_CREATE_BR_FAIL;
	}

	//set br isolation
	if(state == 0)
	{
		sprintf(brcmd,"brctl multicastisolation %s off\n",brname);
	}
	else if(state == 1)
	{
		sprintf(brcmd,"brctl multicastisolation %s on\n",brname);
	}
	wid_syslog_debug_debug(WID_DEFAULT,"set bridge-multicast isolation cmd:%s\n",brcmd);
	
	ret = system(brcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);

	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;//set failure
	}
	else
	{
		AC_WLAN[wlanid]->multicast_isolation_policy = state;
		return 0;//set success
	}
	
}











/*
  *****************************************************************************
  *  
  * NOTES:	 
  * INPUT:	   
  * OUTPUT:	  
  * return:	  
  *  
  * author: 		Huang Leilei 
  * begin time:	2012-11-12 9:00  
  * finish time:		2012-11-15 11:00 
  * history:	
  * 
  **************************************************************************** 
  */
int 
wid_set_wlan_ebr_br_ucast_solicit(unsigned int Wlan_Ebr_ID, unsigned char state, unsigned char Wlan_Ebr_flag)
{
	int ret = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char Wlan_Ebr_File[PATH_MAX + NAME_MAX];
	memset(Wlan_Ebr_File, 0, sizeof(Wlan_Ebr_File));
	
	char Wlan_Ebr_Name[ETH_IF_NAME_LEN];
	memset(Wlan_Ebr_Name,0,ETH_IF_NAME_LEN);

	int ucast_fd = 0;

	if (Wlan_Ebr_flag == 0)
	{
		if (local)
		{
			sprintf(Wlan_Ebr_Name, "wlanl%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}
		else
		{
			sprintf(Wlan_Ebr_Name, "wlan%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}
		
		wid_syslog_debug_debug(WID_DBUS, "wid_set_wlan_ebr_br_ucast_solicit ifname:%s\n",Wlan_Ebr_Name);

		//check br if validity
		ret = Check_Interface_Exist(Wlan_Ebr_Name, &quitreason);
		if(ret != 0)
		{
			wid_syslog_err("%s: interface %s not exist\n", __func__, Wlan_Ebr_Name);
			return WLAN_CREATE_BR_FAIL;
		}

		sprintf(Wlan_Ebr_File, "/proc/sys/net/ipv4/neigh/%s/ucast_solicit", Wlan_Ebr_Name);
		
		wid_syslog_debug_debug(WID_DBUS, "wid_set_wlan_ebr_br_ucast_solicit ifname:%s\n",Wlan_Ebr_File);

	}
	else if (Wlan_Ebr_flag == 1)
	{
		if (local)
		{
			sprintf(Wlan_Ebr_Name, "ebrl%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}
		else
		{
			sprintf(Wlan_Ebr_Name, "ebr%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}

		//check br if validity
		ret = Check_Interface_Exist(Wlan_Ebr_Name,&quitreason);
		if(ret != 0)
		{
			wid_syslog_err("%s: interface %s not exist\n", __func__, Wlan_Ebr_Name);
			return WLAN_CREATE_BR_FAIL;
		}

		sprintf(Wlan_Ebr_File, "/proc/sys/net/ipv4/neigh/%s/ucast_solicit", Wlan_Ebr_Name);
		
		wid_syslog_debug_debug(WID_DBUS, "wid_set_wlan_ebr_br_ucast_solicit ifname:%s\n",Wlan_Ebr_File);

	}
	else
	{
		wid_syslog_err("%s: <error> Wlan_Ebr_flag:%d\n", __func__, Wlan_Ebr_flag);
	}
		
	//未使用文件读写锁
	ucast_fd = open(Wlan_Ebr_File, O_RDWR);
	if(ucast_fd< 0)
	{
		wid_syslog_err("%s: open file %s error, return vlaue is:%d\n", __func__, Wlan_Ebr_File, ucast_fd);
		return WID_DBUS_ERROR;
	}
	if(state == 0)
	{
		if (write(ucast_fd, "0", 1) != 1)
		{
			wid_syslog_err("%s: write \"0\" to file:%s error\n", __func__, Wlan_Ebr_File);
			close(ucast_fd);
			return WID_DBUS_ERROR;
		}
		else
		{
			if (Wlan_Ebr_flag == 1)			/* ebr config mode */
			{
				WID_EBR[Wlan_Ebr_ID]->bridge_ucast_solicit_stat = 0;
			}
			else if (Wlan_Ebr_flag == 0)	/* wlan config mode */
			{
				AC_WLAN[Wlan_Ebr_ID]->bridge_ucast_solicit_stat = 0;
			}
		}
	}
	else if(state == 1)
	{
		
		if (write(ucast_fd, "5", 1) != 1)
		{
			wid_syslog_err("%s: write \"5\" to file:%s error\n", __func__, Wlan_Ebr_File);
			close(ucast_fd);
			return WID_DBUS_ERROR;
		}
		else
		{
			if (Wlan_Ebr_flag == 1)			/* ebr config mode */
			{
				WID_EBR[Wlan_Ebr_ID]->bridge_ucast_solicit_stat = 1;
			}
			else if (Wlan_Ebr_flag == 0)	/* wlan config mode */
			{
				AC_WLAN[Wlan_Ebr_ID]->bridge_ucast_solicit_stat = 1;
			}
		}
	}
	else
	{
		wid_syslog_err("%s: <error> state:%d\n", __func__, state);
	}
	
	close(ucast_fd);
	
	return WID_DBUS_SUCCESS;
}


/*
  *****************************************************************************
  *  
  * NOTES:	 
  * INPUT:	   
  * OUTPUT:	  
  * return:	  
  *  
  * author: 		Huang Leilei 
  * begin time:	2012-11-12 9:00  
  * finish time:		2012-11-15 11:00 
  * history:	
  * 
  **************************************************************************** 
  */
int 
wid_set_wlan_ebr_br_mcast_solicit(unsigned int Wlan_Ebr_ID, unsigned char state, unsigned char Wlan_Ebr_flag)
{
	int ret = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char Wlan_Ebr_File[PATH_MAX + NAME_MAX];
	memset(Wlan_Ebr_File, 0, sizeof(Wlan_Ebr_File));
	
	char Wlan_Ebr_Name[ETH_IF_NAME_LEN];
	memset(Wlan_Ebr_Name,0,ETH_IF_NAME_LEN);

	int mcast_fd = 0;

	if (Wlan_Ebr_flag == 0)
	{
		if (local)
		{
			sprintf(Wlan_Ebr_Name, "wlanl%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}
		else
		{
			sprintf(Wlan_Ebr_Name, "wlan%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}

		//check br if validity
		ret = Check_Interface_Exist(Wlan_Ebr_Name, &quitreason);
		if(ret != 0)
		{
			wid_syslog_err("%s: interface %s not exist\n", __func__, Wlan_Ebr_Name);
			return WLAN_CREATE_BR_FAIL;
		}

		sprintf(Wlan_Ebr_File, "/proc/sys/net/ipv4/neigh/%s/mcast_solicit", Wlan_Ebr_Name);
		
		wid_syslog_debug_debug(WID_DBUS, "wid_set_wlan_ebr_br_mcast_solicit ifname:%s\n",Wlan_Ebr_File);

	}
	else if (Wlan_Ebr_flag == 1)
	{
		if (local)
		{
			sprintf(Wlan_Ebr_Name, "ebrl%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}
		else
		{
			sprintf(Wlan_Ebr_Name, "ebr%d-%d-%d", slotid, vrrid, Wlan_Ebr_ID);
		}

		//check br if validity
		ret = Check_Interface_Exist(Wlan_Ebr_Name,&quitreason);
		if(ret != 0)
		{
			wid_syslog_err("%s: interface %s not exist\n", __func__, Wlan_Ebr_Name);
			return WLAN_CREATE_BR_FAIL;
		}

		sprintf(Wlan_Ebr_File, "/proc/sys/net/ipv4/neigh/%s/mcast_solicit", Wlan_Ebr_Name);

		wid_syslog_debug_debug(WID_DBUS, "wid_set_wlan_ebr_br_mcast_solicit ifname:%s\n",Wlan_Ebr_File);

	}
	else
	{
		wid_syslog_err("%s: <error> Wlan_Ebr_flag:%d\n", __func__, Wlan_Ebr_flag);
	}
	
	//未使用文件读写锁
	mcast_fd = open(Wlan_Ebr_File, O_RDWR);
	if(mcast_fd< 0)
	{
		wid_syslog_err("%s: open file %s error, return vlaue is:%d\n", __func__, Wlan_Ebr_File, mcast_fd);
		return WID_DBUS_ERROR;
	}
	if(state == 0)
	{
		if (write(mcast_fd, "0", 1) != 1)
		{
			wid_syslog_err("%s: write \"0\" to file:%s error\n", __func__, Wlan_Ebr_File);
			close(mcast_fd);
			return WID_DBUS_ERROR;
		}
		else
		{
			if (Wlan_Ebr_flag == 1)			/* ebr config mode */
			{
				WID_EBR[Wlan_Ebr_ID]->bridge_mcast_solicit_stat = 0;
			}
			else if (Wlan_Ebr_flag == 0)	/* wlan config mode */
			{
				AC_WLAN[Wlan_Ebr_ID]->bridge_mcast_solicit_stat = 0;
			}
		}
	}
	else if(state == 1)
	{
		if (write(mcast_fd, "1", 1) != 1)
		{
			wid_syslog_err("%s: write \"1\" to file:%s error\n", __func__, Wlan_Ebr_File);
			close(mcast_fd);
			return WID_DBUS_ERROR;
		}
		else
		{
			if (Wlan_Ebr_flag == 1)			/* ebr config mode */
			{
				WID_EBR[Wlan_Ebr_ID]->bridge_mcast_solicit_stat = 1;
			}
			else if (Wlan_Ebr_flag == 0)	/* wlan config mode */
			{
				AC_WLAN[Wlan_Ebr_ID]->bridge_mcast_solicit_stat = 1;
			}
		}
	}
	else
	{
		wid_syslog_err("%s: <error> state:%d\n", __func__, state);
	}
		
	close(mcast_fd);
						
	return WID_DBUS_SUCCESS;
}



































int wid_set_wlan_br_sameportswitch(unsigned char wlanid,unsigned char state)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brcmd[WID_SYSTEM_CMD_LENTH];
	memset(brcmd,0,WID_SYSTEM_CMD_LENTH);
	
	
	char brname[ETH_IF_NAME_LEN];
	memset(brname,0,ETH_IF_NAME_LEN);
	if(local)
		sprintf(brname,"wlanl%d-%d-%d",slotid,vrrid,wlanid);
	else
		sprintf(brname,"wlan%d-%d-%d",slotid,vrrid,wlanid);		
//	printf("brname:wlan%d\n",wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_wlan_br_sameportswitch ifname:%s\n",brname);

	//check br if validity
	ret = Check_Interface_Config(brname,&quitreason);
	if(ret != 0)
	{
		return WLAN_CREATE_BR_FAIL;
	}

	//set br isolation
	if(state == 0)
	{
		sprintf(brcmd,"brctl sameportswitching %s off\n",brname);
	}
	else if(state == 1)
	{
		sprintf(brcmd,"brctl sameportswitching %s on\n",brname);
	}
	wid_syslog_debug_debug(WID_DEFAULT,"set bridge sameportswitch cmd:%s\n",brcmd);
	
	ret = system(brcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"ret:%d(0 success)\n",ret);

	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;//set failure
	}
	else
	{
		AC_WLAN[wlanid]->sameportswitch = state;
		return 0;//set success
	}
	
}

//ethereal bridge area
int Check_Interface_Exist(char * ifname,WTPQUITREASON *quitreason)
{
	wid_syslog_debug_debug(WID_DEFAULT,"Check_Interface_Config:%s\n",ifname);
	int sockfd;
	struct ifreq	ifr;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));	
	
	if(ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
//		printf("SIOCGIFINDEX error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"SIOCGIFINDEX error\n");
		*quitreason = IF_NOINDEX;
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is IF_NOINDEX error");
		close(sockfd);
		return APPLY_IF_FAIL;
	 }
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1){
	//	printf("SIOCGIFFLAGS error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"SIOCGIFFLAGS error\n");
		*quitreason = IF_NOFLAGS;
		wid_syslog_debug_debug(WID_DEFAULT,"wtp quit reason is IF_NOFLAGS error");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	
	close(sockfd);
	wid_syslog_debug_debug(WID_DEFAULT,"Check_Interface_Exist ifname:%s quitreason:%d\n",ifname,*quitreason);
	return 0;
	
}

int WID_ADD_ETHEREAL_BRIDGE(char *name,unsigned int ID)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	
	//check br 
	if(local)
		sprintf(brname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(brname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	//printf("brname:%s\n",brname);
	
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret == 0)
	{
#if 0
		memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(systemcmd,"ifconfig ebrl%d-%d-%d down",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"ifconfig ebr%d-%d-%d down",slotid,vrrid,ID); 	
		ret = system(systemcmd);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
			return SYSTEM_CMD_ERROR;
		}
		if(ret != 0)
		{
			return SYSTEM_CMD_ERROR;
		}
		
		memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(systemcmd,"brctl delbr ebrl%d-%d-%d",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl delbr ebr%d-%d-%d",slotid,vrrid,ID);
		//printf("systemcmd:%s\n",systemcmd);
		ret = system(systemcmd);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd %s error,error code %d\n",systemcmd,reason);
			return SYSTEM_CMD_ERROR;
		}
		if(ret != 0)
		{
			return SYSTEM_CMD_ERROR;
		}
#endif
	}
	else{
		memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);	
		if(local)
			sprintf(systemcmd,"brctl addbr ebrl%d-%d-%d",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl addbr ebr%d-%d-%d",slotid,vrrid,ID);		
		//printf("systemcmd:%s\n",systemcmd);
		ret = system(systemcmd);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd %s error,error code %d\n",systemcmd,reason);
			return SYSTEM_CMD_ERROR;
		}
		if(ret != 0)
		{
			return SYSTEM_CMD_ERROR;
		}
		/*set isolation default value*/
		memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(systemcmd,"brctl isolation ebrl%d-%d-%d on",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl isolation ebr%d-%d-%d on",slotid,vrrid,ID);		
		ret = system(systemcmd);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd %s error,error code %d\n",systemcmd,reason);
			return SYSTEM_CMD_ERROR;
		}

		memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
		if(local)
			sprintf(systemcmd,"brctl multicastisolation ebrl%d-%d-%d on",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl multicastisolation ebr%d-%d-%d on",slotid,vrrid,ID);		
		ret = system(systemcmd);
		reason = WEXITSTATUS(ret);
		if(reason != 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"system cmd %s error,error code %d\n",systemcmd,reason);
			return SYSTEM_CMD_ERROR;
		}
	}
	WID_EBR[ID] = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE));
	WID_EBR[ID]->EBRID = ID;
	WID_EBR[ID]->name = (char *)malloc(strlen(name)+1);
	memset(WID_EBR[ID]->name, 0, strlen(name)+1);
	memcpy(WID_EBR[ID]->name, name, strlen(name));
	WID_EBR[ID]->state = 0;
	WID_EBR[ID]->isolation_policy = 1;/*sz change default value from 0 to 1 090723*/
	WID_EBR[ID]->multicast_isolation_policy = 1;/*sz change default value from 0 to 1 090723*/
	WID_EBR[ID]->bridge_mcast_solicit_stat = 1;  /* the default config is 1, set by sysctl.conf. zhangdi@autelan.com 2013-05-31 */
	WID_EBR[ID]->bridge_ucast_solicit_stat = 1;
	WID_EBR[ID]->sameportswitch = 0;
	WID_EBR[ID]->iflist = NULL;
	WID_EBR[ID]->uplinklist = NULL;
	WID_EBR[ID]->r_num = 0;
	WID_EBR[ID]->eth_num = 0;
	WID_EBR[ID]->multicast_fdb_learn = 1;

	wid_set_ebr_isolation(ID,1);
	wid_set_ebr_multcast_isolation(ID,1);
	wid_set_ebr_multicast_fdb_learn(ID,1);
	
	return WID_DBUS_SUCCESS;
}
int WID_DELETE_ETHEREAL_BRIDGE(unsigned int ID)
{
	int ret = 0;
	int reason = 0;
	//WTPQUITREASON *quitreason = WTP_INIT;
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(systemcmd,"ifconfig ebrl%d-%d-%d down",slotid,vrrid,ID);
	else
		sprintf(systemcmd,"ifconfig ebr%d-%d-%d down",slotid,vrrid,ID);		
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(systemcmd,"brctl delbr ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(systemcmd,"brctl delbr ebr%d-%d-%d",slotid,vrrid,ID);
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	// delete iflist
	EBR_IF_LIST *iflist = WID_EBR[ID]->iflist;
	while(iflist != NULL)
	{
		WID_EBR[ID]->iflist = iflist->ifnext;
		free(iflist);
		iflist = NULL;
		iflist = WID_EBR[ID]->iflist;
	}
	
	iflist = WID_EBR[ID]->uplinklist;
	while(iflist != NULL)
	{
		WID_EBR[ID]->uplinklist = iflist->ifnext;
		free(iflist);
		iflist = NULL;
		iflist = WID_EBR[ID]->uplinklist;
	}	
	
	CW_FREE_OBJECT(WID_EBR[ID]->name);
	CW_FREE_OBJECT(WID_EBR[ID]);
	
	return WID_DBUS_SUCCESS;
}
int WID_SET_ETHEREAL_BRIDGE_ENABLE(unsigned int ID)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	
	//check br 
	if(local)
		sprintf(brname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(brname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	//printf("brname:%s\n",brname);
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WID_EBR_ERROR;
	}
	if(local)
		sprintf(systemcmd,"ifconfig ebrl%d-%d-%d up",slotid,vrrid,ID);
	else
		sprintf(systemcmd,"ifconfig ebr%d-%d-%d up",slotid,vrrid,ID);
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	WID_EBR[ID]->state = 1;
	
	return WID_DBUS_SUCCESS;
}
int WID_SET_ETHEREAL_BRIDGE_DISABLE(unsigned int ID)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	
	//check br 
	if(local)
		sprintf(brname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(brname,"ebr%d-%d-%d",slotid,vrrid,ID);
	//printf("brname:%s\n",brname);
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WID_EBR_ERROR;
	}
	if(local)
		sprintf(systemcmd,"ifconfig ebrl%d-%d-%d down",slotid,vrrid,ID);
	else
		sprintf(systemcmd,"ifconfig ebr%d-%d-%d down",slotid,vrrid,ID);
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	WID_EBR[ID]->state = 0;
	return WID_DBUS_SUCCESS;
}
int wid_set_ebr_isolation(unsigned int ID,unsigned char state)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	
	//check br 
	if(local)
		sprintf(brname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(brname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	//printf("brname:%s\n",brname);
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WID_EBR_ERROR;
	}
	if(state == 1)
	{
		if(local)
			sprintf(systemcmd,"brctl isolation ebrl%d-%d-%d on\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl isolation ebr%d-%d-%d on\n",slotid,vrrid,ID);			
	}
	else 
	{
		if(local)
			sprintf(systemcmd,"brctl isolation ebrl%d-%d-%d off\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl isolation ebr%d-%d-%d off\n",slotid,vrrid,ID);			
	}
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	WID_EBR[ID]->isolation_policy = state;
	return WID_DBUS_SUCCESS;
}
int wid_set_ebr_multcast_isolation(unsigned int ID,unsigned char state)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	
	//check br 
	if(local)
		sprintf(brname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(brname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	//printf("brname:%s\n",brname);
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WID_EBR_ERROR;
	}
	if(state == 1)
	{
		if(local)
			sprintf(systemcmd,"brctl multicastisolation ebrl%d-%d-%d on\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl multicastisolation ebr%d-%d-%d on\n",slotid,vrrid,ID);
	}
	else 
	{
		if(local)
			sprintf(systemcmd,"brctl multicastisolation ebrl%d-%d-%d off\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl multicastisolation ebr%d-%d-%d off\n",slotid,vrrid,ID);
	}
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	WID_EBR[ID]->multicast_isolation_policy = state;
	return WID_DBUS_SUCCESS;
}
int wid_set_ebr_sameportswitch(unsigned int ID,unsigned char state)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	
	//check br 
	if(local)
		sprintf(brname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(brname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	//printf("brname:%s\n",brname);
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WID_EBR_ERROR;
	}
	if(state == 1)
	{
		if(local)
			sprintf(systemcmd,"brctl sameportswitching ebrl%d-%d-%d on\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl sameportswitching ebr%d-%d-%d on\n",slotid,vrrid,ID);
	}
	else 
	{
		if(local)
			sprintf(systemcmd,"brctl sameportswitching ebrl%d-%d-%d off\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"brctl sameportswitching ebr%d-%d-%d off\n",slotid,vrrid,ID);
	}
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	WID_EBR[ID]->sameportswitch = state;
	return WID_DBUS_SUCCESS;
}

int wid_set_ebr_multicast_fdb_learn(unsigned int ID,unsigned char state)
{
	int ret = 0;
	int reason = 0;
	WTPQUITREASON quitreason = WTP_INIT;
	char brname[ETH_IF_NAME_LEN];
	char systemcmd[WID_SYSTEM_CMD_LENTH];
	memset(brname,0,ETH_IF_NAME_LEN);
	memset(systemcmd,0,WID_SYSTEM_CMD_LENTH);
	
	//check br 
	if(local)
		sprintf(brname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(brname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	//printf("brname:%s\n",brname);
	ret = Check_Interface_Exist(brname,&quitreason);
	if(ret != 0)
	{
		return WID_EBR_ERROR;
	}
	if(state == 1)
	{
		if(local)
			sprintf(systemcmd,"echo 1 > /sys/class/net/ebrl%d-%d-%d/bridge/fdb_multicast_update_state\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"echo 1 > /sys/class/net/ebr%d-%d-%d/bridge/fdb_multicast_update_state\n",slotid,vrrid,ID);
	}
	else 
	{
		if(local)
			sprintf(systemcmd,"echo 0 > /sys/class/net/ebrl%d-%d-%d/bridge/fdb_multicast_update_state\n",slotid,vrrid,ID);
		else
			sprintf(systemcmd,"echo 0 > /sys/class/net/ebr%d-%d-%d/bridge/fdb_multicast_update_state\n",slotid,vrrid,ID);
	}
	//printf("systemcmd:%s\n",systemcmd);
	ret = system(systemcmd);
	reason = WEXITSTATUS(ret);
	if(reason != 0)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"system cmd error,error code %d\n",reason);
		return SYSTEM_CMD_ERROR;
	}
	if(ret != 0)
	{
		return SYSTEM_CMD_ERROR;
	}
	WID_EBR[ID]->multicast_fdb_learn = state;
	return WID_DBUS_SUCCESS;
}

int WID_SET_ETHEREAL_BRIDGE_IF_UPLINK(unsigned int ID,char *ifname,int is_radio,int g_radioid,int wlanid)
{
	//printf("input ebrid %d ifname %s\n",ID,ifname);

	int ret = 0;
	int reason = 0;
	char ebrname[ETH_IF_NAME_LEN];
	char syscmd[WID_SYSTEM_CMD_LENTH];
	char ifcheck[WID_SYSTEM_CMD_LENTH];
	WTPQUITREASON quitreason = WTP_INIT;
	int need_set_mtu = 0;	
	memset(ebrname,0,ETH_IF_NAME_LEN);
	memset(syscmd,0,WID_SYSTEM_CMD_LENTH);
	memset(ifcheck,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(ebrname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(ebrname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	ret = Check_Interface_Exist(ebrname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return WID_EBR_ERROR;
	}
	if(local)
		sprintf(ifcheck,"/sys/class/net/ebrl%d-%d-%d/brif/%s/port_id",slotid,vrrid,ID,ifname);
	else
		sprintf(ifcheck,"/sys/class/net/ebr%d-%d-%d/brif/%s/port_id",slotid,vrrid,ID,ifname);		
	ret = file_check(ifcheck);
	if(ret == 1){
		ret = 0;
		reason = 0;
	}else{
		sprintf(syscmd,"brctl addif %s %s",ebrname,ifname);
		//printf("syscmd %s\n",syscmd);

		ret = system(syscmd);
		reason = WEXITSTATUS(ret);
	}
	//if(ret == WID_DBUS_SUCCESS)
	if(reason == WID_DBUS_SUCCESS)
	{	
		if(is_radio){
			if((g_radioid>0)&&(g_radioid <= WTP_NUM*L_RADIO_NUM)&&(wlanid > 0)&&(wlanid < WLAN_NUM)){
				if((AC_RADIO[g_radioid] != NULL)&&(wlanid < WLAN_NUM+1)){
					memcpy(AC_RADIO[g_radioid]->br_ifname[wlanid],ebrname,strlen(ebrname));//for advise to asd
				}
				wid_syslog_debug_debug(WID_DEFAULT,"AC_RADIO[%d]->br_ifname[%d] %s.\n",g_radioid,wlanid,AC_RADIO[g_radioid]->br_ifname[wlanid]);
				//printf("AC_RADIO[%d]->br_ifname[%d] %s.\n",g_radioid,wlanid,AC_RADIO[g_radioid]->br_ifname[wlanid]);
			}
		}
		EBR_IF_LIST *wif;
		EBR_IF_LIST *wifnext;
		
		wif = (EBR_IF_LIST *)malloc(sizeof(EBR_IF_LIST));
		memset(wif,0,sizeof(EBR_IF_LIST));
		wif->ifname = (char *)malloc(ETH_IF_NAME_LEN);
		memset(wif->ifname,0,ETH_IF_NAME_LEN);
		memcpy(wif->ifname,ifname,strlen(ifname));
		wif->ifnext = NULL;
		
		if(WID_EBR[ID]->iflist == NULL)
		{
			//printf("ebr id:%d ifname :%s\n",ID,ifname);
			WID_EBR[ID]->iflist = wif ;
			WID_EBR[ID]->iflist->ifnext = NULL;			
		}
		else
		{
			
			wifnext = WID_EBR[ID]->iflist;
			while(wifnext != NULL)
			{	
				if((strlen(ifname) == strlen(wifnext->ifname))&&(strncmp(wifnext->ifname,ifname,strlen(ifname)) == 0))
				{
					//printf("already in the list\n");
					free(wif->ifname);
					free(wif);
					wif = NULL;
					return 0;
				}
				wifnext = wifnext->ifnext;
			}
			if(is_radio&&(WID_EBR[ID]->r_num == 0)&&(WID_EBR[ID]->eth_num > 0)){
				need_set_mtu = 1;
			}else if((!is_radio)&&(WID_EBR[ID]->r_num > 0)){
				memset(syscmd,0,WID_SYSTEM_CMD_LENTH);				
				sprintf(syscmd,"ifconfig %s mtu 1416",ifname);
				printf("%s %s\n",__func__,syscmd);
				system(syscmd);
			}
			
			//printf("ebr id:%d ifname :%s\n",ID,ifname);
			wifnext = WID_EBR[ID]->iflist;
			while(wifnext->ifnext != NULL)
			{	
				if(need_set_mtu){
//					if(strncasecmp(wifnext->ifname,"radio",5)){												
					if(strncasecmp(wifnext->ifname,"r",1)){ 											
						memset(syscmd,0,WID_SYSTEM_CMD_LENTH);				
						sprintf(syscmd,"ifconfig %s mtu 1416",wifnext->ifname);
						printf("%s2 %s\n",__func__,syscmd);
						system(syscmd);
					}
				}
				wifnext = wifnext->ifnext;
			}
			if(need_set_mtu){
				//if(strncasecmp(wifnext->ifname,"radio",5)){ 	
				if(strncasecmp(wifnext->ifname,"r",1)){ 	
					memset(syscmd,0,WID_SYSTEM_CMD_LENTH);				
					sprintf(syscmd,"ifconfig %s mtu 1416",wifnext->ifname);
					printf("%s2 %s\n",__func__,syscmd);
					system(syscmd);
				}
			}
			
			wifnext->ifnext = wif;
		}		
		if(is_radio){
			WID_EBR[ID]->r_num++;
		}else{
			WID_EBR[ID]->eth_num++;
		}
		return 0;
	}
	else
	{
		return SYSTEM_CMD_ERROR;
	}
	
}
int WID_SET_ETHEREAL_BRIDGE_IF_DOWNLINK(unsigned int ID,char *ifname,int is_radio,int g_radioid,int wlanid)
{
	//printf("input ebrid %d ifname %s\n",ID,ifname);

	int ret = 0;
	int reason = 0;
	char ebrname[ETH_IF_NAME_LEN];
	char syscmd[WID_SYSTEM_CMD_LENTH];
	WTPQUITREASON quitreason = WTP_INIT;
	int need_set_mtu = 0;	
	memset(ebrname,0,ETH_IF_NAME_LEN);
	memset(syscmd,0,WID_SYSTEM_CMD_LENTH);
	//delete uplink first if exists
	wid_set_ethereal_bridge_del_uplink(ID,ifname);
	
	if(local)
		sprintf(ebrname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(ebrname,"ebr%d-%d-%d",slotid,vrrid,ID);
	ret = Check_Interface_Exist(ebrname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return ret;
	}

	sprintf(syscmd,"brctl delif %s %s",ebrname,ifname);
	//printf("syscmd %s\n",syscmd);

	ret = system(syscmd);
	reason = WEXITSTATUS(ret);
	//if(ret == WID_DBUS_SUCCESS)
	if(reason == WID_DBUS_SUCCESS)
	{
		if(is_radio){
			if((g_radioid>0)&&(g_radioid < WTP_NUM*L_RADIO_NUM)&&(wlanid > 0)&&(wlanid < WLAN_NUM)){
				if(AC_RADIO[g_radioid] != NULL){
					memset(AC_RADIO[g_radioid]->br_ifname[wlanid],0,IF_NAME_MAX);//for advise to asd
				}
				wid_syslog_debug_debug(WID_DEFAULT,"del AC_RADIO[%d]->br_ifname[%d] %s\n",g_radioid,wlanid,AC_RADIO[g_radioid]->br_ifname[wlanid]);
			}
		}
		EBR_IF_LIST *wif;
		EBR_IF_LIST *wifnext;

		wifnext = WID_EBR[ID]->iflist;
		
		if(WID_EBR[ID]->iflist != NULL)
		{

			if(is_radio&&(WID_EBR[ID]->r_num == 1)&&(WID_EBR[ID]->eth_num > 0)){
				need_set_mtu = 1;
			}else if((!is_radio)&&(WID_EBR[ID]->r_num > 0)){
				memset(syscmd,0,WID_SYSTEM_CMD_LENTH);				
				sprintf(syscmd,"ifconfig %s mtu 1500",ifname);				
				printf("%s %s\n",__func__,syscmd);
				system(syscmd);
			}

			if((strlen(ifname) == strlen(wifnext->ifname))&&(strncmp(wifnext->ifname,ifname,strlen(ifname)) == 0))
			{
				WID_EBR[ID]->iflist = wifnext->ifnext;
				free(wifnext->ifname);
				free(wifnext);
				wifnext = NULL;		
				//printf("delete ifname %s from list\n",ifname);				
				wifnext = WID_EBR[ID]->iflist;
				if(need_set_mtu)
					while(wifnext->ifnext != NULL)
					{	
						//if(strncasecmp(wifnext->ifname,"radio",5)){ 											
						if(strncasecmp(wifnext->ifname,"r",1)){ 
							memset(syscmd,0,WID_SYSTEM_CMD_LENTH);				
							sprintf(syscmd,"ifconfig %s mtu 1500",wifnext->ifname);							
							printf("%s2 %s\n",__func__,syscmd);
							system(syscmd);
						}
						wifnext = wifnext->ifnext;
					}
				
				if(is_radio){
					WID_EBR[ID]->r_num--;
				}else{
					WID_EBR[ID]->eth_num--;
				}
			}
			else
			{
								
				while(wifnext->ifnext != NULL)
				{	
					if((strlen(ifname) == strlen(wifnext->ifnext->ifname))&&(strncmp(wifnext->ifnext->ifname,ifname,strlen(ifname)) == 0))
					{
						wif = wifnext->ifnext;
						wifnext->ifnext = wifnext->ifnext->ifnext;
						free(wif->ifname);
						free(wif);
						wif = NULL;				
						//printf("delete ifname %s from list\n",ifname);
						if(is_radio){
							WID_EBR[ID]->r_num--;
						}else{
							WID_EBR[ID]->eth_num--;
						}
						if(!need_set_mtu){							
							return 0;
						}else
							continue;
					}					
					if(need_set_mtu){
						//if(strncasecmp(wifnext->ifname,"radio",5)){ 	
						if(strncasecmp(wifnext->ifname,"r",1)){ 	
							memset(syscmd,0,WID_SYSTEM_CMD_LENTH);				
							sprintf(syscmd,"ifconfig %s mtu 1500",wifnext->ifname);							
							printf("%s3 %s\n",__func__,syscmd);
							system(syscmd);
						}
					}
					wifnext = wifnext->ifnext;
				}				
				if(need_set_mtu){
					//if(strncasecmp(wifnext->ifname,"radio",5)){ 
					if(strncasecmp(wifnext->ifname,"r",1)){
						memset(syscmd,0,WID_SYSTEM_CMD_LENTH);				
						sprintf(syscmd,"ifconfig %s mtu 1500",wifnext->ifname);						
						printf("%s4 %s\n",__func__,syscmd);
						system(syscmd);
					}
				}
			}
		}
		
		return 0;
	}
	else
	{
		return SYSTEM_CMD_ERROR;
	}
}
int wid_add_cpu_value_for_accounting_average(unsigned int wtpid,unsigned int cpu_value){
	struct ap_cpu_info  *cpu_node = NULL;
	if((cpu_node = (struct ap_cpu_info*)malloc(sizeof(struct ap_cpu_info))) == NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"malloc fail in wid_add_cpu_value_for_accounting_average. \n");
		return -1;
	}
	else{
		cpu_node->value = cpu_value;
		cpu_node->next = NULL;
		if(AC_WTP[wtpid]->apcminfo.ap_cpu_info_head == NULL){
			AC_WTP[wtpid]->apcminfo.ap_cpu_info_head = cpu_node;
			AC_WTP[wtpid]->apcminfo.ap_cpu_info_length++;
		}
		else{
			cpu_node->next = AC_WTP[wtpid]->apcminfo.ap_cpu_info_head;
			AC_WTP[wtpid]->apcminfo.ap_cpu_info_head = cpu_node;
			AC_WTP[wtpid]->apcminfo.ap_cpu_info_length++;
		}
 		return 0;
	}
}

int wid_add_mem_value_for_accounting_average(unsigned int wtpid,unsigned int mem_value){
	wid_syslog_debug_debug(WID_DEFAULT,"mem value : %d \n", AC_WTP[wtpid]->wifi_extension_info.memoryuse );
	struct ap_cpu_info  *mem_node = NULL;
	if((mem_node = (struct ap_cpu_info*)malloc(sizeof(struct ap_cpu_info))) == NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"malloc fail in %s. \n",__func__);
		return -1;
	}
	else{
		mem_node->value = mem_value;
		mem_node->next = NULL;
		if(AC_WTP[wtpid]->apcminfo.ap_mem_info_head == NULL){
			AC_WTP[wtpid]->apcminfo.ap_mem_info_head = mem_node;
			AC_WTP[wtpid]->apcminfo.ap_mem_info_length++;
		}
		else{
			mem_node->next = AC_WTP[wtpid]->apcminfo.ap_mem_info_head;
			AC_WTP[wtpid]->apcminfo.ap_mem_info_head = mem_node;
			AC_WTP[wtpid]->apcminfo.ap_mem_info_length++;
		}
 		return 0;
	}
}



int wid_del_cpu_value_for_accounting_average(unsigned int wtpid,unsigned int total_node_num){
	struct ap_cpu_info  *p = AC_WTP[wtpid]->apcminfo.ap_cpu_info_head ;
	struct ap_cpu_info  *tmp = NULL;
	int i=0;
	
	for(i=0;i<total_node_num;i++){
		if (p!= NULL){
			tmp = p;
			p = p->next;
		}
	}
	tmp->next = NULL;
	
	while(p!=NULL){
		tmp = p;
		p = p->next;
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
		AC_WTP[wtpid]->apcminfo.ap_cpu_info_length--;
 	}
	
	wid_syslog_debug_debug(WID_DEFAULT,"AC_WTP[%d]->ap_cpu_info_length = %d\n",\
							wtpid,AC_WTP[wtpid]->apcminfo.ap_cpu_info_length);
	return 0;
}
int wid_del_mem_value_for_accounting_average(unsigned int wtpid,unsigned int total_node_num){
	struct ap_cpu_info  *p = AC_WTP[wtpid]->apcminfo.ap_mem_info_head ;
	struct ap_cpu_info  *tmp = NULL;
	int i=0;
	
	for(i=0;i<total_node_num;i++){
		if (p!= NULL){
			tmp = p;
			p = p->next;
		}
	}
	tmp->next = NULL;
	
	while(p!=NULL){
		tmp = p;
		p = p->next;
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
		AC_WTP[wtpid]->apcminfo.ap_mem_info_length--;
 	}
	
	wid_syslog_debug_debug(WID_DEFAULT,"AC_WTP[%d]->ap_mem_info_length = %d\n",\
							wtpid,AC_WTP[wtpid]->apcminfo.ap_mem_info_length);
	
	return 0;
}

int wid_accouting_cpu_average(unsigned int wtpid){
	unsigned int sum_cpu_value = 0;
	struct ap_cpu_info  *Node_cpu = AC_WTP[wtpid]->apcminfo.ap_cpu_info_head;
	AC_WTP[wtpid]->apcminfo.cpu_peak_value = Node_cpu->value;
	
	while(Node_cpu != NULL){
		if(AC_WTP[wtpid]->apcminfo.cpu_peak_value < Node_cpu->value) {
			AC_WTP[wtpid]->apcminfo.cpu_peak_value = Node_cpu->value;
		}
		sum_cpu_value += Node_cpu->value;
		Node_cpu = Node_cpu->next;
	}
	AC_WTP[wtpid]->wifi_extension_info.cpu_collect_average = 
						sum_cpu_value / AC_WTP[wtpid]->apcminfo.ap_cpu_info_length ;
	
	AC_WTP[wtpid]->apcminfo.cpu_average = AC_WTP[wtpid]->wifi_extension_info.cpu_collect_average;

	return 0 ;
}

int wid_accouting_mem_average(unsigned int wtpid){
	unsigned int sum_mem_value = 0;
	struct ap_cpu_info  *Node_mem = AC_WTP[wtpid]->apcminfo.ap_mem_info_head;
	AC_WTP[wtpid]->apcminfo.mem_peak_value = Node_mem->value;
	
	while(Node_mem != NULL){
		if(AC_WTP[wtpid]->apcminfo.mem_peak_value < Node_mem->value) {
			AC_WTP[wtpid]->apcminfo.mem_peak_value = Node_mem->value;
		}
		sum_mem_value += Node_mem->value;
		Node_mem = Node_mem->next;
	}
	AC_WTP[wtpid]->wifi_extension_info.mem_collect_average = 
						sum_mem_value / AC_WTP[wtpid]->apcminfo.ap_mem_info_length ;
	
	AC_WTP[wtpid]->apcminfo.mem_average = AC_WTP[wtpid]->wifi_extension_info.mem_collect_average;

	return 0 ;
}

/*for sample cpu value  by nl  2010-08-26*/
int wid_accouting_cpu_sample_average_and_peak(unsigned int wtpid){
	unsigned int i_sample = 0;/*采样结点的搜索变量*/
	unsigned int j = 0, k = 0;/*j 上次结点应该在的位置，k 为本次结点应该在的位置*/
	unsigned int i_node = 0;/*为本次结点计数的递增变量*/
	unsigned int D_value = 0;/*本次与上次采样，结点所需要移动的步长*/
	unsigned int sum_cpu_value = 0;/*cpu 采样值的总和*/
	unsigned int sample_times = 0;/*抽样次数*/
	unsigned int total_sample_time = 0;/*抽样总时长*/
	int ret = 0;

	ret = WID_CHECK_ID(WID_WTP_CHECK,wtpid);

	if(ret != 0){
		wid_syslog_info("wtpid error = %d at %s\n",\
							wtpid, __func__);
		return -1 ;
	}
	
	if(sample_infor_interval <= 0){
		wid_syslog_info("sample_infor_interval = %d at %s\n",\
							sample_infor_interval,__func__);
		return -1 ;
	}
	
	total_sample_time = (AC_WTP[wtpid]->apcminfo.ap_cpu_info_length) 
						* (unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
	if(total_sample_time <= 0){
		wid_syslog_info("total_sample_time = %d at %s\n",\
									total_sample_time,__func__);
		return -1 ;
	}
	
	sample_times = total_sample_time/sample_infor_interval;
	
	if(sample_times <= 0){
		wid_syslog_info("sample_times = %d at %s\n",\
							sample_infor_interval,__func__);
		return -1 ;
	}
	
	struct ap_cpu_info  *Node_cpu = AC_WTP[wtpid]->apcminfo.ap_cpu_info_head;
	AC_WTP[wtpid]->apcminfo.cpu_peak_value = Node_cpu->value;

	for(i_sample=0;i_sample<sample_times;i_sample++){
		if(0 == i_sample ){
			sum_cpu_value = Node_cpu->value;
		}
		else{
			k = (((i_sample * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
				/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
			j = ((((i_sample-1) * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
				/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
			D_value = k - j;
			for(i_node = 0;i_node < D_value;i_node++){
				if(Node_cpu!= NULL&&Node_cpu->next != NULL){
					Node_cpu = Node_cpu->next;
				}
			}
			sum_cpu_value += Node_cpu->value;
			if(AC_WTP[wtpid]->apcminfo.cpu_peak_value < Node_cpu->value){
				AC_WTP[wtpid]->apcminfo.cpu_peak_value = Node_cpu->value;
			}
		}
	}

	AC_WTP[wtpid]->wifi_extension_info.cpu_collect_average = 
						sum_cpu_value / sample_times ;

	AC_WTP[wtpid]->apcminfo.cpu_average = AC_WTP[wtpid]->wifi_extension_info.cpu_collect_average;


	return 0 ;
}

/*for sample mem value  by nl  2010-08-26*/
int wid_accouting_mem_sample_average_and_peak(unsigned int wtpid){
	unsigned int i_sample = 0;/*采样结点的搜索变量*/
	unsigned int j = 0, k = 0;/*j 上次结点应该在的位置，k 为本次结点应该在的位置*/
	unsigned int i_node = 0;/*为本次结点计数的递增变量*/
	unsigned int D_value = 0;/*本次与上次采样，结点所需要移动的步长*/
	unsigned int sum_mem_value = 0;/*cpu 采样值的总和*/
	unsigned int sample_times = 0;/*抽样次数*/
	unsigned int total_sample_time = 0;/*抽样总时长*/
	int ret = 0 ;
	
	if(sample_infor_interval <= 0){
		wid_syslog_info("sample_infor_interval = %d at %s\n",\
							sample_infor_interval,__func__);
		return -1 ;
	}
	
	ret = WID_CHECK_ID(WID_WTP_CHECK,wtpid);

	if(ret != 0){
		wid_syslog_info("wtpid error = %d at %s\n",\
							wtpid, __func__);
		return -1 ;
	}
	
	total_sample_time = (AC_WTP[wtpid]->apcminfo.ap_mem_info_length) 
						* (unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;

	if(total_sample_time <= 0){
		wid_syslog_info("total_sample_time = %d at %s\n",\
									total_sample_time,__func__);
		return -1 ;
	}
	
	sample_times = total_sample_time/sample_infor_interval;
	
	if(sample_times <= 0){
		wid_syslog_info("sample_times = %d at %s\n",\
							sample_infor_interval,__func__);
		return -1 ;
	}
	
	struct ap_cpu_info  *Node_mem = AC_WTP[wtpid]->apcminfo.ap_mem_info_head;
	AC_WTP[wtpid]->apcminfo.mem_peak_value = Node_mem->value;

	for(i_sample=0;i_sample<sample_times;i_sample++){
		if(0 == i_sample ){
			sum_mem_value = Node_mem->value;
		}
		else{
			k = (((i_sample * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
				/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
			j = ((((i_sample-1) * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
				/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
			D_value = k - j;
			for(i_node = 0;i_node < D_value;i_node++){
				if(Node_mem!= NULL&&Node_mem->next != NULL){
					Node_mem = Node_mem->next;
				}
			}
			sum_mem_value += Node_mem->value;
			if(AC_WTP[wtpid]->apcminfo.cpu_peak_value < Node_mem->value){
				AC_WTP[wtpid]->apcminfo.cpu_peak_value = Node_mem->value;
			}
		}
	}
	
	AC_WTP[wtpid]->wifi_extension_info.mem_collect_average = 
						sum_mem_value / sample_times ;

	AC_WTP[wtpid]->apcminfo.mem_average = AC_WTP[wtpid]->wifi_extension_info.mem_collect_average;
	/*printf("sample_times %d\n",sample_times);
	printf("mem_average %d\n",AC_WTP[wtpid]->apcminfo.mem_average );*/
	
	return 0 ;
}

int accounting_snr_math_average(unsigned int wtpid,int average_num){
	int i = 0;
	int sum = 0;
	double pi = 0;
	double sum1 = 0;
	
	for(i=0;i<average_num;i++){
		sum = sum + AC_WTP[wtpid]->wtp_wifi_snr_stats.snr[i];
		pi = pow(10,(double)(AC_WTP[wtpid]->wtp_wifi_snr_stats.snr[i]/10.00));
		sum1 = sum1 +pi;
		/*printf(" AC_WTP[wtpid]->wtp_wifi_snr_stats.snr[i] %d\n", AC_WTP[wtpid]->wtp_wifi_snr_stats.snr[i]);
		printf("pi %g\n",pi);
		printf("sum1 %g\n",sum1);*/
	}
	AC_WTP[wtpid]->wtp_wifi_snr_stats.snr_average = sum/average_num;
	AC_WTP[wtpid]->wtp_wifi_snr_stats.snr_math_average = 10*log10(sum1/average_num);

	return 0;
}
/*for sample snr value  by nl  2010-09-07*/
struct ap_snr_info  * find_next_sample_node(unsigned int wtpid,unsigned int i_sample,struct ap_snr_info  *Node_snr){
	unsigned int j =0 ;			/*j 上次结点应该在的位置*/
	unsigned int k = 0; 		/*k 为本次结点应该在的位置*/
	unsigned int D_value;		/*本次与上次采样，结点所需要移动的步长*/
	unsigned int i_node = 0;	/*为本次结点计数的递增变量*/
	int ret = WID_DBUS_SUCCESS;
	struct ap_snr_info  *search_node = Node_snr;

	ret = WID_CHECK_ID(WID_WTP_CHECK,wtpid);

	if(ret != WID_DBUS_SUCCESS ){
		wid_syslog_info("wtpid error = %d at %s\n",\
						wtpid, __func__);
		return NULL ;
	}
	
	if(AC_WTP[wtpid]->wifi_extension_reportinterval <= 0){
		wid_syslog_debug_debug(WID_DEFAULT,"reportinterval <= 0 in func %s \n",__func__);
		return NULL;
	}
	
	k = (((i_sample * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
					/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
	
	j = ((((i_sample-1) * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
		/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
	
	D_value = k - j;
	
	for(i_node = 0;i_node < D_value;i_node++){
		if(search_node!= NULL&&search_node->next != NULL){
			search_node = search_node->next;
		}
	}
	
	return search_node;
}
int wid_add_snr_value_for_accounting_average(unsigned int wtpid,unsigned char snr_value,unsigned int l_radioid){ //fengwenchao modify 20120314 for onlinebug-162//qiuchen copy from v1.3
	wid_syslog_debug_debug(WID_DEFAULT,"snr value : %d \n", snr_value);

	struct ap_snr_info  *snr_node = NULL;
	if((snr_node = (struct ap_snr_info*)malloc(sizeof(struct ap_snr_info))) == NULL){
		wid_syslog_debug_debug(WID_DEFAULT,"malloc fail in %s. \n",__func__);
		return -1;
	}
	else{
		snr_node->value = snr_value;
		snr_node->next = NULL;
		if(AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_head == NULL){
			AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_head = snr_node;
			AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_length++;
		}
		else{
			snr_node->next = AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_head;
			AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_head = snr_node;
			AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_length++;
		}
 		return 0;
	}
}

int wid_del_snr_value_for_accounting_average(unsigned int wtpid,unsigned int total_node_num,unsigned int l_radioid){ //fengwenchao add 20120314 for onlinebug-162//qiuchen copy from v1.3
	struct ap_snr_info  *p = AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_head ;

	struct ap_snr_info  *tmp = NULL;
	int i=0;
	
	for(i=0;i<total_node_num;i++){
		if (p!= NULL){
			tmp = p;
			p = p->next;
		}
	}
	tmp->next = NULL;
	
	while(p!=NULL){
		tmp = p;
		p = p->next;
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
		AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_length--;
 	}
	
	wid_syslog_debug_debug(WID_DEFAULT,"AC_WTP[%d]->ap_snr_info_length = %d\n",\
							wtpid,AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_length);
	
	return 0;
}

/*for sample snr value  by nl  2010-09-09*/
struct ap_cpu_info  * find_next_sample_node2(unsigned int wtpid,unsigned int i_sample,struct ap_cpu_info  *Node_snr){
	unsigned int j =0 ;				/*j 上次结点应该在的位置*/
	unsigned int k = 0;				/*k 为本次结点应该在的位置*/
	unsigned int D_value;			/*本次与上次采样，结点所需要移动的步长*/
	unsigned int i_node = 0;		/*为本次结点计数的递增变量*/
	int ret = WID_DBUS_SUCCESS;
	
	struct ap_cpu_info  *search_node = Node_snr;

	ret = WID_CHECK_ID(WID_WTP_CHECK,wtpid);

	if(ret != WID_DBUS_SUCCESS ){
		wid_syslog_info("wtpid error = %d at %s\n",\
						wtpid, __func__);
		return NULL ;
	}

	if(AC_WTP[wtpid]->wifi_extension_reportinterval <= 0){
		wid_syslog_debug_debug(WID_DEFAULT,"reportinterval <= 0 in func %s \n",__func__);
		return NULL;
	}
	
	k = (((i_sample * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
					/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
	
	j = ((((i_sample-1) * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval))
		/(unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
	
	D_value = k - j;
	
	for(i_node = 0;i_node < D_value;i_node++){
		if(search_node!= NULL&&search_node->next != NULL){
			search_node = search_node->next;
		}
	}
	
	return search_node;
}
/*for sample snr value  by nl  2010-09-08*/
int wid_accouting_snr_average(unsigned int wtpid,unsigned int l_radioid){ //fengwenchao modify 20120314 for onlinebug-162//qiuchen copy from v1.3

	unsigned int sum_snr_value = 0;
	int valued_num = 0;				/*accounting for the not zero value number */
	int ret = WID_DBUS_SUCCESS;

	ret = WID_CHECK_ID(WID_WTP_CHECK,wtpid);

	if(ret != 0){
		wid_syslog_info("wtpid error = %d at %s\n",\
							wtpid, __func__);
		return -1 ;
	}
	
	struct ap_snr_info  *Node_snr = AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_head;
	AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_max_value = Node_snr->value;
	
	while(Node_snr != NULL){
		if(Node_snr->value != 0){
			valued_num ++;
			if(valued_num > AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_length){
				wid_syslog_info("vlued_num  > ap_snr_info_length at %s\n",__func__);
				return -1;
			}
			if(AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_max_value < Node_snr->value) {
				AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_max_value = Node_snr->value;
			}
			if(AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_min_value > Node_snr->value){
				AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_min_value = Node_snr->value;
			}
			
			sum_snr_value += Node_snr->value;
			Node_snr = Node_snr->next;
		}
	}
	
	/*AC_WTP[wtpid]->wifi_extension_info.snr_collect_average = 
						sum_snr_value / AC_WTP[wtpid]->apcminfo.ap_snr_info_length ;
	AC_WTP[wtpid]->apcminfo.snr_average = AC_WTP[wtpid]->wifi_extension_info.snr_collect_average;*/
	
	/*if the omc requir not accounting the 0 node, use the valued num ,else use the AC_WTP[wtpid]->apcminfo.ap_snr_info_length*/
#if 0
	AC_WTP[wtpid]->apcminfo.snr_average = sum_snr_value / AC_WTP[wtpid]->apcminfo.ap_snr_info_length ;*/
#endif

	AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_average = sum_snr_value / valued_num ;

	return WID_DBUS_SUCCESS ;
}

/*for sample snr value  by nl  2010-09-07*/
int wid_accouting_snr_sample_average_and_peak(unsigned int wtpid,unsigned int l_radioid){ //fengwenchao add 20120314 for onlinebug-162//qiuchen copy from v1.3

	//printf("#########wid_accouting_snr_sample_average_and_peak#######\n");
	unsigned int i_sample = 0;				/*采样结点的搜索变量*/
	//unsigned int i_node = 0;					/*为本次结点计数的递增变量*/
	//unsigned int D_value = 0;					/*本次与上次采样，结点所需要移动的步长*/
	unsigned int sum_snr_value = 0;			/*snr 采样值的总和*/
	unsigned int sample_times = 0;			/*抽样次数*/
	unsigned int total_sample_time = 0;		/*抽样总时长*/
	int ret = WID_DBUS_SUCCESS ;
	int valued_num = 0;						/*非零的结点个数*/
	double pi = 0;							/*样本点取10 的样本点次方*/
	double double_sum = 0;					/*pi  的算数和*/
	struct ap_snr_info  *sample_node= NULL;
	
	if(sample_infor_interval <= 0){
		wid_syslog_info("sample_infor_interval = %d at %s\n",\
							sample_infor_interval,__func__);
		return -1 ;
	}
	
	ret = WID_CHECK_ID(WID_WTP_CHECK,wtpid);

	if(ret != WID_DBUS_SUCCESS){
		wid_syslog_info("wtpid error = %d at %s\n",\
							wtpid, __func__);
		return -1 ;
	}
	
	total_sample_time = (AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_length) 
						* (unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;

	if(total_sample_time <= 0){
		wid_syslog_info("total_sample_time = %d at %s\n",\
									total_sample_time,__func__);
		return -1 ;
	}
	
	sample_times = total_sample_time/sample_infor_interval;
	
	if(sample_times <= 0){
		wid_syslog_info("sample_times = %d at %s\n",\
							sample_infor_interval,__func__);
		return -1 ;
	}
	
	struct ap_snr_info  *Node_snr = AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].ap_snr_info_head;
	AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_max_value = Node_snr->value;
	AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_min_value = Node_snr->value;

	for(i_sample=0;i_sample<sample_times;i_sample++){
		if(0 == i_sample ){
			if(0==Node_snr->value){
				sum_snr_value = 0;
				double_sum = 0;
				wid_syslog_debug_debug(WID_DEFAULT,"i_sample ==0 at %s\n",\
							 __func__);
			}
			else{
				sum_snr_value = Node_snr->value;
				pi = pow(10,( Node_snr->value/10.00));
				double_sum += pi;	
				valued_num++;
			}
		}
		else{
			/*
			k = (((i_sample * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->reportinterval))
				/(unsigned int) AC_WTP[wtpid]->reportinterval;
			j = ((((i_sample-1) * sample_infor_interval)+(unsigned int) AC_WTP[wtpid]->reportinterval))
				/(unsigned int) AC_WTP[wtpid]->reportinterval;
			D_value = k - j;
			for(i_node = 0;i_node < D_value;i_node++){
				if(Node_snr!= NULL&&Node_snr->next != NULL){
					Node_snr = Node_snr->next;
				}
			}*/

			sample_node = find_next_sample_node(wtpid,i_sample,Node_snr);
			Node_snr = sample_node ; 

			if(sample_node == NULL){
				wid_syslog_info(" wtp %d sample_node == NULL at %s\n",\
										wtpid, __func__);
			}
			
			if(sample_node->value !=0){
				valued_num ++;
				sum_snr_value += sample_node->value;
				pi = pow(10,( sample_node->value/10.00));
				double_sum = double_sum +pi;

				//AC_WTP[wtpid]->wtp_wifi_snr_stats.snr_average = sum/average_num;
				
				/*printf("### sample_node->value ###%d\n", sample_node->value);
				printf(" sample_node->snr_max_value %d\n", AC_WTP[wtpid]->apcminfo.snr_max_value);
				printf(" sample_node->snr_min_value %d\n", AC_WTP[wtpid]->apcminfo.snr_min_value);*/
				
				if(AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_max_value < sample_node->value){
					AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_max_value = sample_node->value;
				}
				if(AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_min_value > sample_node->value){
					AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_min_value = sample_node->value;
				}
			  /*printf("111 sample_node->snr_max_value %d\n", AC_WTP[wtpid]->apcminfo.snr_max_value);
				printf("111 sample_node->snr_min_value %d\n", AC_WTP[wtpid]->apcminfo.snr_min_value);*/
			}
		}
	}
	
	if(valued_num != 0){
		AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_average = (unsigned char)(sum_snr_value /valued_num) ;
		/*snr_math_average 计算样本点对数平均值*/
		AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_math_average = 10*log10(double_sum/valued_num);	
		//printf("  math_average %g \n",AC_WTP[wtpid]->apcminfo.snr_math_average);
	}
	else{
		AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_average = 0;
		AC_WTP[wtpid]->apcminfo.wifi_snr[l_radioid].snr_math_average = 0 ;
	}
	
	return WID_DBUS_SUCCESS ;
}

int wid_ap_cpu_mem_statistics(unsigned int wtpid)
{
	//struct ap_cpu_info  *cpu_node = NULL;
	//struct ap_cpu_info  *mem_node = NULL;
	unsigned int total_node_num = 0 ;
	unsigned int reportinterval = 0;
	int ret = WID_DBUS_SUCCESS;int ret2 = WID_DBUS_SUCCESS;int ret3 = WID_DBUS_SUCCESS;
	int i = 0;
	
	ret = WID_CHECK_ID(WID_WTP_CHECK,wtpid);

	if(ret != WID_DBUS_SUCCESS){
		wid_syslog_debug_debug(WID_DEFAULT," wtpid %d error in func %s",wtpid,__func__);
		return -1;
	}

	if(AC_WTP[wtpid] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"input wtpid %d error",wtpid);
		return -1;
	}

	reportinterval = (unsigned int) AC_WTP[wtpid]->wifi_extension_reportinterval;
	if(reportinterval <= 0){
		wid_syslog_debug_debug(WID_DEFAULT,"reportinterval <= 0 \n");
		return -1;
	}
	total_node_num = (AC_WTP[wtpid]->collect_time + reportinterval - 1)/reportinterval;
	if(total_node_num == 0){
		wid_syslog_debug_debug(WID_DEFAULT,"total_node_num == 0 \n");
		return -1;
	}

	//cpu
	{
		AC_WTP[wtpid]->apcminfo.cpu_times++;
 		ret = wid_add_cpu_value_for_accounting_average(wtpid,AC_WTP[wtpid]->wifi_extension_info.cpu);
		
 		if(ret == WID_DBUS_SUCCESS){
 			if(AC_WTP[wtpid]->apcminfo.ap_cpu_info_length > total_node_num){
				ret2 = wid_del_cpu_value_for_accounting_average(wtpid,total_node_num);
				if(ret2 != WID_DBUS_SUCCESS){
					wid_syslog_debug_debug(WID_DEFAULT,"wid_del_cpu_value_for_accounting_average error\n");
					return -1;
				}
			}
 			if((AC_WTP[wtpid]->apcminfo.ap_cpu_info_length != 0) && 
				(AC_WTP[wtpid]->apcminfo.ap_cpu_info_head!=NULL)){
				if(sample_infor_interval == 0){
					ret3 = wid_accouting_cpu_average(wtpid);
				}
				else{
					ret3 = wid_accouting_cpu_sample_average_and_peak(wtpid);
				}
				if(ret3 != WID_DBUS_SUCCESS){
					wid_syslog_debug_debug(WID_DEFAULT,"wid_accouting_cpu_sample_average_and_peak error\n");
					return -1;
				}
			}
		}
		else
			return -1;
 	}
	//memory usage
	{
		AC_WTP[wtpid]->apcminfo.mem_times++;
 		ret = wid_add_mem_value_for_accounting_average(wtpid,AC_WTP[wtpid]->wifi_extension_info.memoryuse);
 		if(ret == WID_DBUS_SUCCESS){
 			if(AC_WTP[wtpid]->apcminfo.ap_mem_info_length > total_node_num){
				ret2 = wid_del_mem_value_for_accounting_average(wtpid,total_node_num);
				if(ret2 != WID_DBUS_SUCCESS){
					wid_syslog_debug_debug(WID_DEFAULT,"wid_del_mem_value_for_accounting_average error\n");
					return -1;
				}
			}
 			if((AC_WTP[wtpid]->apcminfo.ap_mem_info_length != 0) && 
				(AC_WTP[wtpid]->apcminfo.ap_mem_info_head!=NULL)){
				if(sample_infor_interval == 0){
					ret3 = wid_accouting_mem_average(wtpid);
				}
				else{
					ret3 = wid_accouting_mem_sample_average_and_peak(wtpid);
				}
				if(ret3 != WID_DBUS_SUCCESS){
					wid_syslog_debug_debug(WID_DEFAULT,"wid_accouting_mem_sample_average_and_peak error\n");
					return -1;
				}
			}
		}
		else
			return -1;
 	}
	
#if 0	
	{
		AC_WTP[wtpid]->apcminfo.mem_times++;
		if((mem_node = (struct ap_cpu_info*)malloc(sizeof(struct ap_cpu_info))) == NULL){
			wid_syslog_debug_debug(WID_DEFAULT,"malloc fail in %s \n",__func__);
		}else{ 
			mem_node->value = AC_WTP[wtpid]->wifi_extension_info.memoryuse;
			mem_node->next = NULL;
			
			if(AC_WTP[wtpid]->apcminfo.ap_mem_info_head == NULL){
				AC_WTP[wtpid]->apcminfo.ap_mem_info_head = mem_node;
				AC_WTP[wtpid]->apcminfo.ap_mem_info_length++;
			}
			else{
				mem_node->next = AC_WTP[wtpid]->apcminfo.ap_mem_info_head;
				AC_WTP[wtpid]->apcminfo.ap_mem_info_head = mem_node;
				AC_WTP[wtpid]->apcminfo.ap_mem_info_length++;
			}
		}
		
		if(AC_WTP[wtpid]->apcminfo.ap_mem_info_length > total_node_num){
			struct ap_cpu_info	*p = AC_WTP[wtpid]->apcminfo.ap_mem_info_head ;
			struct ap_cpu_info	*tmp = NULL;
			
			for(i=0;i<total_node_num;i++){
				if (p!= NULL){
					tmp = p;
					p = p->next;
				}
			}
			tmp->next = NULL;
			
			while(p!=NULL){
				tmp = p;
				p = p->next;
				tmp->next = NULL;
				free(tmp);
				tmp = NULL;
				AC_WTP[wtpid]->apcminfo.ap_mem_info_length--;
			}

		}

		wid_syslog_debug_debug(WID_DEFAULT,"AC_WTP[%d]->ap_mem_info_length = %d\n",wtpid,AC_WTP[wtpid]->apcminfo.ap_mem_info_length);
		if((AC_WTP[wtpid]->apcminfo.ap_mem_info_length != 0) && (AC_WTP[wtpid]->apcminfo.ap_mem_info_head!=NULL)) {
			unsigned int sum_mem_value;
			struct ap_cpu_info	*Node_cpu = AC_WTP[wtpid]->apcminfo.ap_mem_info_head;
			AC_WTP[wtpid]->apcminfo.mem_peak_value = Node_cpu->value;
			for(; Node_cpu; Node_cpu=Node_cpu->next){
				if(AC_WTP[wtpid]->apcminfo.mem_peak_value < Node_cpu->value) {
					AC_WTP[wtpid]->apcminfo.mem_peak_value = Node_cpu->value;
				}
				sum_mem_value += Node_cpu->value;
			}
			AC_WTP[wtpid]->apcminfo.mem_average = sum_mem_value/AC_WTP[wtpid]->apcminfo.ap_mem_info_length ;
		}
	}
#endif
	
	/*accounting snr math average and sample average*/
	for(i =0;i < AC_WTP[wtpid]->wifi_extension_info.wifi_count;i++)
	{
		AC_WTP[wtpid]->apcminfo.wifi_snr[i].snr_times++;
		ret = wid_add_snr_value_for_accounting_average(wtpid,AC_WTP[wtpid]->wifi_extension_info.wifi_snr_new[i],i);
		if(ret == WID_DBUS_SUCCESS){
			if(AC_WTP[wtpid]->apcminfo.wifi_snr[i].ap_snr_info_length > total_node_num){
				ret2 = wid_del_snr_value_for_accounting_average(wtpid,total_node_num,i);
				if(ret2 != WID_DBUS_SUCCESS){
					wid_syslog_debug_debug(WID_DEFAULT,"wid_del_snr_value_for_accounting_average error\n");
					return -1;
				}
			}
			if((AC_WTP[wtpid]->apcminfo.wifi_snr[i].ap_snr_info_length != 0) && 
				(AC_WTP[wtpid]->apcminfo.wifi_snr[i].ap_snr_info_head!=NULL)){
				if(sample_infor_interval == 0){
					ret3 = wid_accouting_snr_average(wtpid,i);
				}
				else{
					ret3 = wid_accouting_snr_sample_average_and_peak(wtpid,i);
				}
				if(ret3 != WID_DBUS_SUCCESS){
					wid_syslog_debug_debug(WID_DEFAULT,"wid_accouting_snr_sample_average_and_peak error\n");
					return -1;
				}
			}
		}
		else
			return -1;
	}
#if 0 
	//wifi snr stat
	{
		if(AC_WTP[wtpid]->wtp_wifi_snr_stats.snr_max < AC_WTP[wtpid]->wifi_extension_info.wifi_snr)
		{
			AC_WTP[wtpid]->wtp_wifi_snr_stats.snr_max = AC_WTP[wtpid]->wifi_extension_info.wifi_snr;
		}
		if(AC_WTP[wtpid]->wifi_extension_info.wifi_snr != 0)
		{
			if(AC_WTP[wtpid]->wtp_wifi_snr_stats.snr_min > AC_WTP[wtpid]->wifi_extension_info.wifi_snr)
			{
				AC_WTP[wtpid]->wtp_wifi_snr_stats.snr_min = AC_WTP[wtpid]->wifi_extension_info.wifi_snr;
			}
		}
		i =  (AC_WTP[wtpid]->apcminfo.mem_times)%10;
		AC_WTP[wtpid]->wtp_wifi_snr_stats.snr[i] = AC_WTP[wtpid]->wifi_extension_info.wifi_snr;

		if((AC_WTP[wtpid]->apcminfo.mem_times) < 10 && (AC_WTP[wtpid]->apcminfo.mem_times > 0)){
			snr_ret= accounting_snr_math_average(wtpid,AC_WTP[wtpid]->apcminfo.mem_times);
		}
		else if(AC_WTP[wtpid]->apcminfo.mem_times >= 10){
			snr_ret = accounting_snr_math_average(wtpid,10);
		}
	}
	return 0;
	
#endif
	return 0;
}

int wid_parse_wtp_cpu_mem_trap_info(unsigned int wtpid)
{
	unsigned char flag = 0;
	int i = 0;
	int m = 0;
	unsigned int times = 0;
	unsigned int interval = 0;
	
	//cpu mem statistics
	wid_ap_cpu_mem_statistics(wtpid);
		
	if(AC_WTP[wtpid] == NULL)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"input wtpid %d error",wtpid);
		return -1;
	}
	m = AC_WTP[wtpid]->trap_collect_time%AC_WTP[wtpid]->wifi_extension_reportinterval;
	interval = AC_WTP[wtpid]->wifi_extension_reportinterval;
	if(interval != 0){
		if(m == 0){
			times = AC_WTP[wtpid]->trap_collect_time/interval;
		}else{
			times = AC_WTP[wtpid]->trap_collect_time/interval + 1;
		}
	}
	
	//cpu
	//- if((AC_WTP[wtpid]->wifi_extension_info.cpu)/100 > g_ap_cpu_threshold)
	if((AC_WTP[wtpid]->wifi_extension_info.cpu)/100 > AC_WTP[wtpid]->wtp_cpu_use_threshold)
	{
		if(AC_WTP[wtpid]->wifi_extension_info.cpu_trap_flag == 0)    //fengwenchao add 20110221 
			{
				AC_WTP[wtpid]->cpu_resend_times ++;
				flag = 1;

				if(gtrapflag>=4){
					//-if((gtrap_ap_cpu_trap_switch == 1)&&(AC_WTP[wtpid]->wtp_cpu_use_threshold < ((AC_WTP[wtpid]->wifi_extension_info.cpu)/100)))
					if((gtrap_ap_cpu_trap_switch == 1)&&(AC_WTP[wtpid]->wtp_cpu_use_threshold < ((AC_WTP[wtpid]->apcminfo.cpu_average)/100)))
						if(AC_WTP[wtpid]->cpu_resend_times >= times){
							AC_WTP[wtpid]->wifi_extension_info.cpu_trap_flag = 1;
							wid_dbus_trap_ap_cpu_threshold(wtpid,flag);							
							AC_WTP[wtpid]->cpu_resend_times = 0;
						}
					}
			}
	}
	else
	{
		if(AC_WTP[wtpid]->wifi_extension_info.cpu_trap_flag == 1)
		{
			flag = 0;
			AC_WTP[wtpid]->cpu_clear_resend_times ++;
			if(gtrapflag>=4){
				//--if((gtrap_ap_cpu_trap_switch == 1)&&(AC_WTP[wtpid]->wtp_cpu_use_threshold > ((AC_WTP[wtpid]->wifi_extension_info.cpu)/100)))
				if((gtrap_ap_cpu_trap_switch == 1)&&(AC_WTP[wtpid]->wtp_cpu_use_threshold > ((AC_WTP[wtpid]->apcminfo.cpu_average)/100)))
					if(AC_WTP[wtpid]->cpu_clear_resend_times >= times){
						AC_WTP[wtpid]->wifi_extension_info.cpu_trap_flag = 0;
						wid_dbus_trap_ap_cpu_threshold(wtpid,flag);
						AC_WTP[wtpid]->cpu_clear_resend_times = 0;
					}
				}
		}
		else
		{
			
		}
	}
	//mem
	//--if(AC_WTP[wtpid]->wifi_extension_info.memoryuse > g_ap_memuse_threshold)
	if(AC_WTP[wtpid]->wifi_extension_info.memoryuse > AC_WTP[wtpid]->wtp_mem_use_threshold)
	{
		if(AC_WTP[wtpid]->wifi_extension_info.mem_trap_flag == 0)     //fengwenchao add 20110221
			{
				flag = 1;
				AC_WTP[wtpid]->memtrap_resend_times ++;
				if(gtrapflag>=4){
					//if((gtrap_ap_mem_trap_switch == 1)&&(AC_WTP[wtpid]->wtp_mem_use_threshold < AC_WTP[wtpid]->wifi_extension_info.memoryuse))
					if((gtrap_ap_mem_trap_switch == 1)&&(AC_WTP[wtpid]->wtp_mem_use_threshold < AC_WTP[wtpid]->apcminfo.mem_average))
						if(AC_WTP[wtpid]->memtrap_resend_times >= times){
							AC_WTP[wtpid]->wifi_extension_info.mem_trap_flag = 1;
							wid_dbus_trap_ap_mem_threshold(wtpid,flag);
							AC_WTP[wtpid]->memtrap_resend_times = 0;
						}
					}
			}
	}
	else
	{
		if(AC_WTP[wtpid]->wifi_extension_info.mem_trap_flag == 1)
		{
			flag = 0;
			AC_WTP[wtpid]->memtrap_clear_resend_times ++;
			if(gtrapflag>=4){
				//--if((gtrap_ap_mem_trap_switch  == 1)&&(AC_WTP[wtpid]->wtp_mem_use_threshold > AC_WTP[wtpid]->wifi_extension_info.memoryuse)){
				if((gtrap_ap_mem_trap_switch  == 1)&&(AC_WTP[wtpid]->wtp_mem_use_threshold > AC_WTP[wtpid]->apcminfo.mem_average)){
					wid_syslog_debug_debug(WID_DBUS,"wtpid:%d,memtrap_clear_resend_times %d\n",wtpid,AC_WTP[wtpid]->memtrap_clear_resend_times);
					if(AC_WTP[wtpid]->memtrap_clear_resend_times >= times){
						AC_WTP[wtpid]->wifi_extension_info.mem_trap_flag = 0;
						wid_dbus_trap_ap_mem_threshold(wtpid,flag);
						AC_WTP[wtpid]->memtrap_clear_resend_times = 0;
					}
				}
			}
		}
		else
		{
			
		}
	}
	//temp
	if(AC_WTP[wtpid]->wifi_extension_info.temperature > g_ap_temp_threshold)
	{
		if(AC_WTP[wtpid]->wifi_extension_info.temp_trap_flag == 0)     //fengwenchao add 20110221
			{
				flag = 1;
				AC_WTP[wtpid]->ap_temp_resend_times ++;
				if(gtrapflag>=4){
					wid_syslog_debug_debug(WID_DBUS,"wtpid:%d,ap_temp_resend_times %d\n",wtpid,AC_WTP[wtpid]->ap_temp_resend_times);
					if(AC_WTP[wtpid]->ap_temp_resend_times >= times){
						AC_WTP[wtpid]->wifi_extension_info.temp_trap_flag = 1;
						wid_dbus_trap_ap_temp_threshold(wtpid,flag);
						AC_WTP[wtpid]->ap_temp_resend_times = 0;
					}
				}
			}
	}
	else
	{
		if(AC_WTP[wtpid]->wifi_extension_info.temp_trap_flag == 1)
		{
			flag = 0;
			AC_WTP[wtpid]->ap_temp_clear_resend_times ++;
			if(gtrapflag>=4){
				wid_syslog_debug_debug(WID_DBUS,"wtpid:%d,ap_temp_clear_resend_times %d\n",wtpid,AC_WTP[wtpid]->ap_temp_clear_resend_times);
				if(AC_WTP[wtpid]->ap_temp_clear_resend_times >= times){
					
					wid_dbus_trap_ap_temp_threshold(wtpid,flag);
					AC_WTP[wtpid]->wifi_extension_info.temp_trap_flag = 0;
					AC_WTP[wtpid]->ap_temp_clear_resend_times = 0;
				}
			}
		}
		else
		{
			
		}
	}
	
	//wifi interface state trap
	if(AC_WTP[wtpid]->wifi_extension_info.wifi_count != 0)
	{
		for(i=0;i<AC_WTP[wtpid]->wifi_extension_info.wifi_count;i++)
		{
			if(AC_WTP[wtpid]->wifi_extension_info.wifi_state[i] == 3)
			{
				AC_WTP[wtpid]->wifi_extension_info.wifi_trap_flag[i] = 1;
				flag = 1;

				if(gtrapflag>=4){
					wid_dbus_trap_ap_wifi_if_error(wtpid,i,flag);
					}
			}
			else if((AC_WTP[wtpid]->wifi_extension_info.wifi_state[i] == 1)||(AC_WTP[wtpid]->wifi_extension_info.wifi_state[i] == 2))
			{
				if(AC_WTP[wtpid]->wifi_extension_info.wifi_trap_flag[i] == 1)
				{
					flag = 0;

					if(gtrapflag>=4){
						wid_dbus_trap_ap_wifi_if_error(wtpid,i,flag);
						AC_WTP[wtpid]->wifi_extension_info.wifi_trap_flag[i] = 0;  //fengwenchao add 20110302
						}
				}
				else
				{
					
				}
				//AC_WTP[wtpid]->wifi_extension_info.wifi_trap_flag[i] = 0;   //fengwenchao modify 20110302
			}
		}
	}
	
	return 0;
}
int wid_set_ap_sta_infomation_report(unsigned int wtpid)
{
	msgq msg;
//	struct msgqlist *elem;
		
	if(AC_WTP[wtpid]->WTPStat == 5)
	{
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_INFO_SET;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = wtpid%THREAD_NUM+1;
		msg.mqinfo.WTPID = wtpid;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_INFO_SET;
		
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_info("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(wtpid, elem);
	}*/

	return 0;
	
}

int  wtp_set_sta_info_report(unsigned int wtpid,int policy)
{	
	msgq msg;
//	struct msgqlist *elem;
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->sta_ip_report != policy))
	{
		printf("%s AC_WTP[%d]->sta_ip_report %d\n",__func__,wtpid,AC_WTP[wtpid]->sta_ip_report);
		AC_WTP[wtpid]->sta_ip_report = policy;
		
		if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
		{
			CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
			if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = wtpid%THREAD_NUM+1;
				msg.mqinfo.WTPID = wtpid;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_INFO_REPORT;
				msg.mqinfo.u.WtpInfo.value2 = policy;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
		}//delete unuseful cod
		/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_INFO_REPORT;
			msg.mqinfo.u.WtpInfo.value2 = policy;
		
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
			elem = NULL;
		}*/
	}
	return 0;
}
/*wcl add for globle variable*/

int  wtp_set_wtp_dhcp_snooping(unsigned int wtpid,int policy)
{	
	msgq msg;
//	struct msgqlist *elem;
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->dhcp_snooping != policy))
	{
		printf("%s AC_WTP[%d]->dhcp_snooping %d\n",__func__,wtpid,AC_WTP[wtpid]->dhcp_snooping);
		AC_WTP[wtpid]->dhcp_snooping = policy;
		
		if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
		{
			CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
			if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = wtpid%THREAD_NUM+1;
				msg.mqinfo.WTPID = wtpid;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_DHCP_SNOOPING;
				msg.mqinfo.u.WtpInfo.value2 = policy;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
		}//delete unuseful cod
		/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_DHCP_SNOOPING;
			msg.mqinfo.u.WtpInfo.value2 = policy;
		
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
			elem = NULL;
		}*/
	}
	return 0;
}

/*end*/
int wid_set_ap_sta_wapi_info_report(unsigned int wtpid)
{
	msgq msg;
//	struct msgqlist *elem;
		
	if(AC_WTP[wtpid]->WTPStat == 5)
	{
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_WAPI_INFO_SET;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
					memset((char*)&msg, 0, sizeof(msg));
					msg.mqid = wtpid%THREAD_NUM+1;
					msg.mqinfo.WTPID = wtpid;
					msg.mqinfo.type = CONTROL_TYPE;
					msg.mqinfo.subtype = WTP_S_TYPE;
					msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_WAPI_INFO_SET;
					
					elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
					if(elem == NULL){
						wid_syslog_info("%s malloc %s",__func__,strerror(errno));
						perror("malloc");
						return 0;
					}
					memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
					elem->next = NULL;
					memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
					WID_INSERT_CONTROL_LIST(wtpid, elem);
				}*/

	return 0;
	
}

int wid_set_ap_if_info_report(unsigned int wtpid)
{
	msgq msg;
//	struct msgqlist *elem;
		
	if(AC_WTP[wtpid]->WTPStat == 5)
	{
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_IF_INFO_SET;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[wtpid] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = wtpid%THREAD_NUM+1;
		msg.mqinfo.WTPID = wtpid;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_IF_INFO_SET;

		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_info("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(wtpid, elem);
	}*/

	return 0;
	
}
int wid_set_ap_wids_set(unsigned int wtpid)
{
	msgq msg;
		
	if(AC_WTP[wtpid]->WTPStat == 5)
	{
		
		CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
		if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_WIDS_SET;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}
		CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
	}

	return 0;
	
}
int wid_set_ap_if_updown(unsigned int wtpid,unsigned char type,unsigned char ifindex,unsigned char policy)
{
	int ret = 0;
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_ap_if_updown wtpid %d,type %d,ifindex %d,policy %d\n",wtpid,type,ifindex,policy);
	char iftype[2][5] = {"eth","wifi"};
	char ifpolicy[2][5] = {"down","up"};
	time_t t;			//xiaodawei add for state time, 20110114
	time(&t);

	if(type == 0)//eth
	{
		if(policy == 0)//down
		{
			if(AC_WTP[wtpid]->apifinfo.eth[ifindex].state == 2)//state down
			{
				return 0;
			}
			else
			{
				AC_WTP[wtpid]->apifinfo.eth[ifindex].state = 2;
				AC_WTP[wtpid]->apifinfo.eth[ifindex].state_time = t;
			}
		}
		else if(policy == 1)//up
		{
			if(AC_WTP[wtpid]->apifinfo.eth[ifindex].state == 1)//state up
			{
				return 0;
			}
			else
			{
				AC_WTP[wtpid]->apifinfo.eth[ifindex].state = 1;
				AC_WTP[wtpid]->apifinfo.eth[ifindex].state_time = t;
			}
		}
	}
	else if(type == 1)//wifi
	{
		if(policy == 0)//down
		{
			if(AC_WTP[wtpid]->apifinfo.wifi[ifindex].state == 2)//state down
			{
				return 0;
			}
			else
			{
				AC_WTP[wtpid]->apifinfo.wifi[ifindex].state = 2;
				AC_WTP[wtpid]->apifinfo.wifi[ifindex].state_time = t;
			}
		}
		else if(policy == 1)//up
		{
			if(AC_WTP[wtpid]->apifinfo.wifi[ifindex].state == 1)//state up
			{
				return 0;
			}
			else
			{
				AC_WTP[wtpid]->apifinfo.wifi[ifindex].state = 1;
				AC_WTP[wtpid]->apifinfo.wifi[ifindex].state_time = t;
			}
		}
	}
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"ifconfig %s%d %s",iftype[type],ifindex,ifpolicy[policy]);
	//printf("apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);

	return 0;
}
int wid_set_wtp_ntp(unsigned int wtpid)
{
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_wtp_ntp wtpid %d\n",wtpid);
/*
	char *addr;
	char *ifname;
	
	ifname = (char *)malloc(ETH_IF_NAME_LEN+1);
	if(ifname == NULL)
	{
		wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
		perror("malloc");
		return MALLOC_ERROR;
	}
	memset(ifname,0,ETH_IF_NAME_LEN+1);
	memcpy(ifname,AC_WTP[wtpid]->BindingIFName,strlen(AC_WTP[wtpid]->BindingIFName));
	//printf("ifname %s\n",ifname);
	
	struct ifi_info *ifi = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi->ifi_name,0,sizeof(ifi->ifi_name));
	strncpy(ifi->ifi_name,ifname,sizeof(ifi->ifi_name));
	ret = Get_Interface_Info(ifname,ifi);
	if(ret != 0){
		if(ifi->ifi_addr != NULL){
			free(ifi->ifi_addr);
			ifi->ifi_addr = NULL;
		}		
		if(ifi->ifi_brdaddr != NULL){
			free(ifi->ifi_brdaddr);
			ifi->ifi_brdaddr = NULL;
		}
		free(ifi);
		ifi = NULL;
		CW_FREE_OBJECT(ifname);
		return WID_DBUS_ERROR;		
	}
	addr = (char *)malloc(ETH_IF_NAME_LEN+1);
	if(addr == NULL)
	{
		wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
		perror("malloc");
		return MALLOC_ERROR;
	}
	memset(addr,0,ETH_IF_NAME_LEN+1);
	sprintf(addr,"%s",inet_ntoa(((struct sockaddr_in*)(ifi->ifi_addr))->sin_addr));
	//printf("addr %s\n",addr);
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"ntpclient -h %s -s",addr);
	wid_syslog_debug_debug(WID_DEFAULT,"apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);

	CW_FREE_OBJECT(ifname);
	CW_FREE_OBJECT(addr);
*/
	{
			msgq msg;	
//			struct msgqlist *elem;
			
			if(AC_WTP[wtpid]->WTPStat == 5)
			{
				
				CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
				if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
				{
					memset((char*)&msg, 0, sizeof(msg));
					msg.mqid = wtpid%THREAD_NUM+1;
					msg.mqinfo.WTPID = wtpid;
					msg.mqinfo.type = CONTROL_TYPE;
					msg.mqinfo.subtype = WTP_S_TYPE;
					msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_NTP_SET;
					
					if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
						wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
						perror("msgsnd");
					}
				}
				CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
			}//delete unuseful cod
			/*else if((AC_WTP[wtpid] != NULL)){
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = wtpid%THREAD_NUM+1;
				msg.mqinfo.WTPID = wtpid;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_NTP_SET;
		
				elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem == NULL){
					wid_syslog_info("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
				elem->next = NULL;
				memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
				WID_INSERT_CONTROL_LIST(wtpid, elem);
			}*/
		
			
		}

	
	return 0;
}

//auto ap area
int wid_auto_ap_login_insert_iflist(char *ifname)
{
	wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_login_insert_iflist ifname :%s\n",ifname);
	//get if index
	unsigned int ifindex = 0;
	int sockfd;
	struct ifreq	ifr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));			
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1)
	{
//		printf("SIOCGIFINDEX error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"SIOCGIFINDEX error\n");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"ifindex %d\n",ifr.ifr_ifindex);
	
	ifindex = ifr.ifr_ifindex;
	close(sockfd);
	//insert to the list
	wid_auto_ap_if *wif = NULL;
	wid_auto_ap_if *wifnext = NULL;
	
	wif = (wid_auto_ap_if *)malloc(sizeof(wid_auto_ap_if));
	wif->ifindex = ifindex;
	wif->wlannum = 0;
	memset(wif->wlanid,0,L_BSS_NUM);
	memset(wif->ifname,0,ETH_IF_NAME_LEN);
	memcpy(wif->ifname,ifname,strlen(ifname));
	wif->ifnext = NULL;
	
	if(g_auto_ap_login.auto_ap_if == NULL)
	{
		g_auto_ap_login.auto_ap_if = wif ;
		g_auto_ap_login.auto_ap_if->ifnext = NULL;
		g_auto_ap_login.ifnum = 1;
	}
	else
	{
		wifnext = g_auto_ap_login.auto_ap_if;
		while(wifnext != NULL)
		{	
			if(strcmp(wifnext->ifname,ifname) == 0) //fengwenchao modify 20110414
			{
				//printf("already in the list\n");
				free(wif);
				wif = NULL;
				return 0;
			}
			wifnext = wifnext->ifnext;
		}
		
		wifnext = g_auto_ap_login.auto_ap_if;
		while(wifnext->ifnext != NULL)
		{	
			wifnext = wifnext->ifnext;
		}
		
		wifnext->ifnext = wif;
		g_auto_ap_login.ifnum++;
	}
	return 0;
}
int wid_auto_ap_login_remove_iflist(char *ifname)
{
	wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_login_remove_iflist ifname %s\n",ifname);

	wid_auto_ap_if *wif = NULL;
	wid_auto_ap_if *wifnext = NULL;

	wifnext = g_auto_ap_login.auto_ap_if;
	
	if(g_auto_ap_login.auto_ap_if != NULL)
	{
		if(strcmp(wifnext->ifname,ifname) == 0)  //fengwenchao modify 20110414
		{
			g_auto_ap_login.auto_ap_if = wifnext->ifnext;
			g_auto_ap_login.ifnum--;
			free(wifnext);
			wifnext = NULL;		
			//printf("delete ifname %s from list\n",ifname);
		}
		else
		{
			while(wifnext->ifnext != NULL)
			{	
				if(strcmp(wifnext->ifnext->ifname,ifname) == 0)  //fengwenchao modify 20110414
				{
					wif = wifnext->ifnext;
					wifnext->ifnext = wifnext->ifnext->ifnext;
					g_auto_ap_login.ifnum--;
					free(wif);
					wif = NULL;				
					//printf("delete ifname %s from list\n",ifname);
					return 0;
				}
				wifnext = wifnext->ifnext;
			}
			return INTERFACE_NOT_BE_BINDED;
		}
	}
	else
	{
		return INTERFACE_NOT_BE_BINDED;
	}
	return 0;
	
}
/*
int wid_auto_ap_login_destroy_iflist()
{
	if(g_auto_ap_login.auto_ap_if == NULL)
	{
		return 0;
	}
	
	wid_auto_ap_if *phead = NULL;
	wid_auto_ap_if *pnext = NULL;
	phead = g_auto_ap_login.auto_ap_if;
	g_auto_ap_login.auto_ap_if = NULL;
			
	while(phead != NULL)
	{			
		pnext = phead->ifnext;
		CW_FREE_OBJECT(phead);
		phead = pnext;
	}
	g_auto_ap_login.ifnum = 0;
	return 0;
}*/

int WID_WDS_BSSID_OP(unsigned int RadioID, unsigned char WlanID, unsigned char *MAC, unsigned char OP)
{
	char buf[DEFAULT_LEN];
	//int ret = -1;
	//int k1 = 0;	
	int i = 0;
	int WtpID = RadioID/L_RADIO_NUM;
	int localradio_id = RadioID%L_RADIO_NUM;	
	WID_BSS * BSS = NULL;
	struct wds_bssid *wds = NULL;
	struct wds_bssid *wds_next = NULL;
	unsigned char oper = 0;
	oper = OP&0x03;
	if(AC_WLAN[WlanID] == NULL)
	{
		return WLAN_ID_NOT_EXIST;
	}
	if(AC_RADIO[RadioID]->Wlan_Id == NULL)
	{
		return RADIO_NO_BINDING_WLAN;
	}
	else
	{	
		while(i < L_BSS_NUM)
		{	
			if(AC_RADIO[RadioID]->BSS[i] == NULL){
				i++;
				if(i == L_BSS_NUM)
					return RADIO_NO_BINDING_WLAN;				
				continue;
			}
			if(AC_RADIO[RadioID]->BSS[i]->WlanID == WlanID)
			{
				BSS = AC_RADIO[RadioID]->BSS[i]; 
				break;
			}
			i++;
			if(i == L_BSS_NUM)
				return RADIO_NO_BINDING_WLAN;
		}
		
		if(BSS->WDSStat == WDS_ANY)
			return WDS_MODE_BE_USED;

		if(oper == 1){//add
			BSS->WDSStat = WDS_SOME;
			if(BSS->wds_bss_list == NULL){
				wds = malloc(sizeof(struct wds_bssid));
				if(wds == NULL){
					return WID_DBUS_ERROR;
				}
				memset(wds, 0, sizeof(struct wds_bssid));
				memcpy(wds->BSSID, MAC, MAC_LEN);
				wds->next = NULL;
				BSS->wds_bss_list = wds;
				if((OP&0x04) == 0){
					BSS->wblwm = 0;
				}
				else{
					BSS->wblwm = 1;
				}
				if((AC_WTP[WtpID]!=NULL) && (AC_WTP[WtpID]->isused == 1)){
					memset(buf,0,DEFAULT_LEN);
					sprintf(buf,"/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",localradio_id,WlanID,MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
//					printf("/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",localradio_id,WlanID,MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
					wid_radio_set_extension_command(WtpID,buf);
				}
			}else{
				wds_next = BSS->wds_bss_list;
				while(wds_next != NULL){
					if(memcmp(wds_next->BSSID, MAC, MAC_LEN) == 0)
						return WID_DBUS_SUCCESS;
					wds_next = wds_next->next;
				}
				wds = malloc(sizeof(struct wds_bssid));
				if(wds == NULL){
					return WID_DBUS_ERROR;
				}
				memset(wds, 0, sizeof(struct wds_bssid));
				memcpy(wds->BSSID, MAC, MAC_LEN);
				wds->next = NULL;
				wds->next = BSS->wds_bss_list;
				BSS->wds_bss_list = wds;
				if((OP&0x04) == 0){
					BSS->wblwm = 0;
				}
				else{
					BSS->wblwm = 1;
				}
				if((AC_WTP[WtpID]!=NULL) && (AC_WTP[WtpID]->isused == 1)){
					memset(buf,0,DEFAULT_LEN);
					sprintf(buf,"/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",localradio_id,WlanID,MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
//					printf("/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",localradio_id,WlanID,MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
					wid_radio_set_extension_command(WtpID,buf);
				}
			}
		}
		else if(oper == 2){//delete
			if(BSS->wds_bss_list == NULL){
				return WID_DBUS_SUCCESS;
			}else{
				wds_next = BSS->wds_bss_list;
				if(memcmp(wds_next->BSSID, MAC, MAC_LEN) == 0){
					BSS->wds_bss_list = wds_next->next;
					wds_next->next = NULL;
					free(wds_next);
					wds_next = NULL;	
					if((OP&0x04) == 0){
						BSS->wblwm = 0;
					}
					else{
						BSS->wblwm = 1;
					}
					if(BSS->State == 1){
						memset(buf,0,DEFAULT_LEN);
						sprintf(buf,"/usr/sbin/wds_del %02X:%02X:%02X:%02X:%02X:%02X\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
//						printf("/usr/sbin/wds_del %02X:%02X:%02X:%02X:%02X:%02X\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
						wid_radio_set_extension_command(WtpID,buf);
					}
					if(BSS->wds_bss_list == NULL){
						BSS->WDSStat = DISABLE;
//						printf("wds bssid disable\n");
					}
					return WID_DBUS_SUCCESS;
				}
				while(wds_next->next != NULL){
					if(memcmp(wds_next->next->BSSID, MAC, MAC_LEN) == 0){
						wds = wds_next->next;
						wds_next->next = wds->next;
						wds->next = NULL;
						free(wds);
						wds = NULL;
						if((OP&0x04) == 0){
							BSS->wblwm = 0;
						}
						else{
							BSS->wblwm = 1;
						}
						if(BSS->State == 1){
							memset(buf,0,DEFAULT_LEN);
							sprintf(buf,"/usr/sbin/wds_del %02X:%02X:%02X:%02X:%02X:%02X\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
//							printf("/usr/sbin/wds_del %02X:%02X:%02X:%02X:%02X:%02X\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
							wid_radio_set_extension_command(WtpID,buf);
						}
						if(BSS->wds_bss_list == NULL){
							BSS->WDSStat = DISABLE;
//							printf("wds bssid disable\n");
						}
						return WID_DBUS_SUCCESS;
					}
					wds_next = wds_next->next;
				}
				return WID_DBUS_SUCCESS;				
			}
		}
	}
			
	return 0;

}

int WID_RADIO_SET_WDS_STATUS(unsigned int RadioID, unsigned char WLANID,unsigned char status)
{

	int k = 0;
	unsigned char wds_mesh = status&0x02;
	status = status&(0x01);
	int wtpid = RadioID/L_RADIO_NUM;
	int local_radioid = RadioID%L_RADIO_NUM;
	msgq msg;
//	struct msgqlist *elem;
	k = WLANID;
	
	if((AC_WLAN[k] != NULL) && (AC_WLAN[k]->want_to_delete != 1))		/* Huangleilei add for ASXXZFI-1622 */
	{				

		if(AC_WLAN[k]->S_WTP_BSS_List[wtpid][local_radioid] != 0)
		{	
			unsigned int BSSIndex = AC_WLAN[k]->S_WTP_BSS_List[wtpid][local_radioid];
			if(!check_bssid_func(BSSIndex)){
				wid_syslog_err("<error>%s\n",__func__);
				return BSS_NOT_EXIST;
			}else{						
			}
			if(AC_BSS[BSSIndex]->WDSStat == status)
			{
				return 0;
			}
			else
			{
				if(AC_BSS[BSSIndex]->WDSStat == WDS_SOME)
					return WDS_MODE_BE_USED;
				AC_BSS[BSSIndex]->WDSStat = status;
				if(wds_mesh!=0){
					AC_BSS[BSSIndex]->wds_mesh = 1;
				}
				else {
					AC_BSS[BSSIndex]->wds_mesh = 0;
				}

			}
			if((AC_BSS[BSSIndex] != NULL)&&(AC_WTP[wtpid] != NULL))
			{
				//if(AC_WTP[wtpid]->CMD->radiowlanid[local_radioid][k] != 0)	//run state
				{
					
						msg.mqid = wtpid%THREAD_NUM+1;
						msg.mqinfo.WTPID = wtpid;
					msg.mqinfo.type = CONTROL_TYPE;
					msg.mqinfo.subtype = WDS_S_TYPE;
					if(wds_mesh == 1)
					{
						msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_ENABLE;
					}
					else
					{
						msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_DISABLE;
					}
					
					msg.mqinfo.u.WlanInfo.WLANID = k;
					msg.mqinfo.u.WlanInfo.Radio_L_ID = local_radioid;
					if(AC_WTP[wtpid]->WTPStat == 5){	
						if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
							wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
							perror("msgsnd");
						}
					}//delete unuseful cod
					/*else{
						elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
						if(elem == NULL){
							wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
							perror("malloc");
							return 0;
						}
						memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
						elem->next = NULL;
						memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
						WID_INSERT_CONTROL_LIST(wtpid, elem);
						
					}*/
				}

			}
			else
			{
				return RADIO_NO_BINDING_WLAN;	
			}
		
		}
		else
		{
			return RADIO_NO_BINDING_WLAN;	
		}
	}
	else if (AC_WLAN[k] != NULL && (AC_WLAN[k]->want_to_delete == 1))		/* Huangleilei add for ASXXZFI-1622 */
	{
		return WID_WANT_TO_DELETE_WLAN;
	}
	else
	{
		return WLAN_ID_NOT_EXIST;
	}

	
	return 0;	
}

void wid_set_wds_state(unsigned int wtpid, unsigned char radioid, unsigned char wlanid, unsigned char state)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
	char apcmd2[WID_SYSTEM_CMD_LENTH];
	memset(apcmd2,0,WID_SYSTEM_CMD_LENTH);

	if(state == WLAN_WDS_ENABLE){
		sprintf(apcmd,"iwpriv ath.%d-%d wds 1",radioid,wlanid);
		sprintf(apcmd2,"iwpriv ath.%d-%d ap_bridge 0",radioid,wlanid);
	}else if(state == WLAN_WDS_DISABLE){
		sprintf(apcmd,"iwpriv ath.%d-%d wds 0",radioid,wlanid);
	}else{
		return;
	}

	ret = wid_radio_set_extension_command(wtpid,apcmd);

	if(state == WLAN_WDS_ENABLE)
	{
		ret = wid_radio_set_extension_command(wtpid,apcmd2);
	}

	return;

}
int CWWIDReInit(){
	int i = 0;
	int flag = 0;
	int m = 0;
	char buf_base[DEFAULT_LEN];
	char strdir[DEFAULT_LEN];
	
	if(g_wtp_binding_count == NULL){
		g_wtp_binding_count = malloc((glicensecount+1)*sizeof(LICENSE_TYPE *));
		memset(g_wtp_binding_count,0,(glicensecount+1)*sizeof(LICENSE_TYPE *));
		for(i=0; i<glicensecount+1; i++){
			g_wtp_binding_count[i] = NULL;
		}
	}else{
		for(i=0; i<glicensecount+1; i++){
			if(g_wtp_binding_count[i]){
				memset(g_wtp_binding_count[i],0,sizeof(LICENSE_TYPE));
			}
		}
	}

	if(g_wtp_count == NULL){
		g_wtp_count = malloc(glicensecount*(sizeof(LICENSE_TYPE *)));
		memset(g_wtp_count,0,sizeof(LICENSE_TYPE *));
	}
	
	for(i=0; i<glicensecount; i++)
	{
		if(g_wtp_count[i] == NULL){
			g_wtp_count[i] = malloc(sizeof(LICENSE_TYPE));
			memset(g_wtp_count[i],0,sizeof(LICENSE_TYPE));
		}
		//g_wtp_count[i]->gcurrent_wtp_count = 0;
		//g_wtp_count[i]->flag = 0;
		g_wtp_count[i]->isShm = 0;
		memset(strdir,0,DEFAULT_LEN);
		memset(buf_base,0,DEFAULT_LEN);	
		sprintf(strdir,"/var/run/wcpss/wtplicense%d-%d-%d",local,vrrid,i+1);
		if(read_ac_info(strdir,buf_base) == 0)
		{
			if(parse_int_ID(buf_base, &g_wtp_count[i]->gmax_wtp_count)==-1)
			g_wtp_count[i]->gmax_wtp_count = WTP_DEFAULT_NUM_OEM;
		}
		else
		{
			g_wtp_count[i]->gmax_wtp_count = WTP_DEFAULT_NUM_OEM;
		}
		g_wtp_count[i]->gmax_wtp_count_assign = g_wtp_count[i]->gmax_wtp_count;
		flag = g_wtp_count[i]->flag;
		if(flag > 0){
			g_wtp_binding_count[flag]->gcurrent_wtp_count+=g_wtp_count[i]->gcurrent_wtp_count;
			g_wtp_binding_count[flag]->gmax_wtp_count+=g_wtp_count[i]->gmax_wtp_count;
		}
		wid_syslog_info("########CWWIDReInit######## maxwtp[%d] = %d\n",i,g_wtp_count[i]->gmax_wtp_count);
	}
	
	for(m=1;m<glicensecount;m++){
		if((g_wtp_binding_count)&&(g_wtp_binding_count[m])){
			wid_syslog_info("g_wtp_binding_count[%d]->gcurrent_wtp_count=%d,max=%d.\n",m,g_wtp_binding_count[m]->gcurrent_wtp_count,g_wtp_binding_count[m]->gmax_wtp_count);
		}
		if((g_wtp_count)&&(g_wtp_count[m])){
			wid_syslog_info("g_wtp_count[%d]->gcurrent_wtp_count=%d,max=%d.\n",m,g_wtp_count[m]->gcurrent_wtp_count,g_wtp_count[m]->gmax_wtp_count);
		}
	}
	//license_binding_Reinit("2,3");
	//license_binding_Reinit("5,6");

	wid_syslog_info("########CWWIDReInit##lic bind reinit###### glicensecount = %d.\n",glicensecount);
	return 0;
}

void wid_parse_legal_essid_list(unsigned int wtpindex,char *name)
{
	int ret = 0;
	int len1 = 0;
	int len2 = 0;
	len1 = strlen(name);
	struct essid_node *list;
	list = g_essid_list.essid_list;
	while(list != NULL)
	{
		len2 = strlen(list->essid);
		if((len1 == len2)&&(memcmp(name,list->essid,len1) == 0))
		{
//			printf("name %s in the list\n",name);
			ret = 1;
		}
		list = list->next;
	}
	if(ret == 0)
	{
		if(gtrapflag>=4){
			wid_dbus_trap_wtp_find_unsafe_essid(wtpindex,name);
			}

		return;
	}
}
void wid_parse_neighbor_ap_list(unsigned int wtpindex)
{
	if((AC_WTP[wtpindex] == NULL)||(AC_WTP[wtpindex]->NeighborAPInfos == NULL))
	{
		return;
	}
	int count = AC_WTP[wtpindex]->NeighborAPInfos->neighborapInfosCount;
	int i = 0;
	if(count == 0)
	{
		return;
	}
	struct Neighbor_AP_ELE *list;
	list = AC_WTP[wtpindex]->NeighborAPInfos->neighborapInfos;

	char *essid = NULL;

	for(i=0;i<count;i++)
	{
		essid = (char *)list->ESSID;
		if(essid != NULL)
		{
			if((g_essid_list.essid_list == NULL)||(g_essid_list.list_len == 0))
			{
				if(gtrapflag>=4){
					wid_dbus_trap_wtp_find_unsafe_essid(wtpindex,essid);
					}
			}
			else
			{
				wid_parse_legal_essid_list(wtpindex,essid);
			}
		}
		list = list->next;
	}
}
void wid_init_wtp_info_in_create(unsigned int WTPID)
{
	int i;
	//wtp wifi snr stats
	{
		AC_WTP[WTPID]->wtp_wifi_snr_stats.ifindex = 0;
		AC_WTP[WTPID]->wtp_wifi_snr_stats.snr_max = 0;
		AC_WTP[WTPID]->wtp_wifi_snr_stats.snr_min = 100;
		AC_WTP[WTPID]->wtp_wifi_snr_stats.snr_average = 0;
		AC_WTP[WTPID]->wtp_wifi_snr_stats.snr_math_average = 0;
		memset(AC_WTP[WTPID]->wtp_wifi_snr_stats.snr,0,10);
	}
	//ap sta info report area
	{
		AC_WTP[WTPID]->ap_sta_report_switch = gSTAREPORTSWITCH; /*wcl modify for globle variable*/
		AC_WTP[WTPID]->ap_sta_report_interval = gSTAREPORTINTERVAL;/*wcl modify for globle variable*/
	}
	wid_apstatsinfo_init(WTPID);	//xiaodawei modify for apstatsinfo init, 20110107

	/*fengwenchao add 20111117 for GM-3*/
	{
		AC_WTP[WTPID]->heart_time.heart_statistics_switch = 0;
		AC_WTP[WTPID]->heart_time.heart_statistics_collect = 300;
		AC_WTP[WTPID]->heart_time.heart_time_delay = 0;
		AC_WTP[WTPID]->heart_time.heart_lose_pkt = 0;
		AC_WTP[WTPID]->heart_time.heart_transfer_pkt = 0;
		AC_WTP[WTPID]->heart_time.heart_time_value_length = 0;
		AC_WTP[WTPID]->heart_time.heart_time_avarge = 0;
		AC_WTP[WTPID]->heart_time.heart_time_value_head = NULL;
	}
	/*fengwenchao add end*/
#if 0
	for(i=0; i<TOTAL_AP_IF_NUM; i++)
	{
		AC_WTP[WTPID]->apstatsinfo[i].radioId = TOTAL_AP_IF_NUM+1;
		AC_WTP[WTPID]->apstatsinfo[i].type = TOTAL_AP_IF_NUM+1;
		AC_WTP[WTPID]->apstatsinfo[i].wlanId = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_bytes = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_bytes = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_drop = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_drop = 0;

		
		AC_WTP[WTPID]->apstatsinfo[i].rx_packets = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_packets = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_errors = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_errors = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_rate = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_rate = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_crcerr = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_badcrypt = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_badmic = 0;
		AC_WTP[WTPID]->apstatsinfo[i].ast_rx_phyerr = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_error_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_error_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_drop_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_drop_frame = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_band= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_band= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_unicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_unicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_broadcast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_broadcast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_multicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_multicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_unicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_unicast = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_broadcast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_broadcast = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_multicast= 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_multicast = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_retry = 0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_pkt_mgmt =0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_pkt_mgmt =0;
		AC_WTP[WTPID]->apstatsinfo[i].rx_mgmt =0;
		AC_WTP[WTPID]->apstatsinfo[i].tx_mgmt =0;

	}
	#endif
	AC_WTP[WTPID]->rx_bytes = 0;
	AC_WTP[WTPID]->tx_bytes = 0;
	AC_WTP[WTPID]->rx_bytes_before = 0;
	AC_WTP[WTPID]->tx_bytes_before = 0;

	//set wifi_info 0
	{
		AC_WTP[WTPID]->wifi_extension_info.cpu_trap_flag = 0;
		AC_WTP[WTPID]->wifi_extension_info.mem_trap_flag = 0;
		AC_WTP[WTPID]->wifi_extension_info.temp_trap_flag = 0;
		memset(AC_WTP[WTPID]->wifi_extension_info.wifi_trap_flag,0,AP_WIFI_IF_NUM);
		
		//AC_WTP[WTPID]->wifi_extension_reportswitch = 0; /*wcl modify for globle variable*/
		AC_WTP[WTPID]->wifi_extension_reportinterval = gWIFIEXTENSIONREPORTINTERVAL;//sz change 1 to 3 0630 /*wcl modify for globle variable*/
		//AC_WTP[WTPID]->wifi_extension_info.reportswitch = 0;
		//AC_WTP[WTPID]->wifi_extension_info.reportinterval = 3;//sz change 1 to 3 0630
		AC_WTP[WTPID]->wifi_extension_info.cpu = 0;
		AC_WTP[WTPID]->collect_time= cpu_mem_collect_time;			//xiaodawei change 3500 to 300, 20101211	/*wcl modify for globle variable*/			
		AC_WTP[WTPID]->wifi_extension_info.tx_mgmt = 0;
		AC_WTP[WTPID]->wifi_extension_info.rx_mgmt = 0;
		AC_WTP[WTPID]->wifi_extension_info.tx_packets = 0;
		AC_WTP[WTPID]->wifi_extension_info.tx_errors = 0;
		AC_WTP[WTPID]->wifi_extension_info.tx_retry = 0;
		AC_WTP[WTPID]->wifi_extension_info.ipmode = 1;
		AC_WTP[WTPID]->wifi_extension_info.memoryall = 0;
		AC_WTP[WTPID]->wifi_extension_info.memoryuse = 0;
		AC_WTP[WTPID]->wifi_extension_info.flashall = 0;
		AC_WTP[WTPID]->wifi_extension_info.wifi_snr = 0;
		/*fengwenchao add 20120314 for onlinebug-162*///qiuchen copy from v1.3
		memset(AC_WTP[WTPID]->wifi_extension_info.wifi_snr_new,0,L_RADIO_NUM);
		memset(AC_WTP[WTPID]->wifi_extension_info.wifi_noise_new,0,L_RADIO_NUM);
		/*fengwenchao add end*/
		AC_WTP[WTPID]->wifi_extension_info.flashempty = 0;
		AC_WTP[WTPID]->wifi_extension_info.eth_count = 0;
		AC_WTP[WTPID]->wifi_extension_info.ath_count = 0;
		AC_WTP[WTPID]->wifi_extension_info.temperature = 0;
		AC_WTP[WTPID]->wifi_extension_info.wifi_count = 0;
		AC_WTP[WTPID]->wifi_extension_info.tx_unicast = 0;
		AC_WTP[WTPID]->wifi_extension_info.tx_broadcast = 0;
		AC_WTP[WTPID]->wifi_extension_info.tx_multicast = 0;
		AC_WTP[WTPID]->wifi_extension_info.tx_drop = 0;
		AC_WTP[WTPID]->wifi_extension_info.rx_unicast = 0;
		AC_WTP[WTPID]->wifi_extension_info.rx_broadcast = 0;
		AC_WTP[WTPID]->wifi_extension_info.rx_multicast = 0;
		AC_WTP[WTPID]->wifi_extension_info.rx_drop = 0;
		AC_WTP[WTPID]->wifi_extension_info.wpi_replay_error = 0;
		AC_WTP[WTPID]->wifi_extension_info.wpi_decryptable_error = 0;
		AC_WTP[WTPID]->wifi_extension_info.wpi_mic_error = 0;
		AC_WTP[WTPID]->wifi_extension_info.disassoc_unnormal = 0;
		AC_WTP[WTPID]->wifi_extension_info.rx_assoc_norate = 0;
		AC_WTP[WTPID]->wifi_extension_info.rx_assoc_capmismatch = 0;
		AC_WTP[WTPID]->wifi_extension_info.assoc_invaild = 0;
		AC_WTP[WTPID]->wifi_extension_info.reassoc_deny = 0;
		memset(AC_WTP[WTPID]->wifi_extension_info.wifi_state,0,AP_WIFI_IF_NUM);
		memset(AC_WTP[WTPID]->wifi_extension_info.eth_updown_time,0,AP_ETH_IF_NUM);
		//memset(AC_WTP[WTPID]->wifi_extension_info.ath_updown_time,0,AP_ATH_IF_NUM);
		memset(AC_WTP[WTPID]->wifi_extension_info.ath_if_info,0,sizeof(wid_ap_ath_info));
	}
	//ap interface info
	{
		AC_WTP[WTPID]->apifinfo.report_switch = gINFOREPORTSWITCH; /*wcl modify  for globle variable*/
		AC_WTP[WTPID]->apifinfo.report_interval = gINFOREPORTINTERVAL;//sz change 1 to 3 0630 /*wcl modify for globle variable*/
		//AC_WTP[WTPID]->apifinfo.eth_num = 1;    //fengwenchao modify 20110325
		AC_WTP[WTPID]->apifinfo.wifi_num = 1;
		memset(AC_WTP[WTPID]->apifinfo.eth,0,AP_ETH_IF_NUM);
		memset(AC_WTP[WTPID]->apifinfo.wifi,0,AP_WIFI_IF_NUM);
		unsigned char jj=0;
		for(jj = 0;jj<AP_ETH_IF_NUM;jj++){
			AC_WTP[WTPID]->apifinfo.eth[jj].eth_rate = gAPIFINFOETH_RATE[jj]; /*wcl modify for globle variable*/
			AC_WTP[WTPID]->apifinfo.eth[jj].eth_mtu = gAPIFINFOETH_MTU[jj];   //fengwenchao add 20110127 for XJDEV-32 from 2.0 /*wcl modify for globle variable*/
		}
	}
	//ap cpu mem statistics
	memset(AC_WTP[WTPID]->apcminfo.cpu_value,0,10);
	memset(AC_WTP[WTPID]->apcminfo.mem_value,0,10);
	AC_WTP[WTPID]->apcminfo.cpu_average = 0;
	AC_WTP[WTPID]->apcminfo.cpu_times = 0;
	AC_WTP[WTPID]->apcminfo.cpu_peak_value = 0;
	AC_WTP[WTPID]->apcminfo.mem_average = 0;
	AC_WTP[WTPID]->apcminfo.mem_times = 0;
	AC_WTP[WTPID]->apcminfo.mem_peak_value = 0;
	/*fengwenchao add 20120314 for onlinebug-162*///qiuchen copy from v1.3
	memset(AC_WTP[WTPID]->apcminfo.wifi_snr,0,L_RADIO_NUM);
	/*fengwenchao add end*/
	//for mib info
	AC_WTP[WTPID]->mib_info.dos_def_switch = 0;
	AC_WTP[WTPID]->mib_info.igmp_snoop_switch = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		AC_WTP[WTPID]->mib_info.wlan_l2isolation[i].wlanid = 0;
		AC_WTP[WTPID]->mib_info.wlan_l2isolation[i].l2_isolation_switch = 0;
	}
	AC_WTP[WTPID]->wid_sample_throughput.past_uplink_throughput = 0;
	AC_WTP[WTPID]->wid_sample_throughput.current_uplink_throughput = 0;
	AC_WTP[WTPID]->wid_sample_throughput.past_downlink_throughput = 0;
	AC_WTP[WTPID]->wid_sample_throughput.current_downlink_throughput = 0;
	AC_WTP[WTPID]->wid_sample_throughput.uplink_rate = 0;
	AC_WTP[WTPID]->wid_sample_throughput.downlink_rate = 0;
	AC_WTP[WTPID]->wifi_extension_reportswitch = g_AC_ALL_EXTENTION_INFORMATION_SWITCH;//nl
	
	AC_WTP[WTPID]->trap_collect_time = 3500;
	AC_WTP[WTPID]->cputrap_resend_times = 0;
	AC_WTP[WTPID]->memtrap_resend_times = 0;
	AC_WTP[WTPID]->rogueaptrap_resend_times = 0;
	AC_WTP[WTPID]->rogueteminaltap_resend_times = 0;
	AC_WTP[WTPID]->cpu_resend_times = 0;
	AC_WTP[WTPID]->cpu_clear_resend_times = 0;
	AC_WTP[WTPID]->memtrap_clear_resend_times = 0;
	AC_WTP[WTPID]->ap_temp_resend_times = 0;
	AC_WTP[WTPID]->ap_temp_clear_resend_times = 0;
}

void merge_wids_list(wid_wids_device *pdestlist, wid_wids_device **psrclist,int wtpid)
{
	if((pdestlist == NULL)||(pdestlist->wids_device_info == NULL)||(pdestlist->count == 0))
	{
//		printf("merge_wids_list dest parameter error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"merge_wids_list dest parameter error\n");
		return ;
	}	
	
	if(((*psrclist) == NULL)||((*psrclist)->wids_device_info == NULL)||((*psrclist)->count== 0))
	{
//		printf("merge_wids_list src parameter error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"merge_wids_list src parameter error\n");
		CW_FREE_OBJECT(*psrclist);
		return;
	}	
	int now_rssi = 0;   //fengwenchao add 20110507
	int i = 0;       //fengwenchao add 20110507
	struct tag_wids_device_ele *phead_trap = pdestlist->wids_device_info;     //fengwenchao add 20110507
	struct tag_wids_device_ele *pnode_trap = pdestlist->wids_device_info;     //fengwenchao add 20110507
	if(pdestlist->wids_device_info->bssid!=NULL){
		unsigned char *name = NULL;
		name= (unsigned char *)malloc(MAC_LEN+1);
		if(name!=NULL){
			memset(name,0,MAC_LEN+1);
			memcpy(name,pdestlist->wids_device_info->bssid,MAC_LEN);  

			time_t now;
			time(&now);
			wid_syslog_warning("<warning>wtp[%d] find attack from %02x:%02x:%02x:%02x:%02x:%02x\n",wtpid,name[0],name[1],name[2],name[3],name[4],name[5]);
			syslog(LOG_WARNING|LOG_LOCAL7, "WTP %d,WTP MAC:"MACSTR",WTP IP:%s,find attack from MAC:"MACSTR", at Time:%s\n",\
				wtpid,MAC2STR(AC_WTP[wtpid]->WTPMAC),AC_WTP[wtpid]->WTPIP,MAC2STR(name),ctime(&now));
			if(gtrapflag>=25){
				if(AC_WTP[wtpid]->find_wids_attack_flag == 1)  //fengwenchao add 20110221
					{
						/*fengwenchao add 20110507*/
						for(i = 0; ((phead_trap != NULL)||(i < pdestlist->count));i++)
						{
							if(phead_trap->rssi > now_rssi)
							{
								now_rssi = phead_trap->rssi;
								pnode_trap = phead_trap;
							}
							phead_trap = phead_trap->next;
						}
						/*fengwenchao add end*/
						wid_dbus_trap_wtp_find_wids_attack(wtpid,pnode_trap);	   //fengwenchao modify 20110509
						AC_WTP[wtpid]->find_wids_attack_flag = 0;   //fengwenchao add 20110221
					}
			}
			CW_FREE_OBJECT(name);

		}
	}else
	{
		return;
	}
	struct tag_wids_device_ele  *pnode = (*psrclist)->wids_device_info;
	struct tag_wids_device_ele  *pnext = pnode->next;
	CWBool isfind = CW_FALSE;
	
	isfind = find_elem_in_wids_list(pdestlist,pnode);

	if(isfind == 0) //add element
	{
		insert_elem_into_wids_list(pdestlist,pnode);
		
	}
	else 
	{
		update_elem_in_wids_list(pdestlist,pnode);

		CW_FREE_OBJECT(pnode);

	}

	while(pnext != NULL)
	{
		pnode = pnext ->next;

		isfind = find_elem_in_wids_list(pdestlist,pnext);
			
		if(isfind == 0) //add element
		{
			insert_elem_into_wids_list(pdestlist,pnext);
			
		}
		else 
		{
			update_elem_in_wids_list(pdestlist,pnext);
		
			CW_FREE_OBJECT(pnext);
		
		}

		pnext = pnode;
		
	}

	CW_FREE_OBJECT(*psrclist);
	
	//printf("merge_ap_list to end\n");
	
	
	return ;
}

CWBool find_elem_in_wids_list(wid_wids_device  *paplist,struct tag_wids_device_ele *elem)
{
	if((elem == NULL)||((paplist) == NULL)||((paplist)->wids_device_info == NULL)||((paplist)->count== 0))
	{
//		printf("find_elem_in_wids_list parameter error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"find_elem_in_wids_list parameter error\n");
		return CW_FALSE;
	}	


	
	struct tag_wids_device_ele *pnode = paplist->wids_device_info;
	struct tag_wids_device_ele *pnext = pnode->next;

	if(memcmp((pnode->bssid),elem->bssid,6) == 0)
	{
		
		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->bssid),elem->bssid,6) == 0)
		{

			return CW_TRUE;

		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}

	return CW_FALSE;//insert success

}

CWBool update_elem_in_wids_list(wid_wids_device  *paplist,struct tag_wids_device_ele *elem)
{
//	printf("update_elem_in_wids_list\n");

	if((elem == NULL)||((paplist) == NULL)||((paplist)->wids_device_info == NULL)||((paplist)->count== 0))
	{
//		printf("update_elem_in_wids_list parameter error\n");
		wid_syslog_debug_debug(WID_DEFAULT,"update_elem_in_wids_list parameter error\n");
		return CW_FALSE;
	}	


	
	struct tag_wids_device_ele *pnode = paplist->wids_device_info;
	struct tag_wids_device_ele *pnext = pnode->next;

	if(memcmp((pnode->bssid),elem->bssid,6) == 0)
	{
		pnode->attackcount++;
		pnode->attacktype = elem->attacktype;
		pnode->frametype = elem->frametype;

		pnode->lst_attack = elem->fst_attack;
		
			
		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->bssid),elem->bssid,6) == 0)
		{
			pnext->attackcount++;
			pnext->attacktype= elem->attacktype;
			pnext->frametype = elem->frametype;
			
			pnext->lst_attack = elem->fst_attack;

			return CW_TRUE;

		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}

	return CW_FALSE;//insert success

}
CWBool insert_elem_into_wids_list(wid_wids_device *paplist,struct tag_wids_device_ele *elem)
{
	wid_syslog_debug_debug(WID_DEFAULT,"insert_elem_into_wids_list\n");
	
	if((elem == NULL)||(paplist == NULL))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"insert_elem_into_ap_list parameter error\n");
		return CW_FALSE;
	}	

	if(paplist->count== 0)
	{
		paplist->wids_device_info = elem;
		elem->next = NULL;
		paplist->count++;	
		return CW_TRUE;
	}


	elem->next = paplist->wids_device_info;
	paplist->wids_device_info= elem;
	paplist->count++;	

	return CW_TRUE;//insert success
	
}
void delete_wids_list(wid_wids_device **paplist)
{
	if(((*paplist) == NULL)||((*paplist)->count == 0))
	{
		CW_FREE_OBJECT(*paplist);
		return;
	}

	struct tag_wids_device_ele *phead = NULL;
	struct tag_wids_device_ele *pnext = NULL;
	phead = (*paplist)->wids_device_info;
	(*paplist)->wids_device_info = NULL;

	while(phead != NULL)
	{	
		
		pnext = phead->next;

		CW_FREE_OBJECT(phead);

		phead = pnext;

	}

	(*paplist)->count = 0;
	
	CW_FREE_OBJECT(*paplist);

}

wid_wids_device * wid_check_wids_device_all()
{
	wid_wids_device * wids_list = wid_get_wids_device_list();
	//printf("wid_check_wids_device_all\n");
	//display_wids_info_list(wids_list);
	if(wids_list == NULL)
	{
		return NULL;
	}
	
	return wids_list;	

}

wid_wids_device * wid_get_wids_device_list()
{
	int i = 0;
	CWBool ret = CW_FALSE;
	struct tag_wids_device_ele *phead = NULL;
	struct tag_wids_device_ele *Pnode = NULL;
	
	wid_wids_device *create_wids_info;
	CW_CREATE_OBJECT_ERR(create_wids_info, wid_wids_device, return NULL;);		
	create_wids_info->count = 0;
	create_wids_info->wids_device_info = NULL;

	for(i=0; i<WTP_NUM; i++)
	{
		
		if ((AC_WTP[i] != NULL))
		{
			
			if(AC_WTP[i]->wids_device_list != NULL)
			{
				CWThreadMutexLock(&(gWTPs[i].WIDSThreadMutex));
				
				phead = AC_WTP[i]->wids_device_list->wids_device_info;

				while(phead != NULL)
				{
					ret = check_elem_in_wids_list(create_wids_info,phead);
					if(ret == CW_FALSE)
					{
						if(find_elem_in_wids_list(wids_ignore_list,phead) == CW_FALSE)
						{
							Pnode = create_wids_elem(phead);
							if(Pnode != NULL)
							{
								if(insert_elem_into_wids_list_head(create_wids_info,Pnode) == CW_FALSE)
								{

									CW_FREE_OBJECT(Pnode);
								}
							}
						}
						
					}
					phead = phead->next;
				}
				CWThreadMutexUnlock(&(gWTPs[i].WIDSThreadMutex));
			}		
			
		}		
	}


	if((create_wids_info->wids_device_info == NULL)||(create_wids_info->count == 0))
	{
		CW_FREE_OBJECT(create_wids_info);
		return NULL;
	}

	
	return create_wids_info;

}

CWBool check_elem_in_wids_list(wid_wids_device *paplist,struct tag_wids_device_ele *elem)
{
	if((elem == NULL)||((paplist) == NULL)||((paplist)->wids_device_info == NULL)||((paplist)->count == 0))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"check_elem_in_wids_list parameter error\n");
		return CW_FALSE;
	}	

	
	struct tag_wids_device_ele *pnode = (paplist)->wids_device_info;
	struct tag_wids_device_ele *pnext = pnode->next;

	if(memcmp((pnode->bssid),elem->bssid,6) == 0)
	{

		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->bssid),elem->bssid,6) == 0)
		{
			return CW_TRUE;
		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}

	return CW_FALSE;

}

struct tag_wids_device_ele * create_wids_elem(struct tag_wids_device_ele *apelem)
{
	struct tag_wids_device_ele *neighborapelem = NULL;
	
	CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct tag_wids_device_ele), return NULL;);

	
	memcpy(neighborapelem->bssid,apelem->bssid, 6);
	
	neighborapelem->attacktype = apelem->attacktype;

	neighborapelem->frametype = apelem->frametype;
	neighborapelem->attackcount = apelem->attackcount;
	neighborapelem->fst_attack = apelem->fst_attack;
	neighborapelem->lst_attack = apelem->lst_attack;

	neighborapelem->channel = apelem->channel;
	neighborapelem->rssi = apelem->rssi ;
	memcpy(neighborapelem->vapbssid,apelem->vapbssid,6);

	neighborapelem->next = NULL;	

	return neighborapelem;
	
}
CWBool insert_elem_into_wids_list_head(wid_wids_device *paplist,struct tag_wids_device_ele *elem)
{
	if((elem == NULL)||(paplist == NULL))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"insert_elem_into_wids_list_head parameter error\n");
		return CW_FALSE;
	}	

	if(paplist->count == 0)
	{
		paplist->wids_device_info = elem;
		paplist->count++;	
		return CW_TRUE;
	}
	
	struct tag_wids_device_ele *pnode = paplist->wids_device_info;

	paplist->wids_device_info= elem;
	elem->next = pnode;
	paplist->count++;	

	return CW_TRUE;


}

wid_wids_device * create_wids_info_list(int count)
{
	wid_wids_device *create_wids_info;
	int i = 0;
	CW_CREATE_OBJECT_ERR(create_wids_info, wid_wids_device, return NULL;);	
				
	create_wids_info->count = count;
	create_wids_info->wids_device_info = NULL;
	
	struct tag_wids_device_ele *neighborapelem = NULL;
	struct tag_wids_device_ele *phead = NULL;

	for(i=0; i<create_wids_info->count; i++)
	{
		CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct tag_wids_device_ele), return NULL;);		
		
		char str[3][6] = {{"111111"},{"222222"},{"333333"}};
		memcpy(neighborapelem->bssid,str[i], 6);

		neighborapelem->bssid[0] = 1;
		neighborapelem->bssid[1] = 1;
		neighborapelem->bssid[2] = 1;
		neighborapelem->bssid[3] = 1;
		neighborapelem->bssid[4] = 1;
		neighborapelem->bssid[5] = 1;
		
		neighborapelem->attackcount = 110;

		neighborapelem->frametype = 1;
		neighborapelem->attacktype = 1;
		time(&neighborapelem->fst_attack);
		time(&neighborapelem->fst_attack);

		neighborapelem->next = NULL;

		if(i== 0)
		{
			//printf("parse first ap info\n");
			create_wids_info->wids_device_info = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		else
		{
			//printf("parse more ap info\n");
			phead->next = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		//printf("######002####\n");
	}	

	return create_wids_info;
	//display_ap_info_list(create_ap_info);


}

void display_wids_info_list(wid_wids_device *paplist)
{

	if((paplist == NULL)||(paplist->wids_device_info== NULL)||(paplist->count== 0))
	{
		printf("display_ap_info_list parameter error\n");
		return;
	}
	
	int i = 0;
	int j = 0;

	struct tag_wids_device_ele *phead = paplist->wids_device_info;
		
	printf("## display_wids_info_list count is = %d: ##\n",paplist->count);
	for(i=0; i<paplist->count; i++)
	{
		printf("## the count i = %d##\n",i);
		printf("mac = ");
		for(j=0; j<6; j++)
		{
			printf("%02x", phead->bssid[j]);
		}
		printf("\n");

		phead = phead->next;
	}
}

int add_ipip_tunnel(unsigned int BSSIndex)
{
	
	ex_ip_info iptunnelinfo;
	iptunnelinfo.sip = 0;
	iptunnelinfo.dip = 0;
	
	int wtpid = 0;
	int l_radioid = 0;
	int wlanid = 0;
	//int reason = 0;
	
	memset(iptunnelinfo.if_name,0,ETH_IF_NAME_LEN);
	memset(iptunnelinfo.wtpmac,0,MAC_LEN);


	wtpid = BSSIndex/(L_BSS_NUM*L_RADIO_NUM);
	l_radioid = AC_BSS[BSSIndex]->Radio_L_ID;
	wlanid = AC_BSS[BSSIndex]->WlanID;
	
//	printf(" add_ipip_tunnel radio%d-%d.%d\n",wtpid,l_radioid,wlanid);
	
//	snprintf(iptunnelinfo.if_name,ETH_IF_NAME_LEN-1,"radio%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	if(local)
		snprintf((char*)iptunnelinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	else
		snprintf((char*)iptunnelinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpid,l_radioid,wlanid);
	memcpy(iptunnelinfo.wtpmac, AC_WTP[wtpid]->WTPMAC, MAC_LEN);
	
	//make sip and dip
	/*fengwenchao modify begin 20110525*/
	if((gWTPs[wtpid].interfaceIndex < gMaxInterfacesCount)&&(gWTPs[wtpid].interfaceIndex >= 0))
	{
		struct sockaddr_in	*sin = (struct sockaddr_in *)((struct sockaddr*) &(gInterfaces[gWTPs[wtpid].interfaceIndex].addr));
		iptunnelinfo.dip = sin->sin_addr.s_addr;
	}
	else
	{
		wid_syslog_err(" gWTPs[%d].interfaceIndex = %d , is not  legal !\n",wtpid,gWTPs[wtpid].interfaceIndex);
		return -1;
	}
	/*fengwenchao add end*/

	char apip[WID_SYSTEM_CMD_LENTH];
	char *delim=":";
	char *papip = NULL;
	strcpy(apip,AC_WTP[wtpid]->WTPIP);
	papip = strtok(apip,delim);

//	printf("ip :%s \n",papip);

	
	iptunnelinfo.sip = inet_addr(papip);	

//	printf("ifname:%s dip:%d sip:%d\n",iptunnelinfo.if_name,iptunnelinfo.dip,iptunnelinfo.sip);
//	printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n",iptunnelinfo.wtpmac[0],iptunnelinfo.wtpmac[1],iptunnelinfo.wtpmac[2],iptunnelinfo.wtpmac[3],iptunnelinfo.wtpmac[4],iptunnelinfo.wtpmac[5]);

	int ret = -1;
	
	int fd = open("/dev/wifi0", O_RDWR);

	if(fd < 0)
	{
		return -1;//create failure
	}

	
	ret = ioctl(fd, WIFI_IOC_IP_ADD, &iptunnelinfo);

	close(fd);
	
	if(ret < 0)
	{
		return -1;
	}
	
	return 0;

}

int delete_ipip_tunnel(unsigned int BSSIndex)
{
	
	ex_ip_info iptunnelinfo;
	iptunnelinfo.sip = 0;
	iptunnelinfo.dip = 0;
	
	int wtpid = 0;
	int l_radioid = 0;
	int wlanid = 0;
	//int reason = 0;
	
	memset(iptunnelinfo.if_name,0,ETH_IF_NAME_LEN);
	memset(iptunnelinfo.wtpmac,0,MAC_LEN);


	wtpid = BSSIndex/(L_BSS_NUM*L_RADIO_NUM);
	l_radioid = AC_BSS[BSSIndex]->Radio_L_ID;
	wlanid = AC_BSS[BSSIndex]->WlanID;
	
//	printf(" delete_ipip_tunnel radio%d-%d.%d\n",wtpid,l_radioid,wlanid);
	
	//snprintf(iptunnelinfo.if_name,ETH_IF_NAME_LEN-1,"radio%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	if(local)
		snprintf((char*)iptunnelinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,wtpid,l_radioid,wlanid);
	else
		snprintf((char*)iptunnelinfo.if_name,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,wtpid,l_radioid,wlanid);
	memcpy(iptunnelinfo.wtpmac, AC_WTP[wtpid]->WTPMAC, MAC_LEN);
	
	//make sip and dip
	/*fengwenchao modify begin 20110525*/
	if((gWTPs[wtpid].interfaceIndex < gMaxInterfacesCount)&&(gWTPs[wtpid].interfaceIndex >= 0))
	{
		struct sockaddr_in	*sin = (struct sockaddr_in *)((struct sockaddr*) &(gInterfaces[gWTPs[wtpid].interfaceIndex].addr));
		iptunnelinfo.dip = sin->sin_addr.s_addr;
	}
	else
	{
		wid_syslog_err(" gWTPs[%d].interfaceIndex = %d , is not  legal !\n",wtpid,gWTPs[wtpid].interfaceIndex);
	}
	/*fengwenchao add end*/
	
	char apip[WID_SYSTEM_CMD_LENTH];
	char *delim=":";
	char *papip = NULL;
	strcpy(apip,AC_WTP[wtpid]->WTPIP);
	papip = strtok(apip,delim);

//	printf("ip :%s \n",papip);

	
	iptunnelinfo.sip = inet_addr(papip);	

//	printf("ifname:%s dip:%d sip:%d\n",iptunnelinfo.if_name,iptunnelinfo.dip,iptunnelinfo.sip);
//	printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n",iptunnelinfo.wtpmac[0],iptunnelinfo.wtpmac[1],iptunnelinfo.wtpmac[2],iptunnelinfo.wtpmac[3],iptunnelinfo.wtpmac[4],iptunnelinfo.wtpmac[5]);
	
	int ret = -1;
	
	int fd = open("/dev/wifi0", O_RDWR);

	if(fd < 0)
	{
		return -1;//create failure
	}

	
	ret = ioctl(fd, WIFI_IOC_IP_DEL, &iptunnelinfo);

	close(fd);
	
	if(ret < 0)
	{
		return -1;
	}
	
	return 0;

}

int wid_set_neighbordead_intervalt(unsigned int wtpid, int neighbordead_interval)
{
	msgq msg;	
//	struct msgqlist *elem;

	memset((char*)&msg, 0, sizeof(msg));
	msg.mqid = wtpid%THREAD_NUM+1;
	msg.mqinfo.WTPID = wtpid;
	msg.mqinfo.type = CONTROL_TYPE;
	msg.mqinfo.subtype = WTP_S_TYPE;
	msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_NEIGHBORDEAD_INTERVAL;
	

	if(AC_WTP[wtpid]->WTPStat == 5)
	{	
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
			wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}//delete unuseful cod
	/*else
	{
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(wtpid, elem);
		
	}*/

	return 0;
	
}
int wid_set_radio_auto_channel_able(unsigned int wtpid,unsigned int l_radioid,unsigned int able)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
	if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->auto_channel == able)
	{
		return 0;
	}
	else
	{
		sprintf(apcmd,"echo %d > /proc/sys/dev/wifi%d/nonoverlapping",able,l_radioid);
		//printf("apcmd %s\n",apcmd);
		wid_syslog_debug_debug(WID_DEFAULT,"apcmd %s\n",apcmd);

		ret = wid_radio_set_extension_command(wtpid,apcmd);
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->auto_channel = able;
		return 0;
	}
	
}
//enter run state,process radio diversity&txantenna
int wid_set_radio_diversity(unsigned int wtpid,unsigned int l_radioid,unsigned int able)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"echo %d > /proc/sys/dev/wifi%d/diversity",able,l_radioid);
	//printf("wid_set_radio_diversity apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_radio_diversity apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
int wid_set_radio_txantenna(unsigned int wtpid,unsigned int l_radioid,unsigned int able)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"echo %d > /proc/sys/dev/wifi%d/txantenna",able,l_radioid);
	//printf("wid_set_radio_txantenna apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_radio_txantenna apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}

int wid_set_radio_diversity_txantenna_after_run(unsigned int wtpid,unsigned int l_radioid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	if(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL)
	{
		return -1;
	}
	
	int diversity = 0;
	int txantenna = 0;
	diversity = AC_WTP[wtpid]->WTP_Radio[l_radioid]->diversity;
	txantenna = AC_WTP[wtpid]->WTP_Radio[l_radioid]->txantenna;
	int code_result = wid_parse_wtp_code_for_radio_set(wtpid);
	/*1 means hardware right; 2 means the second wifi need to be changed;0 means all wifi need to be changed*/
	/*this will be change after ap hardware changed soon  sz 09-07-17*/
	if(code_result == 0)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->txantenna == 1)
		{
			txantenna = 2;//change hardware mistake
			wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
		}
		else if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->txantenna == 2)
		{
			txantenna = 1;//change hardware mistake
			wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
		}
		else
		{
			txantenna = 0;
			wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
		}	
	}
	else if(code_result == 1)
	{
		wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
	}
	else if(code_result == 2)
	{
		if(l_radioid == 1)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->txantenna == 1)
			{
				txantenna = 2;//change hardware mistake
				wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
			}
			else if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->txantenna == 2)
			{
				txantenna = 1;//change hardware mistake
				wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
			}
			else
			{
				wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
			}
		}
		else
		{
			wid_set_radio_txantenna(wtpid,l_radioid,txantenna);
		}
	}

	if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->diversity != 1)
	{
		wid_set_radio_diversity(wtpid,l_radioid,diversity);
	}
	
	return 0;	
}
int wid_set_radio_diversity_txantenna_after_run_new(unsigned int wtpid,unsigned int l_radioid)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	if(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL)
	{
		return -1;
	}
	
	int diversity = 0;
	int txantenna = 0;
	diversity = AC_WTP[wtpid]->WTP_Radio[l_radioid]->diversity;
	txantenna = AC_WTP[wtpid]->WTP_Radio[l_radioid]->txantenna;
	
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"cd / && setantenna wifi%d diversity %d",l_radioid,diversity);
//	printf("wid_set_radio_diversity_txantenna_after_run_new diversity apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_radio_txantenna apcmd diversity %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);

	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"cd / && setantenna wifi%d tx %d",l_radioid,txantenna);
//	printf("wid_set_radio_diversity_txantenna_after_run_new apcmd txantenna%s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_radio_txantenna apcmd txantenna %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);

	return 0;	
}

int wid_parse_wtp_code_for_radio_set(unsigned int wtpid)
{
	int code_result = 0;
	
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	else if(AC_WTP[wtpid]->APCode == NULL)
	{
		return -1;
	}
	else
	{
		int codelen = strlen(AC_WTP[wtpid]->APCode);
		
		if(AC_WTP[wtpid]->APCode[codelen-1] == 'N')
		{
			code_result = 1;
			return code_result;
		}
		else
		{
			if(codelen == 4)
			{
				if((memcmp(AC_WTP[wtpid]->APCode,"2010",4) == 0)||(memcmp(AC_WTP[wtpid]->APCode,"1010",4) == 0))
				{
					code_result = 1;
					return code_result;
				}
			}
			else if(codelen == 6)
			{
				if(memcmp(AC_WTP[wtpid]->APCode,"2010V2",6) == 0)
				{
					code_result = 1;
					return code_result;
				}
			}
			
			if((AC_WTP[wtpid]->APCode[codelen-1] == 'H')||(AC_RADIO[wtpid*L_RADIO_NUM]->ishighpower == 1))
			{
				code_result = 2;
				return code_result;
			}
			else
			{
				code_result = 1;        //zhangshu change code_result from 0 to 1,2010-10-29
				return code_result;
			}
		}
	}
	//printf("ap code %s ;result %d\n",AC_WTP[wtpid]->APCode,code_result);
	return code_result;
}
int wid_set_ap_reboot(unsigned int WtpID)
{
	wid_syslog_debug_debug(WID_DEFAULT,"wtpid %d reboot\n",WtpID);
	wid_trap_remote_restart(WtpID);
	WID_SUSPEND_WTP(WtpID);
	AC_WTP[WtpID]->quitreason = WTP_UNUSED;
	return 0;
}
int wid_set_bss_traffic_limit(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char able)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(able == 1)
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_every_node_flag %d",l_radioid,wlanid,able);
	else if(able == 0){
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_vap_flag %d;"
		"autelan traffic_limit ath.%d-%d set_every_node_flag %d;",
		l_radioid,wlanid,able,\
		l_radioid,wlanid,able);
		/*
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_every_node_flag %d;"
		"autelan traffic_limit ath.%d-%d set_every_node %d;"
		"autelan traffic_limit ath.%d-%d set_every_node_send %d;"
		"autelan traffic_limit ath.%d-%d set_vap %d;"
		"autelan traffic_limit ath.%d-%d set_vap_send %d;",
		l_radioid,wlanid,able,\
		l_radioid,wlanid,STA_DEFAULT_TRAFFIC_LIMIT,\
		l_radioid,wlanid,STA_DEFAULT_TRAFFIC_LIMIT,\
		l_radioid,wlanid,STA_DEFAULT_TRAFFIC_LIMIT,\
		l_radioid,wlanid,STA_DEFAULT_TRAFFIC_LIMIT);
		*/
	}
	
//	printf("wid_set_bss_traffic_limit apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
int wid_set_bss_traffic_limit_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(value == 0)
		value = STA_DEFAULT_TRAFFIC_LIMIT;
	if(issend == 1)
	{
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_vap_flag 1;autelan traffic_limit ath.%d-%d set_vap_send %d",l_radioid,wlanid,l_radioid,wlanid,value);
	}
	else if(issend == 0)
	{		
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_vap_flag 1;autelan traffic_limit ath.%d-%d set_vap %d",l_radioid,wlanid,l_radioid,wlanid,value);
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_value error\n");
	}
//	printf("wid_set_bss_traffic_limit_value apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_value apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
/*fengwenchao add for AXSSZFI-1374*/
int wid_set_bss_traffic_limit_cancel_average_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(issend == 1)
	{
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_every_node_flag 0;autelan traffic_limit ath.%d-%d set_every_node_send %d",l_radioid,wlanid,l_radioid,wlanid,value);
	}
	else if(issend == 0)
	{		
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_every_node_flag 0;autelan traffic_limit ath.%d-%d set_every_node %d",l_radioid,wlanid,l_radioid,wlanid,value);
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_cancel_average_value error\n");
	}
//	printf("wid_set_bss_traffic_limit_average_value apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_cancel_average_value apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;

}
/*fengwenchao add end*/
int wid_set_bss_traffic_limit_average_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(value == 0)
		value = STA_DEFAULT_TRAFFIC_LIMIT;
	if(issend == 1)
	{
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_every_node_flag 1;autelan traffic_limit ath.%d-%d set_every_node_send %d",l_radioid,wlanid,l_radioid,wlanid,value);
	}
	else if(issend == 0)
	{		
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_every_node_flag 1;autelan traffic_limit ath.%d-%d set_every_node %d",l_radioid,wlanid,l_radioid,wlanid,value);
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_average_value error\n");
	}
//	printf("wid_set_bss_traffic_limit_average_value apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_average_value apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
int wid_set_bss_traffic_limit_sta_value(unsigned int wtpid,
												unsigned int l_radioid,
												unsigned char wlanid,
												unsigned char mac0,
												unsigned char mac1,
												unsigned char mac2,
												unsigned char mac3,
												unsigned char mac4,
												unsigned char mac5,
												unsigned int value,
												unsigned char issend)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(value == 0)
		value = STA_DEFAULT_TRAFFIC_LIMIT;
	if(issend == 1)
	{
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_specific_node_flag %02X:%02X:%02X:%02X:%02X:%02X 1;autelan traffic_limit ath.%d-%d set_specific_node_send %02X:%02X:%02X:%02X:%02X:%02X %d",l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,value);
	}
	else if(issend == 0)
	{		
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_specific_node_flag %02X:%02X:%02X:%02X:%02X:%02X 1;autelan traffic_limit ath.%d-%d set_specific_node %02X:%02X:%02X:%02X:%02X:%02X %d",l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,value);
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_sta_value error\n");
	}
//	printf("wid_set_bss_traffic_limit_sta_value apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_sta_value apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}

int wid_cancel_bss_traffic_limit_sta_value(unsigned int wtpid,
												unsigned int l_radioid,
												unsigned char wlanid,
												unsigned char mac0,
												unsigned char mac1,
												unsigned char mac2,
												unsigned char mac3,
												unsigned char mac4,
												unsigned char mac5,
												unsigned int value,
												unsigned char flag,
												unsigned char issend)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(flag == 2)
	{
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_specific_node_flag %02X:%02X:%02X:%02X:%02X:%02X 0;"
			"autelan traffic_limit ath.%d-%d set_specific_node %02X:%02X:%02X:%02X:%02X:%02X %d;"
			"autelan traffic_limit ath.%d-%d set_specific_node_send %02X:%02X:%02X:%02X:%02X:%02X %d",
			l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,
			l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,STA_DEFAULT_TRAFFIC_LIMIT,
			l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,STA_DEFAULT_TRAFFIC_LIMIT);
	}
	else if(issend == 1)
	{
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_specific_node_flag %02X:%02X:%02X:%02X:%02X:%02X 1;autelan traffic_limit ath.%d-%d set_specific_node_send %02X:%02X:%02X:%02X:%02X:%02X %d",l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,value);
	}
	else if(issend == 0)
	{		
		sprintf(apcmd,"autelan traffic_limit ath.%d-%d set_specific_node_flag %02X:%02X:%02X:%02X:%02X:%02X 1;autelan traffic_limit ath.%d-%d set_specific_node %02X:%02X:%02X:%02X:%02X:%02X %d",l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,value);
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_sta_value error\n");
	}
//	printf("wid_set_bss_traffic_limit_sta_value apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_bss_traffic_limit_sta_value apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
/*nl add 20100317*/
int wid_radio_set_whole_wlan_traffic_limit_value(unsigned char wlanid,unsigned int value,unsigned char issend)
{
	int i = 0;
	int l_radio_num = 0;
	int k = 0;
	unsigned int bssindex = 0;
	unsigned char bssid = 0;
	int wtp_num;

	
	for(i=0;i<G_RADIO_NUM;i++){

		wtp_num = i/L_RADIO_NUM;
		l_radio_num = i % L_RADIO_NUM;
				
		if((AC_WTP[wtp_num] == NULL)||(AC_WTP[wtp_num]->WTP_Radio[l_radio_num] == NULL))
			{	
				continue;
			}

		else{
			if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->isBinddingWlan == 0)
				{	
					continue;
				}
			
			else{
				for(k=0;k<L_BSS_NUM;k++){
					
					if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[k] != NULL){
						if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[k]->WlanID == wlanid){
							
							bssindex = AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[k]->BSSIndex;
							bssid = k;
							
							if(issend == 1){/*down*/
								if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->send_traffic_limit == value)
								{
									continue;
								}
							}
							else if(issend == 0){/*up*/
								if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->traffic_limit == value)
								{
									continue;
								}
							} 
							
							wid_set_bss_traffic_limit_value(wtp_num,l_radio_num,wlanid,value,issend);
							AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->traffic_limit_able = 1;
							
							if(issend == 1)
							{
								AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->send_traffic_limit = value;
							}
							else if(issend == 0)
							{
								AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->traffic_limit = value;
							}
							
							wid_asd_bss_traffic_limit(bssindex);
						}
					}
				}
			}
		}
	}
	
	return WID_DBUS_SUCCESS;
	
}
/*nl add 20100401*/
int wid_radio_set_whole_wlan_station_average_traffic_limit_value(unsigned char wlanid,unsigned int value,unsigned char issend)
{
	int i = 0;
	int l_radio_num = 0;
	int k = 0;
	unsigned int bssindex = 0;
	unsigned char bssid = 0;
	int wtp_num;

	
	for(i=0;i<G_RADIO_NUM;i++){

		wtp_num = i/L_RADIO_NUM;
		l_radio_num = i % L_RADIO_NUM;
				
		if((AC_WTP[wtp_num] == NULL)||(AC_WTP[wtp_num]->WTP_Radio[l_radio_num] == NULL))
			{	
				continue;
			}

		else{
			if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->isBinddingWlan == 0)
				{	
					continue;
				}
			
			else{
				for(k=0;k<L_BSS_NUM;k++){
					
					if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[k] != NULL){
						if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[k]->WlanID == wlanid){
							
							bssindex = AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[k]->BSSIndex;
							bssid = k;
							
							if(issend == 1){/*down*/
								if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->send_average_rate == value)
								{
									continue;
								}
							}
							else if(issend == 0){/*up*/
								if(AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->average_rate == value)
								{
									continue;
								}
							} 
							
							wid_set_bss_traffic_limit_average_value(wtp_num,l_radio_num,wlanid,value,issend);
							
							//wid_set_bss_traffic_limit_value(wtp_num,l_radio_num,wlanid,value,issend);
							AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->traffic_limit_able = 1;
							
							if(issend == 1)
							{
								AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->send_average_rate = value;
							}
							else if(issend == 0)
							{
								AC_WTP[wtp_num]->WTP_Radio[l_radio_num]->BSS[bssid]->average_rate = value;
							}
							
							wid_asd_bss_traffic_limit(bssindex);
						}
					}
				}
			}
		}
	}
	
	return WID_DBUS_SUCCESS;
	
}


/*nl add 20100120*/
int wid_radio_set_inter_vap_forwarding_able(unsigned int wtpid,unsigned int l_radioid,unsigned char policy)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int ret = 0;
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
	if(policy == 0){
		sprintf(apcmd,"set interVF diable");
	}
	else if(policy == 1){
		sprintf(apcmd,"set interVF enable");
	}
	//sprintf(apcmd,"set interVF policy %d",policy);
	
	wid_syslog_debug_debug(WID_DEFAULT,"set_inter_VAP_forwarding: %s\n",apcmd);
	printf("set_inter_VAP_forwarding: %s\n",apcmd);
	
	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
/*nl add 20100121*/
int wid_radio_set_intra_vap_forwarding_able(unsigned int wtpid,unsigned int l_radioid,unsigned char policy)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int ret = 0;
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(policy == 0){
		sprintf(apcmd,"set intraVF diable");
	}
	else if(policy == 1){
		sprintf(apcmd,"set intraVF enable");
	}	
	wid_syslog_debug_debug(WID_DEFAULT,"set_intra_VAP_forwarding: %s\n",apcmd);

	
	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
/*nl add 20100128*/
int wid_radio_set_keep_alive_period_value(unsigned int wtpid,unsigned int l_radioid,unsigned int idle_period)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int ret = 0;
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"set keepaliveperiod %d",idle_period);
	
	wid_syslog_debug_debug(WID_DEFAULT,"set_keep_alive_period: %s\n",apcmd);

	
	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}


/*nl add 20100129*/
int wid_radio_set_keep_alive_idle_time_value(unsigned int wtpid,unsigned int l_radioid,unsigned int idle_time)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int ret = 0;
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	sprintf(apcmd,"set keepaliveidletime %d",idle_time);
	
	wid_syslog_debug_debug(WID_DEFAULT,"set_keep_alive_idle_time %s\n",apcmd);

	
	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}
/*nl add 20100130*/
int wid_radio_set_congestion_avoid_state(unsigned int wtpid,unsigned int l_radioid,unsigned int congestion_av_state)
{
	if(AC_WTP[wtpid] == NULL)
	{
		return -1;
	}
	int ret = 0;
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if(0 == congestion_av_state)
		sprintf(apcmd,"set caScheme disable");
	else if(1 == congestion_av_state)
		sprintf(apcmd,"set caScheme tail-drop");
	else if(2 == congestion_av_state)
		sprintf(apcmd,"set caScheme RED");
	else if(3== congestion_av_state)
		sprintf(apcmd,"set caScheme FWRED");
	
	wid_syslog_debug_debug(WID_DEFAULT,"set_congestion_avoid_state %s\n",apcmd);

	
	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}

int wid_radio_set_wlan_traffic_limit_able(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char policy)
{
	if((AC_WTP[wtpid] == NULL)||(AC_WLAN[wlanid] == NULL)||(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL))
	{
		return -1;
	}

	int i = 0;
	unsigned int bssindex = 0;
	unsigned char bssid = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i] != NULL)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->WlanID == wlanid)
			{
				bssindex = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->BSSIndex;
				bssid = i;
				break;
			}
		}
	}

	if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit_able == policy)
	{
		return 0;
	}

	wid_set_bss_traffic_limit(wtpid,l_radioid,wlanid,policy);
	AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit_able = policy;
	/*
	if(policy == 0)
	{
		//AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit = 0;
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->average_rate = 0;
		//AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit = 0;
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_average_rate = 0;
	}
	*/
	wid_asd_bss_traffic_limit(bssindex);
	return 0;
}
int wid_radio_set_wlan_traffic_limit_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend)
{
	if((AC_WTP[wtpid] == NULL)||(AC_WLAN[wlanid] == NULL)||(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL))
	{
		return -1;
	}

	int i = 0;
	unsigned int bssindex = 0;
	unsigned char bssid = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i] != NULL)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->WlanID == wlanid)
			{
				bssindex = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->BSSIndex;
				bssid = i;
				break;
			}
		}
	}
	if(issend == 1)//下行
	{
	/*	if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit == value)
		{
			return 0;
		}*/
	}
	else if(issend == 0)//上行
	{
		/*if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit == value)
		{
			return 0;
		}*/
	}
	else
	{
		return WID_DBUS_ERROR;
	}
	

	wid_set_bss_traffic_limit_value(wtpid,l_radioid,wlanid,value,issend);
	AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit_able = 1;
	if(issend == 1)
	{
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit = value;
	}
	else if(issend == 0)
	{
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit = value;
	}
	else
	{
		return WID_DBUS_ERROR;
	}

	wid_asd_bss_traffic_limit(bssindex);
	return 0;
}
int wid_radio_set_wlan_traffic_limit_average_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend)
{
	if((AC_WTP[wtpid] == NULL)||(AC_WLAN[wlanid] == NULL) || (AC_WLAN[wlanid] != NULL && AC_WLAN[wlanid]->want_to_delete == 1)||(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL))		/* Huangleilei add for ASXXZFI-1622 */
	{
		return -1;
	}

	int i = 0;
	unsigned int bssindex = 0;
	unsigned char bssid = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i] != NULL)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->WlanID == wlanid)
			{
				bssindex = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->BSSIndex;
				bssid = i;
				break;
			}
		}
	}


	
	if(issend == 1)//下行
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit != 0)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit < value)
			{
				return IF_POLICY_CONFLICT;
			}
		}
		
/*		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_average_rate == value)
		{
			return 0;
		}*/
	}
	else if(issend == 0)//上行
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit != 0)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit < value)
			{
				return IF_POLICY_CONFLICT;
			}
		}
		
/*		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->average_rate == value)
		{
			return 0;
		}*/
	}
	else
	{
		return WID_DBUS_ERROR;
	}
	
	wid_set_bss_traffic_limit_average_value(wtpid,l_radioid,wlanid,value,issend);
	AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit_able = 1;
	if(issend == 1)
	{
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_average_rate = value;
	}
	else if(issend == 0)
	{
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->average_rate = value;
	}
	else
	{
		return WID_DBUS_ERROR;
	}
	
	wid_asd_bss_traffic_limit(bssindex);
	return 0;
}
/*fengwenchao add for AXSSZFI-1374*/
int wid_radio_set_wlan_traffic_limit_cancel_average_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char issend)
{
	unsigned int value =0;
	if((AC_WTP[wtpid] == NULL)||(AC_WLAN[wlanid] == NULL)||(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL))
	{
		return -1;
	}

	int i = 0;
	unsigned int bssindex = 0;
	unsigned char bssid = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i] != NULL)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->WlanID == wlanid)
			{
				bssindex = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->BSSIndex;
				bssid = i;
				break;
			}
		}
	}
	#if 0
	if(issend == 1)//下行
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit != 0)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit < value)
			{
				return IF_POLICY_CONFLICT;
			}
		}
	}
	else if(issend == 0)//上行
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit != 0)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit < value)
			{
				return IF_POLICY_CONFLICT;
			}
		}
	}
	else
	{
		return WID_DBUS_ERROR;
	}
	#endif
	
	//AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit_able = 1;
	if(issend == 1)
	{
		//AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_average_rate = value;
		value = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_average_rate;
		if(value == 0)
			value = STA_DEFAULT_TRAFFIC_LIMIT;
		else
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_average_rate = 0;
	}
	else if(issend == 0)
	{
		//AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->average_rate = value;
		value = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->average_rate;
		if(value == 0)
			value = STA_DEFAULT_TRAFFIC_LIMIT;		
		else
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->average_rate = 0;
	}
	else
	{
		return WID_DBUS_ERROR;
	}
	wid_set_bss_traffic_limit_cancel_average_value(wtpid,l_radioid,wlanid,value,issend);
	wid_asd_bss_cancel_average_traffic_limit(bssindex);
	return 0;
}
/*fengwenchao add end*/
int wid_radio_set_wlan_traffic_limit_sta_value(unsigned int wtpid
														,unsigned int l_radioid
														,unsigned char wlanid
														,unsigned char mac0
														,unsigned char mac1
														,unsigned char mac2
														,unsigned char mac3
														,unsigned char mac4
														,unsigned char mac5
														,unsigned int value
														,unsigned char issend)
{
	if((AC_WTP[wtpid] == NULL)||(AC_WLAN[wlanid] == NULL)||(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL))
	{
		return -1;
	}

	int i = 0;
	unsigned int bssindex = 0;
	unsigned char bssid = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i] != NULL)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->WlanID == wlanid)
			{
				bssindex = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->BSSIndex;
				bssid = i;
				break;
			}
		}
	}

	if(issend == 1)//下行
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit != 0)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_traffic_limit < value)
			{
				return IF_POLICY_CONFLICT;
			}
		}
	}
	else if(issend == 0)//上行
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit != 0)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit < value)
			{
				return IF_POLICY_CONFLICT;
			}
		}
	}
	else
	{
		return WID_DBUS_ERROR;
	}
	
	wid_set_bss_traffic_limit_sta_value(wtpid,l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,value,issend);
	if(value != 0)
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit_able = 1;
	return 0;
}
int wid_radio_set_wlan_traffic_limit_cancel_sta_value(unsigned int wtpid
														,unsigned int l_radioid
														,unsigned char wlanid
														,unsigned char mac0
														,unsigned char mac1
														,unsigned char mac2
														,unsigned char mac3
														,unsigned char mac4
														,unsigned char mac5
														,unsigned char flag
														,unsigned char issend)
{
	if((AC_WTP[wtpid] == NULL)||(AC_WLAN[wlanid] == NULL)||(AC_WTP[wtpid]->WTP_Radio[l_radioid] == NULL))
	{
		return -1;
	}

	int i = 0;
	unsigned int bssindex = 0;
	unsigned int value = 0;
	unsigned char bssid = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i] != NULL)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->WlanID == wlanid)
			{
				bssindex = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->BSSIndex;
				bssid = i;
				break;
			}
		}
	}

	if((issend == 1) && (flag == 1))//下行
	{
		value = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->send_average_rate;
		if(value == 0)
			value = STA_DEFAULT_TRAFFIC_LIMIT;
	}
	else if((issend == 0) && (flag == 1))//上行
	{
		value = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->average_rate;
		if(value == 0)
			value = STA_DEFAULT_TRAFFIC_LIMIT;
	}
	else if( flag == 2)
	{
		value = STA_DEFAULT_TRAFFIC_LIMIT;
	}
	else
	{
		return WID_DBUS_ERROR;
	}

	wid_cancel_bss_traffic_limit_sta_value(wtpid,l_radioid,wlanid,mac0,mac1,mac2,mac3,mac4,mac5,value,flag,issend);
	/*AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->traffic_limit_able = 1;*/
	return 0;
}

int wid_set_sta_ip_mac_binding(unsigned int wtpid,	unsigned int l_radioid,unsigned char wlanid,
					unsigned int value)
{
	int ret = 0;
	int i = 0;
	unsigned int	bssindex = 0;
	unsigned char	bssid = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i] != NULL)
		{
			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->WlanID == wlanid)
			{
				bssindex = AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[i]->BSSIndex;
				bssid = i;
				break;
			}
		}
	}

	if((value == 0) || (value == 1))
	{		
		sprintf(apcmd,"/usr/sbin/set_ip_enable ath.%d-%d %d",l_radioid,wlanid,value);
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->BSS[bssid]->ip_mac_binding = value;
	}
	else
	{
		wid_syslog_info("wid_set_sta_ip_mac_binding error\n");
	}
	printf("wid_set_sta_ip_mac_binding apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_sta_ip_mac_binding apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}

int wid_set_radio_sector_value(unsigned int wtpid,	unsigned int l_radioid,unsigned short value,unsigned int policy)
{
	int send_flag = 0;
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
	//CW_CREATE_OBJECT_ERR(AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[value],WID_oem_sector,return NULL;);
	/*for(i=0;i<SECTOR_NUM;i++)
	{	
		CW_CREATE_OBJECT_ERR(AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[i],WID_oem_sector,return NULL;);
	}*/
	if((policy == 1)&&((AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector_state_value & value) != value)){
		if(value&0x1)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[0]->state = 1;
		if(value&0x2)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[1]->state = 1;
		if(value&0x4)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[2]->state = 1;
		if(value&0x8)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[3]->state = 1;
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector_state_value	= value;
		send_flag = 1;
	}
	else if((policy == 0)&&((AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector_state_value & (~value)) != AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector_state_value)){
		if(value&0x1)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[0]->state = 0;
		if(value&0x2)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[1]->state = 0;
		if(value&0x4)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[2]->state = 0;
		if(value&0x8)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[3]->state = 0;

		AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector_state_value = (AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector_state_value)&(~value);
		send_flag = 1;
	}
	else{}
	if(send_flag == 1){
		sprintf(apcmd,"set sector %u",AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector_state_value);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_sector apcmd %s\n",apcmd);
		wid_radio_set_extension_command(wtpid,apcmd);
	}
	return 0;
}
int wid_set_radio_tx_chainmask_value(unsigned int wtpid,	unsigned int l_radioid,unsigned char value,unsigned int policy)
{
	int send_flag = 0;
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
	/*
	wid_syslog_debug_debug(WID_DEFAULT,"5555555555555555\n");

	wid_syslog_debug_debug(WID_DEFAULT,"5555555555555555policy:%d\n",policy);
	
	wid_syslog_debug_debug(WID_DEFAULT,"5555555555555555 tx_mask value:%d\n",AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value);
	wid_syslog_debug_debug(WID_DEFAULT,"value:%d\n",value);*/
	

	if((policy == 1)&&((AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value & value)!= value)){
		
		if(value&0x1)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask[0]->state = 1;
		if(value&0x2)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask[1]->state = 1;
		if(value&0x4)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask[2]->state = 1;
				
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value	= value;
		send_flag = 1;
	}
	else if((policy == 0)&&((AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value & (~value)) != AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value)){
		if(value&0x1)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask[0]->state = 0;
		if(value&0x2)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask[1]->state = 0;
		if(value&0x4)
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask[2]->state = 0;
		
		AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value = (AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value)&(~value);
		send_flag = 1;
		
	}
	else{}
	if(send_flag == 1){
		sprintf(apcmd,"set tx_chainmask %u",AC_WTP[wtpid]->WTP_Radio[l_radioid]->tx_chainmask_state_value);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_tx_chainmask apcmd %s\n",apcmd);
		wid_radio_set_extension_command(wtpid,apcmd);
	}
	return 0;
}

int radio_sectorid_parse_func(unsigned short int sectorid,char* str){
	unsigned int ret = 0;
	//char str[4];
//	str = NULL;
	if (sectorid == 0)
	{
		memcpy(str, "0",1);
	}		
	else if (sectorid == 1)
	{
		memcpy(str, "1",1);
	}
	else if (sectorid == 2)
	{
		memcpy(str, "2",1);
	}
	else if (sectorid == 3)
	{
		memcpy(str, "3",1);
	}
	else if (sectorid == 4)
	{
		memcpy(str, "all",3);
	}
	else
	{
		ret = -1;
	}
	return ret ;
}
int wid_set_radio_sector_tx_power_value(unsigned int wtpid,	unsigned int l_radioid,unsigned short sectorid,unsigned int value)
{
	int j = 0;
	int send_flag = 0;
	char* Sector = NULL;
	Sector = (char*)malloc(3+1);
	memset(Sector,0,3+1);
	radio_sectorid_parse_func(sectorid,Sector);
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	//if((sectorid == 0x1) || (sectorid == 0x2)||(sectorid == 0x4) || (sectorid == 0x8)||(sectorid == 0xF))
	if(sectorid<=4)
	{		
		if(sectorid == 4){
			for(j=0;j<SECTOR_NUM;j++){
				if((AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[j]->state == 1)&&(AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[j]->tx_power != value)){
						AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[j]->tx_power = value;
						send_flag = 1;
				}
			}
		}else if(sectorid<4){
			if((AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[sectorid]->state == 1)&&(AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[sectorid]->tx_power != value)){
				AC_WTP[wtpid]->WTP_Radio[l_radioid]->sector[sectorid]->tx_power = value;
				send_flag = 1;
			}
		}

		if(send_flag == 1){
			sprintf(apcmd,"set power %s %d",Sector,value);
			wid_radio_set_extension_command(wtpid,apcmd);

			printf("wid_set_sector apcmd %s\n",apcmd);
			wid_syslog_debug_debug(WID_DEFAULT,"wid_set_sector apcmd %s\n",apcmd);
		}
	}
	else
	{
		wid_syslog_info("wid_set_sector error\n");
	}

	if(Sector){
		free(Sector);
		Sector = NULL;
	}
	return 0;
}

int wid_set_radio_netgear_supper_g_technology_state(unsigned int wtpid,	unsigned int l_radioid,unsigned short supper_g_type,unsigned int supper_g_state)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);
	if(supper_g_state == 1){
		if(supper_g_type == 1)
		{		
			sprintf(apcmd,"set bursting enable");
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_type |= 0x1;
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_state = 1;
		}
		else if(supper_g_type == 2)
		{		
			sprintf(apcmd,"set fastFrame enable");
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_type |= 0x2;
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_state = 1;
		}
		else if(supper_g_type == 3)
		{		
			sprintf(apcmd,"set compression enable ");
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_type |= 0x4;
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_state = 1;
		}
	}
	else if(supper_g_state == 0){
		if(supper_g_type == 1)
		{		
			sprintf(apcmd,"set bursting disable");
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_type &= ~0x1;
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_state = 0;
		}
		else if(supper_g_type == 2)
		{		
			sprintf(apcmd,"set fastFrame disable");
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_type &= ~0x2;
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_state = 0;
		}
		else if(supper_g_type == 3)
		{		
			sprintf(apcmd,"set compression disable ");
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_type &= ~0x4;
			AC_WTP[wtpid]->WTP_Radio[l_radioid]->supper_g.supper_g_state = 0;
		}

	}
	else
	{
		wid_syslog_info("set (bursting|fastFrame|compression) (enable|disable) error\n");
	}
	
	printf("set (bursting|fastFrame|compression) (enable|disable) apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"set (bursting|fastFrame|compression) (enable|disable) apcmd %s\n",apcmd);
	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return ret;
}
int wid_set_dhcp_before_autherized(unsigned int wtpid,	unsigned int l_radioid,unsigned char wlanid,
					unsigned int value)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if((value == 0) || (value == 1))
	{		
		sprintf(apcmd,"/usr/sbin/set_dhcp ath.%d-%d %d",l_radioid,wlanid,value);
	}
	else
	{
		wid_syslog_info("wid_set_dhcp_before_autherized error\n");
	}
	printf("wid_set_dhcp_before_autherized apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_dhcp_before_autherized apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}

int wid_set_sta_vlan_id(unsigned int wtpid,	unsigned int l_radioid,unsigned char wlanid,
					unsigned char *mac,unsigned int value)
{
	int ret = 0;
	
	char apcmd[WID_SYSTEM_CMD_LENTH];
	memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

	if((value >= 0) && (value <= 4095))
	{		
		sprintf(apcmd,"/usr/sbin/set_mac_vlan ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X %d",l_radioid,wlanid,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],value);
	}
	else
	{
		wid_syslog_info("wid_set_sta_vlan_id error\n");
	}
	printf("wid_set_sta_vlan_id apcmd %s\n",apcmd);
	wid_syslog_debug_debug(WID_DEFAULT,"wid_set_sta_vlan_id apcmd %s\n",apcmd);

	ret = wid_radio_set_extension_command(wtpid,apcmd);
	return 0;
}

void update_next_wtp()
{
	//printf("update_next_wtp %d\n",checkwtpcount);
	int i=0,j=0,result=0;
	CWBool bMatchVersion = CW_FALSE;
	if(checkwtpcount < WTP_NUM)
	{
		checkwtpcount++;
	}
	//printf("update_next_wtp %d\n",checkwtpcount);
	for(i = checkwtpcount; i < WTP_NUM; i++)
	{
		for(j = 0; j < BATCH_UPGRADE_AP_NUM; j++)
		{
			if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5)&&(AC_WTP[i]->updateversion == NULL))//run state
			{
				if((gConfigVersionUpdateInfo[j] != NULL)&&(strcmp(AC_WTP[i]->WTPModel,gConfigVersionUpdateInfo[j]->str_ap_model) == 0))
				{
					CWConfigVersionInfo *tmpnode = gConfigVersionUpdateInfo[j];
					while(tmpnode != NULL){
						wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation111 ****\n"); //for test
						if(strcmp(tmpnode->str_ap_code,AC_WTP[i]->APCode) == 0){
							if((AC_WTP[i]->codever == NULL)&&(strcmp(AC_WTP[i]->ver,tmpnode->str_ap_version_name) == 0))				
							{
								wid_syslog_debug_debug(WID_WTPINFO,"ap model match 222\n"); 	//for test
								bMatchVersion = CW_TRUE;
							}
							else if((AC_WTP[i]->codever != NULL)&&(strcmp(tmpnode->str_ap_version_name,AC_WTP[i]->codever) == 0))
							{
								wid_syslog_debug_debug(WID_WTPINFO,"ap model match 333\n"); 	//for test
								bMatchVersion = CW_TRUE;
							}
							break;
						}
						tmpnode = tmpnode->next;
					}

					if(bMatchVersion != CW_TRUE){
						insert_wtp_list(i);
						wid_set_ap_reboot(i);
						AC_WTP[i]->updateStat = 0;
						AC_WTP[i]->updatefailcount = 0;
						time(&AC_WTP[i]->manual_update_time);
						if(updatewtplist->count >= gupdateCountOneTime)
						{
							checkwtpcount = i;
							result = 1;
						}
					}
					break;
				}
				
			}
		}
		checkwtpcount = i;
		if(result == 1){
			break;
		}
	}

}

void update_current_wtp()
{
	//printf("update_current_wtp %d\n",checkwtpcount);
	int i=0,j=0,result=0;
	CWBool bMatchVersion = CW_FALSE;
	for(i = 0; i < WTP_NUM; i++)
	{
		for(j = 0; j < BATCH_UPGRADE_AP_NUM; j++)
		{
			if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5)&&(AC_WTP[i]->updateversion == NULL))//run state
			{
				if((gConfigVersionUpdateInfo[j] != NULL)&&(strcmp(AC_WTP[i]->WTPModel,gConfigVersionUpdateInfo[j]->str_ap_model) == 0))
				{
					CWConfigVersionInfo *tmpnode = gConfigVersionUpdateInfo[j];
					while(tmpnode != NULL){
						wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation1111 ****\n"); //for test
						if(strcmp(tmpnode->str_ap_code,AC_WTP[i]->APCode) == 0){
							if((AC_WTP[i]->codever == NULL)&&(strcmp(AC_WTP[i]->ver,tmpnode->str_ap_version_name) == 0))				
							{
								wid_syslog_debug_debug(WID_WTPINFO,"ap model match 222\n"); 	//for test
								bMatchVersion = CW_TRUE;
							}
							else if((AC_WTP[i]->codever != NULL)&&(strcmp(tmpnode->str_ap_version_name,AC_WTP[i]->codever) == 0))
							{
								wid_syslog_debug_debug(WID_WTPINFO,"ap model match 333\n"); 	//for test
								bMatchVersion = CW_TRUE;
							}
							break;
						}
						tmpnode = tmpnode->next;
					}
				
					if(bMatchVersion != CW_TRUE){
						insert_wtp_list(i);
						wid_set_ap_reboot(i);
						AC_WTP[i]->updateStat = 0;
						AC_WTP[i]->updatefailcount = 0;
						time(&AC_WTP[i]->manual_update_time);
						if(updatewtplist->count >= gupdateCountOneTime)
						{
							checkwtpcount = i;
							result = 1;
						}
					}
					break;
				}
			}
		}
		checkwtpcount = i;
		if(result == 1){
			break;
		}
	}

	
}
CWBool insert_wtp_list(int id)
{
	//printf("insert_wtp_list\n");

	struct tag_wtpid *wtp_id;
	struct tag_wtpid *wtp_id_next;
	wtp_id = (struct tag_wtpid*)malloc(sizeof(struct tag_wtpid));
	
	wtp_id->wtpid = id;
	wtp_id->next = NULL;
	//printf("*** insert_wtp_list is %d*\n", wtp_id->wtpid);
	
	if(updatewtplist == NULL)
	{
		updatewtplist = (struct tag_wtpid_list*)malloc(sizeof(struct tag_wtpid_list));
		updatewtplist->wtpidlist = wtp_id ;		
		updatewtplist->count = 1;
		//printf("*** wtp id:%d insert first  \n",id);
	}
	else
	{
		
		wtp_id_next = updatewtplist->wtpidlist;
		while(wtp_id_next->next!= NULL)
		{	
			wtp_id_next = wtp_id_next->next;//insert element int tail
		}
		
		wtp_id_next->next= wtp_id;
		updatewtplist->count++;
		
		//printf("*** wtp id:%d insert more  \n",id);
	}
	return CW_TRUE;

}
CWBool delete_wtp_list(int id)
{
	//printf("delete_wtp_list\n");
	if(updatewtplist == NULL)
	{
		return CW_FALSE;
	}

	struct tag_wtpid *wtp_id;
	struct tag_wtpid *wtp_id_next;

	wtp_id = updatewtplist->wtpidlist;
	wtp_id_next = updatewtplist->wtpidlist;

	if(updatewtplist->count == 0)
	{
		return CW_FALSE;
	}
	else if(wtp_id_next->wtpid == id)
	{

		updatewtplist->wtpidlist = wtp_id_next->next;
		free(wtp_id_next);
		wtp_id_next = NULL;
		
		updatewtplist->count--;
		
		if(updatewtplist->wtpidlist == NULL)
		{
			free(updatewtplist);
			updatewtplist = NULL;
		}
		return CW_TRUE;
	}

	else
	{
		while(wtp_id_next->next != NULL)
		{	
			if(wtp_id_next->next->wtpid== id)
			{

				wtp_id = wtp_id_next->next;
				wtp_id_next->next = wtp_id_next->next->next;
				free(wtp_id);
				wtp_id = NULL;
				updatewtplist->count--;
				return CW_TRUE;
			}
			wtp_id_next = wtp_id_next->next;
		}
	}

	return CW_FALSE;

}
CWBool find_in_wtp_list(int id)
{
	//printf("find_in_wtp_list\n");
	if(updatewtplist == NULL)
	{
		return CW_FALSE;
	}
	
	struct tag_wtpid *wtp_id_next;
	
	wtp_id_next = updatewtplist->wtpidlist;
	while(wtp_id_next != NULL)
	{	
		if(wtp_id_next->wtpid == id)
		{
			return CW_TRUE;
		}
		wtp_id_next = wtp_id_next->next;
	}

	return CW_FALSE;

}
void destroy_wtp_list()
{
	if(updatewtplist == NULL)
	{
		return;
	}

	struct tag_wtpid *phead = NULL;
	struct tag_wtpid *pnext = NULL;
	phead = updatewtplist->wtpidlist;
	
	free(updatewtplist);
	updatewtplist = NULL;
	
	while(phead != NULL)
	{	
		
		pnext = phead->next;
	
		CW_FREE_OBJECT(phead);

		phead = pnext;

	}	
}

/*mahz add for ap upgrade automatically*/
int WIDCheckFreeMem(unsigned int fileLen, char *filePath)
{
    FILE *fp = NULL;
    char buf[1024] = {0};
    unsigned int freeMem = 0;
    unsigned int minMem = 0;
    wid_syslog_debug_debug(WID_DEFAULT, "filepath = %s\n",(char*)filePath);
    if(0 == memcmp(filePath, "/blk", 4)){
        system("sudo mount /blk");
        fp = popen("df -l /blk |grep blk |awk '{ print $4 }'","r");
        minMem = CFDISK_MIN_MEM;
    }
    else{
        fp = popen("free | grep + | awk '{ print $4 }'","r");
        minMem = SYSTEM_MIN_MEM;
    }
    if(!fp){
        wid_syslog_err("failed to get memery information\n");
        return -1;
    }
    
    fread(buf, sizeof(char), sizeof(buf), fp);
    pclose(fp);

    if(-1 == parse_int_ID((char*)buf, &freeMem)){
        wid_syslog_err("Error : get free memery failed.\n");
    }
    wid_syslog_debug_debug(WID_DEFAULT, "freemem = %d, minMem = %d, filelen = %d\n",freeMem,minMem,fileLen/1000);
    if(freeMem > (minMem + (fileLen/1000)))
        return 0;
    else
        return -1;
}

unsigned int getfilesize(char *file_path){
    unsigned int file_len = 0;
    FILE *fp = NULL;
    
	fp = fopen(file_path, "rb");
    if(!fp){
        wid_syslog_err("failed to open file %s\n",file_path);
    	wid_syslog_debug_debug(WID_DBUS, "open file %s to read failed\n", file_path);
	    return -1;
    }
	fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    fclose(fp);
	fp = NULL;
	
	return file_len;
}

int create_ac_ip_list_group(unsigned char ID,char *IFNAME){
	int ret = 0;
	int i;
	for(i = 1; i < ACIPLIST_NUM; i++){
		if((AC_IP_GROUP[i] != NULL)&&(strcmp((char*)AC_IP_GROUP[i]->ifname,IFNAME) == 0)){
			return AC_IP_BIND_IF_EXIST;
		}
	}
	ret = Check_And_Bind_Interface_For_WID(IFNAME);
	if(ret != 0)
		return ret;
	AC_IP_GROUP[ID] = (wid_ac_ip_group*)malloc(sizeof(wid_ac_ip_group));
	memset(AC_IP_GROUP[ID], 0, sizeof(wid_ac_ip_group));
	AC_IP_GROUP[ID]->GroupID = ID;
	AC_IP_GROUP[ID]->load_banlance = 0;
	AC_IP_GROUP[ID]->diff_count = 0;
	AC_IP_GROUP[ID]->ifname = (unsigned char*)malloc(strlen(IFNAME)+1);
	memset(AC_IP_GROUP[ID]->ifname, 0, strlen(IFNAME)+1);
	memcpy(AC_IP_GROUP[ID]->ifname, IFNAME, strlen(IFNAME));

	AC_IP_GROUP[ID]->isock = init_client_socket();
	
	if(get_ipv4addr_by_ifname(ID) == INTERFACE_HAVE_NO_IP_ADDR){	//xiaodawei add, 20110324
		if(AC_IP_GROUP[ID]->ifname){
			free(AC_IP_GROUP[ID]->ifname);
			AC_IP_GROUP[ID]->ifname = NULL;
		}
		if(AC_IP_GROUP[ID]){
			free(AC_IP_GROUP[ID]);
			AC_IP_GROUP[ID] = NULL;
		}
		return INTERFACE_HAVE_NO_IP_ADDR;
	}
	
	return ret;
}

int delete_ac_ip_list_group(unsigned char ID){
	int ret = 0;
	struct wid_ac_ip *tmp;
	struct wid_ac_ip *tmp1;
	if(AC_IP_GROUP[ID] == NULL)
		return ret;
	tmp = AC_IP_GROUP[ID]->ip_list;	
	AC_IP_GROUP[ID]->ipnum = 0;
	while(tmp != NULL){
		tmp1 = tmp->next;
		free(tmp->ip);
		tmp->ip = NULL;
		free(tmp);
		tmp = NULL;
		tmp = tmp1;
	}
	AC_IP_GROUP[ID]->ip_list = NULL;
	free(AC_IP_GROUP[ID]->ifname);
	AC_IP_GROUP[ID]->ifname = NULL;
	free(AC_IP_GROUP[ID]->ipaddr);
	AC_IP_GROUP[ID]->ipaddr = NULL;
	close(AC_IP_GROUP[ID]->isock);
	free(AC_IP_GROUP[ID]);
	AC_IP_GROUP[ID] = NULL;
	return ret;
}

int add_ac_ip(unsigned char ID, char * ip, unsigned char priority){
	struct wid_ac_ip *tmp;
	struct wid_ac_ip *tmp1;
	printf("1\n");
	if(AC_IP_GROUP[ID] == NULL)
		return WLAN_ID_NOT_EXIST;
	printf("2\n");
	if(AC_IP_GROUP[ID]->ip_list == NULL){
		printf("3\n");
		tmp = (struct wid_ac_ip *)malloc(sizeof(struct wid_ac_ip));
		memset(tmp, 0, sizeof(struct wid_ac_ip));
		tmp->priority = priority;
		tmp->ip = malloc(strlen(ip)+1);
		tmp->wtpcount = 0;
		tmp->threshold = 0;
		memset(tmp->ip, 0, strlen(ip)+1);
		memcpy(tmp->ip, ip, strlen(ip));
		tmp->next = NULL;
		AC_IP_GROUP[ID]->ip_list = tmp;
		AC_IP_GROUP[ID]->ipnum += 1;
		return WID_DBUS_SUCCESS;
	}else{
		printf("4\n");
		tmp1 = AC_IP_GROUP[ID]->ip_list;
		while(tmp1 != NULL){
			if((strcmp(tmp1->ip, ip)==0)){
				return AC_IP_EXIST;
			}
			tmp1 = tmp1->next;
		}		
		tmp1 = AC_IP_GROUP[ID]->ip_list;
		if((tmp1->priority < priority)){
			printf("5\n");
			tmp = (struct wid_ac_ip *)malloc(sizeof(struct wid_ac_ip));
			memset(tmp, 0, sizeof(struct wid_ac_ip));
			tmp->priority = priority;
			tmp->wtpcount = 0;
			tmp->threshold = 0;
			tmp->ip = malloc(strlen(ip)+1);
			memset(tmp->ip, 0, strlen(ip)+1);
			memcpy(tmp->ip, ip, strlen(ip));
			tmp->next = NULL;
			tmp->next = AC_IP_GROUP[ID]->ip_list;
			AC_IP_GROUP[ID]->ip_list = tmp;			
			AC_IP_GROUP[ID]->ipnum += 1;
			return WID_DBUS_SUCCESS;
		}
		while(tmp1->next != NULL){
			printf("6\n");
			if((tmp1->next->priority < priority)){
				printf("8\n");
				tmp = (struct wid_ac_ip *)malloc(sizeof(struct wid_ac_ip));
				memset(tmp, 0, sizeof(struct wid_ac_ip));
				tmp->priority = priority;
				tmp->wtpcount = 0;
				tmp->threshold = 0;
				tmp->ip = malloc(strlen(ip)+1);
				memset(tmp->ip, 0, strlen(ip)+1);
				memcpy(tmp->ip, ip, strlen(ip));
				tmp->next = NULL;
				tmp->next = tmp1->next;
				tmp1->next = tmp;				
				AC_IP_GROUP[ID]->ipnum += 1;
				return WID_DBUS_SUCCESS;				
			}
			tmp1 = tmp1->next;
			tmp = NULL;
		}
		printf("9\n");
		tmp = (struct wid_ac_ip *)malloc(sizeof(struct wid_ac_ip));
		memset(tmp, 0, sizeof(struct wid_ac_ip));
		tmp->priority = priority;
		tmp->wtpcount = 0;
		tmp->threshold = 0;
		tmp->ip = malloc(strlen(ip)+1);
		memset(tmp->ip, 0, strlen(ip)+1);
		memcpy(tmp->ip, ip, strlen(ip));
		tmp->next = NULL;
		tmp1->next = tmp;				
		AC_IP_GROUP[ID]->ipnum += 1;
		return WID_DBUS_SUCCESS;				
		
	}
	/*printf("10\n");
	
	return WID_DBUS_SUCCESS;*/
}

int delete_ac_ip(unsigned char ID,char *ip){
	int ret = 0;	
	struct wid_ac_ip *tmp;
	struct wid_ac_ip *tmp1;
	
	if(AC_IP_GROUP[ID] == NULL)
		return WLAN_ID_NOT_EXIST;
	if(AC_IP_GROUP[ID]->ip_list == NULL){
		return AC_IP_NOT_EXIST;
	}

	tmp = AC_IP_GROUP[ID]->ip_list;
	if(strcmp(tmp->ip,ip)==0){
		AC_IP_GROUP[ID]->ip_list = tmp->next;
		tmp->next = NULL;
		free(tmp->ip);
		tmp->ip = NULL;
		free(tmp);
		tmp = NULL;
		AC_IP_GROUP[ID]->ipnum -= 1;
		return ret;
	}
	while(tmp->next != NULL){
		tmp1 = tmp->next;
		if(strcmp(tmp1->ip,ip)==0){
			tmp->next = tmp1->next;
			tmp1->next = NULL;
			free(tmp1->ip);
			tmp1->ip = NULL;
			free(tmp1);
			tmp1 = NULL;			
			AC_IP_GROUP[ID]->ipnum -= 1;
			return ret;
		}
		tmp = tmp1;
	}
	return AC_IP_NOT_EXIST;
}

int set_ac_ip_priority(unsigned char ID, char * ip, unsigned char priority){
	struct wid_ac_ip *tmp;
	struct wid_ac_ip *tmp1;
	struct wid_ac_ip *node = NULL;
	printf("1\n");
	if(AC_IP_GROUP[ID] == NULL)
		return WLAN_ID_NOT_EXIST;
	
	if(AC_IP_GROUP[ID]->ip_list == NULL){
		return AC_IP_NOT_EXIST;
	}

	tmp = AC_IP_GROUP[ID]->ip_list;
	if(strcmp(tmp->ip,ip)==0){
		if(tmp->priority == priority)
			return WID_DBUS_SUCCESS;
		AC_IP_GROUP[ID]->ip_list = tmp->next;
		tmp->next = NULL;
		node = tmp;
		node->priority = priority;
	}else{
		while(tmp->next != NULL){
			tmp1 = tmp->next;
			if(strcmp(tmp1->ip,ip)==0){				
				if(tmp1->priority == priority)
					return WID_DBUS_SUCCESS;
				tmp->next = tmp1->next;
				tmp1->next = NULL;
				node = tmp1;
				node->priority = priority;
				break;
			}
			tmp = tmp1;
		}
	}
	if(node == NULL){
		return AC_IP_NOT_EXIST;
	}
	printf("2\n");
	if(AC_IP_GROUP[ID]->ip_list == NULL){
		printf("3\n");
		AC_IP_GROUP[ID]->ip_list = node;
		return WID_DBUS_SUCCESS;
	}else{
		printf("4\n");
		tmp1 = AC_IP_GROUP[ID]->ip_list;
		while(tmp1 != NULL){
			if((strcmp(tmp1->ip, ip)==0)){
				return AC_IP_EXIST;
			}
			tmp1 = tmp1->next;
		}		
		tmp1 = AC_IP_GROUP[ID]->ip_list;
		if((tmp1->priority < priority)){
			printf("5\n");
			node->next = AC_IP_GROUP[ID]->ip_list;
			AC_IP_GROUP[ID]->ip_list = node;			
			return WID_DBUS_SUCCESS;
		}
		while(tmp1->next != NULL){
			printf("6\n");
			if((tmp1->next->priority < priority)){
				printf("8\n");
				node->next = tmp1->next;
				tmp1->next = node;				
				return WID_DBUS_SUCCESS;				
			}
			tmp1 = tmp1->next;
			tmp = NULL;
		}
		printf("9\n");
		tmp1->next = node;				
		return WID_DBUS_SUCCESS;				
		
	}
	/*printf("10\n");
	
	return WID_DBUS_SUCCESS;*/
}

int set_ac_ip_load_banlance(unsigned char ID, unsigned char load_banlance)
{
	if(AC_IP_GROUP[ID] == NULL)
	{
		return WLAN_ID_NOT_EXIST;
	}

	if(AC_IP_GROUP[ID]->load_banlance == load_banlance)
	{
		return WID_DBUS_SUCCESS;
	}

	CWThread loadbanlancethread;
	
	if(load_banlance == 1)
	{
		//create socket listen interface
		if(havecreatethread == 0)
		{
			if(!CWErr(CWCreateThread(&loadbanlancethread, CWLoadbanlanceThread, &ID,0)))
			{
				wid_syslog_crit("Error starting loadbanlancethread Thread");
				return WID_DBUS_ERROR;
			}
			havecreatethread = 1;
		}
		
		gloadbanlance++;
		//just test 
		
	}
	else if(load_banlance == 0)
	{
		//destroy socket listen interface
		gloadbanlance--;
		make_link_sequence_by_priority(ID);
	}


	AC_IP_GROUP[ID]->load_banlance = load_banlance;

	//printf("send count 7\n");
	//test
	//SendActiveWTPCount(7);
	//make_link_sequence_by_wtpcount(ID);
	
	return WID_DBUS_SUCCESS;
}
int  set_ac_ip_diff_banlance(unsigned char ID, unsigned int diff_banlance)
{
	if(AC_IP_GROUP[ID] == NULL)
	{
		return WLAN_ID_NOT_EXIST;
	}

	if(AC_IP_GROUP[ID]->diff_count == diff_banlance)
	{
		return WID_DBUS_SUCCESS;
	}


	AC_IP_GROUP[ID]->diff_count = diff_banlance;
	
	return WID_DBUS_SUCCESS;
}
int set_ac_ip_diffcount(unsigned char ID, char * ip, unsigned int diffcount){
	struct wid_ac_ip *tmp;
	struct wid_ac_ip *tmp1;
	if(AC_IP_GROUP[ID] == NULL)
		return WLAN_ID_NOT_EXIST;
	
	if(AC_IP_GROUP[ID]->ip_list == NULL){
		return AC_IP_NOT_EXIST;
	}

	tmp = AC_IP_GROUP[ID]->ip_list;
	
	if(strcmp(tmp->ip,ip)==0)
	{
		tmp->threshold = diffcount;
		return WID_DBUS_SUCCESS;
	}
	else
	{
		while(tmp->next != NULL)
		{
			tmp1 = tmp->next;
			if(strcmp(tmp1->ip,ip)==0)
			{				
				tmp1->threshold = diffcount;
				return WID_DBUS_SUCCESS;

			}
			tmp = tmp1;
		}
	}
	
	return AC_IP_NOT_EXIST;

}


//save sample throughput info
void save_sample_throughput_info(int WTPIndex, wid_sample_rate_info sample_throughput)
{
	unsigned int uplink = 0;
	unsigned int downlink = 0;
	if(sample_throughput.current_uplink_throughput == 0)
	{
		uplink = AC_WTP[WTPIndex]->wid_sample_throughput.current_uplink_throughput;
	}
	else
	{
		uplink = sample_throughput.current_uplink_throughput - AC_WTP[WTPIndex]->wid_sample_throughput.current_uplink_throughput;
	}
	if(sample_throughput.current_downlink_throughput == 0)
	{
		downlink = AC_WTP[WTPIndex]->wid_sample_throughput.current_downlink_throughput;
	}
	else
	{
		downlink = sample_throughput.current_downlink_throughput - AC_WTP[WTPIndex]->wid_sample_throughput.current_downlink_throughput;
	}
	
	AC_WTP[WTPIndex]->wid_sample_throughput.uplink_rate = uplink;
	AC_WTP[WTPIndex]->wid_sample_throughput.downlink_rate = downlink;

	AC_WTP[WTPIndex]->wid_sample_throughput.past_uplink_throughput = AC_WTP[WTPIndex]->wid_sample_throughput.current_uplink_throughput;
	AC_WTP[WTPIndex]->wid_sample_throughput.past_downlink_throughput = AC_WTP[WTPIndex]->wid_sample_throughput.current_downlink_throughput;

	AC_WTP[WTPIndex]->wid_sample_throughput.current_uplink_throughput = sample_throughput.current_uplink_throughput;
	AC_WTP[WTPIndex]->wid_sample_throughput.current_downlink_throughput = sample_throughput.current_downlink_throughput;
}

//save extension info
CWBool CWSaveWTPExtensionInfo2(wid_wifi_info ap_wifi_info, unsigned int WTPIndex)
{
	if(AC_WTP[WTPIndex] == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	int i = 0;
	AC_WTP[WTPIndex]->wifi_extension_info.cpu = ap_wifi_info.cpu;

	AC_WTP[WTPIndex]->wifi_extension_info.tx_mgmt = ap_wifi_info.tx_mgmt;
	AC_WTP[WTPIndex]->wifi_extension_info.rx_mgmt = ap_wifi_info.rx_mgmt;
	AC_WTP[WTPIndex]->wifi_extension_info.tx_packets = ap_wifi_info.tx_packets;
	AC_WTP[WTPIndex]->wifi_extension_info.tx_errors = ap_wifi_info.tx_errors;
	AC_WTP[WTPIndex]->wifi_extension_info.tx_retry = ap_wifi_info.tx_retry;

	AC_WTP[WTPIndex]->wifi_extension_info.tx_unicast = ap_wifi_info.tx_unicast;
	AC_WTP[WTPIndex]->wifi_extension_info.tx_broadcast = ap_wifi_info.tx_broadcast;
	AC_WTP[WTPIndex]->wifi_extension_info.tx_multicast = ap_wifi_info.tx_multicast;
	AC_WTP[WTPIndex]->wifi_extension_info.tx_drop = ap_wifi_info.tx_drop;
	AC_WTP[WTPIndex]->wifi_extension_info.rx_unicast = ap_wifi_info.rx_unicast;
	AC_WTP[WTPIndex]->wifi_extension_info.rx_broadcast = ap_wifi_info.rx_broadcast;
	AC_WTP[WTPIndex]->wifi_extension_info.rx_multicast = ap_wifi_info.rx_multicast;
	AC_WTP[WTPIndex]->wifi_extension_info.rx_drop = ap_wifi_info.rx_drop;

	/* zhangshu add for save, 2010-09-25 */
	AC_WTP[WTPIndex]->wifi_extension_info.wpi_replay_error = ap_wifi_info.wpi_replay_error;
	AC_WTP[WTPIndex]->wifi_extension_info.wpi_decryptable_error = ap_wifi_info.wpi_decryptable_error;
	AC_WTP[WTPIndex]->wifi_extension_info.wpi_mic_error = ap_wifi_info.wpi_mic_error;
	AC_WTP[WTPIndex]->wifi_extension_info.disassoc_unnormal = ap_wifi_info.disassoc_unnormal;
	AC_WTP[WTPIndex]->wifi_extension_info.rx_assoc_norate = ap_wifi_info.rx_assoc_norate;
	AC_WTP[WTPIndex]->wifi_extension_info.rx_assoc_capmismatch = ap_wifi_info.rx_assoc_capmismatch;
	AC_WTP[WTPIndex]->wifi_extension_info.assoc_invaild = ap_wifi_info.assoc_invaild;
	AC_WTP[WTPIndex]->wifi_extension_info.reassoc_deny = ap_wifi_info.reassoc_deny;
	
	AC_WTP[WTPIndex]->wifi_extension_info.ipmode = ap_wifi_info.ipmode;
	AC_WTP[WTPIndex]->wifi_extension_info.memoryall = ap_wifi_info.memoryall;
	AC_WTP[WTPIndex]->wifi_extension_info.memoryuse = ap_wifi_info.memoryuse;
	AC_WTP[WTPIndex]->wifi_extension_info.flashall = ap_wifi_info.flashall;
	AC_WTP[WTPIndex]->wifi_extension_info.flashempty = ap_wifi_info.flashempty;
	AC_WTP[WTPIndex]->wifi_extension_info.wifi_snr = ap_wifi_info.wifi_snr;
	AC_WTP[WTPIndex]->wifi_extension_info.eth_count = ap_wifi_info.eth_count;
	if(ap_wifi_info.ath_count <= AP_ATH_IF_NUM)
		AC_WTP[WTPIndex]->wifi_extension_info.ath_count = ap_wifi_info.ath_count;
	else{	
		wid_syslog_debug_debug(WID_DBUS,"%s something wrong ap_wifi_info.ath_count %d\n",ap_wifi_info.ath_count);
		AC_WTP[WTPIndex]->wifi_extension_info.ath_count = 1;
	}
	AC_WTP[WTPIndex]->wifi_extension_info.temperature = ap_wifi_info.temperature;
	AC_WTP[WTPIndex]->wifi_extension_info.wifi_count = ap_wifi_info.wifi_count;
		

	for(i=0;i<AP_ETH_IF_NUM;i++)
	{
		
		AC_WTP[WTPIndex]->wifi_extension_info.eth_updown_time[i] = ap_wifi_info.eth_updown_time[i];
	}
	for(i=0;i<AP_WIFI_IF_NUM;i++)
	{
		
		AC_WTP[WTPIndex]->wifi_extension_info.wifi_state[i] = ap_wifi_info.wifi_state[i];
	}
	for(i=0;i<AC_WTP[WTPIndex]->wifi_extension_info.ath_count;i++)
	{
		
		AC_WTP[WTPIndex]->wifi_extension_info.ath_if_info[i].radioid = ap_wifi_info.ath_if_info[i].radioid;
		AC_WTP[WTPIndex]->wifi_extension_info.ath_if_info[i].wlanid = ap_wifi_info.ath_if_info[i].wlanid;
		AC_WTP[WTPIndex]->wifi_extension_info.ath_if_info[i].ath_updown_times = ap_wifi_info.ath_if_info[i].ath_updown_times;
	}
	/*fengwenchao add 20120314 for onlinebug-162*///qiuchen copy from v1.3
	for(i =0;i < AC_WTP[WTPIndex]->wifi_extension_info.wifi_count;i++)
	{
		AC_WTP[WTPIndex]->wifi_extension_info.wifi_snr_new[i] = ap_wifi_info.wifi_snr_new[i];
		AC_WTP[WTPIndex]->wifi_extension_info.wifi_noise_new[i] = ap_wifi_info.wifi_noise_new[i];
	}
	/*fengwenchao add end*/
	return CW_TRUE;
}

void save_extension_info(int WTPIndex, wid_wifi_info ap_wifi_info)
{
	CWSaveWTPExtensionInfo2(ap_wifi_info,WTPIndex);	
	AsdWsm_WTPOp(WTPIndex, WID_WIFI_INFO);
	wid_parse_wtp_cpu_mem_trap_info(WTPIndex);
}

int wid_radio_set_ip_gateway_dns(int wtpid,unsigned int ip,unsigned int gateway,
								unsigned int mask,unsigned int fstdns,unsigned int snddns)

{
	msgq msg;
	
	AC_WTP[wtpid]->ap_ipadd = ip;
	AC_WTP[wtpid]->ap_gateway = gateway;
	AC_WTP[wtpid]->resetflag = 1;
	AC_WTP[wtpid]->ap_mask_new = mask;
	AC_WTP[wtpid]->ap_dnsfirst = fstdns;
	AC_WTP[wtpid]->ap_dnssecend = snddns;
	
	if((AC_WTP[wtpid] != NULL)&&(AC_WTP[wtpid]->WTPStat == 5))
	{
		AC_WTP[wtpid]->WTP_Radio[0]->CMD |= 0x1000;
		AC_WTP[wtpid]->CMD->radioid[0] += 1;
		AC_WTP[wtpid]->CMD->setCMD = 1;	
		int WTPIndex = wtpid;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SET_IP_DNS;
			
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}		
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}
	
	if(gtrapflag>=4){
		wid_dbus_trap_wtp_ip_change_alarm(wtpid);
		}

	return 0;	

}

int wid_set_ap_hotreboot(int hotreboot)
{
	int i = 0;
	char command[DEFAULT_LEN] = {0};
	memset(command,0,DEFAULT_LEN);
	sprintf(command,"/usr/sbin/set_ap_reboot_flag %d",hotreboot);
	
	wid_syslog_debug_debug(WID_DEFAULT,"command %s\n",command);
	
	for(i=0; i<WTP_NUM; i++)
	{
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
			wid_radio_set_extension_command(i,command);
		}
	}

	return 0;

}
CWBool delete_elem_into_wids_list(wid_wids_device  *paplist,struct tag_wids_device_ele *elem)
{

	if((elem == NULL)||((paplist) == NULL)||((paplist)->wids_device_info == NULL)||((paplist)->count== 0))
	{
		wid_syslog_debug_debug(WID_DEFAULT,"delete_elem_into_wids_list parameter error\n");
		return CW_FALSE;
	}	


	
	struct tag_wids_device_ele *pnode = paplist->wids_device_info;
	struct tag_wids_device_ele *pnext = pnode->next;

	if(memcmp((pnode->bssid),elem->bssid,6) == 0)
	{
		CW_FREE_OBJECT(pnode);
		paplist->wids_device_info = pnext;
		paplist->count--;
			
		return CW_TRUE;

	}

	while(pnext != NULL)
	{
		if(memcmp((pnext->bssid),elem->bssid,6) == 0)
		{
			
			pnode->next = pnext->next;
			
			CW_FREE_OBJECT(pnext);
			paplist->count--;


			return CW_TRUE;

		}	

		pnode = pnode->next;
		pnext = pnext->next;
	}

	return CW_FALSE;//insert success

}

int wid_add_del_wids_mac(unsigned char mac[],int isadd)
{
	if(wids_ignore_list == NULL)
	{
		CW_CREATE_OBJECT_ERR(wids_ignore_list, wid_wids_device, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
					
		wids_ignore_list->count = 0;
		wids_ignore_list->wids_device_info = NULL;
	}

	struct tag_wids_device_ele *wids_device_ele;
	CW_CREATE_OBJECT_SIZE_ERR(wids_device_ele, sizeof(struct tag_wids_device_ele), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	memcpy(wids_device_ele->bssid,mac, 6);
	
	int isfind = find_elem_in_wids_list(wids_ignore_list,wids_device_ele);

	if(isfind == CW_FALSE) //add element
	{
		if(isadd == CW_TRUE)
		{
			insert_elem_into_wids_list(wids_ignore_list,wids_device_ele);
		}
		else
		{
			CW_FREE_OBJECT(wids_device_ele);
		}
		
	}
	else 
	{
		if(isadd == CW_FALSE)
		{
			delete_elem_into_wids_list(wids_ignore_list,wids_device_ele);
			CW_FREE_OBJECT(wids_device_ele);
		}
		else{
			CW_FREE_OBJECT(wids_device_ele);
		}
	}

	return 0;
}
int wid_count_countermeasure_rogue_ap(Neighbor_AP_INFOS *paplist,int wtpid)
{

	unsigned int count=0;
	unsigned int RadioID = wtpid*L_RADIO_NUM;
	unsigned short RadioTxp = 20;     //fengwenchao modify 20110329
	CWBool flag = CW_FALSE;
	//fengwenchao add 20110328
	if(AC_RADIO[RadioID]->ishighpower == 1)
	{		
		RadioTxp = 27;
	}
	//fengwenchao add end
	if((paplist == NULL)||(paplist->neighborapInfos == NULL)||(paplist->neighborapInfosCount== 0))
	{
		return 0;
	}	
	struct Neighbor_AP_ELE *phead = paplist->neighborapInfos;
	
	while(phead!=NULL){

			if(phead->status == 1){
				count ++;
				break;
			}

		phead = phead->next;
	}

	wid_syslog_debug_debug(WID_DEFAULT," wid_count_countermeasure_rogue_ap is %d \n",count);
	//fengwenchao add 20110325
	if(count == 0)
	{
		AC_RADIO[RadioID]->radio_countermeasures_flag = 0;  //AP压制功率清零
	}
	//fengwenchao add end
	//countermeasure rouge ap added by weianying 2009/12/28
	if((gapscanset.countermeasures_switch == 1)&&(count >0))
	{
		//maybe add condition like rssi strength or check count
		//if((countermeasurecount==0)||(countermeasurecount%COUNTERMEASURE_DEFAULT_COUNT == 0))
		{
			//fengwenchao modify 20110325
			if(AC_RADIO[RadioID]->radio_countermeasures_flag == 0)   
			{
				WID_RADIO_SET_TXP(RadioID, RadioTxp,flag);
				AC_RADIO[RadioID]->radio_countermeasures_flag = 1;
			}
			//fengwenchao modify end
		}
		//countermeasurecount++;
	}	

	return 1;
}

/* wcl add for RDIR-33 */
int wid_radio_set_acktimeout_distance(unsigned int RadioID)
{
	msgq msg;
//	struct msgqlist *elem;

	int WTPIndex = RadioID/L_RADIO_NUM;
	if((AC_WTP[WTPIndex] != NULL)&&(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5))
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_acktimeout_distance;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		msg.mqinfo.u.RadioInfo.Radio_Op = Radio_acktimeout_distance;
		msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
	
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		elem = NULL;
	}*/
	
	return 0;

}
int wid_radio_set_guard_interval(unsigned int RadioID)
{
	msgq msg;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		//return RADIO_IS_DISABLE;
	}

	int WTPIndex = AC_RADIO[RadioID]->WTPID;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, guardinterval",RadioID);
	
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_GI_MCS_CMMODE;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_GI_MCS_CMMODE;
		msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
		struct msgqlist *elem;
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		elem = NULL;
	}*/

	return 0;

}

/* zhangshu add for set 11n para, 2010-10-09 */
int wid_radio_set_ampdu_able(unsigned int RadioID, unsigned char type)
{
	msgq msg;
//	struct msgqlist *elem;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		//return RADIO_IS_DISABLE;
	}

	int WTPIndex = RadioID/L_RADIO_NUM;
	if((AC_WTP[WTPIndex] != NULL)&&(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5))
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			if(type == 1)
			{
			    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_ampdu_op;
			}
			else
			{
			    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_amsdu_op;
			}
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		if(type == 1)
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_ampdu_op;
		}
		else
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_amsdu_op;
		}
		msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
	
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		elem = NULL;
	}*/
	
	return 0;

}

int wid_radio_set_ampdu_limit(unsigned int RadioID, unsigned char type)
{
	msgq msg;
//	struct msgqlist *elem;
	
	int WTPIndex = RadioID/L_RADIO_NUM;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
			//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			if(type == 1)
    		{
    		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_ampdu_op;
    		}
    		else
    		{
    		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_amsdu_op;
    		}
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		if(type == 1)
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_ampdu_op;
		}
		else
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_amsdu_op;
		}
		msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
	
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		elem = NULL;
	}*/
	return 0;

}
//qiuchen add it
int WID_RADIO_CHANGE_SUPPORT_RATE_BYGI_MCS_CWMODE_1(unsigned int RadioID, int count)
{
	int rate =0;
	int i = 0;
	if(AC_RADIO[RadioID] != NULL)
	{
		if((AC_RADIO[RadioID]->cwmode == 0)&&(AC_RADIO[RadioID]->chainmask_num == 1)&&((AC_RADIO[RadioID]->guardinterval == 1)||(AC_RADIO[RadioID]->guardinterval == 0)))  //ht20   单流
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
				
			rate = 65;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 130;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 190;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 260;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 390;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 520;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 585;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 650;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			for(i=0;i<count;i++){

			if(AC_RADIO[RadioID]->mcs_list[i] == 6)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 5)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 4)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 3)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 2)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 1)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,190);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 0)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,190);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,130);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}	
			}
		}
		else if((AC_RADIO[RadioID]->cwmode == 0)&&(AC_RADIO[RadioID]->chainmask_num == 2)&&((AC_RADIO[RadioID]->guardinterval ==1)||(AC_RADIO[RadioID]->guardinterval == 0)))  //ht20  双流
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 130;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 260;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 390;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 520;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 780;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1040;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1170;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1300;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			for(i=0;i<count;i++){
			
			if(AC_RADIO[RadioID]->mcs_list[i] == 14)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 13)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 12)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 11)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 10)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 9)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 8)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}	
			}
		}
		else if((AC_RADIO[RadioID]->cwmode == 0)&&(AC_RADIO[RadioID]->chainmask_num == 3)&&((AC_RADIO[RadioID]->guardinterval == 1)||(AC_RADIO[RadioID]->guardinterval == 0)))  //ht20  三流
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 195;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 390;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 585;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 780;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1170;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1560;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1755;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1950;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			for(i=0;i<count;i++){
			
			if(AC_RADIO[RadioID]->mcs_list[i] == 22)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 21)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 20)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 19)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 18)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 17)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 16)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			}
		}		
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 1)&&(AC_RADIO[RadioID]->guardinterval == 0))//ht20/40 or ht40, 单流，GI800
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 135;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 270;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 405;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 540;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 810;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1080;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1215;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1350;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

			for(i=0;i<count;i++){
			if(AC_RADIO[RadioID]->mcs_list[i] == 6)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 5)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 4)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 3)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 2)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 1)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,405);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 0)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,405);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,270);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			}
		}
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 2)&&(AC_RADIO[RadioID]->guardinterval == 0))//ht20/40 or ht40, 双流，GI800
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 270;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 540;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 810;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1080;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1620;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2160;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2430;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 2700;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

			for(i=0;i<count;i++){
			if(AC_RADIO[RadioID]->mcs_list[i] == 14)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 13)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 12)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 11)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 10)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 9)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 8)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			}
		}
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 3)&&(AC_RADIO[RadioID]->guardinterval == 0))//ht20/40 or ht40, 三流 GI 800
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 405;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 810;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1215;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1620;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 2430;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 3240;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 3645;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 4050;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

			for(i=0;i<count;i++){
			if(AC_RADIO[RadioID]->mcs_list[i] == 22)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 21)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 20)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 19)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 18)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 17)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 16)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			}
		}		
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 1)&&(AC_RADIO[RadioID]->guardinterval == 1))//ht20/40 or ht40, 单流，GI400
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 150;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 300;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 450;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 600;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 900;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1200;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1350;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1500;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

			for(i=0;i<count;i++){
			if(AC_RADIO[RadioID]->mcs_list[i] == 6)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 5)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 4)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 3)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 2)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 1)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,450);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 0)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,450);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,300);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			}
		}		
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 2)&&(AC_RADIO[RadioID]->guardinterval == 1))//ht20/40 or ht40, 双流，GI400
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 300;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 600;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 900;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1200;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1800;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2400;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2700;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 3000;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

			for(i=0;i<count;i++){
			if(AC_RADIO[RadioID]->mcs_list[i] == 14)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 13)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 12)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 11)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 10)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 9)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 8)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			}
		}				
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 3)&&(AC_RADIO[RadioID]->guardinterval == 1))//ht20/40 or ht40, 双流，GI400
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
								
			rate = 450;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 900;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1350;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1800;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 2700;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 3600;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 4050;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 4500;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

			for(i=0;i<count;i++){
			if(AC_RADIO[RadioID]->mcs_list[i] == 22)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 21)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 20)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 19)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 18)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 17)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs_list[i] == 16)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			}
		}			
	}
	//printf("AC_RADIO[%d]->mcs = %d \n",RadioID,AC_RADIO[RadioID]->mcs);
	return 0;
}
/*fengwenchao add 20110408*/
int WID_RADIO_CHANGE_SUPPORT_RATE_BYGI_MCS_CWMODE(unsigned int RadioID)
{
	int rate =0;
	if(AC_RADIO[RadioID] != NULL)
	{
		if((AC_RADIO[RadioID]->cwmode == 0)&&(AC_RADIO[RadioID]->chainmask_num == 1)&&((AC_RADIO[RadioID]->guardinterval == 1)||(AC_RADIO[RadioID]->guardinterval == 0)))  //ht20   单流
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}
			rate = 65;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 130;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 190;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 260;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 390;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 520;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 585;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 650;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);

			if(AC_RADIO[RadioID]->mcs == 6)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 5)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 4)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 3)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 2)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 1)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,190);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 0)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,650);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,190);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,130);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}				
		}
		else if((AC_RADIO[RadioID]->cwmode == 0)&&(AC_RADIO[RadioID]->chainmask_num == 2)&&((AC_RADIO[RadioID]->guardinterval ==1)||(AC_RADIO[RadioID]->guardinterval == 0)))  //ht20  双流
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}		
			rate = 130;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 260;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 390;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 520;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 780;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1040;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1170;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1300;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 14)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 13)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 12)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 11)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 10)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 9)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 8)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1300);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1040);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,520);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,260);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}					
		}
		else if((AC_RADIO[RadioID]->cwmode == 0)&&(AC_RADIO[RadioID]->chainmask_num == 3)&&((AC_RADIO[RadioID]->guardinterval == 1)||(AC_RADIO[RadioID]->guardinterval == 0)))  //ht20  三流
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}	
			rate = 195;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 390;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 585;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 780;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1170;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1560;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1755;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1950;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 22)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 21)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 20)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 19)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 18)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 17)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 16)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1950);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1755);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1560);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1170);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,780);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,585);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,390);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}					
		}		
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 1)&&(AC_RADIO[RadioID]->guardinterval == 0))//ht20/40 or ht40, 单流，GI800
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}	
			rate = 135;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 270;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 405;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 540;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 810;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1080;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

			rate = 1215;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1350;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					

			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 6)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 5)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 4)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 3)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 2)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 1)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,405);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 0)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,405);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,270);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}			
		}
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 2)&&(AC_RADIO[RadioID]->guardinterval == 0))//ht20/40 or ht40, 双流，GI800
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}		
			rate = 270;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 540;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 810;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1080;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1620;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2160;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2430;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 2700;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 14)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 13)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 12)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 11)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 10)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 9)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 8)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2160);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1080);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,540);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}			
		}
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 3)&&(AC_RADIO[RadioID]->guardinterval == 0))//ht20/40 or ht40, 三流 GI 800
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}	
			rate = 405;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 810;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1215;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1620;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 2430;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 3240;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 3645;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 4050;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 22)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 21)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 20)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 19)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 18)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 17)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 16)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3645);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3240);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2430);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1620);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1215);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,810);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}			
		}		
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 1)&&(AC_RADIO[RadioID]->guardinterval == 1))//ht20/40 or ht40, 单流，GI400
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}		
			rate = 150;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 300;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 450;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 600;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 900;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1200;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1350;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1500;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 6)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 5)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 4)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 3)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 2)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 1)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,450);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 0)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,450);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,300);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}			
		}		
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 2)&&(AC_RADIO[RadioID]->guardinterval == 1))//ht20/40 or ht40, 双流，GI400
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}
			rate = 300;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 600;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 900;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1200;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 1800;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2400;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 2700;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 3000;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 14)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 13)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 12)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 11)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 10)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 9)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 8)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3000);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2400);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1200);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,600);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}			
		}				
		else if(((AC_RADIO[RadioID]->cwmode == 1)||(AC_RADIO[RadioID]->cwmode == 2))&&(AC_RADIO[RadioID]->chainmask_num == 3)&&(AC_RADIO[RadioID]->guardinterval == 1))//ht20/40 or ht40, 双流，GI400
		{
			AC_RADIO[RadioID]->Support_Rate_Count = 8;
			destroy_support_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			AC_RADIO[RadioID]->Radio_Rate = create_support_rate_list(1);
			{
				if (AC_RADIO[RadioID]->Radio_Type & 0x01
					|| (AC_RADIO[RadioID]->Radio_Type & 0x02)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
				if ((AC_RADIO[RadioID]->Radio_Type & 0x01)
					|| (AC_RADIO[RadioID]->Radio_Type & 0x04))
				{ 
					 rate = 10;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 20;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

					rate = 55;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
					
					rate = 110;
					AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				} 

				rate = 60;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 90;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 120;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 180;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 240;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 360;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);

				rate = 480;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
 
				rate = 540;
				AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
				
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
				wid_syslog_debug_debug(WID_DEFAULT, " %s %d: add basic set to the radio, according to Radio_Type", __func__, __LINE__);
				}
			}
			rate = 450;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 900;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1350;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 1800;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 2700;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 3600;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
		
			rate = 4050;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);
			
			rate = 4500;
			AC_RADIO[RadioID]->Radio_Rate = insert_rate_into_list(AC_RADIO[RadioID]->Radio_Rate,rate);					
		
			AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,0);
			AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			
			if(AC_RADIO[RadioID]->mcs == 22)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 21)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 20)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 19)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 18)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 17)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}
			else if(AC_RADIO[RadioID]->mcs == 16)
			{
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4500);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,4050);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,3600);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,2700);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1800);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,1350);
				AC_RADIO[RadioID]->Radio_Rate = delete_rate_from_list(AC_RADIO[RadioID]->Radio_Rate,900);
				AC_RADIO[RadioID]->Support_Rate_Count = length_of_rate_list(AC_RADIO[RadioID]->Radio_Rate);
			}			
		} 
	}
	//printf("AC_RADIO[%d]->mcs = %d \n",RadioID,AC_RADIO[RadioID]->mcs);
	return 0;
}
/*fengwenchao add end*/

int wid_radio_set_ampdu_subframe(unsigned int RadioID, unsigned char type)
{
	msgq msg;
//	struct msgqlist *elem;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
	//	return RADIO_IS_DISABLE;
	}


			int WTPIndex = RadioID/L_RADIO_NUM;
		if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
		{
			AC_RADIO[RadioID]->CMD |= 0x20;
			AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
			AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
				//int WTPIndex = AC_RADIO[RadioID]->WTPID;
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = Radio_S_TYPE;
            if(type == 1)
    		{
    		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_ampdu_op;
    		}
    		else
    		{
    		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_amsdu_op;
    		}
				msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
				msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
				{
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}

			}
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
			}//delete unuseful cod
			/*else if((AC_WTP[WTPIndex] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
        if(type == 1)
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_ampdu_op;
		}
		else
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = Radio_amsdu_op;
		}
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
		
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
			elem = NULL;
		}*/

	return 0;

}
/* zhangshu add for set 11n para END*/


int wid_radio_set_mixed_puren_switch(unsigned int RadioID)
{
	msgq msg;
//	struct msgqlist *elem;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		//return RADIO_IS_DISABLE;
	}

	int WTPIndex = RadioID/L_RADIO_NUM;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_puren_mixed_op;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_puren_mixed_op;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
		
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
			elem = NULL;
		}*/
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, Mixed_Greenfield is %s",RadioID,(AC_RADIO[RadioID]->MixedGreenfield.Mixed_Greenfield == 1)?"puren":"mixed");
	return 0;

}
/*fengwenchao add 20110421*/
void wid_check_radio_max_min_channel(unsigned int RadioID,unsigned int *max_channel,unsigned int *min_channel)
{
	if(AC_RADIO[RadioID] != NULL)
	{
		switch(AC_RADIO[RadioID]->Radio_country_code)/*wcl modify for AUTELAN-2765*/
		{
			case COUNTRY_CHINA_CN : 	
									if (((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 26))&&((AC_RADIO[RadioID]->Radio_Chan == 149)\
										||(AC_RADIO[RadioID]->Radio_Chan == 153)||(AC_RADIO[RadioID]->Radio_Chan == 157)||(AC_RADIO[RadioID]->Radio_Chan == 161)\
										||(AC_RADIO[RadioID]->Radio_Chan == 165)))
									{
										*max_channel = 159;
										*min_channel = 7;
									}else{
										*max_channel = 9;
										*min_channel = 5;
									}
		
									break;
									
			case COUNTRY_EUROPE_EU : 
									if (((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 26))&&((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)\
										||(AC_RADIO[RadioID]->Radio_Chan == 56)||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)||(AC_RADIO[RadioID]->Radio_Chan == 116)\
										||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)||(AC_RADIO[RadioID]->Radio_Chan == 140))) 
									{ /*wcl modify for AUTELAN-2765*/
										*max_channel = 134;
										*min_channel = 7;
									}else{
										*max_channel = 9;
										*min_channel = 5;
									}	
									break;
																	
			case COUNTRY_USA_US : 
									if (((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 26))&&((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)\
										||(AC_RADIO[RadioID]->Radio_Chan == 56)||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)||(AC_RADIO[RadioID]->Radio_Chan == 116)\
										||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)||(AC_RADIO[RadioID]->Radio_Chan == 140)\
										||(AC_RADIO[RadioID]->Radio_Chan == 149)||(AC_RADIO[RadioID]->Radio_Chan == 153)||(AC_RADIO[RadioID]->Radio_Chan == 157)||(AC_RADIO[RadioID]->Radio_Chan == 161)||(AC_RADIO[RadioID]->Radio_Chan == 165)) )
									{ /*wcl modify for AUTELAN-2765*/
										*max_channel = 159;
										*min_channel = 7;
									}else{
										*max_channel = 7;
										*min_channel = 5;
									}
									break;
																	
			case COUNTRY_JAPAN_JP : 
									if (((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)\
										||(AC_RADIO[RadioID]->Radio_Chan == 56)||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)||(AC_RADIO[RadioID]->Radio_Chan == 116)\
										||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)||(AC_RADIO[RadioID]->Radio_Chan == 140)\
										||(AC_RADIO[RadioID]->Radio_Chan == 184)||(AC_RADIO[RadioID]->Radio_Chan == 188)||(AC_RADIO[RadioID]->Radio_Chan == 192)||(AC_RADIO[RadioID]->Radio_Chan == 196)))
									{ /*wcl modify for AUTELAN-2765*/
										*max_channel = 40;
										*min_channel = 7;
									}else{
										*max_channel = 10;
										*min_channel = 5;
									}										
									break;
																	
			case COUNTRY_FRANCE_FR : 
									if (((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 26))&&((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)\
										||(AC_RADIO[RadioID]->Radio_Chan == 56)||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)||(AC_RADIO[RadioID]->Radio_Chan == 116)\
										||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)||(AC_RADIO[RadioID]->Radio_Chan == 140))) 
									{ /*wcl modify for AUTELAN-2765*/
										*max_channel = 134;
										*min_channel = 7;
									}else{
										*max_channel = 7;
										*min_channel = 7;
									}	
									break;
																	
			case COUNTRY_SPAIN_ES : 
									if (((AC_RADIO[RadioID]->Radio_Type&IEEE80211_11A)||(AC_RADIO[RadioID]->Radio_Type == 10)||(AC_RADIO[RadioID]->Radio_Type == 26))&&((AC_RADIO[RadioID]->Radio_Chan == 36)||(AC_RADIO[RadioID]->Radio_Chan == 40)||(AC_RADIO[RadioID]->Radio_Chan == 44)||(AC_RADIO[RadioID]->Radio_Chan == 48)||(AC_RADIO[RadioID]->Radio_Chan == 52)\
										||(AC_RADIO[RadioID]->Radio_Chan == 56)||(AC_RADIO[RadioID]->Radio_Chan == 60)||(AC_RADIO[RadioID]->Radio_Chan == 64)||(AC_RADIO[RadioID]->Radio_Chan == 100)||(AC_RADIO[RadioID]->Radio_Chan == 104)||(AC_RADIO[RadioID]->Radio_Chan == 108)||(AC_RADIO[RadioID]->Radio_Chan == 112)||(AC_RADIO[RadioID]->Radio_Chan == 116)\
										||(AC_RADIO[RadioID]->Radio_Chan == 120)||(AC_RADIO[RadioID]->Radio_Chan == 124)||(AC_RADIO[RadioID]->Radio_Chan == 128)||(AC_RADIO[RadioID]->Radio_Chan == 132)||(AC_RADIO[RadioID]->Radio_Chan == 136)||(AC_RADIO[RadioID]->Radio_Chan == 140))) 
									{ /*wcl modify for AUTELAN-2765*/
										*max_channel = 134;
										*min_channel = 7;
									}else{
										*max_channel = 5;
										*min_channel = 7;
									}	
									break;
		
			default : 
			break;
		}		
	}
  return;
}
/*fengwenchao add end*/
/*fengwenchao add 20110422*/
int wid_set_ap_statistics_v1(int WTPID,int apstatics)
{
	msgq msg;
	if((AC_WTP[WTPID] != NULL)&&(AC_WTP[WTPID]->WTPStat == 5))
	{
//			printf("## wid_set_ap_statistics wtp id= %d##\n",i);
		
		AC_WTP[WTPID]->WTP_Radio[0]->CMD |= 0x0100;
		AC_WTP[WTPID]->CMD->radioid[0] += 1;
		AC_WTP[WTPID]->CMD->setCMD = 1; 
		int WTPIndex = WTPID;
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STATISTICS_REPORT;
			msg.mqinfo.u.WtpInfo.value2 = apstatics;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}		
	}
	return 0;
}
/*fengwenchao add end*/
int wid_radio_set_channel_Extoffset(unsigned int RadioID)
{
	msgq msg;
//	struct msgqlist *elem;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		//return RADIO_IS_DISABLE;
	}

	int WTPIndex = RadioID/L_RADIO_NUM;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_channel_ext_offset;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = Radio_S_TYPE;
				msg.mqinfo.u.RadioInfo.Radio_Op = Radio_channel_ext_offset;
				msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
				msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			
				elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem == NULL){
					wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
				elem->next = NULL;
				memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
				WID_INSERT_CONTROL_LIST(WTPIndex, elem);
				elem = NULL;
			}*/
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, channel_offset is %d",RadioID,AC_RADIO[RadioID]->channel_offset);
	return 0;

}

#if 0
int wid_radio_set_tx_chainmask(unsigned int RadioID)
{
	//printf("in fuc wid_radio_set_tx_chainmask\n");
	msgq msg;
	struct msgqlist *elem;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		//return RADIO_IS_DISABLE;
	}
	//printf("in fuc %d\n",state);
	//printf("in fuc %d\n",AC_RADIO[RadioID]->tx_chainmask_state_value);
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, tx_chainmask_state_value %d",RadioID,AC_RADIO[RadioID]->tx_chainmask_state_value);
	int WTPIndex = RadioID/L_RADIO_NUM;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = RAdio_tx_chainmask;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}	else if((AC_WTP[WTPIndex] != NULL)){
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = Radio_S_TYPE;
				msg.mqinfo.u.RadioInfo.Radio_Op = RAdio_tx_chainmask;
				msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
				msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			
				elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem == NULL){
					wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
				elem->next = NULL;
				memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
				WID_INSERT_CONTROL_LIST(WTPIndex, elem);
				elem = NULL;
			}

	return 0;

}
#endif

/* zhangshu add for set chainmask, 2010-10-09 */
int wid_radio_set_chainmask(unsigned int RadioID, unsigned char type)
{
	wid_syslog_debug_debug(WID_DEFAULT,"@@@@@@ in fuc wid_radio_set_chainmask @@@@@\n");
	msgq msg;
//	struct msgqlist *elem;
	
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is : %d\n",RadioID);
	wid_syslog_debug_debug(WID_DEFAULT,"chainmask type is : %d\n",type);
	wid_syslog_debug_debug(WID_DEFAULT,"tx_chainmask_state_value %d\n",AC_RADIO[RadioID]->tx_chainmask_state_value);
	wid_syslog_debug_debug(WID_DEFAULT,"rx_chainmask_state_value %d\n",AC_RADIO[RadioID]->rx_chainmask_state_value);
	int WTPIndex = RadioID/L_RADIO_NUM;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			if(type == 1)
			{
			    msg.mqinfo.u.RadioInfo.Radio_Op = RAdio_tx_chainmask;
			}
			else
			{
			    msg.mqinfo.u.RadioInfo.Radio_Op = RAdio_rx_chainmask;
			}
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL))
	{
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = Radio_S_TYPE;
		if(type == 1)
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = RAdio_tx_chainmask;
		}
		else
		{
		    msg.mqinfo.u.RadioInfo.Radio_Op = RAdio_rx_chainmask;
		}
		msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
		msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
	
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return 0;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		elem = NULL;
	}*/

	return 0;

}
/* zhangshu add for set chainmask END */

int wid_radio_set_mcs(unsigned int RadioID)
{
	msgq msg;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		//return RADIO_IS_DISABLE;
	}

	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, mcs",RadioID);
	int WTPIndex = AC_RADIO[RadioID]->WTPID;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
	//	int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_GI_MCS_CMMODE;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = Radio_S_TYPE;
				msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_GI_MCS_CMMODE;
				msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
				msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
				struct msgqlist *elem;
				elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem == NULL){
					wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
				elem->next = NULL;
				memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
				WID_INSERT_CONTROL_LIST(WTPIndex, elem);
				elem = NULL;
			}*/

	return 0;

}
/*fengwenchao add 20120314 for requirements-407*/
int check_ac_whether_or_not_set_mcs_list(unsigned int WTPIndex,unsigned int l_radioid)
{
	int ret =0;
	int j =0;
	if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->chainmask_num == 1)
	{
		if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->mcs_count != 8)
		{
			ret = 1;
			return ret;
		}
		for(j=0;j < 8; j++)
		{
			if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->mcs_list[j] != j)
			{
				ret = 1;
				return ret;
			}
		}
	}
	else if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->chainmask_num == 2)
	{
		if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->mcs_count != 16)
		{
			ret = 1;
			return ret;
		}

		for(j=0;j < 16; j++)
		{
			if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->mcs_list[j] != j)
			{
				ret = 1;
				return ret;
			}
		}
	}
	else if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->chainmask_num == 3)
	{
		if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->mcs_count != 24)
		{
			ret = 1;
			return ret;
		}

		for(j=0;j < 24; j++)
		{
			if(AC_WTP[WTPIndex]->WTP_Radio[l_radioid]->mcs_list[j] != j)
			{
				ret = 1;
				return ret;
			}
		}		
	}
	return ret;
}
int wid_radio_set_mcs_list(unsigned int RadioID)
{
	msgq msg;
	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, mcs list",RadioID);
	int WTPIndex = AC_RADIO[RadioID]->WTPID;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
	//	int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_MCS_LIST;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = Radio_S_TYPE;
				msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_MCS_LIST;
				msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
				msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
				struct msgqlist *elem;
				elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem == NULL){
					wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return 0;
				}
				memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
				elem->next = NULL;
				memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
				WID_INSERT_CONTROL_LIST(WTPIndex, elem);
				elem = NULL;
			}*/

	return 0;

}
/*fengwenchao add end*/
int wid_radio_set_cmmode(unsigned int RadioID)
{
	msgq msg;
	//if((gWTPs[AC_RADIO[RadioID]->WTPID].currentState == CW_ENTER_RUN)&&(AC_RADIO[RadioID]->AdStat == 2))
	{
		//return RADIO_IS_DISABLE;
	}

	wid_syslog_debug_debug(WID_DEFAULT,"set radio id is:%d, cmmode",RadioID);
	int WTPIndex = AC_RADIO[RadioID]->WTPID;
	if(AC_WTP[AC_RADIO[RadioID]->WTPID]->WTPStat == 5)
	{
		AC_RADIO[RadioID]->CMD |= 0x20;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->radioid[AC_RADIO[RadioID]->Radio_L_ID] += 1;
		AC_WTP[AC_RADIO[RadioID]->WTPID]->CMD->setCMD = 1;	
		//int WTPIndex = AC_RADIO[RadioID]->WTPID;
		CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
		if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_GI_MCS_CMMODE;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			{
				wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}

		}
		CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
	}//delete unuseful cod
	/*else if((AC_WTP[WTPIndex] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_11N_GI_MCS_CMMODE;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;
			struct msgqlist *elem;
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
			elem = NULL;
		}*/

	return 0;

}

int WID_BINDING_IF_APPLY_WTP_ipv6_ioctl(unsigned int WtpID, char * ifname)
{
	
	if(AC_WTP[WtpID]->isused == 1)
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** error this WTP is used and active, you can not binding interface ***\n");
		return WTP_BE_USING;
	}

	int isystemindex = 1;
	int ret;
	int i = 0;
	struct ifi * tmp;
	struct ifi * tmp2;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi_tmp->ifi_name,0,sizeof(ifi_tmp->ifi_name));
	strncpy(ifi_tmp->ifi_name,ifname,sizeof(ifi_tmp->ifi_name));

	struct tag_ipv6_addr_list *ipv6list = NULL;
	struct tag_ipv6_addr *ipv6addr = NULL;
	
	if(WID_IF_V6 == NULL)
	{
		//get ipv6 adress
		ipv6list = get_ipv6_addr_list(ifname);
		
		if(ipv6list != NULL)
		{
			//display_ipv6_addr_list(ipv6list);
			tmp = (struct ifi*)malloc(sizeof(struct ifi));
			memset(tmp,0,sizeof(struct ifi));
			memcpy(tmp->ifi_name,ifname,strlen(ifname));
			tmp->ifi_index = ipv6list->ifindex;
			ifi_tmp->ifi_index = ipv6list->ifindex; 
			WID_IF_V6 = tmp;
		}
		else
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return -1;
		}
		
		ipv6addr = ipv6list->ipv6list;
			
		ifi_tmp->ifi_addr6 = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
		isystemindex = ipv6list->ifindex;

		for(i=0; i<ipv6list->ipv6num; i++)
		{
			inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
			ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
			//ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
			ipv6addr = ipv6addr->next;
		}	

		if(ipv6list->ipv6num == 0)
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return ret;
		}
		
	}
	else
	{
		tmp = WID_IF_V6;
		while(tmp != NULL)
		{
			printf("ifname = %s name = %s\n",tmp->ifi_name,ifname);
			if(( strlen(ifname) ==	strlen(tmp->ifi_name))&&(strcmp(tmp->ifi_name,ifname)==0))
			{	
				free(ifi_tmp->ifi_addr6);
				ifi_tmp->ifi_addr6 = NULL;
				free(ifi_tmp);
				ifi_tmp = NULL;
				free_ipv6_addr_list(ipv6list);

				if(AC_WTP[WtpID] != NULL)
				{
					
					AC_WTP[WtpID]->BindingSystemIndex= tmp->ifi_index;
					//printf("AC_WTP[WtpID]->BindingSystemIndex = %d\n",AC_WTP[WtpID]->BindingSystemIndex);
					AC_WTP[WtpID]->isipv6addr = 1;
					memset(AC_WTP[WtpID]->BindingIFName, 0, ETH_IF_NAME_LEN);
					memcpy(AC_WTP[WtpID]->BindingIFName,ifname, strlen(ifname));
					
					wid_syslog_debug_debug(WID_DEFAULT,"*** binding iterface name to wtp success ***\n");
					return 0;
				}
				else
				{
					wid_syslog_debug_debug(WID_DEFAULT,"*** can't binding iterface name, please make sure you have create wtp ***\n");
					return WTP_ID_NOT_EXIST;
				}
				
				return 0;
			}
			tmp = tmp->ifi_next;
		}

		//get ipv6 address list
		
		ipv6list = get_ipv6_addr_list(ifname);
		
		if(ipv6list != NULL)
		{
			display_ipv6_addr_list(ipv6list);
			tmp = (struct ifi*)malloc(sizeof(struct ifi));
			memset(tmp,0,sizeof(struct ifi));
			memcpy(tmp->ifi_name,ifname,strlen(ifname));
			tmp->ifi_index = ipv6list->ifindex;
			ifi_tmp->ifi_index = ipv6list->ifindex;
			tmp2 = WID_IF_V6;
			WID_IF_V6 = tmp;
			WID_IF_V6->ifi_next = tmp2;	

		}
		else
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return -1;
		}

		ipv6addr = ipv6list->ipv6list;
			
		ifi_tmp->ifi_addr6 = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
		isystemindex = ipv6list->ifindex;

		
		for(i=0; i<ipv6list->ipv6num; i++)
		{
			inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
			ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
			//ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
			ipv6addr = ipv6addr->next;
		}	

		if(ipv6list->ipv6num == 0)
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return ret;
		}

	
	}	
		
	gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
	gInterfacesCountIpv4 = CWNetworkCountInterfaceAddressesIpv4(&gACSocket);
	gInterfacesCountIpv6 = CWNetworkCountInterfaceAddressesIpv6(&gACSocket);
	free(ifi_tmp->ifi_addr6);
	ifi_tmp->ifi_addr6 = NULL;
	free(ifi_tmp);
	ifi_tmp = NULL;
	free_ipv6_addr_list(ipv6list);

	if(ret != 0)
	return ret;

	
	if(AC_WTP[WtpID] != NULL)
	{
		
		AC_WTP[WtpID]->BindingSystemIndex= isystemindex;
		AC_WTP[WtpID]->isipv6addr = 1;
		memset(AC_WTP[WtpID]->BindingIFName, 0, ETH_IF_NAME_LEN);
		memcpy(AC_WTP[WtpID]->BindingIFName,ifname, strlen(ifname));
		
		wid_syslog_debug_debug(WID_DEFAULT,"*** binding iterface name to wtp success ***\n");
		return 0;
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"*** can't binding iterface name, please make sure you have create wtp ***\n");
		return WTP_ID_NOT_EXIST;
	}
		
}

int WID_ADD_IF_APPLY_WLAN_ipv6_ioctl(unsigned char WlanID, char * ifname)
{
	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}	
	
	struct ifreq	ifr;
	struct ifi *wif;
	struct ifi *wifnext;	
	struct ifi * tmp;
	struct ifi * tmp2;
	int i = 0;
	int ret;
	int isystemindex;
	ret = Check_And_Bind_Interface_For_WID(ifname);
	if(ret != 0)
		return ret;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi_tmp->ifi_name,0,sizeof(ifi_tmp->ifi_name));
	strncpy(ifi_tmp->ifi_name,ifname,sizeof(ifi_tmp->ifi_name));

	struct tag_ipv6_addr_list *ipv6list = NULL;
	
	struct tag_ipv6_addr *ipv6addr = NULL;
		
	ifi_tmp->ifi_addr6 = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));

	
	if(WID_IF_V6== NULL)
	{
		//printf("get addr before\n");
		ipv6list = get_ipv6_addr_list(ifname);
		//printf("get addr before\n");
		
		if(ipv6list != NULL)
		{
			//display_ipv6_addr_list(ipv6list);
			tmp = (struct ifi*)malloc(sizeof(struct ifi));
			memset(tmp,0,sizeof(struct ifi));
			memcpy(tmp->ifi_name,ifname,strlen(ifname));
			
			tmp->ifi_index = ipv6list->ifindex;
			ifi_tmp->ifi_index  = ipv6list->ifindex;
			printf("tmp->ifi_index = %d\n",tmp->ifi_index);
			tmp->isipv6addr = 1;
			WID_IF_V6 = tmp;

		}
		else
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return -1;
		}
		
		ipv6addr = ipv6list->ipv6list;

		isystemindex = ipv6list->ifindex;

		for(i=0; i<ipv6list->ipv6num; i++)
		{
			inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
			ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
			//ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
			WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
			ipv6addr = ipv6addr->next;
		}	

		if(ipv6list->ipv6num == 0)
		{
			free(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
			free(ifi_tmp);
			ifi_tmp = NULL;
			free_ipv6_addr_list(ipv6list);
			return ret;
		}

		
	}
	else
	{
		tmp = WID_IF_V6;
		while(tmp != NULL)
		{
			if((strlen(ifname) == strlen(tmp->ifi_name))&&(strcmp(tmp->ifi_name,ifname)==0))
			{
				/*wcl add*/
				ipv6list = get_ipv6_addr_list(ifname);
				tmp->ifi_index = ipv6list->ifindex;
				/*end*/				
				isystemindex = tmp->ifi_index;
				break;
			}
			tmp = tmp->ifi_next;
		}

		if(tmp == NULL)
		{
			ipv6list = get_ipv6_addr_list(ifname);
			
			if(ipv6list != NULL)
			{
				display_ipv6_addr_list(ipv6list);
				tmp = (struct ifi*)malloc(sizeof(struct ifi));
				memset(tmp,0,sizeof(struct ifi));
				memcpy(tmp->ifi_name,ifname,strlen(ifname));
				tmp->ifi_index = ipv6list->ifindex;
				ifi_tmp->ifi_index = ipv6list->ifindex;
				tmp->isipv6addr = 1;
				tmp2 = WID_IF_V6;
				WID_IF_V6 = tmp;
				WID_IF_V6->ifi_next = tmp2; 

			
			}
			else
			{
				free(ifi_tmp->ifi_addr6);
				ifi_tmp->ifi_addr6 = NULL;
				free(ifi_tmp);
				ifi_tmp = NULL;
				free_ipv6_addr_list(ipv6list);
				return -1;
			}
			ipv6addr = ipv6list->ipv6list;
			
			isystemindex = ipv6list->ifindex;
			for(i=0; i<ipv6list->ipv6num; i++)
			{
				inet_pton(AF_INET6, (char*)ipv6addr->ipv6addr, &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
				ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
				//ret = ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
				WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
				ipv6addr = ipv6addr->next;
			}	

			if(ipv6list->ipv6num == 0); 
			{
				free(ifi_tmp->ifi_addr6);
				ifi_tmp->ifi_addr6 = NULL;
				free(ifi_tmp);
				ifi_tmp = NULL;
				free_ipv6_addr_list(ipv6list);
				return ret;
			}
		}
	
	}	

		
	gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
	gInterfacesCountIpv4 = CWNetworkCountInterfaceAddressesIpv4(&gACSocket);
	gInterfacesCountIpv6 = CWNetworkCountInterfaceAddressesIpv6(&gACSocket);
	free(ifi_tmp->ifi_addr6);
	ifi_tmp->ifi_addr6 = NULL;
	free(ifi_tmp);
	ifi_tmp = NULL;
	free_ipv6_addr_list(ipv6list);

	if(ret != 0)
	return ret;

	ifr.ifr_ifindex = isystemindex;

	wif = (struct ifi*)malloc(sizeof(struct ifi));
	memset(wif->ifi_name,0,ETH_IF_NAME_LEN);
	memcpy(wif->ifi_name,ifname,strlen(ifname));
	wif->ifi_index = ifr.ifr_ifindex;
	printf("wif->ifi_index = %d\n",wif->ifi_index);
	wif->nas_id_len = 0;//zhanglei add
	memset(wif->nas_id,0,NAS_IDENTIFIER_NAME);//zhanglei add
	wif->ifi_next = NULL;
	wif->isipv6addr = 1;
	
	if(AC_WLAN[WlanID]->Wlan_Ifi == NULL){
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		AC_WLAN[WlanID]->Wlan_Ifi = wif ;
		AC_WLAN[WlanID]->Wlan_Ifi->ifi_next = NULL;
	}else{

		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext != NULL)
		{	
			if(wifnext->ifi_index == wif->ifi_index)
			{
				//printf("warnning you have binding this wlan eth ,please do not binding this again");
				free(wif);//zhanglei add
				wif = NULL;//zhanglei add
				return 0;
			}
			wifnext = wifnext->ifi_next;
		}
		
		wid_syslog_debug_debug(WID_DEFAULT,"** wlan binding if wlan id:%d ifname :%s sysindex:%d**\n",WlanID,ifname,wif->ifi_index);
		wifnext = AC_WLAN[WlanID]->Wlan_Ifi;
		while(wifnext->ifi_next!= NULL)
		{	
			wifnext = wifnext->ifi_next;
		}
		
		wifnext->ifi_next= wif;
		//wifnext->ifi_next = NULL;
	}
	AC_WLAN[WlanID]->ifcount++;

	//add wlan to the auto ap login if list
	if((g_auto_ap_login.ifnum != 0)||(g_auto_ap_login.auto_ap_if != NULL))
	{
		int i = 0;
		int result = 0;
		wid_auto_ap_if *iflist = NULL;
		iflist = g_auto_ap_login.auto_ap_if;
		while(iflist != NULL)
		{
			if(iflist->ifindex == ifr.ifr_ifindex)
			{
				if(iflist->wlannum >= L_BSS_NUM)
				{
					//printf("interface %s has already binded to %d wlan\n",ifname,L_BSS_NUM);
					break;
				}
				else
				{
					for(i=0;i<L_BSS_NUM;i++)
					{
						if(iflist->wlanid[i] == WlanID)
						{
							//printf("wlan %d has already in the list\n",WlanID);
							result = 1;
							break;
						}
					}
					if(result != 1)
					{
						for(i=0;i<L_BSS_NUM;i++)
						{
							if(iflist->wlanid[i] == 0)
							{
								iflist->wlanid[i] = WlanID;
								iflist->wlannum++;
								result = 1;
								//printf("add wlan %d at i %d\n",WlanID,i);
								break;
							}
						}
					}
				}
			}
			iflist = iflist->ifnext;		
		}
		if(result == 0)
		{
			//printf("interface %s is not in the auto ap login list\n",ifname);
		}
	}
	return 0;
	
}
/*fengwenchao add for onlinebug-904*/
int wid_check_uplink_whether_or_not_exist(char* ebr_ifname,char *ifname)
{
	//wid_syslog_info("accessinto %s \n",__func__);
	DIR * dir = NULL;
	FILE *f = NULL;
	char bppath[PATH_MAX_LEN] = {0};
	char ifpath[PATH_MAX_LEN] = {0};
	char uplink_ifname_str[PATH_MAX_LEN] = {0};
	sprintf(bppath,"/sys/class/net/%s/bridge", ebr_ifname);
	dir = opendir(bppath);	
	   if (dir)
	   {
		 sprintf(ifpath,"/sys/class/net/%s/bridge/uplink_port", ebr_ifname);
		 f = fopen(ifpath, "r");
		//wid_syslog_info("%s  %d \n",__func__,__LINE__);
		 if(f)
		 {
		   if(fgets(uplink_ifname_str,256,f))
		   {
		  //  wid_syslog_info("%s uplink_ifname_str = %s \n",__func__,uplink_ifname_str);
		   }
			fclose(f);
	   	}

		 closedir(dir);  
	   }

	if(strncmp(uplink_ifname_str,ifname,strlen(ifname)) == 0)
	{
		//wid_syslog_info("%s  %d \n",__func__,__LINE__);
		return 1;
	}
	//wid_syslog_info("%s  %d \n",__func__,__LINE__);
	return 0;
}
/*fengwenchao add end*/
int wid_set_ethereal_bridge_add_uplink(unsigned int ID,char *ifname)

{

	int ret = 0;
	int reason = 0;
	char ebrname[ETH_IF_NAME_LEN];
	char syscmd[WID_SYSTEM_CMD_LENTH];
	WTPQUITREASON quitreason = WTP_INIT;
		
	memset(ebrname,0,ETH_IF_NAME_LEN);
	memset(syscmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(ebrname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(ebrname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	ret = Check_Interface_Exist(ebrname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return WID_EBR_ERROR;
	}
	
	ret = Check_Interface_Exist(ifname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return ret;
	}	
	
	EBR_IF_LIST *ebrif = WID_EBR[ID]->iflist;

	if(ebrif == NULL)
	{
		return WID_EBR_IF_NOEXIT;
	}
	else
	{
		while(ebrif != NULL)
		{	
			if((strlen(ifname) == strlen(ebrif->ifname))&&(strncmp(ebrif->ifname,ifname,strlen(ifname)) == 0))
			{
				break;
			}
			ebrif = ebrif->ifnext;
		}
	}
	
	if(ebrif == NULL)
	{
		return WID_EBR_IF_NOEXIT;
	}

	
	sprintf(syscmd,"brctl adduplink %s %s",ebrname,ifname);

	ret = system(syscmd);
	reason = WEXITSTATUS(ret);

	if((reason == WID_DBUS_SUCCESS)||(wid_check_uplink_whether_or_not_exist(ebrname,ifname) == 1))
	{
		EBR_IF_LIST *wif;
		EBR_IF_LIST *wifnext;
		
		wif = (EBR_IF_LIST *)malloc(sizeof(EBR_IF_LIST));
		memset(wif,0,sizeof(EBR_IF_LIST));
		wif->ifname = (char *)malloc(ETH_IF_NAME_LEN);
		memset(wif->ifname,0,ETH_IF_NAME_LEN);
		memcpy(wif->ifname,ifname,strlen(ifname));
		wif->ifnext = NULL;
		
		if(WID_EBR[ID]->uplinklist== NULL)
		{
			WID_EBR[ID]->uplinklist = wif ;
			WID_EBR[ID]->uplinklist->ifnext = NULL;
		}
		else
		{
			wifnext = WID_EBR[ID]->uplinklist;
			while(wifnext != NULL)
			{	
				if((strlen(ifname) == strlen(wifnext->ifname))&&(strncmp(wifnext->ifname,ifname,strlen(ifname)) == 0))
				{
					free(wif->ifname);
					free(wif);
					wif = NULL;
					return UNKNOWN_ERROR;  //fengwenchao modify "0" to"UNKNOWN_ERROR"  for onlinebug-904
				}
				wifnext = wifnext->ifnext;
			}

			wifnext = WID_EBR[ID]->uplinklist;
			while(wifnext->ifnext != NULL)
			{	
				wifnext = wifnext->ifnext;
			}
			
			wifnext->ifnext = wif;
			
		}
		return 0;
	}
	else
	{
		return SYSTEM_CMD_ERROR;
	}
	
}

int wid_set_ethereal_bridge_del_uplink(unsigned int ID,char *ifname)
{
	int ret = 0;
	int reason = 0;
	char ebrname[ETH_IF_NAME_LEN];
	char syscmd[WID_SYSTEM_CMD_LENTH];
	WTPQUITREASON quitreason = WTP_INIT;
		
	memset(ebrname,0,ETH_IF_NAME_LEN);
	memset(syscmd,0,WID_SYSTEM_CMD_LENTH);
	if(local)
		sprintf(ebrname,"ebrl%d-%d-%d",slotid,vrrid,ID);
	else
		sprintf(ebrname,"ebr%d-%d-%d",slotid,vrrid,ID);		
	ret = Check_Interface_Exist(ebrname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return ret;
	}
	
	ret = Check_Interface_Exist(ifname,&quitreason);
	if(ret != WID_DBUS_SUCCESS)
	{
		return ret;
	}
	
	EBR_IF_LIST *ebrif = WID_EBR[ID]->uplinklist;

	if(ebrif == NULL)
	{
		return WID_EBR_IF_NOEXIT;
	}
	else
	{
		while(ebrif != NULL)
		{	
			if((strlen(ifname) == strlen(ebrif->ifname))&&(strncmp(ebrif->ifname,ifname,strlen(ifname)) == 0))
			{
				break;
			}
			ebrif = ebrif->ifnext;
		}
	}
	
	if(ebrif == NULL)
	{
		return WID_EBR_IF_NOEXIT;
	}

	sprintf(syscmd,"brctl deluplink %s %s",ebrname,ifname);

	ret = system(syscmd);
	reason = WEXITSTATUS(ret);

	if(reason == WID_DBUS_SUCCESS)
	{
		EBR_IF_LIST *wif;
		EBR_IF_LIST *wifnext;

		wifnext = WID_EBR[ID]->uplinklist;
		
		if(WID_EBR[ID]->uplinklist != NULL)
		{
			if((strlen(ifname) == strlen(wifnext->ifname))&&(strncmp(wifnext->ifname,ifname,strlen(ifname)) == 0))
			{
				WID_EBR[ID]->uplinklist = wifnext->ifnext;
				free(wifnext->ifname);
				free(wifnext);
				wifnext = NULL;		
			}
			else
			{
				while(wifnext->ifnext != NULL)
				{	
					if((strlen(ifname) == strlen(wifnext->ifnext->ifname))&&(strncmp(wifnext->ifnext->ifname,ifname,strlen(ifname)) == 0))
					{
						wif = wifnext->ifnext;
						wifnext->ifnext = wifnext->ifnext->ifnext;
						free(wif->ifname);
						free(wif);
						wif = NULL;				
						return 0;
					}
					wifnext = wifnext->ifnext;
				}
			}
		}
		return 0;
	}
	else
	{
		return SYSTEM_CMD_ERROR;
	}
}
int wid_wds_remote_bridge_mac_op(int RadioID, int is_add, unsigned char *mac)
{
	struct wds_rbmac * tmp; 
	struct wds_rbmac * tmp1;

	if(AC_RADIO[RadioID] == NULL){
		return RADIO_ID_NOT_EXIST;
	}
	if(is_add == 1){
		if(AC_RADIO[RadioID]->rbmac_list == NULL)
		{
			tmp = (struct wds_rbmac *)malloc(sizeof(struct wds_rbmac));
			memset(tmp,0,sizeof(struct wds_rbmac));
			memcpy(tmp->mac,mac,MAC_LEN);
			AC_RADIO[RadioID]->rbmac_list = tmp;
			AC_RADIO[RadioID]->rbmacNum++;
			tmp = NULL;
		}else{
			tmp = AC_RADIO[RadioID]->rbmac_list; 
			while((memcmp(tmp->mac,mac,MAC_LEN)!=0)&&(tmp->next != NULL)){
				tmp = tmp->next;
			}
			if(memcmp(tmp->mac,mac,MAC_LEN)==0)
				return 0;
			tmp1 = (struct wds_rbmac *)malloc(sizeof(struct wds_rbmac));
			memset(tmp1,0,sizeof(struct wds_rbmac));
			memcpy(tmp1->mac,mac,MAC_LEN);
			tmp->next = tmp1;
			AC_RADIO[RadioID]->rbmacNum++;
		}
	}else{
		if(AC_RADIO[RadioID]->rbmac_list == NULL)
			return 0;
		tmp = AC_RADIO[RadioID]->rbmac_list;
		if((memcmp(tmp->mac,mac,MAC_LEN) == 0)){
			AC_RADIO[RadioID]->rbmac_list = tmp->next;
			free(tmp);
			tmp = NULL;
			AC_RADIO[RadioID]->rbmacNum--;
			return 0;
		}
		while(tmp->next){
			tmp1 = tmp->next;
			if(memcmp(tmp1->mac,mac,MAC_LEN) == 0){
				tmp->next = tmp1->next;
				free(tmp1);
				tmp1 = NULL;
				AC_RADIO[RadioID]->rbmacNum--;
				return 0;
			}
			tmp = tmp->next;
		}		

	}
	return 0;
}

int wid_wds_remote_bridge_mac_set_aes_key(int RadioID, unsigned char *mac, char *key)
{
	struct wds_rbmac * tmp; 

	if(AC_RADIO[RadioID] == NULL){
		return RADIO_ID_NOT_EXIST;
	}
	if(AC_RADIO[RadioID]->rbmac_list == NULL)
	{
		return RADIO_BRMAC_NOT_EXIST;
	}else{
		tmp = AC_RADIO[RadioID]->rbmac_list; 
		while((tmp != NULL)){
			if(memcmp(tmp->mac,mac,MAC_LEN)==0){
				memset(tmp->key, 0, 32);
				memcpy(tmp->key, key, 32);
				return 0;
			}
			tmp = tmp->next;
		}		
	}
	
	return RADIO_BRMAC_NOT_EXIST;
}



void wid_wtp_radio_extern_command_check(unsigned int WTPIndex,int L_Radio_ID)
{
	unsigned int RadioID = WTPIndex*L_RADIO_NUM+L_Radio_ID;
	char buf[DEFAULT_LEN];
	int len = 0;
	int s_id = 0;
	int s_state = 0;
	struct wds_rbmac *tmp = NULL;
	if(AC_RADIO[RadioID] == NULL){
		return;
	}
	if(AC_RADIO[RadioID]->distance != 0){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set wbeDistance %d",AC_RADIO[RadioID]->distance);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);
	}
	tmp = AC_RADIO[RadioID]->rbmac_list;
	if(AC_RADIO[RadioID]->cipherType == 1){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set cipher wep");
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);		
		memset(buf,0,DEFAULT_LEN);
		len = strlen(AC_RADIO[RadioID]->wepkey);
		if(len > 0){
			if((len == 5)||(len == 10)){
				len = 40;
			}
			else if((len == 13)||(len == 26)){
				len = 104;
			}
			sprintf(buf,"set key 1 %d %s",len,AC_RADIO[RadioID]->wepkey); 		
			wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
			wid_radio_set_extension_command(WTPIndex,buf);
		}
	}
	if(AC_RADIO[RadioID]->cipherType == 2){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set cipher aes");
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);		
		memset(buf,0,DEFAULT_LEN);
	}	
	while(tmp){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"add remoteWbr %02x:%02x:%02x:%02x:%02x:%02x",tmp->mac[0],tmp->mac[1],tmp->mac[2],tmp->mac[3],tmp->mac[4],tmp->mac[5]);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);
		if(AC_RADIO[RadioID]->cipherType == 2){
			memset(buf,0,DEFAULT_LEN);
			sprintf(buf,"set bridgekey %02x:%02x:%02x:%02x:%02x:%02x %s",tmp->mac[0],tmp->mac[1],tmp->mac[2],tmp->mac[3],tmp->mac[4],tmp->mac[5],tmp->key);
			wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
			wid_radio_set_extension_command(WTPIndex,buf);
		}
		tmp = tmp->next;
	}
	if(AC_RADIO[RadioID]->supper_g.supper_g_type & 0x1){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set bursting enable");
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);		
	}
	if(AC_RADIO[RadioID]->supper_g.supper_g_type & 0x2){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set fastFrame enable");
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);		
	}
	if(AC_RADIO[RadioID]->supper_g.supper_g_type & 0x4){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set compression enable");
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);		
	}
	
	if(AC_RADIO[RadioID]->sector_state_value != 0){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set sector %x",AC_RADIO[RadioID]->sector_state_value);
		wid_syslog_debug_debug(WID_DEFAULT,"wid_set_sector apcmd %s\n",buf);
		wid_radio_set_extension_command(WTPIndex,buf);
	}
	{
		for(s_id=0;s_id<4;s_id++){
			s_state += AC_RADIO[RadioID]->sector[s_id]->state;
		}
		if(s_state == 4){
			memset(buf,0,DEFAULT_LEN);
			sprintf(buf,"set power all %d",AC_RADIO[RadioID]->sector[0]->tx_power);
			wid_radio_set_extension_command(WTPIndex,buf);
		}else if(s_state < 4){
			for(s_id=0;s_id<4;s_id++){
				if(0 != AC_RADIO[RadioID]->sector[s_id]->tx_power){
			memset(buf,0,DEFAULT_LEN);
			sprintf(buf,"set power %d %d",s_id,AC_RADIO[RadioID]->sector[s_id]->tx_power);
					wid_radio_set_extension_command(WTPIndex,buf);
				}
			}
		}
	}
	/*countr code for a8*/
	{
		if(gCOUNTRYCODE == COUNTRY_USA_US){
			memset(buf,0,DEFAULT_LEN);
			sprintf(buf,"set regdmn FCC");	
		}
		else if(gCOUNTRYCODE == COUNTRY_EUROPE_EU){
			memset(buf,0,DEFAULT_LEN);
			sprintf(buf,"set regdmn ETSI");	
		}
		else {
			memset(buf,0,DEFAULT_LEN);
			sprintf(buf,"set regdmn RoW");	
		}
		wid_radio_set_extension_command(WTPIndex,buf);
	}

	if(AC_RADIO[RadioID]->inter_vap_able == 1){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set interVF enable");	
		wid_radio_set_extension_command(WTPIndex,buf);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
	}
	if(AC_RADIO[RadioID]->intra_vap_able == 1){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set intraVF enable");	
		wid_radio_set_extension_command(WTPIndex,buf);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
	}

	
	if(AC_RADIO[RadioID]->keep_alive_period != 3600){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set keepaliveperiod %d",AC_RADIO[RadioID]->keep_alive_period);	
		wid_radio_set_extension_command(WTPIndex,buf);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
	}
	if(AC_RADIO[RadioID]->keep_alive_idle_time != 3600){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set keepaliveidletime %d",AC_RADIO[RadioID]->keep_alive_idle_time);	
		wid_radio_set_extension_command(WTPIndex,buf);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
	}

	
	if(AC_RADIO[RadioID]->congestion_avoidance == 1){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set caScheme tail-drop");	
		wid_radio_set_extension_command(WTPIndex,buf);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
	}
	if(AC_RADIO[RadioID]->congestion_avoidance == 2){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set caScheme RED");	
		wid_radio_set_extension_command(WTPIndex,buf);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
	}
	if(AC_RADIO[RadioID]->congestion_avoidance == 3){
		memset(buf,0,DEFAULT_LEN);
		sprintf(buf,"set caScheme FWRED");	
		wid_radio_set_extension_command(WTPIndex,buf);
		wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
	}
	
	return ;
}
/*---------------------BEGIN-------------------------------
---------------liuzhenhua append for wid show fdb--------------*/
static inline void __jiffies_to_tv(struct timeval *tv, unsigned long jiffies)
{
	unsigned long long tvusec;

	tvusec = 10000ULL*jiffies;
	tv->tv_sec = tvusec/1000000;
	tv->tv_usec = tvusec - 1000000 * tv->tv_sec;
}
static inline void __copy_fdb(struct fdb_entry *ent, 
			      const struct __fdb_entry *f)
{
	memcpy(ent->mac_addr, f->mac_addr, 6);
	ent->port_no = f->port_no;
	ent->is_local = f->is_local;
	__jiffies_to_tv(&ent->ageing_timer_value, f->ageing_timer_value);
}

int br_read_fdb(const char *bridge, struct fdb_entry *fdbs, 
		unsigned long offset, int num)
{
	FILE *f;
	int i = 0, n = 0;//init i ,n;
	struct __fdb_entry fe[num];
	memset(fe,0,num*sizeof(struct __fdb_entry));
	char path[SYSFS_PATH_MAX];
	
	/* open /sys/class/net/brXXX/brforward */
	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/brforward", bridge);
	f = fopen(path, "r");
	
	if (f) {
		printf("open file %s succuss\n",path);
		fseek(f, offset*sizeof(struct __fdb_entry), SEEK_SET);
		n = fread(fe, sizeof(struct __fdb_entry), num, f);
		fclose(f);
	}
	else{
		n = -1;
		printf("open file %s failed\n",path);
		return n;
		}
	for (i = 0; i < n; i++) 
		__copy_fdb(fdbs+i, fe+i);

	return n;
}
/*-------------liuzhenhua append for wid show fdb----------------
-----------------------END-------------------------------*/


struct WTP_GROUP_MEMBER *wtp_group_get_ap(WID_WTP_GROUP *group, unsigned int WTPID)
{
	struct WTP_GROUP_MEMBER *s;

	s = group->WTP_HASH[WTP_ID_HASH(WTPID)];
	while (s != NULL && s->WTPID != WTPID)
		s = s->hnext;
	return s;
}


void wtp_group_list_del(WID_WTP_GROUP *group, struct WTP_GROUP_MEMBER *wtp)
{
	struct WTP_GROUP_MEMBER *tmp;

	if (group->WTP_M == wtp) {
		group->WTP_M = wtp->next;
		return;
	}

	tmp = group->WTP_M;
	while (tmp != NULL && tmp->next != wtp)
		tmp = tmp->next;
	if (tmp == NULL) {
		printf("tmp == NULL\n");
	} else
		tmp->next = wtp->next;
}


void wtp_group_hash_add(WID_WTP_GROUP *group, struct WTP_GROUP_MEMBER *wtp)
{
	wtp->hnext = group->WTP_HASH[WTP_ID_HASH(wtp->WTPID)];
	 group->WTP_HASH[WTP_ID_HASH(wtp->WTPID)] = wtp;
}


static void wtp_group_hash_del(WID_WTP_GROUP *group, struct WTP_GROUP_MEMBER *wtp)
{
	struct WTP_GROUP_MEMBER *s;

	s = group->WTP_HASH[WTP_ID_HASH(wtp->WTPID)];
	if (s == NULL) return;
	if (s->WTPID == wtp->WTPID) {
		group->WTP_HASH[WTP_ID_HASH(wtp->WTPID)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       s->hnext->WTPID == wtp->WTPID)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
}

int create_ap_group(unsigned int ID,char *NAME){
	int ret = 0;
	int i,ii,jj;
	unsigned int RadioExternFlag[L_RADIO_NUM]={0};
	unsigned int gwtpid = 0;
	printf("%s ID %d\n",__func__,ID);
	WTP_GROUP[ID] = (WID_WTP_GROUP*)malloc(sizeof(WID_WTP_GROUP));
	memset(WTP_GROUP[ID], 0, sizeof(WID_WTP_GROUP));
	WTP_GROUP[ID]->GID = ID;
	WTP_GROUP[ID]->GNAME = (unsigned char*)malloc(strlen(NAME)+1);
	memset(WTP_GROUP[ID]->GNAME, 0, strlen(NAME)+1);
	memcpy(WTP_GROUP[ID]->GNAME, NAME, strlen(NAME));

	WTP_GROUP[ID]->WTP_CONFIG.wtp_allowed_max_sta_num=gWTP_MAX_STA; //xm/*wcl modify for globle variable*/
	WTP_GROUP[ID]->WTP_CONFIG.wtp_triger_num=1; //xm
	WTP_GROUP[ID]->WTP_CONFIG.wtp_flow_triger= gWTP_FLOW_TRIGER; //xm/*wcl modify for globle variable*/
	WTP_GROUP[ID]->WTP_CONFIG.EchoTimer = gEchoRequestTimer;
	WTP_GROUP[ID]->WTP_CONFIG.ap_sta_wapi_report_interval = gAP_STA_WAPI_REPORT_INTERVAL;/*wcl modify for globle variable*/
	WTP_GROUP[ID]->WTP_CONFIG.ap_sta_wapi_report_switch = gAP_STA_WAPI_REPORT_SWITCH;/*wcl add for globle variable*/
	
	WTP_GROUP[ID]->WTP_CONFIG.updateversion = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.updatepath = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.isipv6addr = 0;
	
	WTP_GROUP[ID]->WTP_CONFIG.sendsysstart = 2;	
	WTP_GROUP[ID]->WTP_CONFIG.WTPStat = 7;
	WTP_GROUP[ID]->WTP_CONFIG.isused = 0;
	WTP_GROUP[ID]->WTP_CONFIG.quitreason = WTP_UNUSED;
	WTP_GROUP[ID]->WTP_CONFIG.tunnel_mode = CW_LOCAL_BRIDGING;
	WTP_GROUP[ID]->WTP_CONFIG.CTR_ID = 0;
	WTP_GROUP[ID]->WTP_CONFIG.DAT_ID = 0;
	WTP_GROUP[ID]->WTP_CONFIG.BindingSock= -1;
	WTP_GROUP[ID]->WTP_CONFIG.BindingSystemIndex= -1;
	memset(WTP_GROUP[ID]->WTP_CONFIG.BindingIFName,0, ETH_IF_NAME_LEN);
	WTP_GROUP[ID]->WTP_CONFIG.wtp_login_mode = 0;
	WTP_GROUP[ID]->WTP_CONFIG.WFR_Index = 0;
	WTP_GROUP[ID]->WTP_CONFIG.NeighborAPInfos = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.rouge_ap_infos = NULL;	
	WTP_GROUP[ID]->WTP_CONFIG.wids_device_list = NULL;	
	WTP_GROUP[ID]->WTP_CONFIG.rx_echocount = 0;
	WTP_GROUP[ID]->WTP_CONFIG.ap_ipadd = 0;
	WTP_GROUP[ID]->WTP_CONFIG.ap_gateway = 0;
	WTP_GROUP[ID]->WTP_CONFIG.ap_mask_new = 0;
	WTP_GROUP[ID]->WTP_CONFIG.ap_dnsfirst = 0;
	WTP_GROUP[ID]->WTP_CONFIG.ap_dnssecend = 0;
	WTP_GROUP[ID]->WTP_CONFIG.resetflag = 0;
	WTP_GROUP[ID]->WTP_CONFIG.ap_mask = 0;
	WTP_GROUP[ID]->WTP_CONFIG.sysver = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.ver = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.codever = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.add_time = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.imagedata_time = 0;
	WTP_GROUP[ID]->WTP_CONFIG.config_update_time = 0;
	WTP_GROUP[ID]->WTP_CONFIG.ElectrifyRegisterCircle = 0;
	WTP_GROUP[ID]->WTP_CONFIG.updateStat = 0;
	WTP_GROUP[ID]->WTP_CONFIG.updatefailcount = 0;
	WTP_GROUP[ID]->WTP_CONFIG.updatefailstate = 0;
	WTP_GROUP[ID]->WTP_CONFIG.manual_update_time = 0;	
	WTP_GROUP[ID]->WTP_CONFIG.location = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.option60_param = NULL;
	//memset(WTP_GROUP[ID]->WTP_CONFIG.wep_flag,0,WTP_WEP_NUM);
	WTP_GROUP[ID]->WTP_CONFIG.ControlList = NULL;
	WTP_GROUP[ID]->WTP_CONFIG.ControlWait = NULL;
	memset(WTP_GROUP[ID]->WTP_CONFIG.cpuType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(WTP_GROUP[ID]->WTP_CONFIG.cpuType,"soc",3);
	memset(WTP_GROUP[ID]->WTP_CONFIG.memType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(WTP_GROUP[ID]->WTP_CONFIG.memType,"flash",5);
	memset(WTP_GROUP[ID]->WTP_CONFIG.flashType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(WTP_GROUP[ID]->WTP_CONFIG.flashType,"flash",5);/*wuwl add.when wtp didn't into run ,display this*/
	WTP_GROUP[ID]->WTP_CONFIG.wtp_trap_switch = 1;/*enable*/
	WTP_GROUP[ID]->WTP_CONFIG.wtp_seqnum_switch = 1;  /*wcl add*/
	WTP_GROUP[ID]->WTP_CONFIG.wtp_trap_lev = 1;
	WTP_GROUP[ID]->WTP_CONFIG.wtp_cpu_use_threshold = gWTP_CPU_USE_THRESHOLD;/*wcl modify for globle variable*/
	WTP_GROUP[ID]->WTP_CONFIG.wtp_mem_use_threshold = gWTP_MEM_USE_THRESHOLD;/*wcl modify for globle variable*/
	WTP_GROUP[ID]->WTP_CONFIG.wtp_rogue_ap_threshold = gWTP_ROGUE_AP_THRESHOLD;/*wcl modify for globle variable*/
	WTP_GROUP[ID]->WTP_CONFIG.wtp_rogue_terminal_threshold = gWTP_ROGUE_TERMINAL_THRESHOLD;/*wcl modify for globle variable*/

	for(i=0;i<L_RADIO_NUM;i++)
	{
		WTP_GROUP[ID]->WTP_CONFIG.WTP_Radio[i] = NULL;
	}


	WTP_GROUP[ID]->WTP_CONFIG.RadioCount = 4;
	for(i=0; (i<L_RADIO_NUM); i++){
		WID_WTP_RADIO	* radio = NULL;
		radio = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
		memset(radio, 0, sizeof(WID_WTP_RADIO));
		radio->WTPID = 0;
		radio->Radio_G_ID = gwtpid;
		radio->Radio_L_ID = i;
		radio->Radio_Chan = 0;
		radio->Radio_TXP = 20;
		radio->Radio_TXPOF= 0;
		radio->channelsendtimes = 1;

		radio->ishighpower = 0;

		radio->Support_Rate_Count = 0;
		radio->Radio_Rate = NULL;
		radio->FragThreshold = 2346;
		radio->BeaconPeriod = 100;
		radio->IsShortPreamble = 1;
		radio->DTIMPeriod = 1; //
		radio->ShortRetry = 7; //
		radio->LongRetry = 4; //
		radio->rtsthreshold = 2346;//zhangshu modify 2010-10-28, Huang Leilei change it for AXSSZFI-1406, 2012-01-09
		radio->AdStat = 2;
		radio->OpStat = 2;
		radio->CMD = 0x0;
		for(ii=0;ii<L_BSS_NUM;ii++){
			radio->BSS[ii] = NULL;
		}
		radio->QOSID = 0;
		radio->QOSstate = 0;
		radio->bandwidth = 108;
		radio->auto_channel = 0;
		radio->auto_channel_cont = 0;
		radio->txpowerautostate = 1;
		radio->wifi_state = 1;
		radio->channelchangetime = 0;
		radio->excommand = NULL;
		radio->diversity = 0;//default is 0
		radio->txantenna = 1;//default is main
		radio->isBinddingWlan = 0;
		radio->BindingWlanCount = 0;
		radio->upcount = 0;
		radio->downcount = 0;
		radio->guardinterval = 1;
		radio->mcs = 0;
		radio->cwmode = 0;
		radio->Wlan_Id = NULL;
		radio->REFlag = RadioExternFlag[i];//zhanglei add for A8
		/*11n set begin*/
		radio->Ampdu.Op= Ampdu_op;//zhangshu add
		radio->Ampdu.Type= RADIO; //zhangshu add
		radio->Ampdu.Able = 1;/*enable*/
		radio->Ampdu.AmpduLimit = 65535;
		radio->Ampdu.subframe = 32;

        /*zhangshu add*/
		radio->Amsdu.Op= Amsdu_op;
		radio->Amsdu.Type= RADIO;
		radio->Amsdu.Able = 0;/*enable*/
		radio->Amsdu.AmsduLimit = 4000;
		radio->Amsdu.subframe = 32;
		
		radio->MixedGreenfield.Mixed_Greenfield = 0;/*enable*/ //book modify
		radio->channel_offset =0;
		radio->tx_chainmask_state_value = 3;
		radio->rx_chainmask_state_value = 3; //zhangshu add for rx_chainmask 2010-10-09

		/*a8 chushihua start*/
		if(radio->REFlag == 1){
			radio->sector_state_value = 0 ;
			radio->inter_vap_able = 0;
			radio->intra_vap_able = 0;
			radio->keep_alive_period = 3600;
			radio->keep_alive_idle_time = 3600;
			radio->congestion_avoidance = 0;

		}
	
		for(ii=0;ii<SECTOR_NUM;ii++)
		{	
			//CW_CREATE_OBJECT_ERR(AC_RADIO[gwtpid]->sector[ii],WID_oem_sector,return NULL;);
			radio->sector[ii] = (WID_oem_sector*)malloc(sizeof(WID_oem_sector));
			radio->sector[ii]->state = 0;
			radio->sector[ii]->tx_power = 0;
		}
		for(jj=0;jj<TX_CHANIMASK_NUM;jj++)
		{	
			radio->tx_chainmask[jj] = (WID_oem_tx_chainmask*)malloc(sizeof(WID_oem_tx_chainmask));
			radio->tx_chainmask[jj]->state = 0;
		}
		if((radio->Radio_Type & 0x08) == 0x08)//if mode is 11n,beacon interval set to 400
		{
			radio->BeaconPeriod = 400;
			radio->diversity = 1;// 11 n default is 1
			radio->txantenna = 0;// 11n default is 0
		}
		//set default support rate list 11g&11b/g 12,11b 4
		radio->Support_Rate_Count = 12;

		//memory leak
		//AC_RADIO[gwtpid]->Radio_Rate = (struct Support_Rate_List *)malloc(sizeof(struct Support_Rate_List));
		radio->Radio_Rate = create_support_rate_list(1);//here add 10 first
		
		//printf("/////the list has %d element////\n",AC_RADIO[gwtpid]->Support_Rate_Count);
		
		int rate = 10;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 20;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 55;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 60;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 90;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 110;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 120;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 180;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 240;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 360;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 480;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);

		rate = 540;
		radio->Radio_Rate = insert_rate_into_list(radio->Radio_Rate,rate);
		/*add 11n rate 1300,3000*/
		delete_rate_from_list(radio->Radio_Rate,0);
		radio->Support_Rate_Count = length_of_rate_list(radio->Radio_Rate);

		WTP_GROUP[ID]->WTP_CONFIG.WTP_Radio[i] = radio;
	}
	//wid_init_wtp_info_in_create(WTPID);
	WTP_GROUP[ID]->WTP_CONFIG.wids_statist.floodingcount = 0;
	WTP_GROUP[ID]->WTP_CONFIG.wids_statist.sproofcount = 0;
	WTP_GROUP[ID]->WTP_CONFIG.wids_statist.weakivcount = 0;
	return ret;
}
int delete_ap_group(unsigned int ID){


	int num = 4;
	int i;
	int j;
	int ii;
	int k= 0;
	WID_WTP_RADIO	* radio = NULL;
	struct WTP_GROUP_MEMBER * tmp = NULL;
	struct WTP_GROUP_MEMBER * tmp1 = NULL;

	for(i=0; i<num; i++){
		radio = WTP_GROUP[ID]->WTP_CONFIG.WTP_Radio[i];
		WTP_GROUP[ID]->WTP_CONFIG.WTP_Radio[i] = NULL;
		for(k=0; k<L_BSS_NUM; k++)
		{
			if(radio->BSS[k] != NULL)
			{
					
				radio->BSS[k]->WlanID = 0;
				radio->BSS[k]->Radio_G_ID = 0;
				radio->BSS[k]->Radio_L_ID = 0;
				radio->BSS[k]->State = 0;
				radio->BSS[k]->BSSIndex = 0;
				memset(radio->BSS[k]->BSSID, 0, 6);
				free(radio->BSS[k]->BSSID);
				radio->BSS[k]->BSSID = NULL;
				free(radio->BSS[k]);
				radio->BSS[k] = NULL;			
			}

		}
		//added end 20080806
		
		if (radio->Support_Rate_Count != 0)
		{
			radio->Support_Rate_Count = 0;
			destroy_support_rate_list(radio->Radio_Rate);

		}		
		struct wlanid *wlan_id = radio->Wlan_Id;
		struct wlanid *wlan_id_next = NULL;
		
		while(wlan_id != NULL)
		{			
			wlan_id_next = wlan_id->next;
		
			CW_FREE_OBJECT(wlan_id);
		
			wlan_id = wlan_id_next;
		}
		for(j=0;j<SECTOR_NUM;j++)
		{	
			if(radio->sector[j]){
				free(radio->sector[j]);
				radio->sector[j] = NULL;
			}
		}
		for(ii=0;ii<TX_CHANIMASK_NUM;ii++)
		{	
			if(radio->tx_chainmask[ii]){
				free(radio->tx_chainmask[ii]);
				radio->tx_chainmask[ii] = NULL;
			}
		}
		
		free(radio);
		radio = NULL;		
	}
	tmp = WTP_GROUP[ID]->WTP_M;
	tmp1 = WTP_GROUP[ID]->WTP_M; 
	while(tmp){
		tmp = tmp1->next;
		free(tmp1);
	}
	if(WTP_GROUP[ID]->GNAME != NULL){
		free(WTP_GROUP[ID]->GNAME);
		WTP_GROUP[ID]->GNAME = NULL;
	}	
	free(WTP_GROUP[ID]);
	WTP_GROUP[ID] = NULL;

	return 0;

}

int do_check_ap_group_config(unsigned int GID, unsigned int WTPID){
	return 0;
}

int add_ap_group_member(unsigned int GID,unsigned int WTPID){
	struct WTP_GROUP_MEMBER *tmp = NULL;
	printf("%s GID %d\n",__func__,GID);
	if(WTP_GROUP[GID]== NULL){
		return GROUP_ID_NOT_EXIST;
	}
	if(AC_WTP[WTPID] == NULL){
		return WTP_ID_NOT_EXIST;
	}else if(AC_WTP[WTPID]->APGroupID != 0){
		return WTP_BE_USING;
	}
	tmp = wtp_group_get_ap(WTP_GROUP[GID],WTPID);
	if(tmp){
		return 0;
	}
	tmp = (struct WTP_GROUP_MEMBER *)malloc(sizeof(struct WTP_GROUP_MEMBER));
	memset(tmp, 0, sizeof(struct WTP_GROUP_MEMBER));
	tmp->WTPID = WTPID;
	tmp->next = WTP_GROUP[GID]->WTP_M;
	WTP_GROUP[GID]->WTP_M = tmp;
	wtp_group_hash_add(WTP_GROUP[GID],tmp);
	WTP_GROUP[GID]->WTP_COUNT += 1;
	AC_WTP[WTPID]->APGroupID = GID;
	return 0;
}
int del_ap_group_member(unsigned int GID,unsigned int WTPID){
	struct WTP_GROUP_MEMBER *tmp = NULL;
	struct WTP_GROUP_MEMBER *tmp1 = NULL;
	if(WTP_GROUP[GID]== NULL){
		return GROUP_ID_NOT_EXIST;
	}
	if((AC_WTP[WTPID] != NULL)&&(AC_WTP[WTPID]->APGroupID != GID)){
		return WTP_BE_USING;
	}
	tmp = wtp_group_get_ap(WTP_GROUP[GID],WTPID);
	if(tmp == NULL){
		return 0;
	}
	if(AC_WTP[WTPID] != NULL){
		AC_WTP[WTPID]->APGroupID = 0;
	}
	tmp1 = WTP_GROUP[GID]->WTP_M;
	if(tmp1 == tmp){
		WTP_GROUP[GID]->WTP_M = tmp1->next;
	}else{
		while(tmp1->next != NULL){
			if(tmp1->next == tmp){
				tmp1->next = tmp->next;
				break;
			}	
			tmp1 = tmp1->next;
		}
	}
	wtp_group_hash_del(WTP_GROUP[GID],tmp);	
	WTP_GROUP[GID]->WTP_COUNT -= 1;
	free(tmp);
	tmp = NULL;
	return 0;
}

CWBool check_radio_bind_wlan(unsigned int wtpid,unsigned char radio_l_id,unsigned char wlanId){

	CWBool  retValue = CW_FALSE;

	struct wlanid *wlan_id = AC_WTP[wtpid]->WTP_Radio[radio_l_id]->Wlan_Id;
	while(wlan_id != NULL)
		{	
		   
            printf("AC_WTP[%d]->WTP_Radio[%d]->Wlan_Id; = %d\n",wtpid,radio_l_id,wlan_id->wlanid);

			if(wlan_id->wlanid == wlanId)
			{
				retValue = CW_TRUE;
				break;
			}
			wlan_id = wlan_id->next;
		}

    return retValue;
}
/*fengwenchao add 20111117 for GM-3*/
int wid_prase_heart_time_avarge(unsigned int wtpid)
{
	int ret = WID_DBUS_SUCCESS;
	unsigned int echotimer = 0;
	unsigned int total_node_length = 0 ;
	struct heart_time_value_head*heart_time_node = NULL;
	ret = WID_CHECK_WTP_ID(wtpid);

	if(ret != WID_DBUS_SUCCESS)
	{
		wid_syslog_debug_debug(WID_WTPINFO," wtpid %d error in func %s",wtpid,__func__);
		return -1;
	}

	if(AC_WTP[wtpid] == NULL)
	{
		wid_syslog_debug_debug(WID_WTPINFO,"input wtpid %d error",wtpid);
		return -1;
	}

	echotimer = AC_WTP[wtpid]->EchoTimer;
	if(echotimer <= 0){
		wid_syslog_debug_debug(WID_WTPINFO,"echotimer <= 0 \n");
		return -1;
	}

	total_node_length = (AC_WTP[wtpid]->heart_time.heart_statistics_collect + echotimer - 1)/echotimer;
	if(total_node_length == 0){
		wid_syslog_debug_debug(WID_WTPINFO,"heart total_node_num == 0 \n");
		return -1;
	}	

	/*save value for list, begin*/
		if((heart_time_node = (struct heart_time_value_head*)malloc(sizeof(struct heart_time_value_head))) == NULL)
		{
			wid_syslog_debug_debug(WID_WTPINFO,"malloc fail in %s. \n",__func__);
			return -1;
		}
		else
		{
			heart_time_node->heart_time_value = AC_WTP[wtpid]->heart_time.heart_time_delay;
			heart_time_node->next = NULL;
			if(AC_WTP[wtpid]->heart_time.heart_time_value_head == NULL)
			{
				AC_WTP[wtpid]->heart_time.heart_time_value_head = heart_time_node;
				AC_WTP[wtpid]->heart_time.heart_time_value_length++;
				wid_syslog_debug_debug(WID_WTPINFO,"head AC_WTP[%d]->heart_time.heart_time_value_length =  %d.   %d\n",wtpid,AC_WTP[wtpid]->heart_time.heart_time_value_length,__LINE__);
			}
			else
			{
				heart_time_node->next = AC_WTP[wtpid]->heart_time.heart_time_value_head;
				AC_WTP[wtpid]->heart_time.heart_time_value_head = heart_time_node;
				AC_WTP[wtpid]->heart_time.heart_time_value_length++;
				wid_syslog_debug_debug(WID_WTPINFO,"node AC_WTP[%d]->heart_time.heart_time_value_length =  %d.   %d\n",wtpid,AC_WTP[wtpid]->heart_time.heart_time_value_length,__LINE__);
			}
		}
	/*save value for list, end*/

	/*delete exceed num,begin*/
	wid_syslog_debug_debug(WID_WTPINFO,"AC_WTP[%d]->heart_time.heart_time_value_length =  %d.   %d\n",wtpid,AC_WTP[wtpid]->heart_time.heart_time_value_length,__LINE__);
	wid_syslog_debug_debug(WID_WTPINFO,"total_node_length =  %d.   %d\n",total_node_length,__LINE__);
	if(AC_WTP[wtpid]->heart_time.heart_time_value_length > total_node_length)
	{
		struct heart_time_value_head  *p = AC_WTP[wtpid]->heart_time.heart_time_value_head ;
		struct heart_time_value_head  *tmp = NULL;
		int i=0;
	
		for(i=0;i<total_node_length;i++)
		{
			if (p!= NULL)
			{
				tmp = p;
				p = p->next;
			}
		}
		tmp->next = NULL;
	
		while(p!=NULL){
			tmp = p;
			p = p->next;
			tmp->next = NULL;
			free(tmp);
			tmp = NULL;
			AC_WTP[wtpid]->heart_time.heart_time_value_length--;
			wid_syslog_debug_debug(WID_WTPINFO,"AC_WTP[%d]->heart_time.heart_time_value_length--  =%d.   %d\n",wtpid,AC_WTP[wtpid]->heart_time.heart_time_value_length,__LINE__);
	 	}		
		//wid_syslog_debug_debug(WID_WTPINFO,"AC_WTP[%d]->heart_time.heart_time_value_length = %d\n",wtpid,AC_WTP[wtpid]->heart_time.heart_time_value_length);	
	}
	/*delete exceed num,end*/

	/*calculate average ,begin*/
	if((AC_WTP[wtpid]->heart_time.heart_time_value_length > 0)&&(AC_WTP[wtpid]->heart_time.heart_time_value_head != NULL))
	{
		unsigned int i_sample = 0;/*采样结点的搜索变量*/
		unsigned int j = 0, k = 0;/*j 上次结点应该在的位置，k 为本次结点应该在的位置*/
		unsigned int i_node = 0;/*为本次结点计数的递增变量*/
		unsigned int D_value = 0;/*本次与上次采样，结点所需要移动的步长*/
		unsigned int sum_heart_value = 0;/*heart 采样值的总和*/
		unsigned int sample_times = 0;/*抽样次数*/
		unsigned int total_sample_time = 0;/*抽样总时长*/	
		
		if(sample_infor_interval <= 0)
		{
			wid_syslog_debug_debug(WID_WTPINFO,"sample_infor_interval = %d at %s\n",\
								sample_infor_interval,__func__);
			return -1 ;
		}	

		total_sample_time = (AC_WTP[wtpid]->heart_time.heart_time_value_length) 
							*AC_WTP[wtpid]->EchoTimer;
		if(total_sample_time <= 0){
			wid_syslog_debug_debug(WID_WTPINFO,"total_sample_time = %d at %s\n",\
										total_sample_time,__func__);
			return -1 ;
		}
		
		sample_times = total_sample_time/sample_infor_interval;
		
		if(sample_times <= 0)
		{
			wid_syslog_debug_debug(WID_WTPINFO,"sample_times = %d at %s\n",\
								sample_times,__func__);
			return -1 ;
		}	

		struct heart_time_value_head  *Node_heart = AC_WTP[wtpid]->heart_time.heart_time_value_head;

		for(i_sample=0;i_sample<sample_times;i_sample++)
		{
			if(0 == i_sample )
			{
				sum_heart_value = Node_heart->heart_time_value;
			}
			else
			{
				k = (((i_sample * sample_infor_interval) + AC_WTP[wtpid]->EchoTimer))
					/AC_WTP[wtpid]->EchoTimer;
				j = ((((i_sample-1) * sample_infor_interval) + AC_WTP[wtpid]->EchoTimer))
					/AC_WTP[wtpid]->EchoTimer;
				D_value = k - j;
				for(i_node = 0;i_node < D_value;i_node++)
				{
					if(Node_heart!= NULL&&Node_heart->next != NULL)
					{
						Node_heart = Node_heart->next;
					}
				}
				sum_heart_value += Node_heart->heart_time_value;
			}
		}
		AC_WTP[wtpid]->heart_time.heart_time_avarge = sum_heart_value / sample_times ;
		wid_syslog_debug_debug(WID_WTPINFO,"maddersky %s  sum_heart_value = %d \n",__func__,sum_heart_value);
		wid_syslog_debug_debug(WID_WTPINFO,"maddersky %s  sample_times = %d \n",__func__,sample_times);
		wid_syslog_debug_debug(WID_WTPINFO,"maddersky %s  AC_WTP[%d]->heart_time.heart_time_avarge = %d \n",__func__,wtpid,AC_WTP[wtpid]->heart_time.heart_time_avarge);
		if(AC_WTP[wtpid]->heart_time.heart_time_avarge == 0)
		{
			AC_WTP[wtpid]->heart_time.heart_time_avarge = AC_WTP[wtpid]->heart_time.heart_time_value_head->heart_time_value;
			wid_syslog_debug_debug(WID_WTPINFO,"%s  AC_WTP[%d]->heart_time.heart_time_avarge = %d \n",__func__,wtpid,AC_WTP[wtpid]->heart_time.heart_time_avarge);
		}
	}
	/*calculate average ,end*/
	return 0;
}
/*fengwenchao add end*/
CWBool oui_mac_filters(unsigned char *mac){
		 char * ouiListType[3] = {"none","black","white"};
         switch(gOuiListType){
         case 0:{//use none
           wid_syslog_debug_debug(WID_DBUS,"use oui list: %s",ouiListType[0]);
           return CW_FALSE;
		 }
		 break;
		 case 1:{//default use black oui list
		 CWOUIInfo *pnode = gBlackOuiInfoList;
          wid_syslog_debug_debug(WID_DBUS,"use oui list: %s\n",ouiListType[1]);
		  while(pnode != NULL){
                 wid_syslog_debug_debug(WID_DBUS,"pnode - > oui_mac:"OUIMACSTR"\n",OUIMAC2STR(pnode->oui_mac));
				 if(memcmp(pnode->oui_mac,mac,OUI_LEN) == 0){
					 wid_syslog_debug_debug(WID_DBUS,"Mattch Successfull BLACK_OUI_MAC:"OUIMACSTR"\n",OUIMAC2STR(pnode->oui_mac));
                     return CW_TRUE;
				  }
                  pnode = pnode->next;
			  }
		    return CW_FALSE;
		 }
		 break;
		 case 2:{//use white oui list
		   CWOUIInfo *pnode = gWhiteOuiInfoList;
		   wid_syslog_debug_debug(WID_DBUS,"use oui list: %s",ouiListType[2]);
		   while(pnode != NULL){
                 wid_syslog_debug_debug(WID_DBUS,"pnode - > oui_mac:"OUIMACSTR"\n",OUIMAC2STR(pnode->oui_mac));
                 if(memcmp(pnode->oui_mac,mac,OUI_LEN) == 0){
                     wid_syslog_debug_debug(WID_DBUS,"Mattch Successfull WHITE_OUI_MAC:"OUIMACSTR"\n",OUIMAC2STR(pnode->oui_mac));
                     return CW_FALSE;
				 }
                 pnode = pnode->next;
		   }
            return CW_TRUE;
		 }
		 break;
		 default:
		 	break;
		}
		return CW_FALSE;
}
//xiaodawei add, 20110312
int measure_quality_of_network_link(char *wtpip, struct NetworkQuality *networkquality)
{
	char command[IPERF_LINE_CONTENT];
	char **string;
	char **str;
	char *tmp_str = NULL;
	char *iperf_ms = "ms";
	char *iperf_dataloss = "%";
	int i = 0;
	int j = 0;
	int k = 0;

	string = (char **)malloc(IPERF_LINE_NUM*sizeof(char *));
	for(i=0; i<IPERF_LINE_NUM; i++){
		string[i] = (char *)malloc(IPERF_LINE_CONTENT*sizeof(char));
		memset(string[i], 0, IPERF_LINE_CONTENT);
	}
	str = (char **)malloc(IPERF_LINE_NUM*sizeof(char *));
	for(i=0; i<IPERF_LINE_NUM; i++){
		str[i] = malloc(IPERF_LINE_CONTENT*sizeof(char));
		memset(str[i], 0, IPERF_LINE_CONTENT);
	}
	
	system("rm -rf /home/wtp_iperf");			//delete file wtp_iperf if exist
	memset(command, 0, IPERF_LINE_CONTENT);
	sprintf(command,"iperf -u -c %s >>/home/wtp_iperf",wtpip);
	system(command);							//e.g. iperf -uc 100.1.1.15, write in file /home/wtp_iperf
	//parse file wtp_iperf, return jitter and datagram loss percent
	FILE *fp;
	if((fp = fopen("/home/wtp_iperf", "r")) == NULL)
	{
		wid_syslog_debug_debug(WID_DBUS,"the file wtp_iperf cannot be opened! or  it may not exist!!\n");
		FREE(string);
		FREE(str);
		return -1;
	}
	else
	{
		for(i=0; fgets(string[i], IPERF_LINE_CONTENT, fp) != NULL && i<IPERF_LINE_NUM; i++){
			wid_syslog_debug_debug(WID_DBUS,"line %d-string[%i]: %s\n",i+1,i,string[i]);
			//[  6]  0.0-10.0 sec  1.24 MBytes  1.03 Mbits/sec  0.048 ms   10/  893 (1.1%)
			//find the line above by "ms" & "%"
			if(strstr(string[i],iperf_ms)&&strstr(string[i],iperf_dataloss)){
				wid_syslog_debug_debug(WID_DBUS,"iperf result found! line: %d\n",i+1);
				wid_syslog_debug_debug(WID_DBUS,"%s\n",string[i]);
				while(string[i]){
					tmp_str = strsep(&string[i]," ");//separate the result with blank
					if(strcmp(tmp_str,"")){
						strncpy(str[k++], tmp_str, strlen(tmp_str));
					}
				}
				for(j=0; j<k&&str[j]!=NULL; j++){
					wid_syslog_debug_debug(WID_DBUS,"str[%d]: %s\n",j,str[j]);
					if(!strcmp(str[j],"ms")){		//jitter
						if(j>1 && str[j-1]){
							networkquality->jitter = (double)atof(str[j-1]);
							wid_syslog_debug_debug(WID_DBUS,"jitter: %s\n",str[j-1]);
							wid_syslog_debug_debug(WID_DBUS,"jitter of float: %.3f\n",networkquality->jitter);
						}
						else{
							FREE(string);
							FREE(str);
							return -1;
						}
						if(k>1 && str[k-1]){		//datagram loss
							strsep(&str[k-1],"(");
							if(str[k-1]){
								tmp_str = strsep(&str[k-1],"%");
								networkquality->datagramloss = (double)atof(tmp_str);
								wid_syslog_debug_debug(WID_DBUS,"datagramloss: %s\%\n",tmp_str);
								wid_syslog_debug_debug(WID_DBUS,"datagramloss of float: %.3f\n",networkquality->datagramloss);
							}
							else{
								FREE(string);
								FREE(str);
								return -1;
							}
						}
						else{
							FREE(string);
							FREE(str);
							return -1;
						}
						FREE(string);
						FREE(str);
						return 0;
					}
				}
			}
		}
		//no response
		wid_syslog_debug_debug(WID_DBUS,"iperf gets no response!\n");
		FREE(string);
		FREE(str);
		fclose(fp);
		return -1;
	}	
}
int check_channel(int check_channel){
	int ret1 = COUNTRY_CODE_SUCCESS;
		switch(gCOUNTRYCODE)
		{
			case COUNTRY_CHINA_CN : 
									if(check_channel >= 14)
									{
										ret1 = COUNTRY_CHINA_CN;
									}
									break;
			case COUNTRY_EUROPE_EU : 
									if(check_channel >= 14)
									{
										ret1 = COUNTRY_EUROPE_EU;
										printf("33\n");
									}
									break;
			case COUNTRY_USA_US : 
									if((check_channel >= 12))
									{
										ret1 = COUNTRY_USA_US;
									}
									break;
			case COUNTRY_JAPAN_JP : 
									if((check_channel >= 15))
									{
										ret1 = COUNTRY_JAPAN_JP;
									}
									break;
			case COUNTRY_FRANCE_FR : 
									if((check_channel != 0)&&(check_channel != 10)&&(check_channel != 11)&&(check_channel != 12)&&(check_channel != 13))
									{
										ret1 = COUNTRY_FRANCE_FR;
									}
									break;
			case COUNTRY_SPAIN_ES : 
									if((check_channel != 0)&&(check_channel != 10)&&(check_channel != 11))
									{
										ret1 = COUNTRY_SPAIN_ES;
									}
									break;
			default : ret1 = COUNTRY_CODE_SUCCESS;break;
		}
		return ret1;
}

int wid_set_ap_scanning_wtp(unsigned int wtpid,APScanningSetting scansetting,unsigned char mode)
{
	int i = 0;
	msgq msg;
	i = wtpid;
	{
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
//			printf("##002 wtp id= %d##\n",i);
			
			AC_WTP[i]->WTP_Radio[0]->CMD |= 0x40;
			AC_WTP[i]->CMD->radioid[0] += 1;
			AC_WTP[i]->CMD->setCMD = 1;	
			int WTPIndex = i;
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gWTPs[WTPIndex].isNotFree && (gWTPs[WTPIndex].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SCANNING_OP;
				msg.mqinfo.u.WtpInfo.value1 = scansetting.opstate;
				msg.mqinfo.u.WtpInfo.value2 = scansetting.reportinterval;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}		
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadMutex));
			if(gtrapflag>=4){
				if(gtrap_rrm_change_trap_switch == 1)
					wid_dbus_trap_ap_rrm_state_change(WTPIndex,AC_WTP[i]->WIDS.scanningMode);
			}
		}
	}

	return 0;

}

CWBool wid_multicast_listen_setting(CWMultiHomedSocket *sockPtr, int port){
	CWNetworkLev4Address wildaddr;
	int yes = 1;
	CWSocket sock;
	struct CWMultiHomedInterface *p;
	if((sock = socket(AF_INET,SOCK_DGRAM, 0)) < 0) goto fail;
	
	goto success;
	
fail:
	CWNetworkRaiseSystemError(CW_ERROR_CREATING); // this wil return
	
success:
	// reuse address
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	CW_ZERO_MEMORY(&wildaddr, sizeof(wildaddr));		
	if (gNetworkPreferredFamily == CW_IPv6) {
		// fill wildaddr considering it an IPv6 addr
		struct sockaddr_in6 *a = (struct sockaddr_in6 *) &wildaddr;
		a->sin6_family = AF_INET6;
		a->sin6_addr = in6addr_any;
		a->sin6_port = htons(port);
	}
	
	if(bind(sock, (struct sockaddr*) &wildaddr, CWNetworkGetAddressSize(&wildaddr)) < 0) {
		close(sock);
		CWNetworkRaiseSystemError(CW_ERROR_CREATING);
	}

	
	CW_CREATE_OBJECT_ERR(p, struct CWMultiHomedInterface, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	memset(p->ifname, 0, IFI_NAME);
	memcpy(p->ifname,"LocalHost",9);
	p->sock = sock;
	p->kind = CW_BROADCAST_OR_ALIAS;
	p->systemIndex = -1; // make sure this can't be confused with an interface
	p->systemIndexbinding = -1;
	
	p->if_next = NULL;
	// addrIPv4 field for the wildcard address cause it is garbage in both cases (IPv4 + IPv6)
	p->addrIPv4.ss_family = AF_UNSPEC;

	CW_COPY_NET_ADDR_PTR(&(p->addr), &wildaddr);
	
	struct CWMultiHomedInterface* inf = sockPtr->interfaces;
	if(sockPtr->interfaces == NULL)
		sockPtr->interfaces = p;
	else{
		while(inf->if_next != NULL)
			inf = inf->if_next;
		inf->if_next = p;			
		p->if_next = NULL;
	}
	sockPtr->count++;
	Check_gACSokcet_Poll(sockPtr);		
	return CW_TRUE;
}

CWBool wid_multicast_listen_close(CWMultiHomedSocket *sockPtr){
	struct CWMultiHomedInterface* inf = sockPtr->interfaces;
	struct CWMultiHomedInterface* inf1 = sockPtr->interfaces;

	if(sockPtr->interfaces == NULL)
		return CW_TRUE;
	else{
		if(inf->kind == CW_BROADCAST_OR_ALIAS){
			sockPtr->interfaces = inf->if_next;
			sockPtr->count--;
			close(inf->sock);
			free(inf);		
			Check_gACSokcet_Poll(sockPtr);
			return CW_TRUE;
		}	
	}

	while(inf->if_next != NULL){
		inf1 = inf->if_next;
		if(inf->if_next->kind == CW_BROADCAST_OR_ALIAS){
			inf->if_next = inf1->if_next;
			close(inf1->sock);
			free(inf1);
			sockPtr->count--;
			break;						
		}
		inf = inf->if_next;
	}
	Check_gACSokcet_Poll(sockPtr);
	return CW_TRUE;

}


/*
** get 11n rate paras from 11n_rate_table 
** and check stream num
** book add, 2011-10-20
*/
int wid_radio_get_11n_rate_paras(struct n_rate_info *nRateInfo, unsigned char stream_num)
{
    wid_syslog_debug_debug(WID_DEFAULT,"call wid_radio_get_11n_rate_paras\n");
    int ret = WTP_NO_SURPORT_Rate;
    int index = 0;
    index = CWHash11nRate(nRateInfo->rate);
    wid_syslog_debug_debug(WID_DEFAULT,"rate = %d, index = %d, chainmask_num = %d",nRateInfo->rate,index,stream_num);

    if((index < 0) || (index >= 32))
    {
        wid_syslog_err("failed to find rate %d int 11n rate table\n",nRateInfo->rate);
    }

    struct n_rate_list *p;
    p = g11nRateTable[index].rate_info_list;

    while(p != NULL)
    {
        if(p->rate_info.rate == nRateInfo->rate)
        {
            if(p->rate_info.stream_num == stream_num)
            {
                nRateInfo->stream_num = stream_num;
                nRateInfo->mcs = p->rate_info.mcs;
                nRateInfo->cwmode = p->rate_info.cwmode;
                nRateInfo->guard_interval = p->rate_info.guard_interval;
                wid_syslog_debug_debug(WID_DEFAULT,"mcs = %d, cwmode = %d, gi = %d",nRateInfo->mcs,nRateInfo->cwmode,nRateInfo->guard_interval);
                ret = 0;
                break;
            }
            else
            {
                ret = RADIO_MODE_IS_11N;
            }
        }
        p = p->next;
    }
    
    return ret;
}

int wid_radio_set_11n_rate_paras(int ID, struct n_rate_info nRateInfo, unsigned char channel)
{
    wid_syslog_debug_debug(WID_DEFAULT,"call wid_radio_set_11n_rate_paras\n");
    int ret = 0;
    wid_syslog_debug_debug(WID_DEFAULT,"mcs = %d, gi = %d, cwmode = %d, channel = %d\n",nRateInfo.mcs,nRateInfo.guard_interval,nRateInfo.cwmode,channel);
    
    if(AC_RADIO[ID] != NULL)
    {
        if(AC_RADIO[ID]->mcs != nRateInfo.mcs)
	        AC_RADIO[ID]->mcs = nRateInfo.mcs;
        if(AC_RADIO[ID]->guardinterval != nRateInfo.guard_interval)
	        AC_RADIO[ID]->guardinterval = nRateInfo.guard_interval;
	    
		/* change channel offset */
		if((AC_RADIO[ID]->cwmode == 0)&&((1 == nRateInfo.cwmode)||(2 == nRateInfo.cwmode)))
		{      
		    if(channel != 0)
			{
			    AC_RADIO[ID]->Radio_Chan = channel;
			    ret = WID_RADIO_SET_CHAN(ID, channel);
			}
			/* change channel */
			if(ret == 0)
			{
		        AC_RADIO[ID]->channel_offset = 1;
		        ret = wid_radio_set_channel_Extoffset(ID);
		        if(ret != 0)
		        {
		            wid_syslog_err("failed to change channel offset");
		            return -1;
		        }
		    }
		    else
		    {
		        wid_syslog_err("failed to change channel");
		        return -1;
		    }
	    }
		else if((AC_RADIO[ID]->cwmode != 0) && (nRateInfo.cwmode == 0))
		{
		    AC_RADIO[ID]->channel_offset = 0;
		    ret = wid_radio_set_channel_Extoffset(ID);
		    if(ret != 0)
		    {
		        wid_syslog_err("failed to change channel offset");
	        return -1;
		    }
		}

		if(ret == 0)
		{
		    if(AC_RADIO[ID]->cwmode != nRateInfo.cwmode)
    	    AC_RADIO[ID]->cwmode = nRateInfo.cwmode;
    	    ret = wid_radio_set_cmmode(ID);
    	    if(ret == 0)
    	    {
    	        WID_RADIO_CHANGE_SUPPORT_RATE_BYGI_MCS_CWMODE(ID);
    	    }
    	    else
    		    ret = -1;
    	}
    }
    else
        ret = -1;
        
    return ret;
}
int add_mac_in_maclist(struct acl_config *conf, unsigned char *addr, char type)
{
	struct maclist *entry, *prev,*tmp;

	if(addr==NULL||(type!=1&&type!=2))
		return -1;

	if(conf == NULL) {
		conf = (struct acl_config*)malloc(sizeof(struct acl_config));
		if(conf == NULL) return -1;
		conf->macaddr_acl = 0;
		conf->accept_mac = NULL;
		conf->num_accept_mac = 0;
		conf->deny_mac = NULL;
		conf->num_accept_mac = 0;
	}

	tmp = malloc(sizeof(*tmp));
	if (tmp == NULL) {
		wid_syslog_debug_debug(WID_DEFAULT,"%s :malloc fail.\n",__func__);
		exit(1);
	}else {
		memset(tmp,0,sizeof(*tmp));
		memcpy(tmp->addr, addr, ETH_ALEN);
		tmp->next = NULL;
	}

	if(1==type){
		entry = conf->deny_mac;
		prev = NULL;
		while (entry) {
			if (memcmp(entry->addr, addr, ETH_ALEN) == 0) {
				wid_syslog_debug_debug(WID_DEFAULT,"entry->add_reason = %d\n",entry->add_reason);
				if(entry->add_reason) {
					entry->add_reason = 0;
					conf->num_deny_mac++;
					conf->num_wids_mac--;
					wid_syslog_debug_debug(WID_DEFAULT,MACSTR" is put in black list from wids black list.\n",MAC2STR(addr));
				}else 
					wid_syslog_debug_debug(WID_DEFAULT,MACSTR" is already in the black list.\n",MAC2STR(addr));
				free(tmp);
				tmp = NULL;
				return 1;
			}
			prev = entry;
			entry = entry->next;
		}
		if(prev == NULL)
			conf->deny_mac = tmp;
		else 
			prev->next = tmp;
		conf->num_deny_mac++;
		wid_syslog_debug_debug(WID_DEFAULT,MACSTR"add success!\n",MAC2STR(tmp->addr));
		return 0;			
	}else {
		entry = conf->accept_mac;
		prev = NULL;
		while (entry) {
			if (memcmp(entry->addr, addr, ETH_ALEN) == 0) {
				wid_syslog_debug_debug(WID_DEFAULT,MACSTR" is already in the white list.\n",MAC2STR(addr));
				free(tmp);
				tmp = NULL;
				return 1;
			}
			prev = entry;
			entry = entry->next;
		}
		if(prev == NULL)
			conf->accept_mac = tmp;
		else 
			prev->next = tmp;
		conf->num_accept_mac++;
		return 0;			
	}

}

int del_mac_in_maclist(struct acl_config *conf, unsigned char  *addr, char type)
{
	struct maclist *entry, *prev, *tmp;
	
	wid_syslog_debug_debug(WID_DEFAULT,"%s :type = %d\n",__func__,type);
	if(addr==NULL||(type!=1&&type!=2))	//1--black list, 2--white list
		return -1;
	
	if(conf == NULL) {
		conf = (struct acl_config*)malloc(sizeof(struct acl_config));
		if(conf == NULL) return -1;
		conf->macaddr_acl = 0;
		conf->accept_mac = NULL;
		conf->num_accept_mac = 0;
		conf->deny_mac = NULL;
		conf->num_accept_mac = 0;
	}

	if(1==type){
		entry = conf->deny_mac;
		prev = NULL;
		while (entry) {
			if ((memcmp(entry->addr, addr, ETH_ALEN) == 0) && (entry->add_reason == 0)) {
				
				wid_syslog_debug_debug(WID_DEFAULT,"%s :find the mac!\n",__func__);
				if (prev)
					prev->next = entry->next;
				else
					conf->deny_mac = entry->next;
				tmp = entry;
				tmp->next = NULL;
				free(tmp);
				conf->num_deny_mac--;
				return 0;
			}
			prev = entry;
			entry = entry->next;
		}
		wid_syslog_debug_debug(WID_DEFAULT,MACSTR" is not in the black list.\n",MAC2STR(addr));
		return -1;			
	}else {
		entry = conf->accept_mac;
		prev = NULL;
		while (entry) {
			if (memcmp(entry->addr, addr, ETH_ALEN) == 0) {
				if (prev)
					prev->next = entry->next;
				else
					conf->accept_mac = entry->next;
				tmp = entry;
				tmp->next = NULL;
				free(tmp);
				conf->num_accept_mac--;
				return 0;
			}
			prev = entry;
			entry = entry->next;
		}
		wid_syslog_debug_debug(WID_DEFAULT,MACSTR" is not in the white list.\n",MAC2STR(addr));
		return -1;			
	}

}
int change_maclist_security(struct acl_config *conf, char type)
{
	wid_syslog_debug_debug(WID_DEFAULT,"%s:type = %d\n",__func__,type);
	if(type!=0&&type!=1&&type!=2)
		return -1;

	if(conf == NULL) {
		conf = (struct acl_config*)malloc(sizeof(struct acl_config));
		if(conf == NULL) return -1;
		conf->macaddr_acl = 0;
		conf->accept_mac = NULL;
		conf->num_accept_mac = 0;
		conf->deny_mac = NULL;
		conf->num_accept_mac = 0;
	}
	
	if(0==type)
		conf->macaddr_acl = 0;
	else if(1==type)
		conf->macaddr_acl = 1;
	else if(2==type)
		conf->macaddr_acl = 2;

	return 0;
}
void free_maclist(struct acl_config *conf, struct maclist *list)
{
	wid_syslog_debug_debug(WID_DEFAULT,"%s:\n",__func__);
	struct maclist *entry, *tmp;

	entry = list;
	while(entry != NULL){
		tmp = entry;
		entry = entry->next;
		memset(tmp,0,sizeof(struct maclist));
		free(tmp);
		tmp = NULL;
	}
}
//weichao add
int wid_set_ap_username_password(unsigned int wtpid,char *username,char *passwd)
{
	wid_syslog_debug_debug(WID_DEFAULT,"the wtpid is %s\n",__func__);
	msgq msg;
//	struct msgqlist *elem = NULL;
	int i =0;
	if(wtpid == 0){
		for(i = 0 ; i < WTP_NUM;i++)
		{
			if(AC_WTP[i] != NULL) 
			{
				wtpid = i;
				wid_syslog_debug_debug(WID_DEFAULT,"the wtpid is %d\n",wtpid);
				if(AC_WTP[wtpid]->WTPStat == 5)
				{
					
					CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
					if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
					{
						memset((char*)&msg, 0, sizeof(msg));
						msg.mqid = wtpid%THREAD_NUM+1;
						msg.mqinfo.WTPID = wtpid;
						msg.mqinfo.type = CONTROL_TYPE;
						msg.mqinfo.subtype = WTP_S_TYPE;
						msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SET_NAME_PASSWD;
						memcpy(msg.mqinfo.u.WtpInfo.username,username,strlen(username));
						memcpy(msg.mqinfo.u.WtpInfo.passwd,passwd,strlen(passwd));
						
						if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
							wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
							perror("msgsnd");
						}
					}
					CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
				}//delete unuseful cod
				/*else if((AC_WTP[wtpid] != NULL)){
					memset((char*)&msg, 0, sizeof(msg));
					msg.mqid = wtpid%THREAD_NUM+1;
					msg.mqinfo.WTPID = wtpid;
					msg.mqinfo.type = CONTROL_TYPE;
					msg.mqinfo.subtype = WTP_S_TYPE;
					msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SET_NAME_PASSWD;
					memcpy(msg.mqinfo.u.WtpInfo.username,username,strlen(username));
					memcpy(msg.mqinfo.u.WtpInfo.passwd,passwd,strlen(passwd));
					elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
					if(elem == NULL){
						wid_syslog_info("%s malloc %s",__func__,strerror(errno));
						perror("malloc");
						return 0;
					}
					memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
					elem->next = NULL;
					memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
					WID_INSERT_CONTROL_LIST(wtpid, elem);
				}*/
			}
		}
	}
	else if(AC_WTP[wtpid] != NULL){
			if(AC_WTP[wtpid]->WTPStat == 5)
			{
			
				CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
				if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
				{
					memset((char*)&msg, 0, sizeof(msg));
					msg.mqid = wtpid%THREAD_NUM+1;
					msg.mqinfo.WTPID = wtpid;
					msg.mqinfo.type = CONTROL_TYPE;
					msg.mqinfo.subtype = WTP_S_TYPE;
					msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SET_NAME_PASSWD;
					memcpy(msg.mqinfo.u.WtpInfo.username,username,strlen(username));
					memcpy(msg.mqinfo.u.WtpInfo.passwd,passwd,strlen(passwd));
					
					if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
						wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
						perror("msgsnd");
					}
				}
				CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
			}//delete unuseful cod
		/*else if((AC_WTP[wtpid] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_SET_NAME_PASSWD;
			memcpy(msg.mqinfo.u.WtpInfo.username,username,strlen(username));
			memcpy(msg.mqinfo.u.WtpInfo.passwd,passwd,strlen(passwd));
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return 0;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/
	}
	return 0;
	
}
void muti_user_optimize_switch(unsigned char wlanid, unsigned int radio_g_id,unsigned char type)
{
	wid_syslog_debug_debug(WID_DEFAULT,"now in func : %s\n",__func__);
	wid_syslog_debug_debug(WID_DEFAULT,"the wlanid is %d\n",wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"the radioid is %d\n",radio_g_id);

	msgq msg;
//	struct msgqlist *elem = NULL;
	unsigned char radio_l_id = 0;
	unsigned int wtpid;
	wtpid = radio_g_id/4;
	radio_l_id = radio_g_id%4;
	
	wid_syslog_debug_debug(WID_DEFAULT,"the wtpid is %d\n",wtpid);
	wid_syslog_debug_debug(WID_DEFAULT,"the radio_l_id is %d\n",radio_l_id);
	if(AC_WTP[wtpid] != NULL) 
	{
	
		wid_syslog_debug_debug(WID_DEFAULT,"the radio_l_id is %d\n",radio_l_id);
		if(AC_WTP[wtpid]->WTPStat == 5)
		{
			
			CWThreadMutexLock(&(gWTPs[wtpid].WTPThreadMutex));
			if(gWTPs[wtpid].isNotFree && (gWTPs[wtpid].currentState == CW_ENTER_RUN))
			{
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = wtpid%THREAD_NUM+1;
				msg.mqinfo.WTPID = wtpid;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_MULTI_USER_OPTIMIZE;
				msg.mqinfo.u.WlanInfo.	multi_user_optimize_switch = type;
				msg.mqinfo.u.WlanInfo.Radio_L_ID =  radio_l_id;
				msg.mqinfo.u.WlanInfo.WLANID = wlanid;
				if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
					wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}
			}
			CWThreadMutexUnlock(&(gWTPs[wtpid].WTPThreadMutex));
			
		}//delete unuseful cod
		/*else {
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = wtpid%THREAD_NUM+1;
			msg.mqinfo.WTPID = wtpid;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_MULTI_USER_OPTIMIZE;
			msg.mqinfo.u.WlanInfo.	multi_user_optimize_switch = type;
			msg.mqinfo.u.WlanInfo.Radio_L_ID = radio_l_id;
			msg.mqinfo.u.WlanInfo.WLANID = wlanid;
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(wtpid, elem);
		}*/
	}
	return ;
}
void set_wtp_5g_switch(unsigned int wtpid,unsigned char type)
{
	wid_syslog_debug_debug(WID_DEFAULT,"%s,wtpid=%d,type=%d.\n",__func__,wtpid,type);

	char command[WID_SYSTEM_CMD_LENTH] = {0};
	if(1 == type){
		sprintf(command,"echo 1 > /proc/sys/dev/wifi0/ join5g_enable");
	}else{
		sprintf(command,"echo 0 > /proc/sys/dev/wifi0/ join5g_enable");
	}
	if(AC_WTP[wtpid] != NULL) 
	{
		wid_radio_set_extension_command(wtpid,command);
	}
	return ;
}
int init_wid_lic_socket(){
	int sock;
	int yes = 1;
	int sndbuf = 65525;
	int rcvbuf = 65525;
	struct sockaddr_in my_addr;
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1)
	{	printf("udp socket create failed\n");		
		exit(1);	
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	my_addr.sin_family = AF_INET;	
	my_addr.sin_port = htons(WID_LIC_AC_PORT+ local*MAX_INSTANCE +vrrid);	
	my_addr.sin_addr.s_addr = INADDR_ANY;	
	memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));	
	if (bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{	
		wid_syslog_info("udp bind failed\n");	
		exit(1);
		return -1;
	}	
	setsockopt(sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
	setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
	return sock;
}

void *lic_active_thread(void *arg){
	wid_pid_write_v2("lic_active_thread",0,vrrid);
	int sockid = (int)arg;
	wid_syslog_info("%s,%d,sock=%d.\n",__func__,__LINE__,sockid); 
	int len = 0;
	socklen_t addr_len;
	char buf[4096];

	while(Lic_ip.isActive == 1){
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(sockid,&fdset);
		addr_len = sizeof(struct sockaddr);
		struct timeval tv;			
		tv.tv_sec = 3;
		tv.tv_usec = 5000;
		wid_syslog_debug_debug(WID_DBUS,"%s,%d,before select.",__func__,__LINE__);
		if(-1 == select(sockid+1,&fdset,(fd_set*)0,(fd_set*)0,&tv)){
			if(Lic_ip.isActive == 1){
				wid_syslog_debug_debug(WID_DBUS,"%s,%d,select fail and continue.",__func__,__LINE__);
				continue;
			}else{
				wid_syslog_debug_debug(WID_DBUS,"%s,%d,select fail and break.",__func__,__LINE__);
				break;
			}
		}
		if(FD_ISSET(sockid,&fdset)){
			len = recvfrom(sockid,buf,4095,0,(struct sockaddr *)&Lic_bak_addr, &addr_len);
			if(len < 0){
				wid_syslog_err("%s,%d,len=%d.\n",__func__,__LINE__,len);
				continue;
			}
			unsigned int ip = 0;
			struct sockaddr_in * temp = (struct sockaddr_in *)&Lic_bak_addr;
			ip = temp->sin_addr.s_addr;
			wid_syslog_debug_debug(WID_DBUS,"%s,%d,sockfd=%d,ip=%d.%d.%d.%d.\n",__func__,__LINE__,sockid,(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
			wid_syslog_debug_debug(WID_DBUS,"%s,%d,recvfrom receive msg.",__func__,__LINE__);
			B_Msg *msg = (B_Msg*) buf;			
			switch(msg->Type){
				case B_LICENSE_REQUEST:
					wid_syslog_debug_debug(WID_DBUS,"%s,%d,sockfd=%d,.\n",__func__,__LINE__,sockid,(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
					compare_license(msg);
					update_license(sockid , (struct sockaddr_in *)&Lic_bak_addr);

					wid_syslog_debug_debug(WID_DBUS,"%s,%d,case B_LICENSE_REQUEST.",__func__,__LINE__);
					break;
				case B_CHECK_LICENSE:
					compare_license(msg);
					break;
				default:
					wid_syslog_debug_debug(WID_DBUS,"%s,%d,case default.",__func__,__LINE__);
					break;
			}
		}
	}


	pthread_exit((void *) 0);
	
}

int set_active_ac_listenning(){
	int sock = 0;
	pthread_t LIC_ACTIVE;
	sock = init_wid_lic_socket();
	lic_active_fd = sock;
	pthread_attr_t attr;
	int s = PTHREAD_CREATE_DETACHED;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,s);
	wid_syslog_info("%s,%d,sock=%d.\n",__func__,__LINE__,sock); 
	if(pthread_create(&LIC_ACTIVE, &attr, lic_active_thread, (char*)sock) != 0) {
		return -1;
	}
	wid_syslog_info("%s,%d.\n",__func__,__LINE__); 
	return 0;
}
void*lic_bak_thread(){
	int sndbuf = 65525;
	int rcvbuf = 65525;
	char buf[2048];
	struct sockaddr tmp_addr;
	socklen_t addr_len;
	wid_pid_write_v2("lic_bak_thread",0,vrrid);
	int numbytes = 0;
	while(Lic_ip.isActive == 2){
		if((lic_bak_fd = socket(PF_INET,SOCK_DGRAM,0)) == -1){
			wid_syslog_err("%s,%d,create sock %d fail.\n",__func__,__LINE__,lic_bak_fd);
			return NULL;
		}
		setsockopt(lic_bak_fd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
		setsockopt(lic_bak_fd,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
		wid_syslog_info("sendto active ac.\n");
		update_license_req(lic_bak_fd , (struct sockaddr_in *)&Lic_Active_addr);
		if(!CWErr(CWTimerRequest(LicBakReqInterval, NULL, &(Lic_bak_req_timer), 901, 0))) { 
			wid_syslog_err("%s,%d,Lic_bak_req_timer request err.\n",__func__,__LINE__);
		}

		while(Lic_ip.isActive == 2){
			
			memset(buf,0,2048);
			fd_set fdset;
			FD_ZERO(&fdset);
			FD_SET(lic_bak_fd,&fdset);
			struct timeval tv;
			tv.tv_sec =2;
			tv.tv_usec = 2500;
			wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,wait for active ac response.\n",__func__,__LINE__);
			if(select(lic_bak_fd+1,&fdset,(fd_set*)0,(fd_set*)0,&tv)==-1){
				if(Lic_ip.isActive == 2){
					continue;
				}else{
					break;
				}
			}
			if(FD_ISSET(lic_bak_fd,&fdset)){
				numbytes = recvfrom(lic_bak_fd,buf,2047,0,&tmp_addr,&addr_len);
				wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,numbytes=%d.\n",__func__,__LINE__,numbytes);
				if(numbytes <= 0){
					wid_syslog_err("%s,recvfrom err,numbytes=%d.\n",numbytes);
					break;
				}
				CWTimerCancel(&Lic_bak_req_timer,1);
				wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,recive from active master.\n",__func__,__LINE__);
				if(Lic_ip.isActive == 2){
					B_Msg *msg = (B_Msg*) buf;
					switch(msg->Type){
						case B_CHECK_LICENSE:
							compare_license(msg);
							wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,B_CHECK_LICENSE.\n",__func__,__LINE__);
							break;
						case B_LICENSE_REQUEST:
							wid_syslog_info("%s B_LICENSE_REQUEST\n",__func__);
							compare_license(msg);
							update_license(lic_bak_fd , (struct sockaddr_in *)&Lic_Active_addr);
							wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,B_LICENSE_REQUEST.\n",__func__,__LINE__);
							break;
						default:
							
							wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,default.\n",__func__,__LINE__);
							break;
					}
				}
			}
			
		}
		close(lic_bak_fd);
	}
	pthread_exit((void *) 0);
	return 0;
}

int set_bakup_ac_update_license(){

	pthread_t LIC_BAK;
	pthread_attr_t attr;
	int s = PTHREAD_CREATE_DETACHED;	
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&Lic_Active_addr;
	inaddr->sin_family = AF_INET;	
	inaddr->sin_port = htons(WID_LIC_AC_PORT+ local*MAX_INSTANCE+vrrid);
	inaddr->sin_addr.s_addr = Lic_ip.lic_active_ac_ip;	
	memset(inaddr->sin_zero, '\0', sizeof(inaddr->sin_zero));

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,s);
	if(pthread_create(&LIC_BAK, &attr, lic_bak_thread, NULL) != 0) {
		return -1;
	}				
	return 0;	
}

int set_wlan_tunnel_mode(unsigned char WlanID, unsigned char state){
	int ret = -1;
	int i=0;
	int j=0;
	int k = 0;
	int fd = 0;
	int ebr_id = 0;
	if_batch_info if_b_info;
	memset(&if_b_info, 0 , sizeof(if_b_info));
	if(AC_WLAN[WlanID] == NULL) //fengwenchao add 20121203 for AXSSZFI-1283
	{
		return WLAN_ID_NOT_EXIST;
	}
	if(AC_WLAN[WlanID]->Status == 0)
	{
		return WLAN_BE_ENABLE;
	}
	if(state == 1)
	{		
		if(AC_WLAN[WlanID]->wlan_if_policy != NO_INTERFACE){
			return WID_DBUS_SUCCESS;
		}
		AC_WLAN[WlanID]->wlan_if_policy = BSS_INTERFACE;
		for(i=0; i<WTP_NUM; i++)
		{
			//if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))
			if(AC_WTP[i]!=NULL)
			{
				for(j=0; j<AC_WTP[i]->RadioCount; j++)
				{
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
						if(!check_bssid_func(bssindex)){
							wid_syslog_err("bssindex %d not exist\n",WlanID);
						}else{						
							if(AC_BSS[bssindex]->BSS_IF_POLICY == NO_INTERFACE)
							{
								if_b_info.ifinfo[k].wlanID = 0;
								if_b_info.ifinfo[k].BSSIndex = bssindex;
								if_b_info.ifinfo[k].vrid = local*MAX_INSTANCE+vrrid;								
								if(local)
									snprintf(if_b_info.ifinfo[k].if_name,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,i,AC_BSS[bssindex]->Radio_L_ID,WlanID);
								else
									snprintf(if_b_info.ifinfo[k].if_name,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,i,AC_BSS[bssindex]->Radio_L_ID,WlanID);
								memset(AC_BSS[bssindex]->BSS_IF_NAME,0,ETH_IF_NAME_LEN);
								memcpy(AC_BSS[bssindex]->BSS_IF_NAME, if_b_info.ifinfo[k].if_name, ETH_IF_NAME_LEN-1);
								AC_BSS[bssindex]->BSS_IF_POLICY = BSS_INTERFACE;
								AC_BSS[bssindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
								if((is_secondary != 2)&&(set_vmac_state == 1)){
									hwaddr_set(AC_BSS[bssindex]->BSS_IF_NAME,v_mac,MAC_LEN);
									AC_BSS[bssindex]->vMAC_STATE = 1;
								}
								k++;
								if(k == PATCH_OP_RADIO_MAX){
									if_b_info.count = k;									
									fd = open("/dev/wifi0", O_RDWR);
									
									if(fd < 0)
									{
										return WID_DBUS_ERROR;
									}
									ret = ioctl(fd, WIFI_IOC_BATCH_IF_CREATE, &if_b_info);
									close(fd);
									if(ret < 0)
									{
										return WID_DBUS_ERROR;
									}
									k = 0;
									memset(&if_b_info, 0 , sizeof(if_b_info));
								}
							}
						}
					}
				}
			}
		}
		if(k != 0){
			if_b_info.count = k;									
			fd = open("/dev/wifi0", O_RDWR);
			
			if(fd < 0)
			{
				return WID_DBUS_ERROR;
			}
			ret = ioctl(fd, WIFI_IOC_BATCH_IF_CREATE, &if_b_info);
			close(fd);
			if(ret < 0)
			{
				return WID_DBUS_ERROR;
			}
			k = 0;
			memset(&if_b_info, 0 , sizeof(if_b_info));
		}
	}	else{
	// check radio interface in ebr cannot delete
		for(i=0; i<WTP_NUM; i++)
		{
			//if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))
			if(AC_WTP[i]!=NULL)
			{
				for(j=0; j<AC_WTP[i]->RadioCount; j++)
				{
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
						if(!check_bssid_func(bssindex)){
							wid_syslog_err("bssindex %d not exist\n",WlanID);
						}
						else{
							if(check_whether_in_ebr(vrrid,i,j,WlanID,&ebr_id))
							{
								wid_syslog_debug_debug(WID_DEFAULT,"%s,%d some radio in ebr\n",__func__,__LINE__);
								return  RADIO_IN_EBR;
	
							}
						}
					}
				}
			}
		}
		if(AC_WLAN[WlanID]->wlan_if_policy == NO_INTERFACE){
			return WID_DBUS_SUCCESS;
		}
		AC_WLAN[WlanID]->wlan_if_policy = NO_INTERFACE;
		for(i=0; i<WTP_NUM; i++)
		{
			//if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->isused == 1))
			if(AC_WTP[i]!=NULL)
			{
				for(j=0; j<AC_WTP[i]->RadioCount; j++)
				{
					if(AC_WLAN[WlanID]->S_WTP_BSS_List[i][j] != 0)
					{
						int bssindex = AC_WLAN[WlanID]->S_WTP_BSS_List[i][j];
						if(!check_bssid_func(bssindex)){
							wid_syslog_err("bssindex %d not exist\n",WlanID);
						}else{						
							if(AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)
							{
								if_b_info.ifinfo[k].wlanID = 0;
								if_b_info.ifinfo[k].BSSIndex = bssindex;
								if_b_info.ifinfo[k].vrid = local*MAX_INSTANCE+vrrid;								
								if(local)
									snprintf(if_b_info.ifinfo[k].if_name,ETH_IF_NAME_LEN,"r%d-%d-%d.%d",vrrid,i,AC_BSS[bssindex]->Radio_L_ID,WlanID);
								else
									snprintf(if_b_info.ifinfo[k].if_name,ETH_IF_NAME_LEN,"r%d-%d-%d-%d.%d",slotid,vrrid,i,AC_BSS[bssindex]->Radio_L_ID,WlanID);
								memset(AC_BSS[bssindex]->BSS_IF_NAME,0,ETH_IF_NAME_LEN);
								AC_BSS[bssindex]->BSS_IF_POLICY = NO_INTERFACE;
								AC_BSS[bssindex]->BSS_TUNNEL_POLICY = CW_802_DOT_11_TUNNEL;
								k++;
								if(k == PATCH_OP_RADIO_MAX){
									if_b_info.count = k;									
									fd = open("/dev/wifi0", O_RDWR);
									
									if(fd < 0)
									{
										return WID_DBUS_ERROR;
									}
									ret = ioctl(fd, WIFI_IOC_BATCH_IF_DELETE, &if_b_info);
									close(fd);
									if(ret < 0)
									{
										return WID_DBUS_ERROR;
									}
									k = 0;
									memset(&if_b_info, 0 , sizeof(if_b_info));
								}
							}
						}
					}
				}
			}
		}
		if(k != 0){
			if_b_info.count = k;									
			fd = open("/dev/wifi0", O_RDWR);
			
			if(fd < 0)
			{
				return WID_DBUS_ERROR;
			}
			ret = ioctl(fd, WIFI_IOC_BATCH_IF_DELETE, &if_b_info);
			close(fd);
			if(ret < 0)
			{
				return WID_DBUS_ERROR;
			}
			k = 0;
			memset(&if_b_info, 0 , sizeof(if_b_info));
		}
	}
	if(state == 1){
		if(AC_WLAN[WlanID]->WLAN_TUNNEL_POLICY != g_WLAN_TUNNEL_POLICY){
			char nodeFlag = 2;
			ret = WID_RADIO_WLAN_TUNNEL_MODE(WlanID,g_WLAN_TUNNEL_POLICY,nodeFlag);
		}
	}
	return 0;
}
void wtp_get_ifindex_check_nas_id(u_int32_t WTPID){
	unsigned char WlanID = 0;
	int localradio_id,k1;
	for(localradio_id = 0 ; localradio_id < AC_WTP[WTPID]->RadioCount ; localradio_id++)
	{
		if(AC_WTP[WTPID]->WTP_Radio[localradio_id] != NULL)
		{
			for(k1 = 0 ; k1 < L_BSS_NUM ; k1++)
			{
				if(AC_WTP[WTPID]->WTP_Radio[localradio_id]->BSS[k1] != NULL)
				{
					WlanID = AC_WTP[WTPID]->WTP_Radio[localradio_id]->BSS[k1]->WlanID;
					if(AC_WLAN[WlanID]&&(AC_WLAN[WlanID]->Wlan_Ifi != NULL)&&(AC_WTP[WTPID]->BindingSystemIndex != -1))
					{
						struct ifi * wlan_ifi = AC_WLAN[WlanID]->Wlan_Ifi;
						while(wlan_ifi != NULL)
						{

							if(AC_WTP[WTPID]->BindingSystemIndex == wlan_ifi->ifi_index)
							{
								if((wlan_ifi->nas_id_len > 0)&&(AC_WTP[WTPID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len == 0))
								{
									AC_WTP[WTPID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id_len= wlan_ifi->nas_id_len;
									memcpy(AC_WTP[WTPID]->WTP_Radio[localradio_id]->BSS[k1]->nas_id,wlan_ifi->nas_id,NAS_IDENTIFIER_NAME);
								}
								break;
							}
							wlan_ifi = wlan_ifi->ifi_next;
						}
					}
				}
			}
		}
	}			
}
int wid_set_wlan_hotspotid(unsigned char Wlanid,unsigned int hotspotid)
{
	if (AC_WLAN[Wlanid]->hotspot_id == hotspotid)
	{
		return WID_DBUS_SUCCESS;
	}
	
	AC_WLAN[Wlanid]->hotspot_id = hotspotid;
	
	{
		int i,j;
		for(i=0; i<WTP_NUM; i++)
		{
			if(AC_WTP[i]!=NULL)
			{
				for(j=0; j<AC_WTP[i]->RadioCount; j++)
				{
					if(AC_WLAN[Wlanid]->S_WTP_BSS_List[i][j] != 0)
					{
						int bssindex = AC_WLAN[Wlanid]->S_WTP_BSS_List[i][j];
						if(!check_bssid_func(bssindex)){
							wid_syslog_err("<error>%s\n",__func__);
						}else{						
							AC_BSS[bssindex]->hotspot_id = hotspotid;
						}
					}
				}
			}
		}
	}
	return WID_DBUS_SUCCESS;
}

/*fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter */
int read_board_ap_max_counter(unsigned int * count)
{
	int fd_counter,len = 0;
	char board_counter_path[DEFAULT_LEN] = {0};
	char board_counter_buff[PATH_LEN]= {0};
	sprintf(board_counter_path,"/dbm/local_board/board_ap_counter");

	/*read /dbm/local_board/board_ap_max_counter*/
	fd_counter = open(board_counter_path,O_RDONLY);	
	if (fd_counter < 0) {
		wid_syslog_err("%s fd_counter= %d \n",__func__,fd_counter);
		return 1;
	}
	len = read(fd_counter,board_counter_buff,PATH_LEN);
	
	if (len < 0) {
		wid_syslog_err("%s fd_counter= %d len = %d\n",__func__,fd_counter,len);
		close(fd_counter);
		return 1;
	}
	if(len != 0)
	{
		if(board_counter_buff[len-1] == '\n')
		{
			board_counter_buff[len-1] = '\0';
		}
	}		
	parse_int_ID(board_counter_buff,count);
	close(fd_counter);
	if(*count >= WTP_NUM)
	{
		wid_syslog_err("%s we find *count %d >= WTP_NUM\n",__func__,*count);
		*count = WTP_NUM-1;
	}
	return 0;
}
/*fengwenchao add end*/
