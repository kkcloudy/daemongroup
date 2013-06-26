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
* AsdDriver.c
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


#include "includes.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef USE_KERNEL_HEADERS
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#include <linux/if_arp.h>
#include <linux/wireless.h>
#else /* USE_KERNEL_HEADERS */
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include "wireless_copy.h"
#endif /* USE_KERNEL_HEADERS */
#include <time.h>     
#include <sys/time.h> 
#include <pthread.h>
#include <fcntl.h>
#include   <netdb.h>   
#include   <sys/ioctl.h>   
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "asd.h"
#include "ASDCallback.h"
#include "ASD8021XOp.h"
#include "circle.h"
//#include "priv_netlink.h"
#include "ASD80211Op.h"
#include "ASDAccounting.h"
#include "ASD80211AuthOp.h"
#include "ASDStaInfo.h"

#include "ASDHWInfo.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "hmd/hmdpub.h"
#include "ASDRadius/radius_client.h"
#include "config.h"
#include "ip_addr.h"
#include "ASDHWInfo.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ASDDbus_handler.h"
#include "ASDRadius/radius.h"
#include "ASDWPAOp.h"
#include "ASDDbus_handler.h"//xm add 08/12/04
#include "ASDDbus.h"//xm add 08/12/04

#include "ASDPMKSta.h"
#include "ASDPreauth.h"
#include "ASDPreauthBSS.h"
#include "roaming_sta.h"
#include "asd_bak.h"
#include "ASDBeaconOp.h"
#include "Inter_AC_Roaming.h"
#include "StaFlashDisconnOp.h"
#include "ASDNetlinkArpOp.h"
#include "asd_iptables.h"			//mahz add 2011.2.25	

#include "include/cert_auth.h"
#include "include/init.h"
#include "include/proc.h"
#include "include/x509_cert.h"
#include "include/none_cert.h"
#include "Inter_AC_Roaming.h"
#include "ASDMlme.h"
#include "ASDEapolSM.h"
#include "se_agent/se_agent_def.h" // for fastfwd
#include "ASDEAPMethod/eap_defs.h"	/* eap method */
#include "ASDEAPAuth.h"

extern int wpa_debug_level;
extern unsigned char gASDLOGDEBUG;//qiuchen
extern unsigned long gASD_AC_MANAGEMENT_IP;
extern unsigned char FD_CHANGE;
unsigned int wids_mac_last_time = 0;
unsigned char wids_enable = 0;		/*0 for wids close,1 for wids open*/
extern int nl_sock;
#define NETLINK_WITH_ASD 27
#define MAX_PAYLOAD 1024 /* maximum payload size*/

int TableSock;
int DataSock;
int TableSend;
int DataSend;
int TipcSend;
int CmdSock;
int CmdSock_s;
int DmSock;						//add for recieve dm message
int first = 1;
#ifdef ASD_MULTI_THREAD_MODE
int LoopSend;
#endif
unixAddr toWSM;
unixAddr toWSM_D;
unixAddr toWID;
unixAddr toEAG;
unixAddr toDHCP;
unixTipc toFASTFWD;
unixAddr toASD_C;
#ifdef ASD_MULTI_THREAD_MODE
unixAddr ASD_LOOP;
#endif
struct wid_driver_data {
	struct asd_data *wasd;

	char iface[IFNAMSIZ + 1];
	int sock; /* raw packet socket for driver access */
	int ioctl_sock; /* socket for ioctl() use */
	int wext_sock; /* socket for wireless events */

	int we_version;

	u8 *generic_ie;
	size_t generic_ie_len;
};

struct asd_data *get_wasd_for_handle_frame(struct wasd_interfaces *interfaces, DataMsg *msg);
static void handle_frame(struct asd_data *wasd, u8 *buf, size_t len);
static void ASD_STA_OPT_STR(int Op,char *msgtype);
/*ht add,11.3.1*/
void handle_netlink(int sock, void *circle_ctx, void *sock_ctx)
{
	char buf[8192] = {0};
	unsigned int len = 0;
	struct sockaddr_nl snl;
	struct nlmsghdr *nlh = (struct nlmsghdr *) buf;
	DataMsg *pdata = NULL;
	struct asd_data *wasd = NULL;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct iovec iov = { buf, sizeof(buf) };
	struct msghdr msg =
		{ (void *) &snl, sizeof(snl), &iov, 1, NULL, 0, 0 };

	/* Read message from kernel */
	len = recvmsg(nl_sock, &msg, 0);
	asd_printf(ASD_DBUS,MSG_DEBUG,"%s Recv len:%d,nl_sock=%d.\n",__func__,len,nl_sock);

	pdata = (DataMsg*)NLMSG_DATA(nlh);
	asd_printf(ASD_DBUS,MSG_DEBUG,"%s msg->BssIndex = %d,nl_sock=%d.\n",__func__,pdata->BSSIndex,nl_sock);
	pthread_mutex_lock(&asd_g_sta_mutex);

	wasd = get_wasd_for_handle_frame(interfaces, pdata);

	if(wasd)
		handle_frame(wasd, (u8 *)pdata->Data, pdata->DataLen);
	pthread_mutex_unlock(&asd_g_sta_mutex);

	return ;
	
}

void *netlink_sendto_wifi(void *priv, const void *buf, size_t len,
				  int flags)
{
	struct msghdr msg;
	struct sockaddr_nl dest_addr;
	struct iovec iov;
	struct nlmsghdr *nlh = NULL;
	size_t send_size = MAX_PAYLOAD;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s len %d.\n",__func__,len);
	if(len > MAX_PAYLOAD){
		asd_printf(ASD_DBUS,MSG_INFO,"Message To kernel is too long.\n");
		send_size = len;
	}
	if ((nlh = (struct nlmsghdr*)os_zalloc(NLMSG_SPACE(send_size))) == NULL) {
		perror("malloc");
		return NULL;
	}	
	
	memset(&msg, 0, sizeof(msg));
	memset(&dest_addr, 0, sizeof(dest_addr));
	
	/* Assign dest_addr */
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* From Linux Kernel */
	dest_addr.nl_groups = 0; /* Unicast */


	/* Fill the netlink message header */

	nlh->nlmsg_len = NLMSG_SPACE(len);
	nlh->nlmsg_pid = getpid(); /* Self pid */
	nlh->nlmsg_flags = 0;

	/* Fill the netlink message payload */
	memcpy(NLMSG_DATA(nlh), buf, len);
		
	iov.iov_base = (void*)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void*)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Send message to kernel */
	sendmsg(nl_sock, &msg, 0);

	if(nlh != NULL)
	{
		free(nlh);
		nlh = NULL;
	}
	return NULL;
}

void ASD_NETLINIK_INIT()
{
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	int buf = SOCK_BUFSIZE;
	int ret = 0;
	struct HANSI_INFO HInfo;
	memset(&msg, 0, sizeof(msg));
	memset(&src_addr, 0, sizeof(src_addr));
	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&iov,0,sizeof(iov));
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in ASD_NETLINIK_INT");
	asd_printf(ASD_DEFAULT,MSG_INFO,"sizeof(struct msghdr)=%d,sizeof(struct iovec)=%d,sizeof(struct sockaddr_nl)=%d,sizeof(struct nlmsghdr)=%d,NETLINK_WITH_ASD=%d.",sizeof(struct msghdr),sizeof(struct iovec),sizeof(struct sockaddr_nl),sizeof(struct nlmsghdr),NETLINK_WITH_ASD);
	if ((nl_sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_WITH_ASD)) < 0) {
		perror("nl_sock socket");
		return ;
	}
	
	if ((setsockopt(nl_sock,SOL_SOCKET,SO_SNDBUF,&buf,sizeof(buf))) < 0) {	
		perror("setsockopt");
		return ;
	}
	if ((setsockopt(nl_sock,SOL_SOCKET,SO_RCVBUF,&buf,sizeof(buf))) < 0) {	
		perror("setsockopt");
		return;
	}
	fcntl(nl_sock, F_SETFL, O_NONBLOCK);	
	/* Assign src_addr */
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */
	src_addr.nl_groups = 0; /* Unicast */

	HInfo.instId = local*MAX_INSTANCE+vrrid;
	HInfo.asd_pid= getpid();

	if (bind(nl_sock, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
		perror("nl_sock bind");
		return ;
	}
	
	if (circle_register_read_sock(nl_sock, handle_netlink, NULL, NULL))	{
		asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register netlink socket.\n");
		return ;
	}

	/* Assign dest_addr */
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* From Linux Kernel */
	dest_addr.nl_groups = 0; /* Unicast */

	if ((nlh = (struct nlmsghdr*)os_zalloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL)	{
		perror("nlh malloc");
		return ;
	}	

	/* Fill the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(sizeof("ASD_NETLINIK_INIT.\n"));
	nlh->nlmsg_pid = getpid(); /* Self pid */
	nlh->nlmsg_flags = 0;

	/* Fill the netlink message payload */
	strcpy(NLMSG_DATA(nlh), "ASD_NETLINIK_INIT.\n");

	iov.iov_base = (void*)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void*)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	asd_printf(ASD_DBUS,MSG_INFO,"iov.iov_len=%d,nlh->nlmsg_len=%d.\n",iov.iov_len,nlh->nlmsg_len);
#ifndef _nl_ioctl_
	int fd = open("/dev/wifi0", O_RDWR);		
	ret = ioctl(fd, WIFI_IOC_ASD_THRED_ID, &HInfo);
	if(ret < 0){
		asd_printf(ASD_DBUS,MSG_ERROR,"IOCTL fail:nl_sock=%d,strerror(errno)=%s.\n", nl_sock,strerror(errno));
	}else{
		asd_printf(ASD_DBUS,MSG_INFO,"IOCTL successfull.\n");
	}
	close(fd);
#else

	/* Send message to kernel */
	//CWCaptrue_asd(sizeof(msg),(unsigned char*)&msg);
	ret = sendmsg(nl_sock, &msg, 0);
	asd_printf(ASD_DBUS,MSG_INFO,"Send message To kernel:nl_sock=%d, %s,ret=%d.\n",nl_sock, (char*)NLMSG_DATA(nlh),ret);
	if(ret < 0){
		//asd_printf(ASD_DEFAULT,MSG_ERROR, "%s: GetLastError: %d", "asd nl_sock socket", (int) GetLastError());
		asd_printf(ASD_DBUS,MSG_ERROR,"Send message To kernel fail: %s,nl_sock=%d,strerror(errno)=%s.\n", (char *)NLMSG_DATA(nlh),nl_sock,strerror(errno));
	}else{
		asd_printf(ASD_DBUS,MSG_INFO,"Send message To kernel successfull: %s\n", (char *)NLMSG_DATA(nlh));
	}
#endif
	if(nlh != NULL)
	{
		free(nlh);
		nlh = NULL;
	}
	return ;
}


void asd_free_bss_conf(struct asd_bss_config *bss)
{
	if(bss == NULL)
		return;
	int i=0;

	if(bss->nas_identifier != NULL) {
		os_free(bss->nas_identifier);
		bss->nas_identifier = NULL;
	}
	
	if(bss->ssid.wpa_passphrase != NULL) {
		os_free(bss->ssid.wpa_passphrase);
		bss->ssid.wpa_passphrase = NULL;
	}
	if(bss->ssid.wpa_psk!= NULL) {
		os_free(bss->ssid.wpa_psk);
		bss->ssid.wpa_psk = NULL;
	}
	for(i=0; i<NUM_WEP_KEYS; i++) {
		if(bss->ssid.wep.key[i] != NULL) {
			os_free(bss->ssid.wep.key[i]);
			bss->ssid.wep.key[i] = NULL;
		}
	}

	if(bss->radius != NULL) {
		if(bss->radius->auth_servers != NULL) {
			struct asd_radius_server *auth_server;
			for(i=0; i<bss->radius->num_auth_servers; i++) {
				auth_server = &bss->radius->auth_servers[i];
				if(auth_server->shared_secret != NULL) {
					os_free(auth_server->shared_secret);
					auth_server->shared_secret = NULL;
				}
			}
			os_free(bss->radius->auth_servers);
			bss->radius->auth_servers = NULL;
		}
		if(bss->radius->acct_servers != NULL) {
			int i=0;
			struct asd_radius_server *acct_server;
			for(i=0; i<bss->radius->num_acct_servers; i++) {
				acct_server = &bss->radius->acct_servers[i];
				if(acct_server->shared_secret != NULL) {
					os_free(acct_server->shared_secret);
					acct_server->shared_secret = NULL;
				}
			}
			os_free(bss->radius->acct_servers);
			bss->radius->acct_servers = NULL;
		}
		os_free(bss->radius);
		bss->radius = NULL;
	}

	os_free(bss);
	bss = NULL;

	return ;
}

void free_maclist(struct acl_config *conf, struct maclist *list)
{
	struct maclist *entry, *tmp;

	entry = list;
	while(entry != NULL){
		if(entry->add_reason == 1)
			circle_cancel_timeout(del_wids_mac_timer, conf, entry);
		tmp = entry;
		entry = entry->next;
		os_memset(tmp,0,sizeof(struct maclist));
		os_free(tmp);
		tmp = NULL;
	}
}

/************************************************************
	add mac in wlan black list for wids. 
	ht add 09/07/14
************************************************************/
int add_mac_in_blacklist_for_wids(struct acl_config *conf, struct maclist **newlist, wids_info *info)
{
	struct maclist *entry, *prev,*tmp;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s \n",__func__);
	if((conf == NULL) || (info == NULL))
		return -1;

	tmp = os_zalloc(sizeof(*tmp));
	if (tmp == NULL) {
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}else {
		os_memcpy(tmp->addr, info->bssid, ETH_ALEN);
		tmp->add_reason = 1;
		os_memcpy(tmp->vapbssid, info->vapbssid, ETH_ALEN);
		tmp->attacktype = info->attacktype;
		tmp->frametype = info->frametype;
		tmp->channel = info->channel;
		tmp->rssi = info->rssi;
		time(&tmp->add_time);
		get_sysruntime(&(tmp->add_time_sysruntime)); //qiuchen add it
		tmp->next = NULL;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" atype:%d ftype:%d %s\n",MAC2STR(tmp->addr),tmp->attacktype,tmp->frametype,ctime(&tmp->add_time));

		entry = conf->deny_mac;
		prev = NULL;
		while (entry) {
			if (os_memcmp(entry->addr, info->bssid, ETH_ALEN) == 0) {
				*newlist = entry;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" is already in the black list.\n",MAC2STR(info->bssid));
				os_free(tmp);
				tmp=NULL;
				return 1;
			}
			prev = entry;
			entry = entry->next;
		}
		if(prev == NULL)
			conf->deny_mac = tmp;
		else 
			prev->next = tmp;
		*newlist = tmp;
		conf->num_wids_mac++;
		
		return 0;			
	}

}


/************************************************************
	del mac in wlan black list for wids. 
	ht add 09/07/14
************************************************************/
int del_mac_in_blacklist_for_wids(struct acl_config *conf,  u8 *addr)
{
	struct maclist *entry, *prev,*tmp;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s \n",__func__);
	if((conf == NULL) || (addr == NULL))
		return -1;

	entry = conf->deny_mac;
	prev = NULL;
	while (entry) {
		if (os_memcmp(entry->addr, addr, ETH_ALEN) == 0) {
			if (prev)
				prev->next = entry->next;
			else
				conf->deny_mac = entry->next;
			tmp = entry;
			tmp->next = NULL;
			os_free(tmp);
			tmp=NULL;
			conf->num_wids_mac--;
			return 0;
		}
		prev = entry;
		entry = entry->next;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" is not in the black list.\n",MAC2STR(addr));
	return 0;			

}


/************************************************************
	clean mac in wlan black list for wids. 
	ht add 09/07/14
************************************************************/
void clean_mac_in_blacklist_for_wids()
{
	struct maclist *entry, *prev,*tmp;
	struct acl_config *conf;
	int i=0;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s \n",__func__);
	for(i=1; i< WLAN_NUM; i++){
		if((ASD_WLAN[i] == NULL) || (ASD_WLAN[i]->acl_conf == NULL)	) {
			continue;
		}else {
			conf = ASD_WLAN[i]->acl_conf;
			conf->macaddr_acl = conf->wlan_last_macaddr_acl;
			if(ASD_WLAN[i]->acl_conf->num_wids_mac == 0)
				continue;
			
			entry = conf->deny_mac;
			prev = NULL;
			while (entry) {
				if (entry->add_reason == 1) {
					circle_cancel_timeout(del_wids_mac_timer, conf, entry);
					if (prev)
						prev->next = entry->next;
					else
						conf->deny_mac = entry->next;
					tmp = entry;
					entry = entry->next;
					tmp->next = NULL;
					os_free(tmp);
					tmp = NULL;
					conf->num_wids_mac--;
				}else {
					prev = entry;
					entry = entry->next;
				}
			}
		}
	}
	return ;			

}


static int wapid_setup_interface(apdata_info *wasd)
{
	int ret = 0;
	
	/*初始化鉴别与密钥管理套间*/
	ap_initialize_akm(wasd);
	return ret;
}


int ASD_WAPI_INTERFACE_INIT(struct wapid_interfaces *user,struct asd_data * wasd){
	unsigned char WLANID;
	unsigned int SID;
	WLANID = wasd->WlanID;
	SID = wasd->SecurityID;
	int ret;
	if((ASD_WLAN[WLANID] == NULL)||(ASD_SECURITY[SID] == NULL))
		return -1;
	user->identity = os_zalloc(strlen(ASD_WLAN[WLANID]->WlanName)+1);
	if (user->identity == NULL) {
		asd_printf(ASD_WAPI,MSG_ERROR,"Failed to allocate memory for VAP "
			   "identity\n");
		return -1;
	}
	memcpy(user->identity, ASD_WLAN[WLANID]->WlanName, strlen(ASD_WLAN[WLANID]->WlanName)+1);
	asd_printf(ASD_WAPI,MSG_DEBUG,"user->identity = %s\n", user->identity);
	user->identity_len = strlen(ASD_WLAN[WLANID]->WlanName);
	user->ssid = os_zalloc(strlen(ASD_WLAN[WLANID]->ESSID)+1);
	if (user->ssid == NULL) {
		asd_printf(ASD_WAPI,MSG_ERROR,"Failed to allocate memory for VAP "
			   "ssid\n");
		os_free(user->identity);
		user->identity=NULL;
		return -1;
	}
	memcpy(user->ssid, ASD_WLAN[WLANID]->ESSID, strlen(ASD_WLAN[WLANID]->ESSID)+1);
	asd_printf(ASD_WAPI,MSG_DEBUG,"user->ssid = %s\n", user->ssid);
	user->ssid_len = strlen(ASD_WLAN[WLANID]->ESSID);
	user->ssid_method = 0;
	if(ASD_SECURITY[SID]->securityType == WAPI_PSK)/*psk*/
	{
		user->wapi_method = 7;
		if(ASD_SECURITY[SID]->keyInputType == HEX)
		{	
			user->psk_type= 1;		
			user->password =os_zalloc((ASD_SECURITY[SID]->keyLen) / 2);
			if (user->password == NULL) {
				asd_printf(ASD_WAPI,MSG_ERROR,"Failed to allocate memory for VAP "
					   "password\n");
				os_free(user->identity);
				user->identity=NULL;
				os_free(user->ssid);
				user->ssid=NULL;
				return -1;
			}
			if (str2byte(ASD_SECURITY[SID]->SecurityKey, (ASD_SECURITY[SID]->keyLen), user->password ) < 0) {
				asd_printf(ASD_WAPI,MSG_DEBUG,"Invalid hex password \n");				
				os_free(user->identity);
				user->identity=NULL;
				os_free(user->ssid);
				user->ssid=NULL;
				os_free(user->password);
				user->password=NULL;
				return -1;
			}
			user->password_len = (ASD_SECURITY[SID]->keyLen) / 2;
			asd_printf(ASD_WAPI,MSG_DEBUG,"user->password %s,%d", user->password,user->password_len);
		}else
		{
			
			user->psk_type= 0;		
			user->password = os_zalloc((ASD_SECURITY[SID]->keyLen)+1);
			if (user->password == NULL) {
				asd_printf(ASD_WAPI,MSG_ERROR,"Failed to allocate memory for VAP "
					   "password\n");
				os_free(user->identity);	
				user->identity=NULL;
				os_free(user->ssid);
				user->ssid=NULL;
				return -1;
			}
			memcpy(user->password,ASD_SECURITY[SID]->SecurityKey,(ASD_SECURITY[SID]->keyLen)+1);
			user->password_len =ASD_SECURITY[SID]->keyLen;
			asd_printf(ASD_WAPI,MSG_DEBUG,"user->password = %s\n", (char *)user->password);
		}
	
	
	}else{
		user->wapi_method = 11;
	}
	
	user ->wapid = os_zalloc(sizeof(apdata_info));
	if (user ->wapid == NULL) {
		asd_printf(ASD_WAPI,MSG_ERROR,"VAP user allocation failed\n");
		if(ASD_SECURITY[SID]->securityType == WAPI_PSK){
			os_free(user->identity);	
			user->identity=NULL;
			os_free(user->ssid);
			user->ssid=NULL;
			os_free(user->password);
			user->password=NULL;
		}
		return -1;
	}
	user->circle_save = wasd->wapi_wasd;
	user->next = NULL;
	memset(user ->wapid, 0, sizeof(apdata_info));
	user ->wapid->wai_policy= user->wapi_method;
	user ->wapid->user_data = (void *)user;
	memcpy(user ->wapid->macaddr,wasd->own_addr,6);
	user ->wapid->mtu = 1416;
	ret = ap_initialize(user ->wapid);
	if(ret == -1) return -1;
		
	wapid_setup_interface(user ->wapid);
	return 0;
}

void ASD_Recv_AS_Data(int sock, void *circle_ctx, void *sock_ctx)
{
	/*** receive Data from as_udp_sk handle****/
	struct asd_data * wasd = circle_ctx;
	socklen_t i = 0;
	int readlen = 0;		
	unsigned char recv_buf[3000] = {0,};
	apdata_info *pap = NULL;
	struct sockaddr_in s_addr;
	i = sizeof(s_addr);
	readlen = recvfrom(sock, recv_buf, 3000, 0,
						(struct sockaddr *)&s_addr, &i);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"recv data from AS\n");
	if(readlen > WAI_HLEN/*WAI head*/)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Receive certificate authentication response from AS, packet len=%d\n",readlen);
		pap = wasd->wapi_wasd->vap_user->wapid;
		if(pap)
			wapi_process_1_of_1_from_as(recv_buf, readlen, pap);
	}	
}


int get_wapi_conf(struct asd_wapi *wapi, int wapi_type, int SID)
{
	if(ASD_SECURITY[SID] == NULL)
		return -1;	
	wapi->cert_info.config.used_cert = wapi_type;/*1:x509 2:GBW */
	if(wapi_type){
		memcpy(wapi->cert_info.config.cert_name,ASD_SECURITY[SID]->wapi_as.unite_cert_path,ASD_SECURITY[SID]->wapi_as.unite_cert_path_len);
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s multi_cert = %d\n",__func__, wapi->multi_cert);
		if(wapi->multi_cert){
			memcpy(wapi->cert_info.config.ca_cert_name,ASD_SECURITY[SID]->wapi_as.ca_cert_path,ASD_SECURITY[SID]->wapi_as.ca_cert_path_len);
			asd_printf(ASD_WAPI,MSG_DEBUG,"CA Certificate file name =%s\n", wapi->cert_info.config.ca_cert_name);
		}
		asd_printf(ASD_WAPI,MSG_DEBUG,"Certificate file name =%s\n", wapi->cert_info.config.cert_name);
		asd_printf(ASD_WAPI,MSG_DEBUG,"get_wapi_conf:cert_index=%d\n",wapi->cert_info.config.used_cert );
		if(strlen(wapi->cert_info.config.cert_name))
		{		
			int res;
			res = register_certificate((void *)wapi);
			if(res !=0){
				asd_printf(ASD_WAPI,MSG_DEBUG,"pppp\n");
			}else {
					asd_printf(ASD_WAPI,MSG_DEBUG,"in %s %d\n", __func__, __LINE__);
					wapi->has_cert = 1;
				/*获得AE的身份*/
				wai_fixdata_id_by_ident(wapi->cert_info.ap_cert_obj->user_cert_st, 
									wapi->cert_info.ap_cert_obj, &(wapi->ae_id), 
									wapi->cert_info.config.used_cert);
			}
		}
		wapi->as_addr.sin_family = AF_INET;
		wapi->as_addr.sin_port = htons(ASD_SECURITY[SID]->wapi_as.as_port);
		
		if(inet_aton(ASD_SECURITY[SID]->wapi_as.as_ip, &(wapi->as_addr.sin_addr)) == 0)
		{
			inet_aton("127.0.0.1", &(wapi->as_addr.sin_addr));
		}
		
		if((wapi->as_udp_sk = open_socket_for_asu()) < 0) 
		{
			asd_printf(ASD_WAPI,MSG_DEBUG,"\nCreat udp socket to AS error!!!\n\n");
	       		exit(-1);
		}		
		if (circle_register_read_sock(wapi->as_udp_sk, ASD_Recv_AS_Data, wapi->wasd_bk, NULL))
		{
			asd_printf(ASD_WAPI,MSG_WARNING,"Could not register read socket\n");
			return -1;
		}
		
	}
	return 0;
}	



static struct asd_hw_modes * asd_get_hw_feature_data_w(void *priv,
							    u16 *num_modes,
							    u16 *flags)
{
	struct asd_hw_modes *mode;
	int i, clen, rlen;
	const short chan2freq[14] = {
		2412, 2417, 2422, 2427, 2432, 2437, 2442,
		2447, 2452, 2457, 2462, 2467, 2472, 2484
	};

	mode = os_zalloc(sizeof(struct asd_hw_modes));
	if (mode == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}

	*num_modes = 1;
	*flags = 0;

	mode->mode = asd_MODE_IEEE80211G;
	mode->num_channels = 14;
	mode->num_rates = 4;

	clen = mode->num_channels * sizeof(struct asd_channel_data);
	rlen = mode->num_rates * sizeof(struct asd_rate_data);

	mode->channels = os_zalloc(clen);
	mode->rates = os_zalloc(rlen);
	if (mode->channels == NULL || mode->rates == NULL) {
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}

	for (i = 0; i < 14; i++) {
		mode->channels[i].chan = i + 1;
		mode->channels[i].freq = chan2freq[i];
	}

	mode->rates[0].rate = 10;
	mode->rates[0].flags = asd_RATE_CCK;
	mode->rates[1].rate = 20;
	mode->rates[1].flags = asd_RATE_CCK;
	mode->rates[2].rate = 55;
	mode->rates[2].flags = asd_RATE_CCK;
	mode->rates[3].rate = 110;
	mode->rates[3].flags = asd_RATE_CCK;

	return mode;
}

int asd_hw_feature_init(struct asd_iface *iface){

	u16 num_modes, flags;
	struct asd_hw_modes *modes;

	modes = asd_get_hw_feature_data_w(NULL, &num_modes, &flags);
	if (modes == NULL) {
		asd_logger(NULL, NULL, asd_MODULE_IEEE80211,
			   asd_LEVEL_DEBUG,
			   "Fetching hardware channel/rate support not "
			   "supported.");
	return -1;
	}

	iface->hw_flags = flags;
	iface->current_rates = NULL;
	iface->num_rates = 0;
	//asd_free_hw_features(iface->hw_features, iface->num_hw_features);
	iface->hw_features = modes;
	iface->current_mode = modes;
	iface->num_hw_features = num_modes;

	return 0;

}

void asd_iface_malloc(unsigned int g_radio_id)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int bssnum = 0;

	if(interfaces->iface[g_radio_id])
		return ;
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"interfaces->iface[%d] setup and init\n",g_radio_id);
	interfaces->iface[g_radio_id] = (struct asd_iface*)os_zalloc(sizeof(struct asd_iface));
	interfaces->iface[g_radio_id]->bss = NULL;
	interfaces->iface[g_radio_id]->interfaces = interfaces;
	interfaces->iface[g_radio_id]->num_bss = 0;
	interfaces->iface[g_radio_id]->bss = os_zalloc(L_BSS_NUM * sizeof(struct asd_data*));
	for(bssnum = 0; bssnum < L_BSS_NUM; bssnum++)
		interfaces->iface[g_radio_id]->bss[bssnum] = NULL;
	if(asd_hw_feature_init(interfaces->iface[g_radio_id]) < 0)
		asd_printf(ASD_DEFAULT,MSG_WARNING,"asd_hw_feature_init failed\n");
}


static void handle_data(struct asd_data *wasd, u8 *buf, size_t len,
			u16 stype)
{
	struct ieee80211_hdr *hdr;
	u16 fc, ethertype;
	u8 *pos, *sa, *da;
	int ret;
	size_t left;
	struct sta_info *sta;

	if (len < sizeof(struct ieee80211_hdr))
		return;

	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);

	if ((fc & (WLAN_FC_FROMDS | WLAN_FC_TODS)) != WLAN_FC_TODS) {
		asd_printf(ASD_80211,MSG_DEBUG,"Not ToDS data frame (fc=0x%04x)\n", fc);
		return;
	}

	sa = hdr->addr2;
	da = hdr->addr3;
	ret = memcmp(hdr->addr3,hdr->addr1,6);
	sta = ap_get_sta(wasd, sa);
	if ((!sta || !(sta->flags & WLAN_STA_ASSOC))&&ret == 0) {
		asd_printf(ASD_80211,MSG_DEBUG,"Data frame from not associated STA " MACSTR "\n",
		       MAC2STR(sa));
		if (sta && (sta->flags & WLAN_STA_AUTH))
			asd_sta_disassoc(
				wasd, sa,
				WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
		else
			asd_sta_deauth(
				wasd, sa,
				WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
		return;
	}
	if(ret != 0)
		asd_printf(ASD_80211,MSG_DEBUG,"\n there is may be a ETH_P_PREAUTH\n");
	pos = (u8 *) (hdr + 1);
	left = len - sizeof(*hdr);

	if (left < sizeof(rfc1042_header)) {
		asd_printf(ASD_80211,MSG_DEBUG,"Too short data frame\n");
		return;
	}

	/* ht added 111202 */
	if((WLAN_FC_GET_STYPE(fc)|WLAN_FC_STYPE_QOS_DATA) == WLAN_FC_STYPE_QOS_DATA){
		asd_printf(ASD_80211,MSG_DEBUG,"Qos Data frame: 0x%02X%02x%02X%02X.\n",pos[0],pos[1],pos[2],pos[3]);
	}
	if (memcmp(pos, rfc1042_header, sizeof(rfc1042_header)) != 0) {
		pos += 4;
		left -=4 ; /* ht added 091208 */
		if(memcmp(pos, rfc1042_header, sizeof(rfc1042_header)) != 0){
			asd_printf(ASD_80211,MSG_DEBUG,"Data frame with no RFC1042 header\n");
			return;
		}
	}
	pos += sizeof(rfc1042_header);
	left -= sizeof(rfc1042_header);

	if (left < 2) {
		asd_printf(ASD_80211,MSG_DEBUG,"No ethertype in data frame\n");
		return;
	}
	//Qc add 
	unsigned char SID = wasd->SecurityID;
	unsigned int securitytype = 0;
	unsigned int extensible_auth = 0;
	if(ASD_SECURITY[SID]){
		securitytype = ASD_SECURITY[SID]->securityType;
		extensible_auth = ASD_SECURITY[SID]->extensible_auth;
	}
	//end
	ethertype = WPA_GET_BE16(pos);
	pos += 2;
	left -= 2;
	switch (ethertype) {
	case ETH_P_PAE:
		if(!((securitytype == IEEE8021X)||(securitytype == WPA_E)||(securitytype == WPA2_E)||(securitytype == MD5)||(securitytype == WAPI_AUTH)||(extensible_auth == 1))){
			syslog(LOG_WARNING|LOG_LOCAL7,"[%d-%d]BSS "MACSTR" receive wrong EAP Package! Sta "MACSTR" securitytype %d\n",slotid,vrrid,MAC2STR(wasd->own_addr),MAC2STR(sa),securitytype);
		}
		ieee802_1x_receive(wasd, sa, pos, left);
		break;
	case ETH_P_PREAUTH:
		if(!((securitytype == IEEE8021X)||(securitytype == WPA_E)||(securitytype == WPA2_E)||(securitytype == MD5)||(securitytype == WAPI_AUTH)||(extensible_auth == 1))){
			syslog(LOG_WARNING|LOG_LOCAL7,"[%d-%d]BSS "MACSTR" receive wrong EAP Package! Sta "MACSTR" securitytype %d\n",slotid,vrrid,MAC2STR(wasd->own_addr),MAC2STR(sa),securitytype);
		}
		asd_printf(ASD_80211,MSG_DEBUG,"\n ETH_P_PREAUTH \n");
		rsn_preauth_receive_thinap(wasd, hdr->addr2, hdr->addr3, pos, left);
		break;
	case ETH_P_WAPI:	
		if(securitytype != WAPI_AUTH || securitytype != WAPI_PSK){
			syslog(LOG_WARNING|LOG_LOCAL7,"[%d-%d]BSS "MACSTR" receive wrong EAP Package! Sta "MACSTR" securitytype %d\n",slotid,vrrid,MAC2STR(wasd->own_addr),MAC2STR(sa),securitytype);
		}
		asd_printf(ASD_80211,MSG_DEBUG,"\n ETH_P_WAPI \n");
		if(wasd->wapi_wasd && wasd->wapi_wasd->vap_user)
			wapi_process_from_sta(pos, left, sa, wasd->wapi_wasd->vap_user->wapid);
		else
			asd_printf(ASD_80211,MSG_DEBUG,"\n ETH_P_WAPI WRONG!\n");
		break;
	default:
		asd_printf(ASD_80211,MSG_DEBUG,"Unknown ethertype 0x%04x in data frame\n", ethertype);
		break;
	}
}

static void handle_tx_callback(struct asd_data *wasd, u8 *buf, size_t len,
			       int ok)
{
	struct ieee80211_hdr *hdr;
	u16 fc, type, stype;
	struct sta_info *sta;

	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);

	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);

	switch (type) {
	case WLAN_FC_TYPE_MGMT:
		asd_printf(ASD_80211,MSG_DEBUG, "MGMT (TX callback) %s",
			   ok ? "ACK" : "fail");
		ieee802_11_mgmt_cb(wasd, buf, len, stype, ok);
		break;
	case WLAN_FC_TYPE_CTRL:
		asd_printf(ASD_80211,MSG_DEBUG, "CTRL (TX callback) %s",
			   ok ? "ACK" : "fail");
		break;
	case WLAN_FC_TYPE_DATA:
		asd_printf(ASD_80211,MSG_DEBUG, "DATA (TX callback) %s",
			   ok ? "ACK" : "fail");
		sta = ap_get_sta(wasd, hdr->addr1);
		if (sta && sta->flags & WLAN_STA_PENDING_POLL) {
			asd_printf(ASD_80211,MSG_DEBUG, "STA " MACSTR
				   " %s pending activity poll",
				   MAC2STR(sta->addr),
				   ok ? "ACKed" : "did not ACK");
			if (ok)
				sta->flags &= ~WLAN_STA_PENDING_POLL;
		}
		if (sta)
			ieee802_1x_tx_status(wasd, sta, buf, len, ok);
		break;
	default:
		asd_printf(ASD_80211,MSG_DEBUG,"unknown TX callback frame type %d\n", type);
		break;
	}
}



static void handle_frame(struct asd_data *wasd, u8 *buf, size_t len)
{
	struct ieee80211_hdr *hdr;
	u16 fc, extra_len, type, stype;
	unsigned char *extra = NULL;
	size_t data_len = len;
	int ver;

	/* PSPOLL is only 16 bytes, but driver does not (at least yet) pass
	 * these to user space */
	asd_printf(ASD_80211,MSG_DEBUG,"%s len = %d.\n",__func__,len);
	if (len < 24) {
		asd_printf(ASD_80211,MSG_MSGDUMP, "handle_frame: too short (%lu)",
			   (unsigned long) len);
		return;
	}

	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);
	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);

	if (type != WLAN_FC_TYPE_MGMT || stype != WLAN_FC_STYPE_BEACON) {
		wpa_hexdump(MSG_MSGDUMP, "Received management frame",
			    buf, len);
	}

	ver = fc & WLAN_FC_PVER;

	/* protocol version 3 is reserved for indicating extra data after the
	 * payload, version 2 for indicating ACKed frame (TX callbacks), and
	 * version 1 for indicating failed frame (no ACK, TX callbacks) */
	if (ver == 3) {
		u8 *pos = buf + len - 2;
		extra_len = WPA_GET_LE16(pos);
		asd_printf(ASD_80211,MSG_DEBUG,"extra data in frame (elen=%d)\n", extra_len);
		if ((size_t) extra_len + 2 > len) {
			asd_printf(ASD_80211,MSG_DEBUG,"  extra data overflow\n");
			return;
		}
		len -= extra_len + 2;
		extra = buf + len;
	} else if (ver == 1 || ver == 2) {
		handle_tx_callback(wasd, buf, data_len, ver == 2 ? 1 : 0);
		return;
	} else if (ver != 0) {
		asd_printf(ASD_80211,MSG_DEBUG,"unknown protocol version %d\n", ver);
		return;
	}

	switch (type) {
	case WLAN_FC_TYPE_MGMT:
		wasd->info->rx_mgmt_pkts++;
		if ((stype != WLAN_FC_STYPE_BEACON)&&(stype != WLAN_FC_STYPE_PROBE_REQ))
			asd_printf(ASD_80211,MSG_MSGDUMP, "MGMT");
		ieee802_11_mgmt(wasd, buf, data_len, stype, NULL);
		break;
	case WLAN_FC_TYPE_CTRL:
		wasd->info->rx_ctrl_pkts++;
		asd_printf(ASD_80211,MSG_DEBUG, "CTRL");
		break;
	case WLAN_FC_TYPE_DATA:
		wasd->info->rx_auth_pkts++;
		asd_printf(ASD_80211,MSG_DEBUG, "DATA");
		handle_data(wasd, buf, data_len, stype);
		break;
	default:
		asd_printf(ASD_80211,MSG_DEBUG, "unknown frame type %d", type);
		break;
	}
}

/*
struct asd_data *get_wasd_for_handle_frame(struct wasd_interfaces *interfaces, u8 *buf, size_t len)
{
	struct ieee80211_hdr *hdr;
	struct ieee80211_mgmt *mgmt;
	struct asd_data * wasd;
	u16 fc, extra_len, type, stype;
	int broadcast,i=0,j=0;
	if (len < 24) {
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP, "handle_frame: too short (%lu)",
			   (unsigned long) len);
		return NULL;
	}
	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);
	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);
	switch(type){
		case WLAN_FC_TYPE_MGMT:
		case WLAN_FC_TYPE_CTRL:
			mgmt = (struct ieee80211_mgmt *) buf;
			broadcast = mgmt->bssid[0] == 0xff && mgmt->bssid[1] == 0xff &&
				mgmt->bssid[2] == 0xff && mgmt->bssid[3] == 0xff &&
				mgmt->bssid[4] == 0xff && mgmt->bssid[5] == 0xff;
			if(broadcast == 1)
				return NULL;
			for(i = 0; i < G_RADIO_NUM; i++){
				if(interfaces->iface[i] != NULL){
					for(j = 0; j < 4; j++){
						if(interfaces->iface[i]->bss[j] != NULL){
							wasd = interfaces->iface[i]->bss[j];
							if (!broadcast &&
								os_memcmp(mgmt->bssid, wasd->own_addr, ETH_ALEN) != 0 &&
								(wasd->assoc_ap_state == DO_NOT_ASSOC ||
							 	os_memcmp(mgmt->bssid, wasd->conf->assoc_ap_addr, ETH_ALEN) != 0))
							{
								asd_printf(ASD_DEFAULT,MSG_DEBUG,"MGMT: BSSID=" MACSTR " not our address\n",
								   MAC2STR(mgmt->bssid));
								return NULL;
							}
							return wasd;
						}
					}
				}
			}
		case WLAN_FC_TYPE_DATA:
			return NULL;
	}
		return NULL;

}
*/
struct asd_data *get_wasd_for_handle_frame(struct wasd_interfaces *interfaces, DataMsg *msg){

	struct asd_data * wasd;
	int i;
	if (msg->Radio_G_ID != 0)
		i = msg->BSSIndex % L_BSS_NUM ;
	else {
		asd_printf(ASD_80211,MSG_DEBUG,"msg->Radio_G_ID =0\n");
		return NULL;
	}
	if(interfaces->iface[msg->Radio_G_ID] == NULL)
		return NULL;
	else if (interfaces->iface[msg->Radio_G_ID]->bss[i] == NULL)
		return NULL;
	wasd = interfaces->iface[msg->Radio_G_ID]->bss[i];
//	asd_printf(ASD_80211,MSG_DEBUG,"get wasd in get_wasd_for_handle_frame\n");
	return wasd;
}

struct asd_data *get_wasd_for_handle_smsg(struct wasd_interfaces *interfaces, STAStatistics *msg){

	struct asd_data * wasd;
	int i;
	if (msg->Radio_G_ID != 0)
		i = msg->BSSIndex % L_BSS_NUM ;
	else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->Radio_G_ID =0\n");
		return NULL;
	}
	
	if(interfaces->iface[msg->Radio_G_ID] == NULL)
		return NULL;
	else if (interfaces->iface[msg->Radio_G_ID]->bss[i] == NULL)
		return NULL;
	wasd = interfaces->iface[msg->Radio_G_ID]->bss[i];
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"get wasd in get_wasd_for_handle_smsg\n");
	return wasd;
}

#if 0
static void handle_smsg(u8 *buf,u32 cnt,u32 len)		//ht add,081027
{
	STAStatistics *stainfo;
	struct asd_data *wasd ; 
	struct sta_info *sta;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int    i;
	
	if( len < cnt*sizeof(STAStatistics) ) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"handle_smsg : msg is not enough\n");
		return;
		}

	
	for(i = 0; i < cnt; i++){
		stainfo = (STAStatistics *)(buf + i*sizeof(STAStatistics));
		wasd = get_wasd_for_handle_smsg(interfaces, stainfo);
		if (wasd == NULL)	
			continue ;
		sta = ap_get_sta(wasd,stainfo->STAMAC);
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"%4d  "MACSTR "\n",i+1,MAC2STR(stainfo->STAMAC));
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"rx_u bytes:     %llu\n",stainfo->rx_pkt_unicast);
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"tx_u bytes:     %llu\n",stainfo->tx_pkt_unicast);
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"rx_u packets:   %llu\n",stainfo->rx_unicast);
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"tx_u packets:   %llu\n",stainfo->tx_unicast);
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"retry bytes:    %llu\n",stainfo->retry);
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"retry packets:  %llu\n",stainfo->retry_pkt);
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP,"err packets:    %llu\n\n",stainfo->err);
		if (sta == NULL)
			continue ;
		if(wasd->bss_iface_type){
			sta->rxbytes = stainfo->rx_pkt_unicast + stainfo->rx_pkt_broadcast;
			sta->txbytes = stainfo->tx_pkt_unicast + stainfo->tx_pkt_broadcast;
			sta->rxpackets = stainfo->rx_unicast + stainfo->rx_broadcast;
			sta->txpackets = stainfo->tx_unicast + stainfo->tx_broadcast;

			sta->retrybytes= stainfo->retry;
			sta->retrypackets = stainfo->retry_pkt;
			sta->errpackets = stainfo->err;
		}
		if (asd_sta_arp_listen == 0){
			if (asd_get_ip(sta->addr,sta->in_addr))					//ht add,081030
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"Can't get ip by mac in the arp cache.\n");
			else
				inet_aton(sta->in_addr,&sta->ip_addr);
		}
	}	
	return ;
}
#endif

#ifndef ASD_MULTI_THREAD_MODE 

void handle_read(int sock, void *circle_ctx, void *sock_ctx)
{
	struct asd_data *wasd ;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	DataMsg *msg;
	int len;
	unsigned char buf[3000];

	len = recvfrom(sock, buf, sizeof(buf), 0, NULL, 0);
	if (len < 0) {
		perror("recv");
		return;
	}
	msg = (DataMsg*)buf;
	if(msg->BSSIndex==0){
		msg->BSSIndex=(msg->WTPID*L_RADIO_NUM)*L_BSS_NUM;
		msg->Radio_G_ID=msg->WTPID*L_RADIO_NUM;
	}
	pthread_mutex_lock(&asd_g_sta_mutex);
	wasd = get_wasd_for_handle_frame(interfaces, msg);
	

	if(wasd == NULL)
	{
		pthread_mutex_unlock(&asd_g_sta_mutex);
		return;
	}
	handle_frame(wasd, (unsigned char *)msg->Data, msg->DataLen);
	pthread_mutex_unlock(&asd_g_sta_mutex);
}

#else
static void handle_read()
{
	while(1){
		struct asd_data *wasd ;	
		DataMsg *msg;
		int len;
		unsigned char buf[3000];
		memset(buf, 0, 3000);
		len = recvfrom(DataSock, buf, sizeof(buf), 0, NULL, 0);
		if (len < 0) {
			perror("recv");
			return;
		}		
		struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
		msg = (DataMsg*)buf;
		if(msg->BSSIndex==0){
			msg->BSSIndex=(msg->WTPID*L_RADIO_NUM)*L_BSS_NUM;
			msg->Radio_G_ID=msg->WTPID*L_RADIO_NUM;
		}
		pthread_mutex_lock(&asd_g_sta_mutex);

		wasd = get_wasd_for_handle_frame(interfaces, msg);
		

		if(wasd == NULL)
		{
			pthread_mutex_unlock(&asd_g_sta_mutex);
			return;
		}
#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_lock(&(wasd->asd_sta_mutex));
#endif
		handle_frame(wasd, msg->Data, msg->DataLen);

#ifdef ASD_USE_PERBSS_LOCK
		pthread_mutex_unlock(&(wasd->asd_sta_mutex));
#endif
		pthread_mutex_unlock(&asd_g_sta_mutex);
	}
}

#endif
//qiuchen
void asd_free_radius(struct heart_test_radius_data* radius)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s,%d\n",__func__,__LINE__);
	struct asd_radius_server *tmp = NULL;
	unsigned char *temp = NULL;
	if(radius != NULL){
		if(radius->conf != NULL){
			if(radius->conf->auth_servers){
				if(radius->conf->num_auth_servers > 1){
					if(&(radius->conf->auth_servers[1])){
						tmp = &(radius->conf->auth_servers[1]);
						if(tmp->shared_secret)
							free(tmp->shared_secret);
						tmp = NULL;
					}
				}
				if(&(radius->conf->auth_servers[0])){
					tmp = &(radius->conf->auth_servers[0]);
					if(tmp->shared_secret)
						free(tmp->shared_secret);
					tmp = NULL;
				}
				free(radius->conf->auth_servers);
			}
			if(radius->conf->acct_servers){
				if(radius->conf->num_acct_servers > 1){
					if(&(radius->conf->acct_servers[1])){
						tmp = &(radius->conf->acct_servers[1]);
						if(tmp->shared_secret)
							free(tmp->shared_secret);
						tmp = NULL;
					}
				}
				if(&(radius->conf->acct_servers[0])){
					tmp = &(radius->conf->acct_servers[0]);
					if(tmp->shared_secret)
						free(tmp->shared_secret);
					tmp = NULL;
				}
				free(radius->conf->acct_servers);
			}
			if(radius->conf->c_test_window_acct){
				temp = radius->conf->c_test_window_acct;
				free(temp);
				temp = NULL;
				radius->conf->c_test_window_acct = NULL;
			}
			if(radius->conf->r_test_window_acct){
				temp = radius->conf->r_test_window_acct;
				free(temp);
				temp = NULL;
				radius->conf->r_test_window_acct = NULL;
			}
			if(radius->conf->c_test_window_auth){
				temp = radius->conf->c_test_window_auth;
				free(temp);
				temp = NULL;
				radius->conf->c_test_window_auth = NULL;

			}
			if(radius->conf->r_test_window_auth){
				temp = radius->conf->r_test_window_auth;
				free(temp);
				temp = NULL;
				radius->conf->r_test_window_auth = NULL;
			}
			free(radius->conf);
			radius->conf = NULL;
		}
		free(radius);
		radius = NULL;
	}
}
int ASD_RADIUS_DEINIT(unsigned char SID){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);
	if(ASD_SECURITY[SID] == NULL)
		return 0;
	struct heart_test_radius_data *radius = ASD_SECURITY[SID]->radius_conf;
	if(radius == NULL)
		return 0;
	else{
		radius_test_client_deinit(radius);
	}
	return 0;

}
int ASD_RADIUS_INIT(unsigned char SID){
	unsigned int ret = 0;
	struct heart_test_radius_data *radius = NULL;
	struct asd_radius_servers *conf = NULL;
	radius = (struct heart_test_radius_data*)os_zalloc(sizeof(struct heart_test_radius_data));	
	if(radius == NULL){
		ret = ASD_DBUS_MALLOC_FAIL;
		return ret;
	}
	conf = (struct asd_radius_servers*)os_zalloc(sizeof(struct asd_radius_servers));
	if(conf == NULL){
		asd_free_radius(radius);
		ret = ASD_DBUS_MALLOC_FAIL;
		return ret;
	}
	radius->conf = conf;
	radius->SecurityID = SID;
	radius->ieee802_1x = 1;	
	if (asd_parse_ip_addr(ASD_SECURITY[SID]->host_ip, &radius->own_ip_addr)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
			  SID,ASD_SECURITY[SID]->host_ip);
	}
	if( os_strncmp(ASD_SECURITY[SID]->host_ip,"127.0.0.1",9) )	{			
		if (asd_parse_ip_addr(ASD_SECURITY[SID]->host_ip, &conf->client_addr) == 0) {
			conf->force_client_addr = 1;
		}else {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",SID,ASD_SECURITY[SID]->host_ip);
			asd_free_radius(radius);
			return ASD_SECURITY_RADIUSINIT_FAIL;
		}
	}
	if(ASD_SECURITY[SID]->auth.auth_ip == NULL) {
		asd_free_radius(radius);
		return ASD_SECURITY_RADIUSINIT_FAIL;
	}
	if (asd_config_read_radius_addr(
			&conf->auth_servers,
			&conf->num_auth_servers, ASD_SECURITY[SID]->auth.auth_ip, 1812,
			&conf->auth_server)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
			   SID, ASD_SECURITY[SID]->auth.auth_ip);
		
	}
	
	conf->auth_server->port = ASD_SECURITY[SID]->auth.auth_port;
	
	int len = os_strlen(ASD_SECURITY[SID]->auth.auth_shared_secret);
	if (len == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
			   "allowed.\n", SID);
	}
	conf->auth_server->shared_secret =
		(u8 *) os_strdup(ASD_SECURITY[SID]->auth.auth_shared_secret);
	conf->auth_server->shared_secret_len = len;
	
	if(ASD_SECURITY[SID]->acct.acct_ip == NULL){
		asd_free_radius(radius);
		return ASD_SECURITY_RADIUSINIT_FAIL;
	}
	if (asd_config_read_radius_addr(
			&conf->acct_servers,
			&conf->num_acct_servers, ASD_SECURITY[SID]->acct.acct_ip, 1813,
			&conf->acct_server)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
			   SID, ASD_SECURITY[SID]->acct.acct_ip);
	}
	conf->acct_server->port = ASD_SECURITY[SID]->acct.acct_port;
	len = os_strlen(ASD_SECURITY[SID]->acct.acct_shared_secret);
	if (len == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
			   "allowed.\n", SID);
	}
	conf->acct_server->shared_secret =
		(u8 *) os_strdup(ASD_SECURITY[SID]->acct.acct_shared_secret);
	conf->acct_server->shared_secret_len = len;

	conf->radius_heart_test_type = ASD_SECURITY[SID]->radius_heart_test_type;
	conf->radius_server_binding_type = ASD_SECURITY[SID]->radius_server_binding_type;
	conf->radius_res_fail_percent = ASD_SECURITY[SID]->radius_res_fail_percent;
	conf->radius_res_suc_percent = ASD_SECURITY[SID]->radius_res_suc_percent;
	conf->radius_access_test_interval = ASD_SECURITY[SID]->radius_access_test_interval;
	conf->radius_server_change_test_timer = ASD_SECURITY[SID]->radius_server_change_test_timer;
	conf->radius_server_reuse_test_timer = ASD_SECURITY[SID]->radius_server_reuse_test_timer;
	if(conf->radius_access_test_interval != 0){
		conf->c_window_size = conf->radius_server_change_test_timer/conf->radius_access_test_interval;
		conf->r_window_size = conf->radius_server_reuse_test_timer/conf->radius_access_test_interval;
	}
	if((conf->c_window_size < TEST_WINDOW_LEAST_SIZE) || (conf->r_window_size < TEST_WINDOW_LEAST_SIZE)){
		conf->radius_server_change_test_timer = RADIUS_SERVER_CHANGE_TIMER;
		conf->radius_access_test_interval = RADIUS_ACCESS_TEST_INTERVAL;
		conf->radius_server_reuse_test_timer = RADIUS_SERVER_REUSE_TIMER;
		conf->c_window_size = conf->radius_server_change_test_timer/conf->radius_access_test_interval;
		conf->r_window_size = conf->radius_server_reuse_test_timer/conf->radius_access_test_interval;
	}
	if(conf->radius_heart_test_type == RADIUS_AUTH_TEST){
		conf->c_test_window_auth = (unsigned char*)os_zalloc(conf->c_window_size);
		conf->r_test_window_auth = (unsigned char*)os_zalloc(conf->r_window_size);
		if((conf->c_test_window_auth == NULL)||(conf->r_test_window_auth== NULL)){
			asd_free_radius(radius);
			return ASD_DBUS_MALLOC_FAIL;
		}
	}
	else if(conf->radius_heart_test_type == RADIUS_ACCT_TEST){
		conf->c_test_window_acct = (unsigned char*)os_zalloc(conf->c_window_size);
		conf->r_test_window_acct = (unsigned char*)os_zalloc(conf->r_window_size);
		if((conf->c_test_window_acct== NULL)||(conf->r_test_window_acct== NULL)){
			asd_free_radius(radius);
			return ASD_DBUS_MALLOC_FAIL;
		}
	}
	else if(conf->radius_heart_test_type == RADIUS_BOTH_TEST){
		conf->c_test_window_auth = (unsigned char*)os_zalloc(conf->c_window_size);
		conf->r_test_window_auth = (unsigned char*)os_zalloc(conf->r_window_size);
		if((conf->c_test_window_auth == NULL)||(conf->r_test_window_auth== NULL)){
			asd_free_radius(radius);
			return ASD_DBUS_MALLOC_FAIL;
		}
		conf->c_test_window_acct = (unsigned char*)os_zalloc(conf->c_window_size);
		conf->r_test_window_acct = (unsigned char*)os_zalloc(conf->r_window_size);
		if((conf->c_test_window_acct== NULL)||(conf->r_test_window_acct== NULL)){
			asd_free_radius(radius);
			return ASD_DBUS_MALLOC_FAIL;
		}
	}
	if(ASD_SECURITY[SID]->auth.secondary_auth_ip != NULL){
		if (asd_config_read_radius_addr(
				&conf->auth_servers,
				&conf->num_auth_servers, ASD_SECURITY[SID]->auth.secondary_auth_ip, 1812,
				&conf->auth_server)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->auth.secondary_auth_ip);
			
		}
		ASD_SECURITY[SID]->auth_server_current_use = RADIUS_MASTER;
		conf->auth_server_current_use = RADIUS_MASTER;
		conf->auth_server->port = ASD_SECURITY[SID]->auth.secondary_auth_port;
		
		int len = os_strlen(ASD_SECURITY[SID]->auth.secondary_auth_shared_secret);
		if (len == 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		conf->auth_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->auth.secondary_auth_shared_secret);
		conf->auth_server->shared_secret_len = len;
	}
	else{
		ASD_SECURITY[SID]->auth_server_current_use = RADIUS_DISABLE;
		conf->auth_server_current_use = RADIUS_DISABLE;
	}
	if(ASD_SECURITY[SID]->acct.secondary_acct_ip != NULL){
		if (asd_config_read_radius_addr(
				&conf->acct_servers,
				&conf->num_acct_servers, ASD_SECURITY[SID]->acct.secondary_acct_ip, 1813,
				&conf->acct_server)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->acct.secondary_acct_ip);
		}
		ASD_SECURITY[SID]->acct_server_current_use = RADIUS_MASTER;
		conf->acct_server_current_use = RADIUS_MASTER;
		conf->acct_server->port = ASD_SECURITY[SID]->acct.secondary_acct_port;
		len = os_strlen(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
		if (len == 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		conf->acct_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
		conf->acct_server->shared_secret_len = len;
	}
	else{
		ASD_SECURITY[SID]->acct_server_current_use = RADIUS_DISABLE;
		conf->acct_server_current_use = RADIUS_DISABLE;//qiuchen
	}
	conf->distribute_off = ASD_SECURITY[SID]->distribute_off;
	conf->auth_server = conf->auth_servers;
	conf->acct_server = conf->acct_servers;
	conf->slot_value = ASD_SECURITY[SID]->slot_value;	//mahz add 2011.10.26
	conf->inst_value = ASD_SECURITY[SID]->inst_value;
	ret = radius_test_client_init(radius);
	if(ret){
		asd_free_radius(radius);
		return ASD_SECURITY_RADIUSINIT_FAIL;
	}
	ASD_SECURITY[SID]->radius_conf = radius;
	asd_printf(ASD_80211,MSG_DEBUG,"radius_res_fail_percent is %f,radius_res_suc_percent is %f,radius_access_test_interval is %d,radius_server_change_test_timer is %d,radius_server_reuse_test_timer is %d,radius_heart_test_type is %d,radius_server_binding_type is %d,auth_server_current_use is %d,acct_server_current_use is %d,c_window_size is %d,r_window_size is %d\n"
		,conf->radius_res_fail_percent,
		conf->radius_res_suc_percent,
		conf->radius_access_test_interval,
		conf->radius_server_change_test_timer,
		conf->radius_server_reuse_test_timer,
		conf->radius_heart_test_type,
		conf->radius_server_binding_type,
		conf->auth_server_current_use,
		conf->acct_server_current_use,
		conf->c_window_size,
		conf->r_window_size
		);
	return 0;
}
void radius_heart_test_on(unsigned char SID){
	unsigned char type = ASD_SECURITY[SID]->radius_heart_test_type;
	struct heart_test_radius_data *wasd = ASD_SECURITY[SID]->radius_conf;
	if(wasd == NULL)
		return;
	/*if(ASD_SECURITY[SID]->auth_server_last_use < RADIUS_DISABLE)
		ASD_SECURITY[SID]->auth_server_current_use = ASD_SECURITY[SID]->auth_server_last_use;
	if(ASD_SECURITY[SID]->acct_server_last_use < RADIUS_DISABLE)
		ASD_SECURITY[SID]->acct_server_current_use = ASD_SECURITY[SID]->acct_server_last_use;
	When this function is stopped to change the heart test parameters as the time as it is using the bak servers,
	during the time it is not restarted,if there are new bss created,these bss init radius server as the master ones.
	So in this situation,when we restart this function,
	it should update the radius server of the bss to make sure it is right!
	if(ASD_SECURITY[SID]->acct_server_current_use == RADIUS_BAK)
		radius_change_server_acct(SID,ASD_SECURITY[SID]->acct_server_current_use);
	if(ASD_SECURITY[SID]->auth_server_current_use == RADIUS_BAK)
		radius_change_server_auth(SID,ASD_SECURITY[SID]->auth_server_current_use);*/

	radius_encapsulate_heart_test_radius(wasd,&type);
}
void radius_heart_test_off(unsigned char SID){
	circle_cancel_timeout(radius_encapsulate_heart_test_radius,ASD_SECURITY[SID]->radius_conf,&ASD_SECURITY[SID]->radius_heart_test_type);
	if(ASD_SECURITY[SID]->radius_heart_test_type == RADIUS_AUTH_TEST)
		circle_cancel_timeout(radius_encapsulate_heart_test_radius,ASD_SECURITY[SID]->radius_conf,&(ASD_SECURITY[SID]->radius_auth));
	else if(ASD_SECURITY[SID]->radius_heart_test_type == RADIUS_ACCT_TEST)
		circle_cancel_timeout(radius_encapsulate_heart_test_radius,ASD_SECURITY[SID]->radius_conf,&(ASD_SECURITY[SID]->radius_acct));
	else if(ASD_SECURITY[SID]->radius_heart_test_type == RADIUS_BOTH_TEST){
		if(ASD_SECURITY[SID]->radius_server_binding_type == RADIUS_SERVER_BINDED)
			circle_cancel_timeout(radius_encapsulate_heart_test_radius,ASD_SECURITY[SID]->radius_conf,&(ASD_SECURITY[SID]->radius_both));
		else{
			circle_cancel_timeout(radius_encapsulate_heart_test_radius,ASD_SECURITY[SID]->radius_conf,&(ASD_SECURITY[SID]->radius_auth));
			circle_cancel_timeout(radius_encapsulate_heart_test_radius,ASD_SECURITY[SID]->radius_conf,&(ASD_SECURITY[SID]->radius_acct));
		}
	}
	ASD_RADIUS_DEINIT(SID);

}
void radius_test_cancel_timers(unsigned SID){
	circle_cancel_timeout((void*)radius_test_client_init_auth,ASD_SECURITY[SID]->radius_conf,NULL);
	circle_cancel_timeout((void*)radius_test_client_init_acct,ASD_SECURITY[SID]->radius_conf,NULL);
}
//end
void ASD_BSS_DEFAULT_INIT(struct asd_data * wasd){
	
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
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	wasd->conf = os_zalloc(sizeof(struct asd_bss_config));
	if(wasd->conf == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	wasd->conf->radius = os_zalloc(sizeof(struct asd_radius_servers));
	if(wasd->conf->radius == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}
	
	asd_config_defaults_bss(wasd->conf,wasd->SecurityID);
	wasd->default_wep_key_idx = 1;
	wasd->iconf->num_bss = 1;
	wasd->iconf->bss = wasd->conf;

	wasd->iconf->beacon_int = 100;
	wasd->iconf->rts_threshold = -1; /* use driver default: 2347 */
	wasd->iconf->fragm_threshold = -1; /* user driver default: 2346 */
	wasd->iconf->send_probe_response = 1;
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

int ASD_BSS_INIT(struct asd_data * wasd){
	unsigned char SID;
	int pairwise = 0;
	unsigned BSSIndex = wasd->BSSIndex;
	struct asd_bss_config *bss;
	if(ASD_WLAN[wasd->WlanID] == NULL)
		return -1;
	
	SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
	wasd->SecurityID = SID;
	ASD_BSS_DEFAULT_INIT(wasd);
	bss = wasd->conf;
	memset(bss->ssid.ssid, 0, asd_MAX_SSID_LEN + 1);
	memcpy(bss->ssid.ssid, ASD_WLAN[wasd->WlanID]->ESSID, strlen(ASD_WLAN[wasd->WlanID]->ESSID));
	bss->ssid.ssid_len = strlen(ASD_WLAN[wasd->WlanID]->ESSID);


	if(SID >= WLAN_NUM){
		asd_printf(ASD_DEFAULT,MSG_INFO,"SecurityID %d error!\n",SID);
		return -1;
	}
		
	if(ASD_SECURITY[SID] == NULL){
		asd_printf(ASD_DEFAULT,MSG_INFO,"ASD_SECURITY[%d] == NULL\n",SID);
		return -1;
	}
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_SECURITY[%d]->securityType = %d\n",SID,ASD_SECURITY[SID]->securityType);
	if((ASD_SECURITY[SID]->securityType < OPEN) || (ASD_SECURITY[SID]->securityType > WAPI_AUTH)){
		asd_printf(ASD_DEFAULT,MSG_INFO,"ASD_SECURITY[%d]->securityType = %d\n",SID,ASD_SECURITY[SID]->securityType);
		return -1;
	}
	//mahz modified
	if((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)||(ASD_SECURITY[SID]->wapi_radius_auth == 1)){
		bss->ieee802_1x = 1;	
		bss->wpa = 0;
		if((ASD_SECURITY[SID]->extensible_auth == 1) || (ASD_SECURITY[SID]->wapi_radius_auth == 1)){
			bss->default_wep_key_len = 0;
			bss->individual_wep_key_len = 0;
		}else{
			bss->default_wep_key_len = 5;
			bss->individual_wep_key_len = 5;
		}
		bss->wep_rekeying_period = 0;
		bss->eapol_key_index_workaround = 0;
		if (asd_parse_ip_addr(ASD_SECURITY[SID]->host_ip, &bss->own_ip_addr)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				  SID,ASD_SECURITY[SID]->host_ip);
		}
		if(ASD_BSS[BSSIndex]->nas_id_len == 0){
			bss->nas_identifier = os_strdup(ASD_SECURITY[SID]->host_ip);
		}else{
			bss->nas_identifier = os_strdup((ASD_BSS[BSSIndex]->nas_id));
		}

		if(ASD_SECURITY[SID]->eap_reauth_priod != 3600){				//ht add,090121
			bss->eap_reauth_period = ASD_SECURITY[SID]->eap_reauth_priod;
		}

		if(ASD_SECURITY[SID]->acct_interim_interval != 0){				//ht add,090205
			bss->radius->acct_interim_interval = ASD_SECURITY[SID]->acct_interim_interval;
		}
		if( os_strncmp(ASD_SECURITY[SID]->host_ip,"127.0.0.1",9) ) 	{			//ht add,081105
			if (asd_parse_ip_addr(ASD_SECURITY[SID]->host_ip, &bss->radius->client_addr) == 0) {
				bss->radius->force_client_addr = 1;
			}else {
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",SID,ASD_SECURITY[SID]->host_ip);
				return -1;
			}
		}
		
		if(ASD_SECURITY[SID]->auth.auth_ip == NULL) {
			return -1;
		}
		if (asd_config_read_radius_addr(
				&bss->radius->auth_servers,
				&bss->radius->num_auth_servers, ASD_SECURITY[SID]->auth.auth_ip, 1812,
				&bss->radius->auth_server)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->auth.auth_ip);
			
		}
		
		bss->radius->auth_server->port = ASD_SECURITY[SID]->auth.auth_port;
		
		int len = os_strlen(ASD_SECURITY[SID]->auth.auth_shared_secret);
		if (len == 0) {
			/* RFC 2865, Ch. 3 */
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
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
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->acct.acct_ip);
		}
		bss->radius->acct_server->port = ASD_SECURITY[SID]->acct.acct_port;
		len = os_strlen(ASD_SECURITY[SID]->acct.acct_shared_secret);
		if (len == 0) {
			/* RFC 2865, Ch. 3 */
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
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
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
					   SID, ASD_SECURITY[SID]->auth.secondary_auth_ip);
				
			}
			
			bss->radius->auth_server->port = ASD_SECURITY[SID]->auth.secondary_auth_port;
			
			int len = os_strlen(ASD_SECURITY[SID]->auth.secondary_auth_shared_secret);
			if (len == 0) {
				/* RFC 2865, Ch. 3 */
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
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
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
					   SID, ASD_SECURITY[SID]->acct.secondary_acct_ip);
			}
			bss->radius->acct_server->port = ASD_SECURITY[SID]->acct.secondary_acct_port;
			len = os_strlen(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
			if (len == 0) {
				/* RFC 2865, Ch. 3 */
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
					   "allowed.\n", SID);
			}
			bss->radius->acct_server->shared_secret =
				(u8 *) os_strdup(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
			bss->radius->acct_server->shared_secret_len = len;
		}
		//qiuchen change it
		if((ASD_SECURITY[SID]->auth_server_current_use == RADIUS_BAK) && bss->radius->num_auth_servers > 1)
			bss->radius->auth_server = &bss->radius->auth_servers[1];
		else
			bss->radius->auth_server = bss->radius->auth_servers;
		if((ASD_SECURITY[SID]->acct_server_current_use == RADIUS_BAK) && bss->radius->num_acct_servers > 1)
			bss->radius->acct_server = &bss->radius->acct_servers[1];
		else	
			bss->radius->acct_server = bss->radius->acct_servers;
		bss->radius->slot_value = ASD_SECURITY[SID]->slot_value;	//mahz add 2011.10.26
		bss->radius->inst_value = ASD_SECURITY[SID]->inst_value;
		if(ASD_SECURITY[SID]->accounting_on_disable){
			bss->radius->accounting_on_disable = 1;
		}		//ht add,090827
		if(ASD_SECURITY[SID]->radius_extend_attr){
			bss->radius->radius_extend_attr = 1;
		}		//ht add,091015
		if(ASD_SECURITY[SID]->distribute_off){
			bss->radius->distribute_off = 1;
		}		//mahz add 2011.10.25
		if(ASD_SECURITY[SID]->securityType == WPA_E){		
			bss->wpa = 1;
			bss->wpa_key_mgmt = WPA_KEY_MGMT_IEEE8021X;
			if(ASD_SECURITY[SID]->encryptionType == TKIP)
				bss->wpa_pairwise = WPA_CIPHER_TKIP;
			else if(ASD_SECURITY[SID]->encryptionType == AES)
				bss->wpa_pairwise = WPA_CIPHER_CCMP;
		}else if(ASD_SECURITY[SID]->securityType == WPA2_E){
			if(ASD_SECURITY[SID]->pre_auth){
				bss->rsn_preauth = 1;
				PreAuth_wlan_bss_add(ASD_WLAN[wasd->WlanID],wasd->own_addr,wasd->BSSIndex);
			}
			bss->wpa = 2;
			bss->wpa_key_mgmt = WPA_KEY_MGMT_IEEE8021X;
			if(ASD_SECURITY[SID]->encryptionType == TKIP)
				bss->wpa_pairwise = WPA_CIPHER_TKIP;
			else if(ASD_SECURITY[SID]->encryptionType == AES)
				bss->wpa_pairwise = WPA_CIPHER_CCMP;
		}
		//mahz add
		else if(ASD_SECURITY[SID]->securityType == WAPI_AUTH){	
			struct asd_wapi *wapi_wasd;
			struct wapid_interfaces *user = NULL;
			int ret;
			wasd->wapi_wasd = os_zalloc(sizeof(struct asd_wapi));
			if(wasd->wapi_wasd == NULL)
				return -1;
			memset(wasd->wapi_wasd, 0, sizeof(struct asd_wapi));
			wasd->wapi_wasd->wasd_bk = wasd;
			wasd->wapi_wasd->cert_info.ap_cert_obj = os_zalloc(sizeof(struct cert_obj_st_t));
			if(wasd->wapi_wasd->cert_info.ap_cert_obj == NULL)
				return -1;
			wapi_wasd = wasd->wapi_wasd;	
			
	//		if(ASD_SECURITY[SID]->securityType == WAPI_AUTH){		
	
			wasd->wapi = 1;
			bss->wapi = 1;
			if(ASD_SECURITY[SID]->wapi_as.multi_cert == 1)
				wapi_wasd->multi_cert = 1;
			X509_cert_init(wapi_wasd);			
			get_wapi_conf(wapi_wasd, 1,SID);

			//mahz add 2010.11.24
			if(ASD_SECURITY[SID]->wapi_radius_auth){
				bss->wapi_radius_auth_enable = 1;
			}
			//mahz add 2010.12.9
			len = os_strlen(ASD_SECURITY[SID]->user_passwd);
					if (len == 0) {
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: user passswd is empty\n", SID);
					}
					bss->user_passwd =
						(u8 *) os_strdup(ASD_SECURITY[SID]->user_passwd);
					bss->user_passwd_len = len;
					
	//		}
						
			wapi_wasd->vap_user = os_zalloc(sizeof(*user));
			user = wapi_wasd->vap_user;
			if (user == NULL) {
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"VAP user allocation failed\n");
				os_free(wasd->wapi_wasd);
				wasd->wapi_wasd=NULL;
				return -1;
			}
			memset(user, 0, sizeof(*user));
			ret = ASD_WAPI_INTERFACE_INIT(user,wasd);
			if(ret < 0){				
				os_free(user);
				user=NULL;
				os_free(wasd->wapi_wasd);
				wasd->wapi_wasd=NULL;
				return -1;
			}
			
		}
		//
		
	}//mahz modified 2011.2.28
	else if((ASD_SECURITY[SID]->securityType == OPEN)&&(ASD_SECURITY[SID]->extensible_auth == 0)&&(ASD_SECURITY[SID]->hybrid_auth == 0)){
		bss->ieee802_1x = 0;	
		bss->wpa = 0;
		bss->ssid.wep.keys_set = 0;
	}else if((ASD_SECURITY[SID]->securityType == SHARED)){
		bss->auth_algs &= ~(WPA_AUTH_ALG_OPEN);
		if(ASD_SECURITY[SID]->extensible_auth == 0){
			bss->ieee802_1x = 0;	
			bss->wpa = 0;
		}
		bss->ssid.wep.keys_set = 1;
		bss->ssid.wep.idx = 1;
		///////////////////////////////////////////////////////////////////////////////////
		if(ASD_SECURITY[SID]->keyInputType==ASCII){
			bss->ssid.wep.len[1] = ASD_SECURITY[SID]->keyLen;
			bss->ssid.wep.key[1] = (unsigned char *)os_zalloc(ASD_SECURITY[SID]->keyLen +1);
			if(bss->ssid.wep.key[1] == NULL)
				return -1;
			memset(bss->ssid.wep.key[1],0, ASD_SECURITY[SID]->keyLen);
			memcpy(bss->ssid.wep.key[1],ASD_SECURITY[SID]->SecurityKey,ASD_SECURITY[SID]->keyLen);
		}
		else if(ASD_SECURITY[SID]->keyInputType==HEX){
			bss->ssid.wep.len[1] = ASD_SECURITY[SID]->keyLen/2;
			bss->ssid.wep.key[1] = (unsigned char *)os_zalloc(ASD_SECURITY[SID]->keyLen/2 +1);
			if(bss->ssid.wep.key[1] == NULL)
				return -1;
			memset(bss->ssid.wep.key[1],0, ASD_SECURITY[SID]->keyLen/2+1);
			hexstr2bin(ASD_SECURITY[SID]->SecurityKey, bss->ssid.wep.key[1], ASD_SECURITY[SID]->keyLen/2);
			////////////////////////////////////////////////////////////////////////////////
		}
	}
	else if(ASD_SECURITY[SID]->securityType == WPA_P){		
		bss->ieee802_1x = 0;	
		bss->wpa = 1;
		bss->wpa_key_mgmt = WPA_KEY_MGMT_PSK;
		if(ASD_SECURITY[SID]->encryptionType == TKIP)
			bss->wpa_pairwise = WPA_CIPHER_TKIP;
		else if(ASD_SECURITY[SID]->encryptionType == AES)
			bss->wpa_pairwise = WPA_CIPHER_CCMP;
		//////////////////////////////////////////////////////////////////////////////////
		if(ASD_SECURITY[SID]->keyInputType==ASCII){
			bss->ssid.wpa_passphrase = (char*)os_zalloc(ASD_SECURITY[SID]->keyLen+1);
			if(bss->ssid.wpa_passphrase == NULL)
				return -1;
			memset(bss->ssid.wpa_passphrase, 0, ASD_SECURITY[SID]->keyLen+1);
			memcpy(bss->ssid.wpa_passphrase, ASD_SECURITY[SID]->SecurityKey, ASD_SECURITY[SID]->keyLen);
		}
		else if(ASD_SECURITY[SID]->keyInputType==HEX){
			os_free(bss->ssid.wpa_psk);
			bss->ssid.wpa_psk=NULL;
			bss->ssid.wpa_psk =os_zalloc(sizeof(struct asd_wpa_psk));
			if(bss->ssid.wpa_psk == NULL){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
				exit(1);
			}	
			hexstr2bin(ASD_SECURITY[SID]->SecurityKey, bss->ssid.wpa_psk->psk,32);
			bss->ssid.wpa_psk->group = 1;
		}
			////////////////////////////////////////////////////////////////////////////////	
	}
	else if(ASD_SECURITY[SID]->securityType == WPA2_P){		
		bss->ieee802_1x = 0;	
		bss->wpa = 2;
		bss->wpa_key_mgmt = WPA_KEY_MGMT_PSK;
		if(ASD_SECURITY[SID]->encryptionType == TKIP)
			bss->wpa_pairwise = WPA_CIPHER_TKIP;
		else if(ASD_SECURITY[SID]->encryptionType == AES)
			bss->wpa_pairwise = WPA_CIPHER_CCMP;
		//////////////////////////////////////////////////////////////////////////////////////////
		if(ASD_SECURITY[SID]->keyInputType==ASCII){
			bss->ssid.wpa_passphrase = (char*)os_zalloc(ASD_SECURITY[SID]->keyLen+1);
			if(bss->ssid.wpa_passphrase == NULL)
				return -1;
			memset(bss->ssid.wpa_passphrase, 0, ASD_SECURITY[SID]->keyLen+1);
			memcpy(bss->ssid.wpa_passphrase, ASD_SECURITY[SID]->SecurityKey, ASD_SECURITY[SID]->keyLen);
		}
		else if(ASD_SECURITY[SID]->keyInputType==HEX){
			os_free(bss->ssid.wpa_psk);
			bss->ssid.wpa_psk=NULL;
			bss->ssid.wpa_psk =os_zalloc(sizeof(struct asd_wpa_psk));
			if(bss->ssid.wpa_psk == NULL){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
				exit(1);
			}	
			hexstr2bin(ASD_SECURITY[SID]->SecurityKey, bss->ssid.wpa_psk->psk,32);
			bss->ssid.wpa_psk->group = 1;
		}/////////////////////////////////////////////////////////////////////////////////////////
	}
	else if((ASD_SECURITY[SID]->securityType == WAPI_AUTH)||(ASD_SECURITY[SID]->securityType == WAPI_PSK)){		
		struct asd_wapi *wapi_wasd;
		struct wapid_interfaces *user = NULL;
		int ret;
		bss->ieee802_1x = 0;	
		bss->wpa = 0;
		wasd->wapi_wasd = os_zalloc(sizeof(struct asd_wapi));
		if(wasd->wapi_wasd == NULL)
			return -1;
		memset(wasd->wapi_wasd, 0, sizeof(struct asd_wapi));
		wasd->wapi_wasd->wasd_bk = wasd;
		wasd->wapi_wasd->cert_info.ap_cert_obj = os_zalloc(sizeof(struct cert_obj_st_t));
		if(wasd->wapi_wasd->cert_info.ap_cert_obj == NULL)
			return -1;
		wapi_wasd = wasd->wapi_wasd;	
		if(ASD_SECURITY[SID]->securityType == WAPI_AUTH){			
			wasd->wapi = 1;
			bss->wapi = 1;
			if(ASD_SECURITY[SID]->wapi_as.multi_cert == 1)
				wapi_wasd->multi_cert = 1;
			X509_cert_init(wapi_wasd);			
			get_wapi_conf(wapi_wasd, 1,SID);
		}
		if(ASD_SECURITY[SID]->securityType == WAPI_PSK){	
			wasd->wapi = 2;			
			bss->wapi = 2;
			none_cert_init(wapi_wasd);
			get_wapi_conf(wapi_wasd, 0,SID);
		}
		wapi_wasd->vap_user = os_zalloc(sizeof(*user));
		user = wapi_wasd->vap_user;
		if (user == NULL) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"VAP user allocation failed\n");
			os_free(wasd->wapi_wasd);
			wasd->wapi_wasd=NULL;
			return -1;
		}
		memset(user, 0, sizeof(*user));
		ret = ASD_WAPI_INTERFACE_INIT(user,wasd);
		if(ret < 0){				
			os_free(user);
			user=NULL;
			os_free(wasd->wapi_wasd);
			wasd->wapi_wasd=NULL;
			return -1;
		}
		
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
int ASD_BSS_SECURITY_INIT(struct asd_data * wasd){

	if (asd_setup_wpa_psk(wasd->conf)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"WPA-PSK setup failed.\n");
		return -1;
	}

	if (wpa_debug_level == MSG_MSGDUMP)
		wasd->conf->radius->msg_dumps = 1;
	//wasd->radius = radius_client_init(wasd, wasd->conf->radius);
/*	if (wasd->radius == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"RADIUS client initialization failed.\n");
		return -1;
	}
*/
/*	if (asd_acl_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"ACL initialization failed.\n");
		return -1;
	}
*/
	if (ieee802_1x_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"IEEE 802.1X initialization failed.\n");
		return -1;
	}

	if (wasd->conf->wpa && asd_setup_wpa(wasd))
		return -1;

/*	if (accounting_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"Accounting initialization failed.\n");
		return -1;
	}*/
	return 0;
}
static void asd_wlan_bss_free(unsigned int bssindex,unsigned char WLANID){
	struct asd_data *wasd = NULL;
	int i = 0;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct PreAuth_BSSINFO *preauth_bss = NULL;
	if(ASD_BSS[bssindex] != NULL){		
		pthread_mutex_lock(&asd_g_sta_mutex); 
		i = ASD_BSS[bssindex]->BSSIndex % L_BSS_NUM;
		if(interfaces->iface[ASD_BSS[bssindex]->Radio_G_ID] != NULL){
			if(interfaces->iface[ASD_BSS[bssindex]->Radio_G_ID]->bss[i] != NULL){
				wasd = interfaces->iface[ASD_BSS[bssindex]->Radio_G_ID]->bss[i];
				if(NULL == wasd)
				{
					asd_printf(ASD_DEFAULT,MSG_ERROR,"wasd bssindex %d is not exit,del error!\n",bssindex);
					pthread_mutex_unlock(&asd_g_sta_mutex); 
					return;
				}
				int wtpid=0;
				wtpid=wasd->Radio_G_ID/L_RADIO_NUM;
				if(ASD_WTP_AP_HISTORY[wtpid] != NULL){
					ASD_WTP_AP_HISTORY[wtpid]->usr_auth_tms += wasd->usr_auth_tms;
					ASD_WTP_AP_HISTORY[wtpid]->ac_rspauth_tms += wasd->ac_rspauth_tms;
					ASD_WTP_AP_HISTORY[wtpid]->auth_fail += wasd->auth_fail;
					ASD_WTP_AP_HISTORY[wtpid]->auth_success += wasd->auth_success;
					ASD_WTP_AP_HISTORY[wtpid]->num_assoc += wasd->num_assoc;
					ASD_WTP_AP_HISTORY[wtpid]->num_reassoc += wasd->num_reassoc;
					ASD_WTP_AP_HISTORY[wtpid]->num_assoc_failure += wasd->num_assoc_failure;
					ASD_WTP_AP_HISTORY[wtpid]->num_reassoc_failure += wasd->num_reassoc_failure;
					ASD_WTP_AP_HISTORY[wtpid]->assoc_success += wasd->assoc_success;
					ASD_WTP_AP_HISTORY[wtpid]->reassoc_success += wasd->reassoc_success;
					ASD_WTP_AP_HISTORY[wtpid]->assoc_req += wasd->assoc_req;
					ASD_WTP_AP_HISTORY[wtpid]->assoc_resp += wasd->assoc_resp;
					ASD_WTP_AP_HISTORY[wtpid]->total_ap_flow_record += ASD_WTP_AP_HISTORY[wtpid]->total_ap_flow;
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"total_assoc_num = %d\n",ASD_WTP_AP_HISTORY[wtpid]->assoc_req);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"total_ap_flow_record = %llu(KB)\n",ASD_WTP_AP_HISTORY[wtpid]->total_ap_flow_record);
				}
				//
				if((ASD_WLAN[WLANID]!=NULL)&&(ASD_WLAN[WLANID]->AC_Roaming_Policy == 1)){
					unsigned char group_id;
					unsigned int j;
					group_id = ASD_WLAN[WLANID]->group_id;
					if(AC_GROUP[group_id] != NULL)
					for(j = 0; j < G_AC_NUM; j++){
						int sock;
						if((AC_GROUP[group_id]->Mobility_AC[j] == NULL)||(AC_GROUP[group_id]->Mobility_AC[j]->is_conn == 0))
							continue;
						sock = AC_GROUP[group_id]->Mobility_AC[j]->sock;
						ac_group_add_del_bss_info(sock,group_id,wasd,G_DEL);
						//close(sock);
					}
					
				}
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(wasd->asd_sta_mutex)); 
#endif
			//	circle_cancel_timeout(assoc_update_timer, wasd, NULL);
				asd_free_stas(wasd);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_free_stas finish\n"); 					
				circle_cancel_timeout((void *)radius_client_init_auth, wasd->radius, (void*)1);
				circle_cancel_timeout((void *)radius_client_init_acct, wasd->radius, (void*)1);
				asd_cleanup(wasd);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_cleanup finish\n");
				
				unsigned char SID = 0;
				if(ASD_WLAN[wasd->WlanID])
					SID = ASD_WLAN[wasd->WlanID]->SecurityID;
				if((ASD_SECURITY[SID] != NULL) && ((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->extensible_auth == 1)))){
					if(ASD_SECURITY[SID]->pre_auth){
						preauth_bss = PreAuth_wlan_get_bss(ASD_WLAN[wasd->WlanID],wasd->own_addr);
						if(preauth_bss != NULL){
							PreAuth_wlan_free_bss(ASD_WLAN[wasd->WlanID],preauth_bss);
						}
					}
				}
				
				asd_free_bss_conf(wasd->conf);
				if((ASD_SECURITY[SID] != NULL) && ((ASD_SECURITY[SID]->securityType == WAPI_AUTH)||(ASD_SECURITY[SID]->securityType == WAPI_PSK))){
					if((wasd->wapi_wasd != NULL)&&(wasd->wapi_wasd->vap_user!=NULL))
						free_one_interface(wasd->wapi_wasd->vap_user);
					if((wasd->wapi_wasd != NULL)){
						free_one_wapi(wasd->wapi_wasd);
					}
				}
				if(wasd->info != NULL) {
					os_free(wasd->info);
					wasd->info = NULL;
				}
				if(wasd->iconf != NULL){
					os_free(wasd->iconf);
					wasd->iconf = NULL;
				}
#ifdef ASD_USE_PERBSS_LOCK
			
				pthread_mutex_unlock(&(wasd->asd_sta_mutex)); 
				CWDestroyThreadMutex(&(wasd->asd_sta_mutex));
#endif
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->iconf) finish\n");
				wasd->driver = NULL;
				wasd->drv_priv = NULL;
				os_free(wasd);
				wasd = NULL;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd) finish\n");
				interfaces->iface[ASD_BSS[bssindex]->Radio_G_ID]->bss[i] = NULL;
				interfaces->iface[ASD_BSS[bssindex]->Radio_G_ID]->num_bss--;
				os_free(ASD_BSS[bssindex]->BSSID);
				ASD_BSS[bssindex]->BSSID = NULL;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(ASD_BSS[%d]->BSSID) finish\n",bssindex);
				ASD_BSS[bssindex]->BSSIndex = 0;
				ASD_BSS[bssindex]->Radio_G_ID = 0;
				ASD_BSS[bssindex]->Radio_L_ID = 0;
				ASD_BSS[bssindex]->WlanID = 0;
				ASD_BSS[bssindex]->State = 0;
				ASD_BSS[bssindex]->bss_accessed_sta_num= 0;
				ASD_BSS[bssindex]->bss_max_allowed_sta_num= 0;
				
				if( ASD_BSS[bssindex]->acl_conf != NULL){	//ht add
					if( ASD_BSS[bssindex]->acl_conf->accept_mac != NULL) {
						free_maclist(ASD_BSS[bssindex]->acl_conf,ASD_BSS[bssindex]->acl_conf->accept_mac);
						ASD_BSS[bssindex]->acl_conf->accept_mac = NULL;
						}
					if( ASD_BSS[bssindex]->acl_conf->deny_mac != NULL) {
						free_maclist(ASD_BSS[bssindex]->acl_conf,ASD_BSS[bssindex]->acl_conf->deny_mac);
						ASD_BSS[bssindex]->acl_conf->deny_mac = NULL;
						}
					os_free(ASD_BSS[bssindex]->acl_conf);
					ASD_BSS[bssindex]->acl_conf = NULL;
				}
				os_free(ASD_BSS[bssindex]);
				ASD_BSS[bssindex] = NULL;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(ASD_BSS[msg->u.BSS.BSSIndex]) finish\n");
				pthread_mutex_unlock(&asd_g_sta_mutex);
				return;


			}
		}
		pthread_mutex_unlock(&asd_g_sta_mutex); 
		return;
	}else
		return;
}
static void asd_wlan_all_bss_free(unsigned char wlanid)
{
	asd_printf(ASD_DEFAULT,MSG_INFO,"*******asd_wlan_all_bss_free**********\n");
	int i=0,j=0;
	unsigned int bssindex = 0;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	if(interfaces == NULL)
		return;
	
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else if(interfaces->iface[i]->bss[j] != NULL){
					if(interfaces->iface[i]->bss[j]->WlanID == wlanid){
						bssindex = interfaces->iface[i]->bss[j]->BSSIndex;
						asd_wlan_bss_free(bssindex,wlanid);
					}
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
	}
}

void WLAN_OP(TableMsg *msg){
	unsigned char SecurityID;
	int i;
	switch(msg->Op){
		case WID_ADD :{
			if(ASD_WLAN[msg->u.WLAN.WlanID] == NULL){
				asd_printf(ASD_DEFAULT,MSG_INFO,"WLAN[%d] setup and init\n",msg->u.WLAN.WlanID);
				ASD_WLAN[msg->u.WLAN.WlanID] = (WID_WLAN*)os_zalloc(sizeof(WID_WLAN));	
				if(ASD_WLAN[msg->u.WLAN.WlanID] == NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}	
				ASD_WLAN[msg->u.WLAN.WlanID]->WlanName = (char*)os_zalloc(strlen(msg->u.WLAN.WlanName)+1);
				if(ASD_WLAN[msg->u.WLAN.WlanID]->WlanName == NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}	
				memset(ASD_WLAN[msg->u.WLAN.WlanID]->WlanName,0,strlen(msg->u.WLAN.WlanName)+1);
				memcpy(ASD_WLAN[msg->u.WLAN.WlanID]->WlanName,msg->u.WLAN.WlanName,strlen(msg->u.WLAN.WlanName));
				ASD_WLAN[msg->u.WLAN.WlanID]->ESSID = (char*)os_zalloc(strlen(msg->u.WLAN.ESSID)+1);	
				if(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID == NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}	
				memset(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID,0,strlen(msg->u.WLAN.ESSID)+1);
				memcpy(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID,msg->u.WLAN.ESSID,strlen(msg->u.WLAN.ESSID));
				ASD_WLAN[msg->u.WLAN.WlanID]->WlanID = msg->u.WLAN.WlanID;
				ASD_WLAN[msg->u.WLAN.WlanID]->SecurityID = 0;
				ASD_WLAN[msg->u.WLAN.WlanID]->Status = msg->u.WLAN.WlanState;
				ASD_WLAN[msg->u.WLAN.WlanID]->wlan_accessed_sta_num=0;
				ASD_WLAN[msg->u.WLAN.WlanID]->wlan_max_allowed_sta_num=msg->u.WLAN.wlan_max_sta_num;
				ASD_WLAN[msg->u.WLAN.WlanID]->Roaming_Policy = msg->u.WLAN.Roaming_policy;
				//weichao add 2011.11.08
				ASD_WLAN[msg->u.WLAN.WlanID]->flow_check = msg->u.WLAN.flow_check;
				ASD_WLAN[msg->u.WLAN.WlanID]->no_flow_time= msg->u.WLAN.no_flow_time;
				ASD_WLAN[msg->u.WLAN.WlanID]->limit_flow= msg->u.WLAN.limit_flow;
				if(msg->u.WLAN.balance_switch == 0){
					ASD_WLAN[msg->u.WLAN.WlanID]->balance_para=msg->u.WLAN.balance_para + 1000;
					ASD_WLAN[msg->u.WLAN.WlanID]->flow_balance_para=msg->u.WLAN.flow_balance_para + 1000;
				}
				else{
					ASD_WLAN[msg->u.WLAN.WlanID]->balance_para=msg->u.WLAN.balance_para;
					ASD_WLAN[msg->u.WLAN.WlanID]->flow_balance_para=msg->u.WLAN.flow_balance_para;
				}
				ASD_WLAN[msg->u.WLAN.WlanID]->balance_switch=msg->u.WLAN.balance_switch;  //  xm add 08/12/29
				ASD_WLAN[msg->u.WLAN.WlanID]->balance_method=msg->u.WLAN.balance_method;
				
				ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf = (struct acl_config *)os_zalloc(sizeof(struct acl_config));
				if(ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf == NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}	
				os_memset(ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf,0,sizeof(struct acl_config));
				if (wids_enable == 1)
					ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->macaddr_acl = 1;
				else
					ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->macaddr_acl = 0;
				ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->accept_mac = NULL;
				ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->num_accept_mac = 0;
				ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->deny_mac = NULL;
				ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->num_deny_mac = 0;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"wlan max sta num %d\n",ASD_WLAN[msg->u.WLAN.WlanID]->wlan_max_allowed_sta_num);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"wlan index %d\n",msg->u.WLAN.WlanID);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"SecurityID %d\n",ASD_WLAN[msg->u.WLAN.WlanID]->SecurityID);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"wlanid %d,wlanname %s,essid %s\n",ASD_WLAN[msg->u.WLAN.WlanID]->WlanID,ASD_WLAN[msg->u.WLAN.WlanID]->WlanName,ASD_WLAN[msg->u.WLAN.WlanID]->ESSID);
				ASD_WLAN[msg->u.WLAN.WlanID]->sta_from_which_bss_list=NULL; //xm add 08/12/17
				ASD_WLAN[msg->u.WLAN.WlanID]->sta_list_len=0;
				ASD_WLAN[msg->u.WLAN.WlanID]->sta_list = NULL;
				ASD_WLAN[msg->u.WLAN.WlanID]->num_sta = 0;
				ASD_WLAN[msg->u.WLAN.WlanID]->OldSecurityIndex = 0;         //fengwenchao add 20110310
				ASD_WLAN[msg->u.WLAN.WlanID]->OldSecurityIndex_flag = 0;    //fengwenchao add 20110310
				ASD_WLAN[msg->u.WLAN.WlanID]->NowSecurityIndex_flag = 0;    //fengwenchao add 20110310
				for(i=0; i<255; i++)
					ASD_WLAN[msg->u.WLAN.WlanID]->sta_hash[i] = NULL;
				ASD_WLAN[msg->u.WLAN.WlanID]->bss_list = NULL;
				for(i=0; i<255; i++)
					ASD_WLAN[msg->u.WLAN.WlanID]->bss_hash[i] = NULL;
				ASD_WLAN[msg->u.WLAN.WlanID]->num_bss = 0;
				ASD_WLAN[msg->u.WLAN.WlanID]->PreAuth = 0;
				ASD_WLAN[msg->u.WLAN.WlanID]->r_sta_list = NULL;
				ASD_WLAN[msg->u.WLAN.WlanID]->r_num_sta = 0;
				for(i=0; i<255; i++)
					ASD_WLAN[msg->u.WLAN.WlanID]->r_sta_hash[i] = NULL;
				ASD_WLAN[msg->u.WLAN.WlanID]->a_sta_list = NULL;	//weichao modify 20110802
				ASD_WLAN[msg->u.WLAN.WlanID]->a_num_sta = 0;
				for(i=0; i<255; i++)
					ASD_WLAN[msg->u.WLAN.WlanID]->a_sta_hash[i] = NULL;
			
			}
			break;

		}
		case WID_DEL :{
			if(ASD_WLAN[msg->u.WLAN.WlanID] != NULL){
				SecurityID = ASD_WLAN[msg->u.WLAN.WlanID]->SecurityID;
				if((SecurityID > 0) && (SecurityID < WLAN_NUM) &&(ASD_SECURITY[SecurityID] != NULL)){
					if((ASD_SECURITY[SecurityID]->securityType == WPA2_E)||(ASD_SECURITY[SecurityID]->securityType == WPA2_P))
						pmk_wlan_del_all_sta(msg->u.WLAN.WlanID);
					if(ASD_SECURITY[SecurityID]->pre_auth == 1)
						PreAuth_wlan_del_all_bss(msg->u.WLAN.WlanID);
				}
				ASD_WLAN[msg->u.WLAN.WlanID]->SecurityID = 0;
				ASD_WLAN[msg->u.WLAN.WlanID]->WlanID = 0;	
				ASD_WLAN[msg->u.WLAN.WlanID]->wlan_accessed_sta_num=0;
				ASD_WLAN[msg->u.WLAN.WlanID]->wlan_max_allowed_sta_num=0;
				ASD_WLAN[msg->u.WLAN.WlanID]->balance_para=1;
				ASD_WLAN[msg->u.WLAN.WlanID]->flow_balance_para=1;
				ASD_WLAN[msg->u.WLAN.WlanID]->balance_switch=0;
				ASD_WLAN[msg->u.WLAN.WlanID]->balance_method=0;
				ASD_WLAN[msg->u.WLAN.WlanID]->OldSecurityIndex = 0;         //fengwenchao add 20110310
				ASD_WLAN[msg->u.WLAN.WlanID]->OldSecurityIndex_flag = 0;    //fengwenchao add 20110310
				ASD_WLAN[msg->u.WLAN.WlanID]->NowSecurityIndex_flag = 0;    //fengwenchao add 20110310
				//weichao add 2011.11.08
				ASD_WLAN[msg->u.WLAN.WlanID]->flow_check = msg->u.WLAN.flow_check;
				ASD_WLAN[msg->u.WLAN.WlanID]->no_flow_time= msg->u.WLAN.no_flow_time;
				ASD_WLAN[msg->u.WLAN.WlanID]->limit_flow= msg->u.WLAN.limit_flow;
				if(ASD_WLAN[msg->u.WLAN.WlanID]->Roaming_Policy != 0){
					roaming_del_all_sta(msg->u.WLAN.WlanID);
					ASD_WLAN[msg->u.WLAN.WlanID]->Roaming_Policy=0;
				}
				memset(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID,0,strlen(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID)+1);
				os_free(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID);
				ASD_WLAN[msg->u.WLAN.WlanID]->ESSID = NULL;
				memset(ASD_WLAN[msg->u.WLAN.WlanID]->WlanName,0,strlen(ASD_WLAN[msg->u.WLAN.WlanID]->WlanName)+1);
				os_free(ASD_WLAN[msg->u.WLAN.WlanID]->WlanName);
				ASD_WLAN[msg->u.WLAN.WlanID]->WlanName = NULL;

				free_balance_info_all_in_wlan(msg->u.WLAN.WlanID);
				wlan_free_stas(msg->u.WLAN.WlanID);            //weichao add
				circle_cancel_timeout(clear_prob_log_timer, ASD_WLAN[msg->u.WLAN.WlanID], NULL);
				
				if( ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf != NULL){			//ht add
					if( ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->accept_mac != NULL) {
						free_maclist(ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf,ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->accept_mac);
						ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->accept_mac = NULL;
					}
					if( ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->deny_mac != NULL) {
						free_maclist(ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf,ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->deny_mac);
						ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf->deny_mac = NULL;
					}
					os_free(ASD_WLAN [msg->u.WLAN.WlanID]->acl_conf);
					ASD_WLAN[msg->u.WLAN.WlanID]->acl_conf = NULL;
				}
				asd_wlan_radius_free(msg->u.WLAN.WlanID);
				os_free(ASD_WLAN[msg->u.WLAN.WlanID]);
				ASD_WLAN[msg->u.WLAN.WlanID] = NULL;
			}else
				return;

			
			signal_de_key_conflict();
			break;

		}
		case WID_MODIFY :{
			if(ASD_WLAN[msg->u.WLAN.WlanID] != NULL){
				//Qc
				int ret = 0;
				if(ASD_WLAN[msg->u.WLAN.WlanID]->Status == 0 && msg->u.WLAN.WlanState == 1){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"*******1111WID_MODIFY**********\n");
					asd_wlan_all_bss_free(msg->u.WLAN.WlanID);
					asd_wlan_radius_free(msg->u.WLAN.WlanID);
				}
				else if((ASD_WLAN[msg->u.WLAN.WlanID]->Status == 1 && msg->u.WLAN.WlanState == 0)){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"*******2222WID_MODIFY**********\n");
					SecurityID = ASD_WLAN[msg->u.WLAN.WlanID]->SecurityID;
					ret = asd_wlan_radius_init(msg->u.WLAN.WlanID,SecurityID);
					if(ret){
						asd_printf(ASD_DEFAULT,MSG_CRIT,"asd_wlan_radius_init failed! ret %d\n",ret);
						return;
					}
				}
				//end
				ASD_WLAN[msg->u.WLAN.WlanID]->Status = msg->u.WLAN.WlanState;
				ASD_WLAN[msg->u.WLAN.WlanID]->wlan_max_allowed_sta_num=msg->u.WLAN.wlan_max_sta_num;				
				if(msg->u.WLAN.balance_switch == 0){
					ASD_WLAN[msg->u.WLAN.WlanID]->balance_para=msg->u.WLAN.balance_para + 1000;
					ASD_WLAN[msg->u.WLAN.WlanID]->flow_balance_para=msg->u.WLAN.flow_balance_para + 1000;
				}
				else{
					ASD_WLAN[msg->u.WLAN.WlanID]->balance_para=msg->u.WLAN.balance_para;
					ASD_WLAN[msg->u.WLAN.WlanID]->flow_balance_para=msg->u.WLAN.flow_balance_para;
				}
				ASD_WLAN[msg->u.WLAN.WlanID]->balance_switch=msg->u.WLAN.balance_switch;
				ASD_WLAN[msg->u.WLAN.WlanID]->balance_method=msg->u.WLAN.balance_method;

				asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_WLAN[msg->u.WLAN.WlanID]->balance_switch=%u\n",ASD_WLAN[msg->u.WLAN.WlanID]->balance_switch);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_WLAN[msg->u.WLAN.WlanID]->balance_method=%u\n",ASD_WLAN[msg->u.WLAN.WlanID]->balance_method);
				
				if(ASD_WLAN[msg->u.WLAN.WlanID]->balance_switch){
					circle_cancel_timeout(clear_prob_log_timer, ASD_WLAN[msg->u.WLAN.WlanID], NULL);
					circle_register_timeout(CLEAR_PROB_INTERVAL, 0, clear_prob_log_timer, ASD_WLAN[msg->u.WLAN.WlanID], NULL);
				}
				
				if((ASD_WLAN[msg->u.WLAN.WlanID]->Roaming_Policy != 0)&&(msg->u.WLAN.Roaming_policy == 0)){
					roaming_del_all_sta(msg->u.WLAN.WlanID);
				}
				ASD_WLAN[msg->u.WLAN.WlanID]->Roaming_Policy=msg->u.WLAN.Roaming_policy;

				//weichao add 2011.11.08
				ASD_WLAN[msg->u.WLAN.WlanID]->flow_check = msg->u.WLAN.flow_check;
				ASD_WLAN[msg->u.WLAN.WlanID]->no_flow_time= msg->u.WLAN.no_flow_time;
				ASD_WLAN[msg->u.WLAN.WlanID]->limit_flow= msg->u.WLAN.limit_flow;


				/*xm for change essid*/
				if(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID!=NULL
					&&(0!=strcmp(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID,msg->u.WLAN.ESSID))
				){
				
					os_free(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID);
					ASD_WLAN[msg->u.WLAN.WlanID]->ESSID = NULL;
					ASD_WLAN[msg->u.WLAN.WlanID]->ESSID = (char*)os_zalloc(strlen(msg->u.WLAN.ESSID)+1);	
					if(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID == NULL){
						asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
						exit(1);
					}	
					memset(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID,0,strlen(msg->u.WLAN.ESSID)+1);
					memcpy(ASD_WLAN[msg->u.WLAN.WlanID]->ESSID,msg->u.WLAN.ESSID,strlen(msg->u.WLAN.ESSID));
				}
				break;
			}else
				return;
			break;
		}
		default :
			return;
	}
	return;
}

//xm add 08/12/04
void WTP_OP(TableMsg *msg){
	switch(msg->Op){
		case WID_ADD :{
			if(ASD_WTP_AP[msg->u.WTP.WtpID] == NULL){
				if((msg->u.WTP.WtpID)>WTP_NUM )		//mahz add 2011.4.7
					return;
				pthread_mutex_lock(&asd_g_wtp_mutex);		//mahz add 2011.3.24
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"WTP[%d] setup and init\n",msg->u.WTP.WtpID);
				ASD_WTP_AP[msg->u.WTP.WtpID]=(ASD_WTP_ST *)os_zalloc(sizeof(ASD_WTP_ST ));
				if(ASD_WTP_AP[msg->u.WTP.WtpID] == NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}	
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID],0,sizeof(ASD_WTP_ST));
				
				ASD_WTP_AP[msg->u.WTP.WtpID]->WTPID=msg->u.WTP.WtpID;

				//mahz add 2011.4.30
				if(ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID] == NULL){
					ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID]=(ASD_WTP_ST_HISTORY *)os_zalloc(sizeof(ASD_WTP_ST_HISTORY));
					if(ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID] == NULL){
						asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
						exit(1);
					}	
					ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID]->WTPID=msg->u.WTP.WtpID;
				}
				//
				
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf = (struct acl_config *)os_zalloc(sizeof(struct acl_config));
				if(ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf == NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}	
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf,0,sizeof(struct acl_config));
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->macaddr_acl = 0;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->accept_mac = NULL;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->num_accept_mac = 0;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->deny_mac = NULL;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->num_deny_mac = 0;
				
				/* zhangshu add to initial netid,2010-10-26 */
            	os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->NETID, 0, WTP_NETID_LEN);
            	os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->NETID, "defaultcode", 11);
            	/* end */
            	
				struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM];
				unsigned int num = 0;
				unsigned int sta_num=0;
				int i=0;
				num= ASD_SEARCH_WTP_HAS_STA(msg->u.WTP.WtpID, bss);
	
				for(i=0;i<num;i++)
					sta_num+=bss[i]->num_sta;

				for(i=0;i<L_RADIO_NUM;i++)
					ASD_WTP_AP[msg->u.WTP.WtpID]->ra_ch[i]=0;
				
				ASD_WTP_AP[msg->u.WTP.WtpID]->ap_accessed_sta_num=sta_num;////////////
				
				ASD_WTP_AP[msg->u.WTP.WtpID]->ap_max_allowed_sta_num=msg->u.WTP.wtp_max_sta_num;
				ASD_WTP_AP[msg->u.WTP.WtpID]->wtp_triger_num=msg->u.WTP.wtp_triger_num;
				ASD_WTP_AP[msg->u.WTP.WtpID]->wtp_flow_triger=msg->u.WTP.wtp_flow_triger;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"wtp max sta %d\nmsg->max sta %d\n",ASD_WTP_AP[msg->u.WTP.WtpID]->ap_max_allowed_sta_num,msg->u.WTP.wtp_max_sta_num);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"ap accessed sta %d---%d\n",ASD_WTP_AP[msg->u.WTP.WtpID]->ap_accessed_sta_num,sta_num);

				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC,0,MAC_LEN);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC,msg->u.WTP.WTPMAC,MAC_LEN);
				ASD_WTP_AP[msg->u.WTP.WtpID]->WTPIP = msg->u.WTP.WTPIP.u.ipv4_addr;
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN,0,128);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN,msg->u.WTP.WTPSN,128);

				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME,0,256);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME,msg->u.WTP.WTPNAME,256);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp mac: "MACSTR"\n",__func__,MAC2STR(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC));
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp ip: %d\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPIP);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp sn: %s\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp name: %s\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME);
				for(i=0;i<L_RADIO_NUM;i++){
					ASD_WTP_AP[msg->u.WTP.WtpID]->radio_flow_info[i].tx_bytes=0;
					ASD_WTP_AP[msg->u.WTP.WtpID]->radio_flow_info[i].old_tx_bytes=0;
					ASD_WTP_AP[msg->u.WTP.WtpID]->radio_flow_info[i].trans_rates=0;
				}
				pthread_mutex_unlock(&asd_g_wtp_mutex);		//mahz add 2011.3.24

				return;
			}else
				return;
			break;
		}
		case WID_DEL :{
			if(ASD_WTP_AP[msg->u.WTP.WtpID] != NULL){
				pthread_mutex_lock(&asd_g_wtp_mutex);		//mahz add 2011.3.24
				if( ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf != NULL){	//ht add
					if( ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->accept_mac != NULL) {
						free_maclist(ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf,ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->accept_mac);
						ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->accept_mac = NULL;
						}
					if( ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->deny_mac != NULL) {
						free_maclist(ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf,ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->deny_mac);
						ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->deny_mac = NULL;
						}
					os_free(ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf);
					ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf = NULL;
				}
				os_free(ASD_WTP_AP[msg->u.WTP.WtpID]);
				ASD_WTP_AP[msg->u.WTP.WtpID] = NULL;

				//mahz add 2011.5.5
				if(ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID] != NULL){
					os_free(ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID]);
					ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID] = NULL;
				}//				
				pthread_mutex_unlock(&asd_g_wtp_mutex);		//mahz add 2011.3.24
				return;
			}else
				return;
			break;

		}
		case WID_MODIFY :{
			pthread_mutex_lock(&asd_g_wtp_mutex);		//mahz add 2011.3.24
			if(ASD_WTP_AP[msg->u.WTP.WtpID] != NULL){
				ASD_WTP_AP[msg->u.WTP.WtpID]->ap_max_allowed_sta_num=msg->u.WTP.wtp_max_sta_num;
				ASD_WTP_AP[msg->u.WTP.WtpID]->wtp_triger_num=msg->u.WTP.wtp_triger_num;
				ASD_WTP_AP[msg->u.WTP.WtpID]->wtp_flow_triger=msg->u.WTP.wtp_flow_triger;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"\t\t\t2wtp max sta %d\n\t\t\tmsg->max sta %d\n",ASD_WTP_AP[msg->u.WTP.WtpID]->ap_max_allowed_sta_num,msg->u.WTP.wtp_max_sta_num);
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->Binding_IF_NAME,0,ETH_IF_NAME_LEN);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->Binding_IF_NAME,msg->u.WTP.BindingIFName,ETH_IF_NAME_LEN);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"WTP[%d]->Binding_IF_NAME = %s\n",msg->u.WTP.WtpID,ASD_WTP_AP[msg->u.WTP.WtpID]->Binding_IF_NAME);
				
                //zhangshu modify for netid, 2010-10-26
                os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->NETID, 0, WTP_NETID_LEN);
                os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->NETID, msg->u.WTP.NETID, WTP_NETID_LEN);
                //printf("ASD_WTP_AP[msg->u.WTP.WtpID]->NETID = %s\n",ASD_WTP_AP[msg->u.WTP.WtpID]->NETID);
                asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:ASD_WTP_AP[msg->u.WTP.WtpID]->NETID = %s\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->NETID);
                
				
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC,0,MAC_LEN);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC,msg->u.WTP.WTPMAC,MAC_LEN);
				ASD_WTP_AP[msg->u.WTP.WtpID]->WTPIP = msg->u.WTP.WTPIP.u.ipv4_addr;
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN,0,128);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN,msg->u.WTP.WTPSN,128);

				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME,0,256);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME,msg->u.WTP.WTPNAME,256);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp mac: "MACSTR"\n",__func__,MAC2STR(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC));
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp ip: %d\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPIP);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp sn: %s\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp name: %s\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME);
				pthread_mutex_unlock(&asd_g_wtp_mutex);		//mahz add 2011.3.24
				return;
			}
			else if(ASD_WTP_AP[msg->u.WTP.WtpID] == NULL){
				if((msg->u.WTP.WtpID)>WTP_NUM || (msg->u.WTP.WtpID)<0){		//mahz add 2011.4.7
					pthread_mutex_unlock(&asd_g_wtp_mutex);
					return;
				}
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"WTP[%d] setup and init\n",msg->u.WTP.WtpID);
				
				ASD_WTP_AP[msg->u.WTP.WtpID]=(ASD_WTP_ST *)os_zalloc(sizeof(ASD_WTP_ST ));
				if(ASD_WTP_AP[msg->u.WTP.WtpID] == NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}	
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID],0,sizeof(ASD_WTP_ST));
				
				ASD_WTP_AP[msg->u.WTP.WtpID]->WTPID=msg->u.WTP.WtpID;
				
				//mahz add 2011.4.30
				if(ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID] == NULL){
					ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID]=(ASD_WTP_ST_HISTORY *)os_zalloc(sizeof(ASD_WTP_ST_HISTORY));
					if(ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID] == NULL){
						asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
						exit(1);
					}	
					ASD_WTP_AP_HISTORY[msg->u.WTP.WtpID]->WTPID=msg->u.WTP.WtpID;
				}
				//

				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf = (struct acl_config *)os_zalloc(sizeof(struct acl_config));
				if(ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf == NULL) {
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}				
				
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf,0,sizeof(struct acl_config));
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->macaddr_acl = 0;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->accept_mac = NULL;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->num_accept_mac = 0;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->deny_mac = NULL;
				ASD_WTP_AP[msg->u.WTP.WtpID]->acl_conf->num_deny_mac = 0;
				struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM];
				unsigned int num = 0;
				unsigned int sta_num=0;
				int i=0;
				num= ASD_SEARCH_WTP_HAS_STA(msg->u.WTP.WtpID, bss);
	
				for(i=0;i<num;i++)
					sta_num+=bss[i]->num_sta;
				
				ASD_WTP_AP[msg->u.WTP.WtpID]->ap_accessed_sta_num=sta_num;////////////
				
				ASD_WTP_AP[msg->u.WTP.WtpID]->ap_max_allowed_sta_num=msg->u.WTP.wtp_max_sta_num;
				ASD_WTP_AP[msg->u.WTP.WtpID]->wtp_triger_num=msg->u.WTP.wtp_triger_num;
				ASD_WTP_AP[msg->u.WTP.WtpID]->wtp_flow_triger=msg->u.WTP.wtp_flow_triger;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"wtp max sta %d\nmsg->max sta %d\n",ASD_WTP_AP[msg->u.WTP.WtpID]->ap_max_allowed_sta_num,msg->u.WTP.wtp_max_sta_num);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"ap accessed sta %d---%d\n",ASD_WTP_AP[msg->u.WTP.WtpID]->ap_accessed_sta_num,sta_num);

				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC,0,MAC_LEN);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC,msg->u.WTP.WTPMAC,MAC_LEN);
				ASD_WTP_AP[msg->u.WTP.WtpID]->WTPIP = msg->u.WTP.WTPIP.u.ipv4_addr;
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN,0,128);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN,msg->u.WTP.WTPSN,128);
				os_memset(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME,0,256);
				os_memcpy(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME,msg->u.WTP.WTPNAME,256);
				
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp mac: "MACSTR"\n",__func__,MAC2STR(ASD_WTP_AP[msg->u.WTP.WtpID]->WTPMAC));
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp ip: %d\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPIP);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp sn: %s\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPSN);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s:wtp name: %s\n",__func__,ASD_WTP_AP[msg->u.WTP.WtpID]->WTPNAME);

				pthread_mutex_unlock(&asd_g_wtp_mutex);		//mahz add 2011.3.24
				return;
			}
			else{
				pthread_mutex_unlock(&asd_g_wtp_mutex);		//mahz add 2011.3.24
				return;
			}
			break;
		}
		case RADIO_INFO :{
			if(ASD_WTP_AP[msg->u.RadioFlow.wtpid] != NULL){
				int ii;
				for(ii=0;ii<L_RADIO_NUM;ii++){
					ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].old_tx_bytes = ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].tx_bytes;
					ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].tx_bytes = msg->u.RadioFlow.radio_flow[ii].tx_bytes;
					ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].trans_rates = (ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].tx_bytes-ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].old_tx_bytes)*8/(1024*30);
					if(msg->u.RadioFlow.radio_flow[ii].tx_bytes != 0){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"wtp[%d]->radio[%d] \n",msg->u.RadioFlow.wtpid,ii);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"old_tx_bytes = %u (byte)\n",ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].old_tx_bytes);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"new_tx_bytes = %u (byte)\n",ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].tx_bytes);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"trans_rates  = %u (kbps)\n",ASD_WTP_AP[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].trans_rates);
					}
					//mahz add 2011.4.30
					if(ASD_WTP_AP_HISTORY[msg->u.RadioFlow.wtpid] != NULL){
						if(msg->u.RadioFlow.radio_flow[ii].tx_bytes != 0){
							ASD_WTP_AP_HISTORY[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].tx_bytes = msg->u.RadioFlow.radio_flow[ii].tx_bytes;
							ASD_WTP_AP_HISTORY[msg->u.RadioFlow.wtpid]->total_ap_flow = 0;
						}
					}
				}
				
				for(ii=0;ii<L_RADIO_NUM;ii++){
					ASD_WTP_AP_HISTORY[msg->u.RadioFlow.wtpid]->total_ap_flow += ASD_WTP_AP_HISTORY[msg->u.RadioFlow.wtpid]->radio_flow_info[ii].tx_bytes;
				}
				ASD_WTP_AP_HISTORY[msg->u.RadioFlow.wtpid]->total_ap_flow /= 1024;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"total_ap_flow = %llu (KB)\n",ASD_WTP_AP_HISTORY[msg->u.RadioFlow.wtpid]->total_ap_flow);
			}
			break;
		}
		case CHANNEL_CHANGE_INFO :{ //    xm add 09.5.13
			if((ASD_WTP_AP[msg->u.WTP_chchange.WTPID] != NULL)
				&&(msg->u.WTP_chchange.radioid<=L_RADIO_NUM-1)
				&&(msg->u.WTP_chchange.channel<=13)
				){
				
				ASD_WTP_AP[msg->u.WTP_chchange.WTPID]->ra_ch[msg->u.WTP_chchange.radioid]=msg->u.WTP_chchange.channel;
			}
			break;
		}	
		case WID_WIFI_INFO:{
			if(ASD_WTP_AP[msg->u.WTP.WtpID] != NULL){
				memset(&(ASD_WTP_AP[msg->u.WTP.WtpID]->wifi_extension_info),0,sizeof(wid_wifi_info));
				memcpy(&(ASD_WTP_AP[msg->u.WTP.WtpID]->wifi_extension_info),&(msg->u.WTP.wifi_extension_info),sizeof(wid_wifi_info));
			}
			break;
		}
		default :
			return;
	}
	return;
}

/*
void RADIO_OP(WIDOperate type, WID_WTP_RADIO *RADIO){
	switch(type){
		case WID_ADD :{
			if(ASD_RADIO[RADIO->Radio_G_ID] == NULL){
				return;
			}else
				return;
			break;

		}
		case WID_DEL :{
			if(ASD_RADIO[RADIO->Radio_G_ID] != NULL){
				return;
			}else
				return;
			break;

		}
		case WID_MODIFY :{
			if(ASD_RADIO[RADIO->Radio_G_ID] != NULL){
				return;
			}else
				return;
			break;
		}
		default :
			return;
	}
	return;
}
*/
void BSS_OP(TableMsg *msg, struct wasd_interfaces *interfaces){
	struct asd_data *wasd;
	struct PreAuth_BSSINFO *preauth_bss;
	unsigned char WLANID;
	WLANID = msg->u.BSS.WlanID;
	int i = 0 ,j = 0,len = 0;
	unsigned int g_radio_id = 0;
	if((msg->Op == WID_ADD) || (msg->Op == WID_DEL) || (msg->Op == WID_MODIFY))
		asd_printf(ASD_DEFAULT,MSG_INFO,"BSS_OP type %u(ADD--0,DEL--1,MODIFY--2),BSSIndex %u\n",msg->Op,msg->u.BSS.BSSIndex);
	switch(msg->Op){
		case WID_ADD :{
			if(ASD_WLAN[WLANID] && ASD_WLAN[WLANID]->Status == 1){
				return;
			}
			if(ASD_BSS[msg->u.BSS.BSSIndex] == NULL){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_BSS[%u] = NULL.\n",msg->u.BSS.BSSIndex);
				
				ASD_BSS[msg->u.BSS.BSSIndex] = (WID_BSS*)os_zalloc(sizeof(WID_BSS));
				if(ASD_BSS[msg->u.BSS.BSSIndex] == NULL) {
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}
				
				ASD_BSS[msg->u.BSS.BSSIndex]->BSSID = (unsigned char*)os_zalloc(7);
				if(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID==NULL){
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}
				memset(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID,0,7);
				memcpy(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID,msg->u.BSS.BSSID,6);
				
				//mahz add 2011.5.26
				len = os_strlen(msg->u.BSS.nas_port_id);
				if(len > 0){
					memset(ASD_BSS[msg->u.BSS.BSSIndex]->nas_port_id,0,sizeof(ASD_BSS[msg->u.BSS.BSSIndex]->nas_port_id));
					memcpy(ASD_BSS[msg->u.BSS.BSSIndex]->nas_port_id,msg->u.BSS.nas_port_id,len);
				}
				asd_printf(ASD_DBUS,MSG_DEBUG,"%s :nas_port_id %s\n",__func__,msg->u.BSS.nas_port_id);//for test

				ASD_BSS[msg->u.BSS.BSSIndex]->BSSIndex = msg->u.BSS.BSSIndex;
				ASD_BSS[msg->u.BSS.BSSIndex]->WlanID = msg->u.BSS.WlanID;
				ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID = msg->u.BSS.Radio_G_ID;
				ASD_BSS[msg->u.BSS.BSSIndex]->Radio_L_ID = msg->u.BSS.Radio_L_ID;
				ASD_BSS[msg->u.BSS.BSSIndex]->bss_accessed_sta_num=0;
				ASD_BSS[msg->u.BSS.BSSIndex]->bss_max_allowed_sta_num=msg->u.BSS.bss_max_sta_num; //zhanglei change 64 to msg->u.BSS.bss_max_sta_num
				ASD_BSS[msg->u.BSS.BSSIndex]->vlanid = msg->u.BSS.vlanid;				
				ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf = (struct acl_config *)os_zalloc(sizeof(struct acl_config));
				if(ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf == NULL) {
					asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
					exit(1);
				}
				os_memset(ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf,0,sizeof(struct acl_config));
				ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->macaddr_acl = 0;
				ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->accept_mac = NULL;
				ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->num_accept_mac = 0;
				ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->deny_mac = NULL;
				ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->num_deny_mac = 0;
				os_memset(ASD_BSS[msg->u.BSS.BSSIndex]->nas_id,0,NAS_IDENTIFIER_NAME);
				if(msg->u.BSS.nas_id_len == 0){
					ASD_BSS[msg->u.BSS.BSSIndex]->nas_id_len = 0;
				}else{
					ASD_BSS[msg->u.BSS.BSSIndex]->nas_id_len = msg->u.BSS.nas_id_len;
					memcpy(ASD_BSS[msg->u.BSS.BSSIndex]->nas_id,msg->u.BSS.nas_id,NAS_IDENTIFIER_NAME);
				}
				ASD_BSS[msg->u.BSS.BSSIndex]->sta_static_arp_policy = msg->u.BSS.sta_static_arp_policy;
				memcpy(ASD_BSS[msg->u.BSS.BSSIndex]->arp_ifname, msg->u.BSS.arp_ifname,ETH_IF_NAME_LEN);
				asd_printf(ASD_DEFAULT,MSG_INFO,"%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x\n",ASD_BSS[msg->u.BSS.BSSIndex]->BSSIndex,ASD_BSS[msg->u.BSS.BSSIndex]->WlanID,ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID,ASD_BSS[msg->u.BSS.BSSIndex]->Radio_L_ID,
					ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[0],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[1],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[2],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[3],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[4],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[5]);
				if((msg->u.BSS.BSSIndex / L_BSS_NUM) == msg->u.BSS.Radio_G_ID){
					i = msg->u.BSS.BSSIndex % L_BSS_NUM;
					g_radio_id = msg->u.BSS.Radio_G_ID;
					for(j=g_radio_id%4; j>=0; j--){
						unsigned int radio_id = g_radio_id - j;
						asd_printf(ASD_DEFAULT,MSG_INFO,"interfaces->iface[%d] setup and init\n",radio_id);
						if(interfaces->iface[radio_id] == NULL)
							asd_iface_malloc(radio_id);
					}
					if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID] == NULL){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"interfaces->iface[%d] setup and init\n",ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID);
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID] = (struct asd_iface*)os_zalloc(sizeof(struct asd_iface));
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss = NULL;
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->interfaces = interfaces;
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss = 0;
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss = os_zalloc(L_BSS_NUM * sizeof(struct asd_data*));
						if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss == NULL){
							asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
							exit(1);
						}	
						int bssnum = 0;
						for(bssnum = 0; bssnum < L_BSS_NUM; bssnum++)
							interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[bssnum] = NULL;
						if(asd_hw_feature_init(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]) < 0)
							asd_printf(ASD_DEFAULT,MSG_WARNING,"asd_hw_feature_init failed\n");
					}
					if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] == NULL){
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss[%d] setup and init\n",i);
							wasd = interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] = os_zalloc(sizeof(struct asd_data));			
							if(wasd == NULL){
								asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
								exit(1);
							}	
							interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss++;
#ifdef ASD_USE_PERBSS_LOCK
							CWCreateThreadMutex(&(wasd->asd_sta_mutex));
#endif
							if(msg->u.BSS.bss_ifaces_type)
								wasd->bss_iface_type = 1;
							else
								wasd->bss_iface_type = 0;
							memcpy(wasd->br_ifname, msg->u.BSS.br_ifname,ETH_IF_NAME_LEN);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss_iface_type = %d.\n",wasd->bss_iface_type);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"br_ifname = %s.\n",wasd->br_ifname);
							wasd->th_deny_num=0;  //xm add
							wasd->abnormal_st_down_num=0;//nl091120
							wasd->normal_st_down_num=0;//nl091120
							wasd->assoc_reject_no_resource=0;
							
							//mahz add 2011.5.26
							if(len > 0){
								memset(wasd->nas_port_id,0,sizeof(wasd->nas_port_id));
								memcpy(wasd->nas_port_id,msg->u.BSS.nas_port_id,len);
								asd_printf(ASD_DBUS,MSG_DEBUG,"wasd->nas_port_id: %s\n",wasd->nas_port_id);//for test
							}
							
							wasd->hotspot_id = msg->u.BSS.hotspot_id;
							wasd->Radio_G_ID = msg->u.BSS.Radio_G_ID;
							wasd->Radio_L_ID = msg->u.BSS.Radio_L_ID;
							wasd->WlanID = msg->u.BSS.WlanID;
							wasd->BSSIndex = msg->u.BSS.BSSIndex;
							memcpy(wasd->own_addr,msg->u.BSS.BSSID,6);
							
							wasd->iface = interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID];
							wasd->info = os_zalloc(sizeof(struct stat_info));	//ht add 090223
							if(wasd->info == NULL){
								asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
								exit(1);
							}	
							wasd->driver = interfaces->config->bss[0]->driver;
							wasd->drv_priv = interfaces->config->bss[0]->drv_priv;
							if((ASD_WLAN[WLANID]!=NULL)&&(ASD_WLAN[WLANID]->AC_Roaming_Policy == 1)){
								unsigned char group_id;
								unsigned int j;
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
									//fcntl(sock, F_SETFL, O_NONBLOCK);
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
									ac_group_add_del_bss_info(sock,group_id,wasd,G_ADD);
									//close(sock);
								}
								
							}
							if(ASD_BSS_INIT(wasd) < 0){
								asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_BSS_INIT failed\n");
								asd_free_bss_conf(wasd->conf);
								if(wasd->info != NULL) {
									os_free(wasd->info);
									wasd->info = NULL;
								}
								os_free(wasd->iconf);
								wasd->iconf = NULL;
								wasd->driver = NULL;
								wasd->drv_priv = NULL;
								os_free(wasd);
								wasd = NULL;
								interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] = NULL;
								interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss--;
								os_free(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID);
								ASD_BSS[msg->u.BSS.BSSIndex]->BSSID = NULL;
								os_free(ASD_BSS[msg->u.BSS.BSSIndex]);
								ASD_BSS[msg->u.BSS.BSSIndex] = NULL;
								return;
							}
							else if (((wasd->conf->wpa == 1)||(wasd->conf->wpa == 2)||(wasd->conf->ieee802_1x == 1))&&(ASD_BSS_SECURITY_INIT(wasd) <0) ){
								asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_BSS_SECURITY_INIT failed\n");
								asd_free_bss_conf(wasd->conf);
								if(wasd->info != NULL) {
									os_free(wasd->info);
									wasd->info = NULL;
								}
								if(wasd->iconf != NULL){
									os_free(wasd->iconf);
									wasd->iconf = NULL;
								}
								wasd->driver = NULL;
								wasd->drv_priv = NULL;
							//	circle_cancel_timeout(assoc_update_timer, wasd, NULL);
								circle_cancel_timeout((void *)radius_client_init_auth, wasd->radius, (void*)1);
								circle_cancel_timeout((void *)radius_client_init_acct, wasd->radius, (void*)1);
								if(wasd->default_wep_key){									
									os_free(wasd->default_wep_key);
									wasd->default_wep_key = NULL;
								}
								if(wasd->radius){
									radius_client_deinit(wasd->radius);
									wasd->radius = NULL;
								}
								if(wasd->wpa_auth){
									wpa_deinit(wasd->wpa_auth);
									wasd->wpa_auth = NULL;
								}
								os_free(wasd);
								wasd = NULL;

								interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] = NULL;
								interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss--;
								os_free(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID);
								ASD_BSS[msg->u.BSS.BSSIndex]->BSSID = NULL;
								os_free(ASD_BSS[msg->u.BSS.BSSIndex]);
								ASD_BSS[msg->u.BSS.BSSIndex] = NULL;
								return;
							}
								
							//memcpy(wasd->conf->ssid.ssid, ASD_WLAN[msg->u.BSS.WlanID]->ESSID, strlen(ASD_WLAN[msg->u.BSS.WlanID]->ESSID));
							//wasd->conf->ssid.ssid_len = strlen(ASD_WLAN[msg->u.BSS.WlanID]->ESSID);
							//wasd->driver = interfaces->config->bss[0]->driver;
							//wasd->drv_priv = interfaces->config->bss[0]->drv_priv;

						//	circle_register_timeout(ASSOC_UPDATE_TIME, 0, assoc_update_timer, wasd, NULL);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x\n",wasd->BSSIndex,wasd->WlanID,wasd->Radio_G_ID,wasd->Radio_L_ID,
								wasd->own_addr[0],wasd->own_addr[1],wasd->own_addr[2],wasd->own_addr[3],wasd->own_addr[4],wasd->own_addr[5]);

							break;
					}else
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] != NULL\n");
				}
			}
			else if((ASD_BSS[msg->u.BSS.BSSIndex]->BSSID==NULL) && (ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf!=NULL)){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_BSS[%u]->BSSID = NULL,acl_conf != NULL.\n",msg->u.BSS.BSSIndex);
				ASD_BSS[msg->u.BSS.BSSIndex]->BSSID = (unsigned char*)os_zalloc(7);
				memset(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID,0,7);
				memcpy(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID,msg->u.BSS.BSSID,6);
				ASD_BSS[msg->u.BSS.BSSIndex]->BSSIndex = msg->u.BSS.BSSIndex;
				ASD_BSS[msg->u.BSS.BSSIndex]->WlanID = msg->u.BSS.WlanID;
				ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID = msg->u.BSS.Radio_G_ID;
				ASD_BSS[msg->u.BSS.BSSIndex]->Radio_L_ID = msg->u.BSS.Radio_L_ID;
				ASD_BSS[msg->u.BSS.BSSIndex]->bss_accessed_sta_num=0;
				ASD_BSS[msg->u.BSS.BSSIndex]->bss_max_allowed_sta_num = msg->u.BSS.bss_max_sta_num; //zhanglei change 64 to msg->u.BSS.bss_max_sta_num
				ASD_BSS[msg->u.BSS.BSSIndex]->vlanid = msg->u.BSS.vlanid;
				ASD_BSS[msg->u.BSS.BSSIndex]->sta_static_arp_policy = msg->u.BSS.sta_static_arp_policy;
				memcpy(ASD_BSS[msg->u.BSS.BSSIndex]->arp_ifname, msg->u.BSS.arp_ifname,ETH_IF_NAME_LEN);

				asd_printf(ASD_DEFAULT,MSG_INFO,"%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x\n",ASD_BSS[msg->u.BSS.BSSIndex]->BSSIndex,ASD_BSS[msg->u.BSS.BSSIndex]->WlanID,ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID,ASD_BSS[msg->u.BSS.BSSIndex]->Radio_L_ID,
					ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[0],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[1],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[2],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[3],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[4],ASD_BSS[msg->u.BSS.BSSIndex]->BSSID[5]);
				if((msg->u.BSS.BSSIndex / L_BSS_NUM) == msg->u.BSS.Radio_G_ID){
					i = msg->u.BSS.BSSIndex % L_BSS_NUM;
					g_radio_id = msg->u.BSS.Radio_G_ID;
					for(j=g_radio_id%4; j>=0; j--){
						unsigned radio_id = g_radio_id - j;
						asd_printf(ASD_DEFAULT,MSG_INFO,"interfaces->iface[%d] setup and init\n",radio_id);
						if(interfaces->iface[radio_id] == NULL)
							asd_iface_malloc(radio_id);
					}
					if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID] == NULL){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"interfaces->iface[%d] setup and init\n",ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID);
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID] = (struct asd_iface*)os_zalloc(sizeof(struct asd_iface));
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss = NULL;
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->interfaces = interfaces;
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss = 0;
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss = os_zalloc(L_BSS_NUM * sizeof(struct asd_data*));
						if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss == NULL){
							asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
							exit(1);
						}	
						int bssnum = 0;
						for(bssnum = 0; bssnum < L_BSS_NUM; bssnum++)
							interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[bssnum] = NULL;
						if(asd_hw_feature_init(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]) < 0)
							asd_printf(ASD_DEFAULT,MSG_WARNING,"asd_hw_feature_init failed\n");
					}
					if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] == NULL){
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss[%d] setup and init\n",i);
							wasd = interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] = os_zalloc(sizeof(struct asd_data));			
							if(wasd == NULL){
								asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
								exit(1);
							}	
							interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss++;
#ifdef ASD_USE_PERBSS_LOCK
							CWCreateThreadMutex(&(wasd->asd_sta_mutex));
#endif
							if(msg->u.BSS.bss_ifaces_type)
								wasd->bss_iface_type = 1;
							else
								wasd->bss_iface_type = 0;
							memcpy(wasd->br_ifname, msg->u.BSS.br_ifname,ETH_IF_NAME_LEN);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss_iface_type = %d.\n",wasd->bss_iface_type);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"br_ifname = %s.\n",wasd->br_ifname);
							wasd->Radio_G_ID = msg->u.BSS.Radio_G_ID;
							wasd->Radio_L_ID = msg->u.BSS.Radio_L_ID;
							wasd->WlanID = msg->u.BSS.WlanID;
							wasd->BSSIndex = msg->u.BSS.BSSIndex;
							memcpy(wasd->own_addr,msg->u.BSS.BSSID,6);

							wasd->iface = interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID];
							wasd->info = os_zalloc(sizeof(struct stat_info));	//ht add 090223
							if(wasd->info == NULL){
								asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
								exit(1);
							}	
							wasd->driver = interfaces->config->bss[0]->driver;
							wasd->drv_priv = interfaces->config->bss[0]->drv_priv;
							if((ASD_WLAN[WLANID]!=NULL)&&(ASD_WLAN[WLANID]->AC_Roaming_Policy == 1)){
								unsigned char group_id;
								unsigned int j;
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
									//fcntl(sock, F_SETFL, O_NONBLOCK);
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
									ac_group_add_del_bss_info(sock,group_id,wasd,G_ADD);
									//close(sock);
								}
								
							}

							if(ASD_BSS_INIT(wasd) < 0){
								asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_BSS_INIT failed\n");
								asd_free_bss_conf(wasd->conf);
								if(wasd->info != NULL) {
									os_free(wasd->info);
									wasd->info = NULL;
								}
								os_free(wasd->iconf);
								wasd->iconf = NULL;
								wasd->driver = NULL;
								wasd->drv_priv = NULL;
								os_free(wasd);
								wasd = NULL;
								interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] = NULL;
								interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss--;
								os_free(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID);
								ASD_BSS[msg->u.BSS.BSSIndex]->BSSID = NULL;
								os_free(ASD_BSS[msg->u.BSS.BSSIndex]);
								ASD_BSS[msg->u.BSS.BSSIndex] = NULL;
								return;
							}
							else if (((wasd->conf->wpa == 1)||(wasd->conf->wpa == 2)||(wasd->conf->ieee802_1x == 1))&&(ASD_BSS_SECURITY_INIT(wasd) <0) ){
								radius_client_deinit(wasd->radius);
								asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_BSS_SECURITY_INIT failed\n");
								return;
							}
								
							//memcpy(wasd->conf->ssid.ssid, ASD_WLAN[msg->u.BSS.WlanID]->ESSID, strlen(ASD_WLAN[msg->u.BSS.WlanID]->ESSID));
							//wasd->conf->ssid.ssid_len = strlen(ASD_WLAN[msg->u.BSS.WlanID]->ESSID);
							//wasd->driver = interfaces->config->bss[0]->driver;
							//wasd->drv_priv = interfaces->config->bss[0]->drv_priv;
							
						//	circle_register_timeout(ASSOC_UPDATE_TIME, 0, assoc_update_timer, wasd, NULL);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x\n",wasd->BSSIndex,wasd->WlanID,wasd->Radio_G_ID,wasd->Radio_L_ID,
								wasd->own_addr[0],wasd->own_addr[1],wasd->own_addr[2],wasd->own_addr[3],wasd->own_addr[4],wasd->own_addr[5]);

							break;
					}else
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] != NULL\n");
				}
			}else{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSS ADD ERROR.\n");
				if(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID != NULL){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_BSS[%u]->BSSID = "MACSTR"\n",msg->u.BSS.BSSIndex,MAC2STR(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID));
				}
				return;
			}
			break;

		}
		case WID_DEL :{
			if(ASD_BSS[msg->u.BSS.BSSIndex] != NULL){		
				pthread_mutex_lock(&asd_g_sta_mutex); 
			    i = ASD_BSS[msg->u.BSS.BSSIndex]->BSSIndex % L_BSS_NUM;
				if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID] != NULL){
					if(interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] != NULL){
						wasd = interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i];
						if(NULL == wasd)
						{
							asd_printf(ASD_DEFAULT,MSG_ERROR,"wasd bssindex %d is not exit,del error!\n",msg->u.BSS.BSSIndex);
							break;
						}
						//mahz add 2011.4.30
						wasd->master_to_disable = msg->u.BSS.master_to_disable;//qiuchen add it for AXSSZFI-1191
						int wtpid=0;
						wtpid=wasd->Radio_G_ID/L_RADIO_NUM;
						if(ASD_WTP_AP_HISTORY[wtpid] != NULL){
							ASD_WTP_AP_HISTORY[wtpid]->usr_auth_tms += wasd->usr_auth_tms;
							ASD_WTP_AP_HISTORY[wtpid]->ac_rspauth_tms += wasd->ac_rspauth_tms;
							ASD_WTP_AP_HISTORY[wtpid]->auth_fail += wasd->auth_fail;
							ASD_WTP_AP_HISTORY[wtpid]->auth_success += wasd->auth_success;
							ASD_WTP_AP_HISTORY[wtpid]->num_assoc += wasd->num_assoc;
							ASD_WTP_AP_HISTORY[wtpid]->num_reassoc += wasd->num_reassoc;
							ASD_WTP_AP_HISTORY[wtpid]->num_assoc_failure += wasd->num_assoc_failure;
							ASD_WTP_AP_HISTORY[wtpid]->num_reassoc_failure += wasd->num_reassoc_failure;
							ASD_WTP_AP_HISTORY[wtpid]->assoc_success += wasd->assoc_success;
							ASD_WTP_AP_HISTORY[wtpid]->reassoc_success += wasd->reassoc_success;
							ASD_WTP_AP_HISTORY[wtpid]->assoc_req += wasd->assoc_req;
							ASD_WTP_AP_HISTORY[wtpid]->assoc_resp += wasd->assoc_resp;
							ASD_WTP_AP_HISTORY[wtpid]->total_ap_flow_record += ASD_WTP_AP_HISTORY[wtpid]->total_ap_flow;
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"total_assoc_num = %d\n",ASD_WTP_AP_HISTORY[wtpid]->assoc_req);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"total_ap_flow_record = %llu(KB)\n",ASD_WTP_AP_HISTORY[wtpid]->total_ap_flow_record);
						}
						//
						if((ASD_WLAN[WLANID]!=NULL)&&(ASD_WLAN[WLANID]->AC_Roaming_Policy == 1)){
							unsigned char group_id;
							unsigned int j;
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
								ac_group_add_del_bss_info(sock,group_id,wasd,G_DEL);
								//close(sock);
							}
							
						}
#ifdef ASD_USE_PERBSS_LOCK
						pthread_mutex_lock(&(wasd->asd_sta_mutex)); 
#endif
					//	circle_cancel_timeout(assoc_update_timer, wasd, NULL);
						asd_free_stas(wasd);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_free_stas finish\n");						
						circle_cancel_timeout((void *)radius_client_init_auth, wasd->radius, (void*)1);
						circle_cancel_timeout((void *)radius_client_init_acct, wasd->radius, (void*)1);
						asd_cleanup(wasd);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_cleanup finish\n");
						
						unsigned char SID = 0;
						if(ASD_WLAN[wasd->WlanID])
							SID = ASD_WLAN[wasd->WlanID]->SecurityID;
						if((ASD_SECURITY[SID] != NULL) && ((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->extensible_auth == 1)))){
							if(ASD_SECURITY[SID]->pre_auth){
								preauth_bss = PreAuth_wlan_get_bss(ASD_WLAN[wasd->WlanID],wasd->own_addr);
								if(preauth_bss != NULL){
									PreAuth_wlan_free_bss(ASD_WLAN[wasd->WlanID],preauth_bss);
								}
							}
						}
						
						asd_free_bss_conf(wasd->conf);
						if((ASD_SECURITY[SID] != NULL) && ((ASD_SECURITY[SID]->securityType == WAPI_AUTH)||(ASD_SECURITY[SID]->securityType == WAPI_PSK))){
							if((wasd->wapi_wasd != NULL)&&(wasd->wapi_wasd->vap_user!=NULL))
								free_one_interface(wasd->wapi_wasd->vap_user);
							if((wasd->wapi_wasd != NULL)){
								free_one_wapi(wasd->wapi_wasd);
							}
						}
						if(wasd->info != NULL) {
							os_free(wasd->info);
							wasd->info = NULL;
						}
						if(wasd->iconf != NULL){
							os_free(wasd->iconf);
							wasd->iconf = NULL;
						}
#ifdef ASD_USE_PERBSS_LOCK
					
						pthread_mutex_unlock(&(wasd->asd_sta_mutex)); 
						CWDestroyThreadMutex(&(wasd->asd_sta_mutex));
#endif
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->iconf) finish\n");
						wasd->driver = NULL;
						wasd->drv_priv = NULL;
						os_free(wasd);
						wasd = NULL;
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd) finish\n");
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->bss[i] = NULL;
						interfaces->iface[ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID]->num_bss--;
						os_free(ASD_BSS[msg->u.BSS.BSSIndex]->BSSID);
						ASD_BSS[msg->u.BSS.BSSIndex]->BSSID = NULL;
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(ASD_BSS[%d]->BSSID) finish\n",msg->u.BSS.BSSIndex);
						ASD_BSS[msg->u.BSS.BSSIndex]->BSSIndex = 0;
						ASD_BSS[msg->u.BSS.BSSIndex]->Radio_G_ID = 0;
						ASD_BSS[msg->u.BSS.BSSIndex]->Radio_L_ID = 0;
						ASD_BSS[msg->u.BSS.BSSIndex]->WlanID = 0;
						ASD_BSS[msg->u.BSS.BSSIndex]->State = 0;
						ASD_BSS[msg->u.BSS.BSSIndex]->bss_accessed_sta_num= 0;
						ASD_BSS[msg->u.BSS.BSSIndex]->bss_max_allowed_sta_num= 0;
						
						if( ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf != NULL){	//ht add
							if( ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->accept_mac != NULL) {
								free_maclist(ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf,ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->accept_mac);
								ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->accept_mac = NULL;
								}
							if( ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->deny_mac != NULL) {
								free_maclist(ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf,ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->deny_mac);
								ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf->deny_mac = NULL;
								}
							os_free(ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf);
							ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf = NULL;
						}
						os_free(ASD_BSS[msg->u.BSS.BSSIndex]);
						ASD_BSS[msg->u.BSS.BSSIndex] = NULL;
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(ASD_BSS[msg->u.BSS.BSSIndex]) finish\n");
						pthread_mutex_unlock(&asd_g_sta_mutex);
						return;


					}
					


				}
				pthread_mutex_unlock(&asd_g_sta_mutex); 
				return;
			}else
				return;
			break;

		}
		case WID_MODIFY :{
			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"WID_MODIFY\n");
			if(ASD_BSS[msg->u.BSS.BSSIndex] != NULL){     //xm add 08/12/04
				ASD_BSS[msg->u.BSS.BSSIndex]->bss_max_allowed_sta_num=msg->u.BSS.bss_max_sta_num;
				ASD_BSS[msg->u.BSS.BSSIndex]->sta_static_arp_policy = msg->u.BSS.sta_static_arp_policy;
				memcpy(ASD_BSS[msg->u.BSS.BSSIndex]->arp_ifname, msg->u.BSS.arp_ifname,ETH_IF_NAME_LEN);
				return;                                  
			}else
				return;
			break;
		}
		case BSS_INFO :{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSS_INFO\n");
			for(i=0; i<L_BSS_NUM; i++){
				if(msg->u.BSS.BssData[i].BSSIndex == 0)
					return;
				
				if(interfaces->iface[msg->u.BSS.BssData[i].Radio_G_ID] != NULL){
					j = msg->u.BSS.BssData[i].BSSIndex % L_BSS_NUM;
					if(interfaces->iface[msg->u.BSS.BssData[i].Radio_G_ID]->bss[j] != NULL){
						wasd = interfaces->iface[msg->u.BSS.BssData[i].Radio_G_ID]->bss[j];
						wasd->info->rx_data_pkts = msg->u.BSS.BssData[i].rx_pkt_data;
						wasd->info->tx_data_pkts = msg->u.BSS.BssData[i].tx_pkt_data;
						wasd->info->rx_data_bytes = msg->u.BSS.BssData[i].rx_data_bytes;	//xiaodawei add for bss rx_data_bytes, 20110224
						wasd->info->tx_data_bytes = msg->u.BSS.BssData[i].tx_data_bytes;	//xiaodawei add for bss tx_data_bytes, 20110224
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"BssData[%d].rx_pkt_data %u\n",i ,msg->u.BSS.BssData[i].rx_pkt_data);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"BssData[%d].tx_pkt_data %u\n",i ,msg->u.BSS.BssData[i].tx_pkt_data);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"BssData[%d].rx_data_bytes %llu\n",i ,msg->u.BSS.BssData[i].rx_data_bytes);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"BssData[%d].tx_data_bytes %llu\n",i ,msg->u.BSS.BssData[i].tx_data_bytes);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSSIndex %d rx_pkts %d tx_pkts %d\n",wasd->BSSIndex,wasd->info->rx_data_pkts,wasd->info->tx_data_pkts);
					}
				}
			}
			break;
		}
		case MAC_LIST_ADD:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in case : MAC_LIST_ADD!\n");
			if(ASD_BSS[msg->u.BSS.BSSIndex] == NULL)
			{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_bss is not exist!");
				return;
			}
			int i;
			int m = 0;
			unsigned int accept_mac_num=0,deny_mac_num=0;
			struct acl_config *conf = NULL; 
			accept_mac_num = msg->u.BSS.accept_mac_num;
			deny_mac_num = msg->u.BSS.deny_mac_num;
			conf = ASD_BSS[msg->u.BSS.BSSIndex]->acl_conf;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"the  msg->u.BSS.deny_mac_num is %d\n", msg->u.BSS.deny_mac_num);
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"the deny_mac_num is %d\n",deny_mac_num);
			if(conf == NULL) {
				conf = (struct acl_config*)os_zalloc(sizeof(struct acl_config));
				if(conf == NULL) return;
				conf->macaddr_acl = 0;
				conf->accept_mac = NULL;
				conf->num_accept_mac = 0;
				conf->deny_mac = NULL;
				conf->num_accept_mac = 0;
			}
			conf->macaddr_acl = msg->u.BSS.macaddr_acl;
			for( i = 0 ; (i < accept_mac_num)&&(i < 50); i ++)
			{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"the mac is"MACSTR"\n",MAC2STR(msg->u.BSS.accept_mac[i]));
				add_mac_in_maclist(conf,msg->u.BSS.accept_mac[i],2);
			}
			for(i = 0 ; (i < deny_mac_num)&&(i<50) ; i ++)
			{			
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"the black mac is"MACSTR"\n",MAC2STR(msg->u.BSS.deny_mac[i]));
				add_mac_in_maclist(conf,msg->u.BSS.deny_mac[i],1);
			}
			m = change_maclist_security(conf,msg->u.BSS.macaddr_acl);
			if(m !=0)
				   asd_printf(ASD_DEFAULT,MSG_DEBUG,"change mac list failed\n");
			break;
		}
		default :
			return;
	}
	return;
}

void STA_OP(TableMsg *msg){	
	struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM];
	struct asd_data *wasd = NULL;
	struct asd_data *prewasd = NULL;			
	struct sta_info *sta = NULL;
	struct sta_info *presta = NULL;	
	time_t *timec;
	time_t times;//qiuchen add it
	unsigned int WTPID = msg->u.STA.WTPID;
	unsigned int num = 0, i = 0, j = 0;
	unsigned int count = 0;
	unsigned int authorize_num = 0;
	unsigned char ret;	
	unsigned int BSSIndex = msg->u.STA.BSSIndex;
	unsigned int preBSSIndex = msg->u.STA.preBSSIndex;
	struct PMK_STAINFO *pmk_sta;
	struct ROAMING_STAINFO *r_sta;	
	unsigned char SID = 0;
	char msgtype[64] = {0};
	ASD_STA_OPT_STR(msg->Op,msgtype);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA_OP type %s\n",msgtype);	
	switch(msg->Op){
		case WID_DEL:			
			ret = AsdCheckWTPID(WTPID);
			if(ret == 0)
				return;
			pthread_mutex_lock(&(asd_g_sta_mutex));			//mahz add  2011.4.20
			num = ASD_SEARCH_WTP_STA(WTPID, bss);
			for(i = 0; i < num; i++){
				ASD_WTP_ST *WTP = ASD_WTP_AP[WTPID];
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
				sta = ap_get_sta(bss[i], msg->u.STA.STAMAC);
				if(sta != NULL){
					if((ASD_WLAN[bss[i]->WlanID]!=NULL)&&(ASD_WLAN[bss[i]->WlanID]->Roaming_Policy!=0)){
						r_sta = roaming_get_sta(ASD_WLAN[bss[i]->WlanID],msg->u.STA.STAMAC);
						if(r_sta != NULL)
							roaming_free_sta(ASD_WLAN[bss[i]->WlanID],r_sta);
					}
				
					if(1 == check_sta_authorized(bss[i],sta))
						bss[i]->authorized_sta_num--;
					sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);					
					sta->flags |= WLAN_STA_DEL ;					
					bss[i]->abnormal_st_down_num ++;//nl091120
					signal_sta_leave_abnormal(sta->addr,bss[i]->Radio_G_ID,bss[i]->BSSIndex,bss[i]->WlanID,sta->rssi);
					if(ASD_WLAN[bss[i]->WlanID])
						SID = ASD_WLAN[bss[i]->WlanID]->SecurityID;
					if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
						if(ASD_SECURITY[SID]->securityType == WPA2_E){
							pmk_sta = pmk_ap_get_sta(ASD_WLAN[bss[i]->WlanID],sta->addr);
							if(pmk_sta)
								pmk_ap_free_sta(ASD_WLAN[bss[i]->WlanID],pmk_sta);
						}
						wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
						mlme_deauthenticate_indication(
							bss[i], sta, 0);
						 if(sta->acct_terminate_cause == 0){
							sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
						 }
						ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
					}
					//mahz add 2011.11.9 for GuangZhou Mobile
					if(sta->security_type == NO_NEED_AUTH){
						bss[i]->no_auth_sta_abnormal_down_num ++;
					}else if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)
						||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)
						||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == SHARED)
						||(ASD_SECURITY[SID]->extensible_auth == 1)||(ASD_SECURITY[SID]->securityType == WAPI_PSK)
						||(ASD_SECURITY[SID]->securityType == WAPI_AUTH))){
							bss[i]->assoc_auth_sta_abnormal_down_num ++;
					}
					//qiuchen
					bss[i]->statistics.sta_drop_cnt++;
					if (ASD_AUTH_TYPE_WEP_PSK(SID))
					{
						bss[i]->u.weppsk.total_sta_drop_cnt++;
					}
					else if (ASD_AUTH_TYPE_EAP(SID))
					{
						if (ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
						{
							bss[i]->u.eap_auth.autoauth.total_sta_drop_cnt++;
						}
						asd_printf(ASD_1X,MSG_DEBUG, "total_sta_drop_cnt is %d",
		  				bss[i]->u.eap_auth.autoauth.total_sta_drop_cnt);
					}

					
					//UpdateStaInfoToWSM(bss[i], sta->addr, WID_DEL);	
					//ap_free_sta(bss[i], sta,1);
					if(WTP){
						time_t now;
						time(&now);
						unsigned char *ip = (unsigned char*)&(WTP->WTPIP);
						syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]STA :"MACSTR" leave WTP %d,WTP MAC:"MACSTR",WTP IP:%d.%d.%d.%d,Leave Time:%s\n",
							slotid,vrrid,MAC2STR(msg->u.STA.STAMAC),msg->u.STA.WTPID,MAC2STR(WTP->WTPMAC),ip[0],ip[1],ip[2],ip[3],ctime(&now));
					}
					//qiuchen
					char *SSID = NULL;
					u8 *identity = NULL;
					if(sta->eapol_sm)
						identity = sta->eapol_sm->identity;
					if(ASD_WLAN[bss[i]->WlanID])
						SSID = ASD_WLAN[bss[i]->WlanID]->ESSID;
					if(gASDLOGDEBUG & BIT(0)){
						if(ASD_SECURITY[SID]&&((ASD_SECURITY[SID]->securityType == OPEN) || (ASD_SECURITY[SID]->securityType == SHARED)))
							asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MACSTR" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(sta->addr),SSID,1);
						else if((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->securityType == WAPI_AUTH) && (ASD_SECURITY[SID]->wapi_radius_auth == 1))||(ASD_SECURITY[SID]->extensible_auth == 1)){
							asd_syslog_h(LOG_INFO,"WSTA","PORTSEC_DOT1X_LOGOFF:-IfName=GRadio-BSS%d:%d-MACAddr="MACSTR"-VlanId=%d-UserName=%s-ErrCode=%d; Session of the 802.1X user was terminated.\n",bss[i]->Radio_G_ID,bss[i]->BSSIndex%128,MAC2STR(sta->addr),sta->vlan_id,identity,1);
						}
					}
					u8 WTPMAC[6] = {0};
					if(ASD_WTP_AP[bss[i]->Radio_G_ID/4])
						memcpy(WTPMAC,ASD_WTP_AP[bss[i]->Radio_G_ID/4]->WTPMAC,6);
					if(gASDLOGDEBUG & BIT(1))
						syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]DISASSOC:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d, ErrorCode:%d.\n",
							slotid,vrrid,MAC2STR(sta->addr),MAC2STR(WTPMAC),bss[i]->BSSIndex,999);//qiuchen 2013.01.14
					//end
					if(ASD_NOTICE_STA_INFO_TO_PORTAL)
						AsdStaInfoToEAG(bss[i],sta,WID_DEL);
					if(ASD_WLAN[bss[i]->WlanID]!=NULL&&ASD_WLAN[bss[i]->WlanID]->balance_switch == 1&&ASD_WLAN[bss[i]->WlanID]->balance_method==1){
						ap_free_sta(bss[i], sta, 1);
					}
					else{
						ap_free_sta(bss[i], sta, 0);
					}
				}				

#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
			}
			pthread_mutex_unlock(&(asd_g_sta_mutex));			//mahz add 2011.4.20
			break;
		case WID_MODIFY:
			pthread_mutex_lock(&(asd_g_sta_mutex));			//mahz add  2011.4.20
			wasd = AsdCheckBSSIndex(BSSIndex);
			prewasd = AsdCheckBSSIndex(preBSSIndex);			
			if(wasd != NULL){
				sta = ap_get_sta(wasd, msg->u.STA.STAMAC);
				if(sta != NULL){
					;
				}else{
					sta = ap_sta_add(wasd, msg->u.STA.STAMAC, 0);
					if(sta != NULL){
						local_success_roaming_count++;
						sta->flags |= WLAN_STA_AUTH;
						sta->flags |= WLAN_STA_ASSOC;
						sta->flags |= WLAN_STA_ROAMING;
					}else{
						pthread_mutex_unlock(&(asd_g_sta_mutex));			//mahz add 2011.4.20
						break;
					}
				}				
			}
			if(prewasd != NULL){
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(prewasd->asd_sta_mutex));   
#endif
				presta = ap_get_sta(prewasd, msg->u.STA.STAMAC);
				if(presta != NULL){	
					{						
						timec = presta->add_time;
						presta->add_time = sta->add_time;
						sta->add_time = timec;
						//qiuchen add it
						times = presta->add_time_sysruntime;
						presta->add_time_sysruntime =  sta->add_time_sysruntime;
						sta->add_time_sysruntime = times;
						//end
					}
				
					if(1 == check_sta_authorized(prewasd,presta))
						prewasd->authorized_sta_num--;
					presta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
					presta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
					if(ASD_WLAN[prewasd->WlanID])
						SID = ASD_WLAN[prewasd->WlanID]->SecurityID;
					if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
						wpa_auth_sm_event(presta->wpa_sm, WPA_DEAUTH);
						mlme_deauthenticate_indication(
							prewasd, presta, 0);
						ieee802_1x_notify_port_enabled(presta->eapol_sm, 0);
					}
					//UpdateStaInfoToWSM(bss[i], sta->addr, WID_DEL);	
					//ap_free_sta(bss[i], sta,1);
					
					if(ASD_WLAN[prewasd->WlanID]!=NULL&&ASD_WLAN[prewasd->WlanID]->balance_switch == 1&&ASD_WLAN[prewasd->WlanID]->balance_method==1)
						ap_free_sta_without_wsm(prewasd, presta, 1);
					else
						ap_free_sta_without_wsm(prewasd, presta, 0);
				}
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(prewasd->asd_sta_mutex));   
#endif
			}
			pthread_mutex_unlock(&(asd_g_sta_mutex));			//mahz add 2011.4.20
			break;
			//================================================
		case STA_INFO:
			WTPID = msg->u.STAINFO[0].WTPID;
			ret = AsdCheckWTPID(WTPID);
			if(ret == 0)
				return;
			count = msg->u.STAINFO[0].count;
			num = ASD_SEARCH_WTP_STA(WTPID, bss);
			for(j=0;j<count;j++){
				for(i = 0; i < num; i++){
					
#ifdef ASD_USE_PERBSS_LOCK
					pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
					sta = ap_get_sta(bss[i], msg->u.STAINFO[j].STAMAC);
					if(sta != NULL){
						sta->mode=msg->u.STAINFO[j].mode;
						sta->channel=msg->u.STAINFO[j].channel;
						sta->rssi=msg->u.STAINFO[j].rssi;
						sta->nRate=msg->u.STAINFO[j].tx_Rate;
						sta->txRate=msg->u.STAINFO[j].rx_Rate;
						sta->isPowerSave=msg->u.STAINFO[j].isPowerSave;
						sta->isQos=msg->u.STAINFO[j].isQos;

						sta->txbytes=msg->u.STAINFO[j].rx_data_bytes;
	                    sta->rxbytes=msg->u.STAINFO[j].tx_data_bytes;
	                    sta->txdatapackets = (unsigned long long)msg->u.STAINFO[j].rx_data_frames;
	                    sta->rxdatapackets = (unsigned long long)msg->u.STAINFO[j].tx_data_frames;
	                    sta->txpackets = (unsigned long long)msg->u.STAINFO[j].rx_frames;
	                    sta->rxpackets = (unsigned long long)msg->u.STAINFO[j].tx_frames;
	                    sta->txframepackets = (unsigned long long)msg->u.STAINFO[j].rx_frag_packets;
	                    sta->rxframepackets = (unsigned long long)msg->u.STAINFO[j].tx_frag_packets;

						sta->MAXofRateset = msg->u.STAINFO[j].MAXofRateset;
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->MAXofRateset = %d\n", sta->MAXofRateset);
						
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA_OP %s "MACSTR"\n",msgtype,MAC2STR(msg->u.STAINFO[j].STAMAC));
						if(!bss[i]->bss_iface_type){
							sta->rxbytes=(unsigned long long)(msg->u.STAINFO[j].rx_bytes);
							sta->txbytes=(unsigned long long)(msg->u.STAINFO[j].tx_bytes);
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta "MACSTR" rxbytes %llu txbytes %llu\n",MAC2STR(msg->u.STAINFO[j].STAMAC),sta->rxbytes,sta->txbytes);
						}
					}
#ifdef ASD_USE_PERBSS_LOCK
					pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
				}
			}
			break;
		case STA_WTP_TERMINAL_STATISTICS:
			{
				WTPID = msg->u.STAINFO[0].WTPID;
				ret = AsdCheckWTPID(WTPID);
				if(ret == 0)
					return;
				count = msg->u.STAINFO[0].count;
				num = ASD_SEARCH_WTP_STA(WTPID, bss);
				for(j=0;j<count;j++)
				{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"------ %s %d ------\n", __func__, __LINE__);
					for(i = 0; i < num; i++)
					{					
#ifdef ASD_USE_PERBSS_LOCK
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"------ %s %d ------\n", __func__, __LINE__);
						pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
						sta = ap_get_sta(bss[i], msg->u.STAINFO[j].STAMAC);
						if(sta != NULL)
						{
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"------ %s %d ------\n", __func__, __LINE__);
							memcpy(&(sta->wtp_sta_statistics_info),
									&(msg->u.STAINFO[j].wtp_sta_statistics_info),
									sizeof(msg->u.STAINFO[j].wtp_sta_statistics_info));
							#if 0
							int n = 0;
							for (n = 0; n <WTP_SUPPORT_RATE_NUM; n++)
							{
								asd_printf(ASD_DEFAULT,
										   MSG_DEBUG,
										   "------- %s %d APStaRxDataRatePkts[%d] = %d ------\n", 
										   __func__, __LINE__,
										   n,
										   sta->wtp_sta_statistics_info.APStaRxDataRatePkts[n]);
								asd_printf(ASD_DEFAULT,
										   MSG_DEBUG,
										   "------- %s %d APStaTxDataRatePkts[%d] = %d ------\n", 
										   __func__, __LINE__,
										   n,
										   sta->wtp_sta_statistics_info.APStaTxDataRatePkts[n]);
							}
							for (n =- 0; n < WTP_RSSI_INTERVAL_NUM; n++)
							{
								asd_printf(ASD_DEFAULT,
										   MSG_DEBUG,
										   "------- %s %d APStaTxSignalStrengthPkts[%d] = %d ------\n", 
										   __func__, __LINE__,
										   n,
										   sta->wtp_sta_statistics_info.APStaTxSignalStrengthPkts[n]);
							}
							#endif
						}
#ifdef ASD_USE_PERBSS_LOCK
						pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
					}
				}
			}
			break;
			//===================================================
		case WID_CONFLICT :
			
			pthread_mutex_lock(&(asd_g_sta_mutex));
			prewasd = AsdCheckBSSIndex(preBSSIndex);
			if(prewasd != NULL){
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(prewasd->asd_sta_mutex));   
#endif
				presta = ap_get_sta(prewasd, msg->u.STA.STAMAC);
				if(presta != NULL){	
					wasd = AsdCheckBSSIndex(BSSIndex);
					if(wasd != NULL)
					{
						sta = ap_get_sta(wasd,msg->u.STA.STAMAC);
						if(sta != NULL)
						{
							if((presta->ipaddr != 0)&&(sta->ipaddr == 0))
							{
								sta->ipaddr = presta->ipaddr;
								sta->ip_addr.s_addr = presta->ip_addr.s_addr;
								memset(sta->in_addr,0,16);
								memcpy(sta->in_addr,presta->in_addr,strlen(presta->in_addr));
								memset(sta->arpifname,0,sizeof(sta->arpifname));
								memcpy(sta->arpifname,presta->arpifname,sizeof(presta->arpifname));
								SID = wasd->SecurityID;
								if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
									if((ASD_SECURITY[SID]->account_after_authorize == 0)&&(sta->flags &WLAN_STA_AUTHORIZED)){
										accounting_sta_start(wasd,sta);
									}
								}
							}
							sta->PreBssIndex = preBSSIndex;
							sta->preAPID = prewasd->Radio_G_ID/4;
							memcpy(sta->PreBSSID,prewasd->own_addr,6);
							sta->rflag = ASD_ROAM_2;
							char *SSID = NULL;
							if(ASD_WLAN[wasd->WlanID])
								SSID = ASD_WLAN[wasd->WlanID]->ESSID;
							if(gASDLOGDEBUG & BIT(1) && !(sta->logflag&BIT(1))){
								if((ASD_SECURITY[SID]) && (ASD_SECURITY[SID]->securityType == OPEN || ASD_SECURITY[SID]->securityType == SHARED) && (sta->flags & WLAN_STA_ASSOC)){
									syslog(LOG_INFO|LOG_LOCAL7,"[%d-%d]STA_ROAM_SUCCESS:UserMAC:"MACSTR" From AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR") To AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR").\n",
										slotid,vrrid,MAC2STR(sta->addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
										prewasd->Radio_G_ID/4,MAC2STR(prewasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
										wasd->Radio_G_ID/4,MAC2STR(wasd->own_addr)
									);
									sta->logflag = BIT(1);
								}
								else if((sta->flags & WLAN_STA_AUTHORIZED)){
									syslog(LOG_INFO|LOG_LOCAL7,"[%d-%d]STA_ROAM_SUCCESS:UserMAC:"MACSTR" From AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR") To AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR").\n",
										slotid,vrrid,MAC2STR(sta->addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
										prewasd->Radio_G_ID/4,MAC2STR(prewasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
										wasd->Radio_G_ID/4,MAC2STR(wasd->own_addr)
									);
									sta->logflag = BIT(1);
								}
							}
							if(gASDLOGDEBUG & BIT(0))
								asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MAC_ADDRESS" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(sta->addr),SSID,65534);
							if(gASDLOGDEBUG & BIT(0) && !(sta->logflag&BIT(0))){
								if((ASD_SECURITY[SID]) && (ASD_SECURITY[SID]->securityType == OPEN || ASD_SECURITY[SID]->securityType == SHARED) && (sta->flags & WLAN_STA_ASSOC)){
									asd_syslog_h(LOG_INFO,"WSTA","WROAM_ROAM_HAPPEN:Client "MAC_ADDRESS" roamed from BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu to BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu.\n",MAC2STR(sta->addr),MAC2STR(prewasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
										((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),MAC2STR(wasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
									sta->logflag = BIT(0);
								}
								else if((sta->flags & WLAN_STA_AUTHORIZED) && !(sta->logflag&BIT(0))){
									asd_syslog_h(LOG_INFO,"WSTA","WROAM_ROAM_HAPPEN:Client "MAC_ADDRESS" roamed from BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu to BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu.\n",MAC2STR(sta->addr),MAC2STR(prewasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
										((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),MAC2STR(wasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
									sta->logflag = BIT(0);
								}
							}
						}
					}
					AsdStaInfoToWID(prewasd, msg->u.STA.STAMAC, WID_DEL);			//mahz add 2011.1.14
					prewasd->normal_st_down_num ++;
					presta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
					if(ASD_WLAN[prewasd->WlanID])
						SID = ASD_WLAN[prewasd->WlanID]->SecurityID;
					if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
						wpa_auth_sm_event(presta->wpa_sm, WPA_DEAUTH);
						mlme_deauthenticate_indication(
							prewasd, presta, 0);
						ieee802_1x_notify_port_enabled(presta->eapol_sm, 0);
					}
					
					if(ASD_NOTICE_STA_INFO_TO_PORTAL)
						AsdStaInfoToEAG(prewasd,presta,OPEN_ROAM);		
					if(ASD_WLAN[prewasd->WlanID]!=NULL&&ASD_WLAN[prewasd->WlanID]->balance_switch == 1&&ASD_WLAN[prewasd->WlanID]->balance_method==1)
						ap_free_sta_without_wsm(prewasd, presta, 1);
					else
						ap_free_sta_without_wsm(prewasd, presta, 0);
				}
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(prewasd->asd_sta_mutex));	
#endif
			}
			
			pthread_mutex_unlock(&(asd_g_sta_mutex));
			break;
			
		case STA_WAPI_INFO:
			WTPID =  msg->u.StaWapi.WTPID;
			ret = AsdCheckWTPID(WTPID);
			if(ret == 0)
				return;
			num = ASD_SEARCH_WTP_STA(WTPID, bss);
			for(i = 0; i < num; i++){	
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->sta_num = %d\n",msg->u.StaWapi.sta_num);
				for(j = 0; ((j < msg->u.StaWapi.sta_num)&&(j < 64)); j++){
					if( msg->u.StaWapi.StaWapiInfo[j].WlanId == bss[i]->WlanID){
						sta = ap_get_sta(bss[i], msg->u.StaWapi.StaWapiInfo[j].mac);
						if(sta != NULL){	
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->flag = %d\n",msg->u.StaWapi.StaWapiInfo[j].wapi_trap_flag);
							if(msg->u.StaWapi.StaWapiInfo[j].wapi_trap_flag == 1){
								signal_wapi_trap(sta->addr,bss[i]->BSSIndex,ATTACK_ADDR_REDIRECTION);
							}else if(msg->u.StaWapi.StaWapiInfo[j].wapi_trap_flag == 2){
								// clean trap
							}else if(msg->u.StaWapi.StaWapiInfo[j].wapi_trap_flag == 3){
                               signal_wapi_trap(sta->addr,bss[i]->BSSIndex,ATTACK_CHALLENGE_REPLAY);
							}else{
								sta->wapi_sta_info.wapi_mib_stats.wapi_version = msg->u.StaWapi.StaWapiInfo[j].WAPIVersion;
								sta->wapi_sta_info.wapi_mib_stats.controlled_port_status = msg->u.StaWapi.StaWapiInfo[j].ControlledPortStatus;
								memcpy(sta->wapi_sta_info.wapi_mib_stats.selected_unicast_cipher, msg->u.StaWapi.StaWapiInfo[j].SelectedUnicastCipher,4);
								sta->wapi_sta_info.wapi_mib_stats.wpi_replay_counters = msg->u.StaWapi.StaWapiInfo[j].WPIReplayCounters;
								sta->wapi_sta_info.wapi_mib_stats.wpi_decryptable_errors = msg->u.StaWapi.StaWapiInfo[j].WPIDecryptableErrors;
								sta->wapi_sta_info.wapi_mib_stats.wpi_mic_errors = msg->u.StaWapi.StaWapiInfo[j].WPIMICErrors;
							}
						}
					}			
				}
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
			}
			break;
		//================================================
		//mahz add 2011.3.8
		case  EAG_MAC_AUTH:
			ret = AsdCheckWTPID(WTPID);
			if(ret == 0)
				return;
			num = ASD_SEARCH_WTP_STA(WTPID, bss);
			for(i = 0; i < num; i++){
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
				sta = ap_get_sta(bss[i], msg->u.STA.STAMAC);
				if(sta != NULL){
					if(ASD_WLAN[bss[i]->WlanID])
						SID = ASD_WLAN[bss[i]->WlanID]->SecurityID;
					if(ASD_SECURITY[SID]&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1)){
						sta->security_type = HYBRID_AUTH_PORTAL;
						ieee802_1x_free_alive(sta,&ASD_SECURITY[SID]->eap_alive_period);
						ieee802_1x_free_station(sta);
					}
					else if(ASD_SECURITY[SID]&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 0)){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive EAG_AUTH msg in MAC AUTH\n");
					}
				}
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
			}
			break;
		//================================================
		case EAG_MAC_DEL_AUTH:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive EAG_MAC_DEL_AUTH msg\n");
			break;
			//weichao add 2011.11.03	
		case STA_FLOW_CHECK:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in case STA_FLOW_CHECK\n");
			if(is_secondary == 1){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"it is bak now,return\n");
				return;
			}
			ret = AsdCheckWTPID(WTPID);
			if(ret == 0)
				return;
			num = ASD_SEARCH_WTP_STA(WTPID, bss);
			for(i = 0; i < num; i++){
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
				sta = ap_get_sta(bss[i], msg->u.STA.STAMAC);
				if(sta != NULL){
					sta->txbytes=msg->u.STA.rx_data_bytes;
                   			sta->rxbytes=msg->u.STA.tx_data_bytes;
                  			sta->txpackets = (unsigned long long)msg->u.STA.rx_frames;
                   			sta->rxpackets = (unsigned long long)msg->u.STA.tx_frames;
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->txbytes = %llu\n",sta->txbytes);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->rxbytes = %llu\n",sta->rxbytes);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->txpackets = %llu\n",sta->txpackets);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->rxpackets = %llu\n",sta->rxpackets);
				
					if(1 == check_sta_authorized(bss[i],sta))
						bss[i]->authorized_sta_num--;
					sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
					sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
					SID = (unsigned char)bss[i]->SecurityID;
					if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
						//Qiuchen add it for AXSSZFI-1732
						if(ASD_SECURITY[SID]->securityType == WPA2_E){
							pmk_sta = pmk_ap_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
							if(pmk_sta)
								pmk_ap_free_sta(ASD_WLAN[wasd->WlanID],pmk_sta);
						}
						//end
						wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
						mlme_deauthenticate_indication(
						bss[i], sta, 0);
						ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
					}
					if(ASD_NOTICE_STA_INFO_TO_PORTAL)
						AsdStaInfoToEAG(bss[i],sta,WID_DEL);
					char *SSID = NULL;
					u8 *identity = NULL;
					if(sta->eapol_sm)
						identity = sta->eapol_sm->identity;
					if(ASD_WLAN[bss[i]->WlanID])
						SSID = ASD_WLAN[bss[i]->WlanID]->ESSID;
					if(gASDLOGDEBUG & BIT(0)){
						if((ASD_SECURITY[SID])&& (ASD_SECURITY[SID]->securityType == OPEN || ASD_SECURITY[SID]->securityType == SHARED))
							asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MAC_ADDRESS" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(sta->addr),SSID,9);
						else if((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->securityType == WAPI_AUTH) && (ASD_SECURITY[SID]->wapi_radius_auth == 1))||(ASD_SECURITY[SID]->extensible_auth == 1)){
							asd_syslog_h(LOG_INFO,"WSTA","PORTSEC_DOT1X_LOGOFF:-IfName=GRadio-BSS%d:%d-MACAddr="MACSTR"-VlanId=%d-UserName=%s-ErrCode=%d; Session of the 802.1X user was terminated.\n",bss[i]->Radio_G_ID,bss[i]->BSSIndex%128,MAC2STR(sta->addr),sta->vlan_id,identity,1);
						}
					}
					u8 WTPMAC[6] = {0};
					if(ASD_WTP_AP[bss[i]->Radio_G_ID/4])
						memcpy(WTPMAC,ASD_WTP_AP[bss[i]->Radio_G_ID/4]->WTPMAC,6);
					if(gASDLOGDEBUG & BIT(1))
						syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]DISASSOC:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d, ErrorCode:%d.\n",
							slotid,vrrid,MAC2STR(sta->addr),MAC2STR(WTPMAC),bss[i]->BSSIndex,999);//qiuchen 2013.01.14
					if(ASD_WLAN[bss[i]->WlanID]!=NULL&&ASD_WLAN[bss[i]->WlanID]->balance_switch == 1&&ASD_WLAN[bss[i]->WlanID]->balance_method==1){
						ap_free_sta(bss[i], sta, 1);
					}
					else{				
						ap_free_sta(bss[i], sta, 0);
					}
					ieee802_11_send_deauth(bss[i], msg->u.STA.STAMAC, 4);
					{
						TableMsg STA;
						int len;
						STA.Op = WID_DEL;
						STA.Type = STA_TYPE;
						STA.u.STA.BSSIndex = bss[i]->BSSIndex;
						STA.u.STA.WTPID = ((bss[i]->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
						memcpy(STA.u.STA.STAMAC, msg->u.STA.STAMAC, ETH_ALEN);
						len = sizeof(STA);
						if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
							perror("send(wASDSocket)");
						}
						
					}
				}
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
			}
			break;	
				//weichao add 2011.11.11
			case  DHCP_IP:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"in sta case DHCP_IP\n");
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->ip = %d\n",msg->u.STA.ipv4Address);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSSIndex = %d\n",BSSIndex);
				asd_printf(ASD_DBUS,MSG_DEBUG,"MAC:"MACSTR"\n",MAC2STR(msg->u.STA.STAMAC));
				unsigned char *ip = (unsigned char *)&msg->u.STA.ipv4Address;
				pthread_mutex_lock(&(asd_g_sta_mutex)); 		
				wasd = AsdCheckBSSIndex(BSSIndex);
				if(wasd != NULL){
					sta = ap_get_sta(wasd, msg->u.STA.STAMAC);
					if(sta != NULL)
					{
						
						asd_printf(ASD_DBUS,MSG_DEBUG,"the sta is exist!\n\n");
					if(msg->u.STA.ipv4Address == 0){						
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg ip is null!\n");
						pthread_mutex_unlock(&(asd_g_sta_mutex));					
						break;
					}	
					else if(sta->ip_addr.s_addr == msg->u.STA.ipv4Address){						
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta has got IP from listen arp!\n");
						pthread_mutex_unlock(&(asd_g_sta_mutex));					
						break;
					}	
					else if((sta->ip_addr.s_addr != 0)&&(sta->ip_addr.s_addr != msg->u.STA.ipv4Address))
					{
							if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL_SUCCESS(sta)))
							{
								if(asd_ipset_switch)
									eap_connect_down(sta->ip_addr.s_addr);
								else
									AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
							}
							//qiuchen
							if(gASDLOGDEBUG & BIT(1)){
								unsigned char *ipold = (unsigned char *)&(sta->ipaddr);
								syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]STA_IP_UPDATE:UserMAC:" MACSTR "OLD_IP:%d.%d.%d.%d NEW_IP:%d.%d.%d.%d\n",
									slotid,vrrid,MAC2STR(sta->addr),ipold[0],ipold[1],ipold[2],ipold[3],ip[0],ip[1],ip[2],ip[3]);//qiuchen 2013.01.14
							}
					}
					else{
						//qiuchen
						time_t at = time(NULL);
						if(gASDLOGDEBUG & BIT(1))
							syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]STA_GET_ONLINE:UserMAC:" MACSTR "IP:%d.%d.%d.%d TIME:%s\n",
								slotid,vrrid,MAC2STR(sta->addr),ip[0],ip[1],ip[2],ip[3],ctime(&at));//qiuchen 2013.01.14
					}
					sta->ip_addr.s_addr = msg->u.STA.ipv4Address;
					sta->ipaddr = msg->u.STA.ipv4Address;
					memset(sta->in_addr,0,16);
					memcpy(sta->in_addr,inet_ntoa(sta->ip_addr),strlen(inet_ntoa(sta->ip_addr)));
					memcpy(sta->arpifname,msg->u.STA.arpifname,sizeof(msg->u.STA.arpifname));
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->arpifname:%s\n",sta->arpifname);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->in_addr "MACSTR"\n",MAC2STR(sta->in_addr));
					if(ASD_WLAN[wasd->WlanID] == NULL){
						pthread_mutex_unlock(&(asd_g_sta_mutex));					
						break ;
					}	
					SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
					
					if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)))
					{
							if((ASD_SECURITY[SID]->account_after_authorize == 0)&&(sta->flags &WLAN_STA_AUTHORIZED)){
								accounting_sta_start(wasd,sta);
							}
					}	
					if(is_secondary == 0)
						bak_update_sta_ip_info(wasd, sta);
					if( 0 != sta->ipaddr){
						 if((NO_NEED_AUTH == sta->security_type)||(HYBRID_AUTH_EAPOL_SUCCESS(sta))) 
						{
							if(asd_ipset_switch)
								eap_connect_up(sta->ip_addr.s_addr);
							else
								AsdStaInfoToEAG(wasd,sta,ASD_AUTH);
						}
						
					}
					}
					else
					{
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"In case DHCP_IP,the sta is not exit!!!!\n");
					}	
				}
				else	
				{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"In case DHCP_IP,the wasd is not exit!!!!\n");
				}
				pthread_mutex_unlock(&(asd_g_sta_mutex));					
			break;

			case STA_LEAVE_REPORT:
				asd_printf(ASD_LEAVE,MSG_DEBUG,"now in case STA_LEAVE_REPORT\n");
				WTPID = msg->u.STAINFO[0].WTPID;
				ret = AsdCheckWTPID(WTPID);
				if(ret == 0)
					return;
				char WlanID = 0;
				char RadioID = 0;
				WlanID = msg->u.STAINFO[0].wlanId;
				RadioID = msg->u.STAINFO[0].radioId;
				unsigned int radio_g_id = 0;
				radio_g_id = WTPID*4+RadioID;
				num = ASD_SEARCH_RADIO_STA(radio_g_id,bss);
				pthread_mutex_lock(&(asd_g_sta_mutex)); 		
				for( i= 0 ; i < num ; i++){
					if(bss[i]->WlanID == WlanID)
					{
						wasd = bss[i];
						break;
					}
				}
				if(wasd == NULL){

					asd_printf(ASD_DEFAULT,MSG_INFO,"in case sta_leave_report ,wasd is not found!\n");
					pthread_mutex_unlock(&(asd_g_sta_mutex)); 		
					break ;
				}
				count = msg->u.STAINFO[0].count;
				asd_printf(ASD_LEAVE,MSG_DEBUG,"count = %d\n",count);
				if(count == 255){
					asd_printf(ASD_LEAVE,MSG_DEBUG,"del all sta of wlan %d ,wtp %d ,radio %d.\n",WlanID,WTPID,RadioID);
					asd_free_stas(wasd);
					pthread_mutex_unlock(&(asd_g_sta_mutex)); 		
					break;					
				}
				STA_LEAVE_REASON  reason = 0;
				unsigned int  sub_reason = 0;
				unsigned int reasonhn = 1;//qiuchen
				if(count > 20){
					asd_printf(ASD_LEAVE,MSG_DEBUG,"count = %d, change to 20!\n",count);
					count = 20;
				}
				for(i = 0; (i< count)&&(i < 64)  ; i++){
					reason = msg->u.STAINFO[i].sta_reason;
					sub_reason = msg->u.STAINFO[i].sub_reason;
					if(reason == AC_KICK){
						if(sub_reason != VAP_DISABLE_DELETE)
							asd_printf(ASD_LEAVE,MSG_DEBUG,"sta"MACSTR" leave because of AC kick sta!\n",MAC2STR(msg->u.STAINFO[i].STAMAC));
					}

						sta = ap_get_sta(wasd, msg->u.STAINFO[i].STAMAC);
						if(sta != NULL){

							sta->txbytes=msg->u.STAINFO[i].rx_data_bytes;
							sta->rxbytes=msg->u.STAINFO[i].tx_data_bytes;
							sta->txpackets = (unsigned long long)msg->u.STAINFO[i].rx_frames;
							sta->rxpackets = (unsigned long long)msg->u.STAINFO[i].tx_frames;
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->u.STA.rx_data_bytes= %llu\n",msg->u.STAINFO[i].rx_data_bytes); 
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->u.STA.tx_data_bytes= %llu\n",msg->u.STAINFO[i].tx_data_bytes); 
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->u.STA.rx_data_frames= %u\n",msg->u.STAINFO[i].rx_frames);	
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->u.STA.tx_data_frames= %u\n",msg->u.STAINFO[i].tx_frames);	
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"	sta->txbytes= %llu\n",sta->txbytes); 
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"	sta->rxbytes= %llu\n",sta->rxbytes); 
						
							if(1 == check_sta_authorized(wasd,sta))
								wasd->authorized_sta_num--;
							sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);					
							sta->flags |= WLAN_STA_DEL ;					
							sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
							SID = (unsigned char)wasd->SecurityID;
							if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
								if(ASD_SECURITY[SID]->securityType == WPA2_E){
									pmk_sta = pmk_ap_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
									if(pmk_sta)
										pmk_ap_free_sta(ASD_WLAN[wasd->WlanID],pmk_sta);
								}
								wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
								mlme_deauthenticate_indication(
								wasd, sta, 0);
								ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
							}
							switch(reason){
								case AUTH_FRAME:
										if(AUTH_TO_ONE_WLAN == sub_reason)
											asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of recieved two or more auth frame one wlan!\n",MAC2STR(sta->addr));
										
										else if( AUTH_TO_WLAN==sub_reason)
											asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of recieved auth frame in other wlan!\n",MAC2STR(sta->addr));
									sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
									break;
								case AC_KICK:
									reasonhn = 255;
									if(sub_reason == VAP_DISABLE_DELETE)
										asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of interface ath is down !\n",MAC2STR(sta->addr));
									else if(sub_reason == SEND_BROADCAST_DEAUTH )
										asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of AC kick sta!(ap send deauth probe)\n",MAC2STR(sta->addr));
									sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
									break;
								case AP_KICK:
									reasonhn = 255;
									asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of AP kick sta!!\n",MAC2STR(sta->addr));
									if(sub_reason == STA_SIGNAL_WEAK)
										sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_LOST_CARRIER;
									else if(sub_reason == RETRANSMIT_TOO_MORE)
										sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_SESSION_TIMEOUT;
									else if(sub_reason == IDLE_TIME_OUT)
										sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
									break;
								case AP_STA_DEAUTH:
									asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of sta request(deauth frame) !\n",MAC2STR(sta->addr));
									sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
									break;
								case AP_STA_DISASSOC:
									asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of sta request(disassoc frame) !\n",MAC2STR(sta->addr));
									sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
									break;
								case WDS_CHANGE:
									asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of wds change(AP decide)!\n",MAC2STR(sta->addr));
									sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_HOST_REQUEST;
									break;
								case WTPD_REBOOT:
									asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of wtpd reboot (lost this sta)!\n",MAC2STR(sta->addr));
									sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_HOST_REQUEST;
									break;
								default:
									if(msg->u.STAINFO[i].authorize_failed)
										asd_printf(ASD_LEAVE,MSG_INFO,"sta : "MACSTR"leave because AP authorize error!",MAC2STR(sta->addr));
									else
										asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because no unknow reason!\n",MAC2STR(sta->addr));
									
									sta->acct_terminate_cause = 0;
						
							}
							
							char *SSID = NULL;
							u8 *identity = NULL;
							if(sta->eapol_sm)
								identity = sta->eapol_sm->identity;
							if(ASD_WLAN[wasd->WlanID])
								SSID = ASD_WLAN[wasd->WlanID]->ESSID;
							if(gASDLOGDEBUG & BIT(0)){
								if((ASD_SECURITY[SID])&& (ASD_SECURITY[SID]->securityType == OPEN || ASD_SECURITY[SID]->securityType == SHARED))
									asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MAC_ADDRESS" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(sta->addr),SSID,reasonhn);
								else if((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->securityType == WAPI_AUTH) && (ASD_SECURITY[SID]->wapi_radius_auth == 1))||(ASD_SECURITY[SID]->extensible_auth == 1)){
									asd_syslog_h(LOG_INFO,"WSTA","PORTSEC_DOT1X_LOGOFF:-IfName=GRadio-BSS%d:%d-MACAddr="MACSTR"-VlanId=%d-UserName=%s-ErrCode=%d; Session of the 802.1X user was terminated.\n",wasd->Radio_G_ID,wasd->BSSIndex%128,MAC2STR(sta->addr),sta->vlan_id,identity,1);
								}
							}
							u8 WTPMAC[6] = {0};
							if(ASD_WTP_AP[wasd->Radio_G_ID/4])
								memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,6);
							if(gASDLOGDEBUG & BIT(1))
								syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]DISASSOC:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d, ErrorCode:%d.\n",
									slotid,vrrid,MAC2STR(sta->addr),MAC2STR(WTPMAC),wasd->BSSIndex,(reason == AP_STA_DEAUTH || reason == AP_STA_DISASSOC)?0:999);//qiuchen 2013.01.14
							asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR"leave reason is %d\n",MAC2STR(sta->addr),reason);
							asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR"leave sub_reason is %d\n",MAC2STR(sta->addr),sub_reason);
							if((reason != AP_STA_DEAUTH)&&(reason != AP_STA_DISASSOC)){//qiuchen 
								wasd->abnormal_st_down_num ++;
								signal_sta_leave_abnormal(sta->addr,wasd->Radio_G_ID,wasd->BSSIndex,wasd->WlanID,sta->rssi);
							 	if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->securityType != OPEN)&&(sta->security_type == NO_NEED_AUTH)){
									wasd->no_auth_sta_abnormal_down_num ++;
								}
								else if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType != OPEN)||(ASD_SECURITY[SID]->extensible_auth == 1)))
								{
									wasd->assoc_auth_sta_abnormal_down_num++;
								}
								if((ASD_WLAN[wasd->WlanID]!=NULL)&&(ASD_WLAN[wasd->WlanID]->Roaming_Policy!=0)){
									struct ROAMING_STAINFO *r_sta;
									r_sta = roaming_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
									if(r_sta != NULL)
										roaming_free_sta(ASD_WLAN[wasd->WlanID],r_sta);
								}	
								//qiuchen
								wasd->statistics.sta_drop_cnt++;
								if (ASD_AUTH_TYPE_WEP_PSK(SID))
								{
									wasd->u.weppsk.total_sta_drop_cnt++;
								}
								else if (ASD_AUTH_TYPE_EAP(SID))
								{
									if (ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
									{
										wasd->u.eap_auth.autoauth.total_sta_drop_cnt++;
									}
									asd_printf(ASD_1X,MSG_DEBUG, "total_sta_drop_cnt is %d",
									wasd->u.eap_auth.autoauth.total_sta_drop_cnt);
								}
							}
							else							
								wasd->normal_st_down_num ++;
							if(ASD_NOTICE_STA_INFO_TO_PORTAL)
								AsdStaInfoToEAG(wasd,sta,WID_DEL);
							if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1){
								ap_free_sta(wasd, sta, 1);
							}
							else{
								ap_free_sta(wasd, sta, 0);
							}		
							
						}
						else{
							asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR"is not found in wlan %d bssindex %d g_radio %d\n",MAC2STR(msg->u.STAINFO[i].STAMAC),wasd->WlanID,wasd->BSSIndex,wasd->Radio_G_ID);
							}	
				}
				pthread_mutex_unlock(&(asd_g_sta_mutex));					
			break;
		case STA_CHECK_DEL:
			asd_printf(ASD_LEAVE,MSG_DEBUG,"now in case STA_CHECK_DEL\n");
			BSSIndex  = msg->u.STAINFO[0].BSSIndex;
			count = msg->u.STAINFO[0].count;
			authorize_num = msg->u.STAINFO[0].sta_num;
			pthread_mutex_lock(&(asd_g_sta_mutex));			
			wasd = AsdCheckBSSIndex(BSSIndex);
			if(wasd != NULL)
			{
				if(authorize_num != wasd->authorized_sta_num)
				{
					asd_printf(ASD_LEAVE,MSG_INFO,"wasd authorized sta num:%d and manage thread sta num: %d\n",wasd->authorized_sta_num,authorize_num);
					wasd->authorized_sta_num = authorize_num;
				}
				for(i = 0 ; (i < count)&&(i < 64) ;  i++)
				{
					sta = ap_get_sta(wasd,msg->u.STAINFO[i].STAMAC);
					if(sta)
					{						
						u8 WTPMAC[6] = {0};
						if(ASD_WTP_AP[wasd->Radio_G_ID/4])
							memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,6);
						if(gASDLOGDEBUG & BIT(1))
							syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]DISASSOC:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d, ErrorCode:%d.\n",
								slotid,vrrid,MAC2STR(sta->addr),MAC2STR(WTPMAC),wasd->BSSIndex,999);//qiuchen 2013.01.14
						wasd->abnormal_st_down_num ++;			
						signal_sta_leave_abnormal(sta->addr,wasd->Radio_G_ID,wasd->BSSIndex,wasd->WlanID,sta->rssi);
						if(1 == check_sta_authorized(wasd,sta))
							wasd->authorized_sta_num--;
						sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);					
						sta->flags |= WLAN_STA_DEL ;					
						sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
						SID = (unsigned char)wasd->SecurityID;
						if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
							if(ASD_SECURITY[SID]->securityType == WPA2_E){
								pmk_sta = pmk_ap_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
								if(pmk_sta)
									pmk_ap_free_sta(ASD_WLAN[wasd->WlanID],pmk_sta);
							}
							wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
							mlme_deauthenticate_indication(
							wasd, sta, 0);
							ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
						}
						asd_printf(ASD_LEAVE,MSG_DEBUG,"sta:"MACSTR" leave because of ap notice (sta is not in ap)!\n",MAC2STR(sta->addr));
						if(ASD_NOTICE_STA_INFO_TO_PORTAL)
							AsdStaInfoToEAG(wasd,sta,WID_DEL);
						if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1){
							ap_free_sta(wasd, sta, 1);
						}
						else{
							ap_free_sta(wasd, sta, 0);
						}		
						
					}
				}	
			}
			else
				asd_printf(ASD_LEAVE,MSG_DEBUG,"case STA_CHECK_DEL wasd is NULL(bssindex :%d)!\n",BSSIndex);
			pthread_mutex_unlock(&(asd_g_sta_mutex));
			break;				
		case WID_ADD:
		case RADIO_INFO:
		case WTP_DENEY_STA:
		case STA_COME:
		case STA_LEAVE:
		case VERIFY_INFO:
		case VERIFY_FAIL_INFO:
		case WTP_DE_DENEY_STA:
		case BSS_INFO:
		case ASSOC_FAIL_INFO:
		case JIANQUAN_FAIL_INFO:
		case CHANNEL_CHANGE_INFO:
		case WID_UPDATE:
		case WID_ONE_UPDATE:
		case TRAFFIC_LIMIT:
		case WIDS_INFO:
		case WIDS_SET:
		case WAPI_INVALID_CERT:
		case WAPI_CHALLENGE_REPLAY:
		case WAPI_MIC_JUGGLE:
		case WAPI_LOW_SAFE_LEVEL:
		case WAPI_ADDR_REDIRECTION:
		case OPEN_ROAM:
		case VRRP_IF:
		case CANCEL_TRAFFIC_LIMIT:
		case WTP_STA_CHECK:
		case WID_WIFI_INFO:
		case ASD_AUTH:
		case ASD_DEL_AUTH:
		case BSS_UPDATE:
		case ASD_MAC_AUTH:
		case ASD_MAC_DEL_AUTH:
		case IDLE_STA_DEL:
		case MAC_LIST_ADD:
		case RADIUS_STA_UPDATE:
		default:
			return;
	}

}

void BAK_OP(TableMsg *msg){
	//struct asd_data **bss;
	struct asd_data *wasd = NULL;
	//unsigned int num;
	unsigned int i;
	unsigned int j;
	unsigned int count = 0;
	//pthread_t ASD_UPDATE;
	struct sta_info *sta;
	switch(msg->Op){
		case WID_ADD:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"state change\n");
			if(msg->u.BAK.state==0){
				//primary  ac	
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"master!!!!!\n");
#ifndef ASD_MULTI_THREAD_MODE
				if(is_secondary == 1){
					#ifndef _AC_BAK_UDP_
					circle_unregister_read_sock(new_sock);
					#endif
					circle_unregister_read_sock(asd_sock);
					FD_CHANGE = 1;
				}
			//	circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
				circle_cancel_timeout(bak_update_bss_req,NULL,NULL);
#endif
				update_pass = 0;
				is_secondary=0;
				bak_unreach = 0;
				#ifndef _AC_BAK_UDP_
				close(new_sock);
				#endif				
				for( i = 0; i < WLAN_NUM ; i++)
				{
					if(!ASD_WLAN[i])
						continue;
					asd_wlan_radius_init(i,ASD_WLAN[i]->SecurityID);
				}
			//	circle_unregister_read_sock(asd_master_sock);
			//	close(asd_master_sock);
			}
			else if(msg->u.BAK.state==2){
				//primary  ac		
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"disable!!!!!\n");
				/*bss = malloc(BSS_NUM*sizeof(struct asd_data *));				
				num = ASD_SEARCH_ALL_STA(bss);
				for(i=0;i<num;i++){ 
					TableMsg msg;					
					struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
					msg.Op = WID_DEL;
					msg.u.BSS.BSSIndex = bss[i]->BSSIndex;
					BSS_OP(&msg,interfaces);
				}*/
#ifndef ASD_MULTI_THREAD_MODE
					#ifndef _AC_BAK_UDP_
				circle_unregister_read_sock(new_sock);
					#endif
				
				circle_unregister_read_sock(asd_sock);
					FD_CHANGE = 1;
			//	circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
				circle_cancel_timeout(bak_update_bss_req,NULL,NULL);
#endif
				update_pass = 0;
				is_secondary=2;
				bak_unreach = 0;
				#ifndef _AC_BAK_UDP_
				close(new_sock);
				#endif
				//circle_unregister_read_sock(asd_master_sock);
				//close(asd_master_sock);			
			}else if(msg->u.BAK.state==1){
				//secondary  ac
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"secondary!!!!!\n");
#ifdef ASD_MULTI_THREAD_MODE
				if(is_secondary != 1){
					is_secondary = 1;	
					update_pass = 0;					
					close(new_sock);
					close(asd_master_sock);
					vrrid = msg->u.BAK.vrrid;					
					pthread_attr_t attr;
					size_t ss;	
					int s = PTHREAD_CREATE_DETACHED;
					pthread_attr_init(&attr);
					pthread_attr_setdetachstate(&attr,s);
					if(pthread_create(&ASD_BAK, &attr, asd_bak_thread, NULL) != 0) {
						asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
						return ;
					}
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_BAK %d\n",ASD_BAK);
				}
#else				
				if(is_secondary != 1){
					is_secondary = 1;	
					update_pass = 0;
					bak_unreach = 0;
					circle_unregister_read_sock(new_sock);					
					circle_unregister_read_sock(asd_sock);
					FD_CHANGE = 1;
				//	circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
					circle_register_timeout(120, 0, bak_update_bss_req, NULL, NULL);//qiuchen change it
					#ifndef _AC_BAK_UDP_
					close(new_sock);
					#endif
					//circle_unregister_read_sock(asd_master_sock);
					//close(asd_master_sock);
					//vrrid = msg->u.BAK.vrrid;	
					if(first == 1){
						if (circle_register_read_sock(asd_sock, asd_bak_select_mode, NULL, NULL))
						{
							asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
							return ;
						}
						first = 1;
					}
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_BAK %lu\n",ASD_BAK);
				}
#endif
			}	
			break;
		}
		case WID_UPDATE:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"update sta info\n");
			/*if(is_secondary == 0){				
				B_addr.sin_family = AF_INET;	
				B_addr.sin_port = htons(29527);	
				B_addr.sin_addr.s_addr = msg->u.BAK.virip;	
				memset(B_addr.sin_zero, '\0', sizeof(B_addr.sin_zero));				
				bss = malloc(BSS_NUM*sizeof(struct asd_data *));				
				num = ASD_SEARCH_ALL_STA(bss);
				for(i=0;i<num;i++){				
					sta = bss[i]->sta_list;
					for(j = 0; j < bss[i]->num_sta; j++){
						bak_add_sta(bss[i],sta);
						sta = sta->next;
					}				
				}
				update_end();
				free(bss);
			}*/
			if(is_secondary == 0){
#ifdef ASD_MULTI_THREAD_MODE				
				if(update_pass == 0){
					update_pass = 1;
					close(new_sock);
					close(asd_master_sock);					
					B_addr.sin_family = AF_INET;	
					B_addr.sin_port = htons(ASD_BAK_AC_PORT+local*MAX_INSTANCE+vrrid);	
					B_addr.sin_addr.s_addr = msg->u.BAK.virip;	
					memset(B_addr.sin_zero, '\0', sizeof(B_addr.sin_zero));				
					pthread_attr_t attr;
					size_t ss;	
					int s = PTHREAD_CREATE_DETACHED;
					pthread_attr_init(&attr);
					pthread_attr_setdetachstate(&attr,s);
					if(pthread_create(&ASD_UPDATE, &attr, asd_update_thread, NULL) != 0) {
						return ;
					}
				}
#else				
		//		circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
				bak_unreach = 0;
				update_pass = 1;
				#ifndef _AC_BAK_UDP_				
				close(new_sock);
				#endif
				circle_unregister_read_sock(asd_master_sock);
				FD_CHANGE = 1;
				//close(asd_master_sock);	
				if(local){
					struct sockaddr_tipc *tipcaddr = (struct sockaddr_tipc *)&B_addr;
					tipcaddr->family = AF_TIPC;
					tipcaddr->addrtype = TIPC_ADDR_NAME;
					tipcaddr->addr.name.name.type = SYN_ASD_SERVER_TYPE;
					tipcaddr->addr.name.name.instance = SYN_SERVER_BASE_INST + vrrid*MAX_SLOT_NUM + msg->u.BAK.neighbor_slotid;
					tipcaddr->addr.name.domain = 0; 			
				}else{
					struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
					inaddr->sin_family = AF_INET;	
					inaddr->sin_port = htons(ASD_BAK_AC_PORT+local*MAX_INSTANCE+vrrid);	
					inaddr->sin_addr.s_addr = msg->u.BAK.virip;	
					memset(inaddr->sin_zero, '\0', sizeof(inaddr->sin_zero));	
				}			
		//		circle_register_timeout(0, 0,asd_update_select_mode, NULL, NULL);	
					asd_update_batch_sta();
					asd_update_radius_state();//qiuchen
					circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL);
					int h_times =0;
						while(h_times < 3){
							if(-1 == notice_hmd_update_state_change((unsigned int)vrrid,0)){
								h_times ++;
								asd_printf(ASD_DEFAULT,MSG_INFO,"%s,%d,h_times%d,sleep(2),send again.\n",__func__,__LINE__,h_times);
								sleep(2);
							}else{
								h_times = 4;
								asd_printf(ASD_DEFAULT,MSG_INFO,"%s,%d,h_times%d,send successfull,break.\n",__func__,__LINE__,h_times);
								break;
							}
						}
						asd_printf(ASD_DEFAULT,MSG_INFO,"%s,%d,send to hmd.\n",__func__,__LINE__);
#endif
			}
			break;
		}
		case BSS_UPDATE:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"update bss_sta info\n");
			if(is_secondary == 0){
				count = msg->u.BAK.bss_count;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"**********count from WID is %d,func %s,line %d*******\n", count,__func__,__LINE__);
				for(i=0; i<count&&count<=4096; i++){

					if((msg->u.BAK.bssindex[i] < L_RADIO_NUM*L_BSS_NUM)
						||(msg->u.BAK.bssindex[i] > G_RADIO_NUM*L_BSS_NUM)){
						asd_printf(ASD_DEFAULT,MSG_WARNING,"the value is error! bssindex = %d\n", msg->u.BAK.bssindex[i]);
						continue;
					}

					wasd = AsdCheckBSSIndex(msg->u.BAK.bssindex[i]);				
					if(wasd == NULL){
						continue;
					}	
					unsigned char sid = wasd->SecurityID;
					if((ASD_SECURITY[sid]== NULL)||((ASD_SECURITY[sid]!= NULL)&&(ASD_SECURITY[sid]->securityType != OPEN)))
						continue;
					sta = wasd->sta_list;
					for(j = 0; (j < wasd->num_sta)&&(sta!=NULL); j++){
						bak_add_sta(wasd,sta);
						sta = sta->next;
					}				
				}
			}
			break;
		}
		case WID_ONE_UPDATE:{
			if(is_secondary == 0){
				wasd = AsdCheckBSSIndex(msg->u.BAK.BSSIndex);				
				if(wasd == NULL){
					asd_printf(ASD_DBUS,MSG_DEBUG,"wasd is null.\n");
					break;
				}	
				unsigned char sid = wasd->SecurityID;
				if((ASD_SECURITY[sid]== NULL)||((ASD_SECURITY[sid]!= NULL)&&(ASD_SECURITY[sid]->securityType != OPEN)))
					break;
				sta = wasd->sta_list;
				for(j = 0; (j < wasd->num_sta)&&(sta!=NULL); j++){
					bak_add_sta(wasd,sta);
					sta = sta->next;
				}				
			}
			break;
		}
		default:
			break;
	}
}

void WIDS_OP(TableMsg *msg){
	//struct sta_info *sta = NULL;
	struct acl_config *conf = NULL;
	struct maclist *list = NULL;
	wids_info *info = NULL;
	wids_set *set = NULL;
	unsigned char wlanid = 0;
	int i = 0;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"WIDS_OP ");
	switch(msg->Op){
		case WIDS_INFO:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"WIDS_INFO\n");
			info = &(msg->u.WIDS_info);
			if(ASD_BSS[info->bssindex] == NULL)		//mahz add 2011.4.2
				break;
			wlanid = ASD_BSS[info->bssindex]->WlanID;
			if((ASD_WLAN[wlanid] != NULL) && (ASD_WLAN[wlanid]->acl_conf != NULL)
				&& (wids_enable == 1) &&(ASD_WLAN[wlanid]->acl_conf->macaddr_acl == 1)) {
				conf = ASD_WLAN[wlanid]->acl_conf;
				if(add_mac_in_blacklist_for_wids(conf,&list,info) != 0) {
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"add mac failed\n");
					break;
				}
			}
			circle_register_timeout(wids_mac_last_time, 0, del_wids_mac_timer, conf, list);
			break;
		}
		case WIDS_SET:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"WIDS_SET\n");
			set = &(msg->u.WIDS_set);
			if(set->able == 1) {
				wids_enable = 1;
				wids_mac_last_time = set->lasttime;
				for(i=1; i< WLAN_NUM; i++){
					if((ASD_WLAN[i] == NULL) || (ASD_WLAN[i]->acl_conf == NULL)	) {
						continue;
					}else {
						ASD_WLAN[i]->acl_conf->wlan_last_macaddr_acl = ASD_WLAN[i]->acl_conf->macaddr_acl;
						ASD_WLAN[i]->acl_conf->macaddr_acl = 1;
					}
				}
			}else {
				wids_enable = 0;
				wids_mac_last_time = 0;
				clean_mac_in_blacklist_for_wids();
			}
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"wids_enable:%d\n",wids_enable);
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"wids_mac_last_time:%d\n",wids_mac_last_time);
			break;
		}
		default:
			break;
	}
}



void BSS_TRAFFIC_LIMIT_OP(TableMsg *msg){
/*xm0723*/

	struct asd_data *wasd=NULL; 
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);
	printf("%s\n",__func__);
	switch(msg->Op){
		case TRAFFIC_LIMIT:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s	type TRAFFIC_LIMIT\n",__func__);

			if((msg->u.traffic_limit.bssindex<L_RADIO_NUM*L_BSS_NUM)
				||(msg->u.traffic_limit.bssindex>G_RADIO_NUM*L_BSS_NUM)){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s	receive bssindex %u\n",__func__,
					msg->u.traffic_limit.bssindex);
				return;
			}

			wasd=AsdCheckBSSIndex(msg->u.traffic_limit.bssindex);

			if(wasd==NULL){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s	wasd not found.\n",__func__);
				return;
			}
			
			if(msg->u.traffic_limit.able==0){
				wasd->sta_average_traffic_limit=0;
				wasd->sta_average_send_traffic_limit=0;
				update_sta_traffic_limit_info(wasd,1);
			} else{
				wasd->traffic_limit=msg->u.traffic_limit.value;
				wasd->sta_average_traffic_limit=msg->u.traffic_limit.average_value;
				wasd->send_traffic_limit=msg->u.traffic_limit.send_value;
				wasd->sta_average_send_traffic_limit=msg->u.traffic_limit.send_average_value;
				if(msg->u.traffic_limit.cancel_average_flag == 1)   //fengwenchao add for AXSSZFI-1374
					cancel_sta_traffic_limit_average_info(wasd,0);
				else
					update_sta_traffic_limit_info(wasd,0);
			}
			break;
		}
		default:
			break;
			
	}
}

void STA_IP_ARP_OP(TableMsg *msg){	
	struct asd_data *bss[L_BSS_NUM];
	struct sta_info *sta = NULL;
	unsigned int WTPID = msg->u.STA.WTPID;
	unsigned int num = 0, i = 0;
	unsigned char ret;
	unsigned char WLANID = msg->u.STA.wlanId;
	unsigned int RadioID = msg->u.STA.WTPID*L_RADIO_NUM + msg->u.STA.radioId;
	unsigned int BSSIndex = 0;
	unsigned char *ip;
	char ip_buf[128];
	char mac_buf[128];
	memset(ip_buf, 0, 128);	
	memset(mac_buf, 0, 128);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA_IP_ARP_OP type %u(ADD--0,DEL--1,MODIFY--2)\n",msg->Op);
	ret = AsdCheckWTPID(WTPID);
	if(ret == 0)
		return;
	num = ASD_SEARCH_RADIO_STA(RadioID, bss);
	for(i = 0; i < num; i++){
		if(bss[i]->WlanID == WLANID){			
			sta = ap_get_sta(bss[i], msg->u.STA.STAMAC);
			BSSIndex = bss[i]->BSSIndex;
			break;
		}
	}
	if(sta == NULL)
		return;
	ip = (unsigned char *)&(msg->u.STA.ipv4Address);
	sprintf(ip_buf,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
	sprintf(mac_buf,"%02X:%02X:%02X:%02X:%02X:%02X",msg->u.STA.STAMAC[0],msg->u.STA.STAMAC[1],msg->u.STA.STAMAC[2],msg->u.STA.STAMAC[3],msg->u.STA.STAMAC[4],msg->u.STA.STAMAC[5]);	
	switch(msg->Op){
		case WID_ADD:
		case WID_MODIFY:
			memset(sta->in_addr, 0, 16);
			strcpy(sta->in_addr, ip_buf);
			//mahz
			inet_aton(sta->in_addr,&sta->ip_addr);	//
			if((ASD_BSS[BSSIndex])&&(ASD_BSS[BSSIndex]->sta_static_arp_policy == 1)){
				ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,ip_buf, mac_buf,ASD_BSS[BSSIndex]->arp_ifname);
			}
			break;
		case WID_DEL:
			memset(sta->in_addr, 0, 16);
			if((ASD_BSS[BSSIndex])&&(ASD_BSS[BSSIndex]->sta_static_arp_policy == 1)){
				ipneigh_modify(RTM_DELNEIGH, 0,ip_buf, mac_buf,ASD_BSS[BSSIndex]->arp_ifname);
			}
			break;			
		case STA_INFO:
		case STA_WAPI_INFO:
		case RADIO_INFO:
		case WTP_DENEY_STA:
		case STA_COME:
		case STA_LEAVE:
		case VERIFY_INFO:
		case VERIFY_FAIL_INFO:
		case WTP_DE_DENEY_STA:
		case BSS_INFO:
		case ASSOC_FAIL_INFO:
		case JIANQUAN_FAIL_INFO:
		case CHANNEL_CHANGE_INFO:
		case WID_UPDATE:
		case WID_CONFLICT:
		case WID_ONE_UPDATE:
		case TRAFFIC_LIMIT:
		case WIDS_INFO:
		case WIDS_SET:
		case WAPI_INVALID_CERT:
		case WAPI_CHALLENGE_REPLAY:
		case WAPI_MIC_JUGGLE:
		case WAPI_LOW_SAFE_LEVEL:
		case WAPI_ADDR_REDIRECTION:
		case OPEN_ROAM:
		case VRRP_IF:
		case CANCEL_TRAFFIC_LIMIT:
		case WTP_STA_CHECK:
		case WID_WIFI_INFO:
		case ASD_AUTH:
		case ASD_DEL_AUTH:
		case EAG_AUTH:
		case EAG_DEL_AUTH:
		case BSS_UPDATE:
		case EAG_MAC_AUTH:
		case EAG_MAC_DEL_AUTH:
		case ASD_MAC_AUTH:
		case ASD_MAC_DEL_AUTH:
		case STA_FLOW_CHECK:	
		case DHCP_IP:
		case IDLE_STA_DEL:
		case MAC_LIST_ADD:
		case RADIUS_STA_UPDATE:
		case STA_LEAVE_REPORT:
		case EAG_NTF_ASD_STA_INFO:
		case STA_CHECK_DEL:
		case STA_WTP_TERMINAL_STATISTICS:
			break;
	}
}
void EAG_OP(EagMsg *msg)
{
	if(!msg)
		return;
	unsigned int wtpid = msg->STA.wtp_id;
	unsigned int ret = 0;
	unsigned int num = 0;
	struct asd_data *bss[L_RADIO_NUM*L_BSS_NUM] = {0};
	struct sta_info *sta = NULL;
	unsigned char SID = 0;
	unsigned int i = 0;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"EAG_OP type %u\n",msg->Op);
	switch(msg->Op)
	{
		case  EAG_MAC_AUTH:
			ret = AsdCheckWTPID(wtpid);
			if(ret == 0)
				return;
			num = ASD_SEARCH_WTP_STA(wtpid, bss);
			for(i = 0; i < num; i++){
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
				sta = ap_get_sta(bss[i], msg->STA.addr);
				if(sta != NULL){
					if(ASD_WLAN[bss[i]->WlanID])
						SID = ASD_WLAN[bss[i]->WlanID]->SecurityID;
					if(ASD_SECURITY[SID]&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1)){
						sta->security_type = HYBRID_AUTH_PORTAL;
						ieee802_1x_free_alive(sta,&ASD_SECURITY[SID]->eap_alive_period);
						ieee802_1x_free_station(sta);
					}
					else if(ASD_SECURITY[SID]&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 0)){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive EAG_AUTH msg in MAC AUTH\n");
					}
				}
				
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
			}
			break;
		case EAG_MAC_DEL_AUTH:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive EAG_MAC_DEL_AUTH msg\n");
			break;
		case EAG_AUTH:
			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive EAG_AUTH msg\n");
			ret = AsdCheckWTPID(wtpid);
			if(ret == 0)
				return;
			num = ASD_SEARCH_WTP_STA(wtpid, bss);
			for(i = 0; i < num; i++){
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
				sta = ap_get_sta(bss[i], msg->STA.addr);
				if(sta != NULL){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta != NULL\n");
					sta->portal_auth_success = 1;
					
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->portal_auth_suc = %d\n",sta->portal_auth_success);

				}
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
			}
			break;
		case EAG_DEL_AUTH:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive EAG_DEL_AUTH msg\n");
				ret = AsdCheckWTPID(wtpid);
				if(ret == 0)
					return;
				num = ASD_SEARCH_WTP_STA(wtpid, bss);
				for(i = 0; i < num; i++){
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
					sta = ap_get_sta(bss[i], msg->STA.addr);
					if(sta != NULL){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"EAG_DEL_AUTH,del sta "MACSTR"\n",MAC2STR(sta->addr));
						sta->portal_auth_success = 0;

					}
#ifdef ASD_USE_PERBSS_LOCK
					pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
				}
		
			break;
		case EAG_NTF_ASD_STA_INFO:		
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"receive EAG_NTF_ASD_STA_INFO msg\n");
			ret = AsdCheckWTPID(wtpid);
			if(ret == 0)
				return;
			num = ASD_SEARCH_WTP_STA(wtpid, bss);
			for(i = 0; i < num; i++){
#ifdef ASD_USE_PERBSS_LOCK
			pthread_mutex_lock(&(bss[i]->asd_sta_mutex));
#endif
			pthread_mutex_lock(&(asd_g_sta_mutex)); 		
			if(bss[i] != NULL)
				sta = ap_get_sta(bss[i], msg->STA.addr);
			if(sta != NULL){
				if(0 == msg->STA.ipaddr)
				{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"eag has not give the ip!\n");
					pthread_mutex_unlock(&(asd_g_sta_mutex));		
					return;
				}
				if(sta->ipaddr != 0)
				{
					 if(sta->ipaddr != msg->STA.ipaddr)
					{
						 if((NO_NEED_AUTH == sta->security_type)||(HYBRID_AUTH_EAPOL_SUCCESS(sta)))
						 {
							 if(asd_ipset_switch)
								 eap_connect_down(sta->ip_addr.s_addr);
							 else
							 	AsdStaInfoToEAG(bss[i],sta,ASD_DEL_AUTH);
						 }
					}
				}
				sta->ip_addr.s_addr = msg->STA.ipaddr;
				sta->ipaddr = msg->STA.ipaddr;
				memset(sta->in_addr,0,16);
				memcpy(sta->in_addr,inet_ntoa(sta->ip_addr),strlen(inet_ntoa(sta->ip_addr)));
				memcpy(sta->arpifname,msg->STA.arpifname,sizeof(msg->STA.arpifname));
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->arpifname:%s\n",sta->arpifname);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->in_addr %s\n",sta->in_addr);
				
				SID = (unsigned char)ASD_WLAN[bss[i]->WlanID]->SecurityID;
				
				if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)))
				{
						if((ASD_SECURITY[SID]->account_after_authorize == 0)&&(sta->flags &WLAN_STA_AUTHORIZED)){
							accounting_sta_start(bss[i],sta);
						}
				}	
				
				if(is_secondary == 0)
					bak_update_sta_ip_info(bss[i], sta);
				
				if(sta->ipaddr != 0)
				{
					if((NO_NEED_AUTH == sta->security_type)||(HYBRID_AUTH_EAPOL_SUCCESS(sta)))
					{			
						if(asd_ipset_switch)
							eap_connect_up(sta->ip_addr.s_addr);
						else
							AsdStaInfoToEAG(bss[i],sta,ASD_AUTH);
					}
					
				}
			}
			pthread_mutex_unlock(&(asd_g_sta_mutex));					
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(bss[i]->asd_sta_mutex));
#endif
			}
				
		break;
		default:
			asd_printf(ASD_DEFAULT,MSG_INFO,"eag send unknow eag op:%d\n",msg->Op);
			return;
	}
}
int asd_neighborap_sta_info(unsigned int wtpindex,unsigned int n_wtpid,unsigned char mac[6],unsigned int stanum)
{
	TableMsg N_WTP;
	int len;
	if(is_secondary == 1){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"it is bak now,return\n");
		return 0;
	}
	
	N_WTP.Op = WTP_STA_CHECK;
	N_WTP.Type = NEIGHBOR_AP_STA_CHECK;
	N_WTP.u.WTP.WtpID = wtpindex;
	N_WTP.u.WTP.N_WTP.N_WtpID = n_wtpid;
	N_WTP.u.WTP.N_WTP.cur_sta_num = stanum;
	memset(N_WTP.u.WTP.N_WTP.WTPMAC,0,MAC_LEN);
	memcpy(N_WTP.u.WTP.N_WTP.WTPMAC,mac,MAC_LEN);
	len = sizeof(N_WTP);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"rogue_terminal,%s\n",__func__);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"neighbor ap sta info sendto begin\n");
	if(sendto(TableSend, &N_WTP, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		asd_printf(ASD_DEFAULT,MSG_WARNING,"%s sendto %s\n",__func__,strerror(errno));
		perror("send(wASDSocket)");
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss_kick_sta2\n");
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"neighbor ap sta info sendto end\n");
	
	return 0;
}

void ASD_SEARCH_NEIGHBORAP_STA(TableMsg *msg){	
	unsigned int C_WTPID = msg->u.WTP.WtpID;
	unsigned int N_WTPID = msg->u.WTP.N_WTP.N_WtpID;
	unsigned int num = 0;
	unsigned char ret;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"rogue_terminal,%s\n",__func__);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"rogue_terminal,WTP_STA_CHECK type %u(check--WTP_STA_CHECK)\n",msg->Op);
	ret = AsdCheckWTPID(N_WTPID);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"rogue_terminal,ret = %d,N_WTPID = %d\n",ret,N_WTPID);
	if(ret == 0)
		return;
	num = ASD_SEARCH_WTP_STA_NUM(N_WTPID);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"rogue_terminal,stanum=%d,%s\n",num,__func__);
	switch(msg->Op){
		case WTP_STA_CHECK:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"rogue_terminal,in case WTP_STA_CHECK.\n");
			asd_neighborap_sta_info(C_WTPID,N_WTPID,msg->u.WTP.N_WTP.WTPMAC,num);
			break;
		default:
			break;			
	}
}

static void handle_buf(struct wasd_interfaces *interfaces, u8 *buf, size_t len)
{
	TableMsg *msg = (TableMsg*)buf;
	EagMsg *msg1 = NULL;
	STAStatisticsMsg *smsg;
	int num,datalen;
	switch(msg->Type){
		
		case WLAN_TYPE :{			
			pthread_mutex_lock(&asd_g_wlan_mutex);
			WLAN_OP(msg);			
			pthread_mutex_unlock(&asd_g_wlan_mutex);
			break;
		}
		case WTP_TYPE :{
			
			WTP_OP(msg);
			break;
		}/*
		case RADIO_TYPE :{
			
			RADIO_OP(msg);
			break;
		}*/
		case STA_TYPE :{
			STA_OP(msg);
			break;
		}
		case BSS_TYPE :{
			
			BSS_OP(msg, interfaces);
			break;
		}
		case STA_PKT_TYPE:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA_PKT_TYPE\n");
			smsg = (STAStatisticsMsg*)buf;
			num = smsg->cnt;
			datalen = len - sizeof(STAStatisticsMsg);
			//handle_smsg((char *)(smsg+1),num,datalen);
			break;
		}
		case BAK_TYPE:{
			BAK_OP(msg);
			break;
		}
		case BSS_TRAFFIC_LIMIT_TYPE :{
			BSS_TRAFFIC_LIMIT_OP(msg); /*xm0723*/
			break;
		}
		case WIDS_TYPE:{
			WIDS_OP(msg);
			break;
		}
		case AP_REPORT_STA_TYPE:{
			STA_IP_ARP_OP(msg);
			break;
		}
		case NEIGHBOR_AP_STA_CHECK:{
			ASD_SEARCH_NEIGHBORAP_STA(msg);
			break;
		}
		case EAG_TYPE:
			msg1 = (EagMsg*)buf;
			EAG_OP(msg1);
		default:
			break;
	}
	return;
}
#ifndef ASD_MULTI_THREAD_MODE 

void handle_msg(int sock, void *circle_ctx, void *sock_ctx)
{
	
	int len;
	unsigned char buf[65535];
	memset(buf, 0, 65535);
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	len = recvfrom(TableSock, buf, sizeof(buf), 0, NULL, 0);
	if (len < 0) {
		perror("recv");
		return;
	}
	
	handle_buf(interfaces, buf, len);
}
#else
	static void handle_msg()
	{
		while(1){
			int len;
			unsigned char buf[65535];
			memset(buf, 0, 65535);
			len = recvfrom(TableSock, buf, sizeof(buf), 0, NULL, 0);
			if (len < 0) {
				perror("recv");
				return;
			}	
			struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
			handle_buf(interfaces, buf, len);
		}
	}
#endif

#ifdef ASD_MULTI_THREAD_MODE
static void handle_msg1()
{
	struct sockaddr_un local;
	int sock;
	int len;
	char *GWSM_PATH = "/var/run/wcpss/get_wsm_info";		
	sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
	   perror("socket[PF_UNIX,PF_UNIX]");
	   asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
	   exit(1);
	}
	local.sun_family = PF_UNIX;
	strcpy(local.sun_path,GWSM_PATH);
	unlink(local.sun_path);    
	len = strlen(local.sun_path) + sizeof(local.sun_family);	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"Trying to bind sock path [%s].\n",local.sun_path);   
	
	if (bind(sock, (struct sockaddr *) &local, len) < 0) {
		perror("bind(PF_UNIX)");
		return -1;
	}

	while(1){
		int len;
		unsigned char buf[4096];
		memset(buf, 0, 4096);
		len = recvfrom(sock, buf, sizeof(buf), 0, NULL, 0);
		if (len < 0) {
			perror("recv");
			return;
		}	
		struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
		handle_buf(interfaces, buf, len);
	}
}

static void free_loop(int sock, void *circle_ctx, void *sock_ctx)
{
	
	int len;
	unsigned char buf[4096];
	len = recvfrom(sock, buf, sizeof(buf), 0, NULL, 0);
	if (len < 0) {
		perror("recv");
		return;
	}
	if(strcmp(buf,"hello") == 0){
		printf("register timer\n");
		return;
	}
}
#endif
void CMD_SECURITY_OP(ASDCmdMsg *msg)
{
	unsigned char wlan_id = 0;
	unsigned char SID = 0;
	int ret = 0;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"CMD_SECURITY_OP\n");
	switch(msg->Op){
		case ASD_CMD_APPLY_SECURITY:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_CMD_APPLY_SECURITY\n");
			wlan_id = msg->u.secinfo.wlanid;
			SID = msg->u.secinfo.SID;
			pthread_mutex_lock(&asd_g_wlan_mutex);
			ret = asd_wlan_radius_init(wlan_id,SID);
			if(ASD_WLAN[wlan_id]->SecurityID != 0){
			
				ASD_WLAN[wlan_id]->OldSecurityIndex =  ASD_WLAN[wlan_id]->SecurityIndex;	
				ASD_WLAN[wlan_id]->OldSecurityIndex_flag = ASD_WLAN[wlan_id]->NowSecurityIndex_flag;
			}
			
			if(((ASD_SECURITY[SID]->securityType == OPEN)||(ASD_SECURITY[SID]->securityType == SHARED))&&(ASD_SECURITY[SID]->encryptionType == WEP))
			{
				ASD_WLAN[wlan_id]->NowSecurityIndex_flag = 1;
			}
			else
			{
				ASD_WLAN[wlan_id]->NowSecurityIndex_flag = 0;
			}
			
			ASD_WLAN[wlan_id]->SecurityID = SID;
			ASD_WLAN[wlan_id]->SecurityIndex = ASD_SECURITY[SID]->index;  //fengwenchao add 20110310 for autelan-2200
			ASD_WLAN[wlan_id]->ap_max_inactivity = ASD_SECURITY[SID]->ap_max_inactivity;//weichao add 
			asd_printf(ASD_DBUS,MSG_DEBUG,"ASD_WLAN_INF_OP\n");
			if(!ret && ASD_WLAN_INF_OP(wlan_id, SID, WID_MODIFY))
				asd_printf(ASD_DBUS,MSG_DEBUG,"update wlan security type\n");
			pthread_mutex_unlock(&asd_g_wlan_mutex);
			break;
		default:
			break;
	}
}

void CMD_STA_OP(ASDCmdMsg *msg){	
	switch(msg->Op){
		case ASD_CMD_DEL:
			pthread_mutex_lock(&asd_g_sta_mutex); 			
			unsigned char mac[MAC_LEN] = {0};
			struct sta_info *sta=NULL;
			int bss_num=0;
			int i=0;			
			struct asd_data **bss = NULL;
			bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
			if( bss == NULL){
				asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
				return ;
			}
			memcpy(mac, msg->u.stainfo.MAC, MAC_LEN);
			bss_num= ASD_SEARCH_BSS_HAS_STA(bss);
			for(i=0;i<bss_num;i++){
				sta = ap_get_sta(bss[i], mac);
				if(sta != NULL){
					if(1 == check_sta_authorized(bss[i],sta))
						bss[i]->authorized_sta_num--;
					sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
					bss[i]->normal_st_down_num++;						//mahz add 2011.1.14
					sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
					unsigned char SID = (unsigned char)bss[i]->SecurityID;
					if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->extensible_auth == 1)&&(ASD_SECURITY[SID]->hybrid_auth == 0))){
						wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
						mlme_deauthenticate_indication(
						bss[i], sta, 0);
						ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
					}
					char *SSID = NULL;
					if(ASD_WLAN[bss[i]->WlanID])
						SSID = ASD_WLAN[bss[i]->WlanID]->ESSID;
					if(gASDLOGDEBUG & BIT(0)){
						asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MAC_ADDRESS" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(sta->addr),SSID,255);//idletime out
					}
					if(ASD_NOTICE_STA_INFO_TO_PORTAL)
						AsdStaInfoToEAG(bss[i],sta,WID_DEL);
					if(ASD_WLAN[bss[i]->WlanID]!=NULL&&ASD_WLAN[bss[i]->WlanID]->balance_switch == 1&&ASD_WLAN[bss[i]->WlanID]->balance_method==1){
						ap_free_sta(bss[i], sta, 1);
					}
					else{				
						ap_free_sta(bss[i], sta, 0);
					}
					ieee802_11_send_deauth(bss[i], mac, 3);
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
							asd_printf(ASD_DBUS,MSG_DEBUG,"222222222\n");
						}
					}
				}		
			}
			if(bss)
			{
				free(bss);
				bss = NULL;
			}
			pthread_mutex_unlock(&asd_g_sta_mutex);			
			break;
		default:
			break;
	}

}
void CMD_FD_OP(ASDCmdMsg *msg){
	int len;
	int sndbuf = SOCK_BUFSIZE;
	unsigned char socketfd = msg->u.fdinit.socketfd;
	unsigned char operate = msg->u.fdinit.operate; 
	unsigned int ret = ASD_DBUS_SUCCESS;
	struct sockaddr_un local;
	char AWSM_PATH[PATH_LEN] = "/var/run/wcpss/asd_table";	
	char AWSM_PATHWSM[PATH_LEN] = "/var/run/wcpss/asd";	
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
			else if(socketfd == 5){
				circle_unregister_read_sock(nl_sock);
				close(nl_sock);
				netlink_flag = 1;
				asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close nl_sock : %d\n",nl_sock);
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
			else  if((socketfd == 5)&&(netlink_flag ==1)){
				ASD_NETLINIK_INIT();
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
			else if (5 == socketfd )
			{
				circle_unregister_read_sock(nl_sock);
				close(nl_sock);
				asd_printf(ASD_DBUS,MSG_DEBUG,"successfully close nl_sock : %d\n",nl_sock);

				ASD_NETLINIK_INIT();
			}
		}

}
void CMD_ARP_LISTEN_OP(ASDCmdMsg *msg){
	switch(msg->Op){
		case ASD_CMD_ADD:
			rtnl_wilddump_request(&rth1, AF_INET, RTM_GETNEIGH);
			circle_register_read_sock(rth1.fd, do_asd_sta_arp_listen, NULL, NULL);
			asd_sta_arp_listen = msg->u.arplisten.type;
			break;
		case ASD_CMD_DEL:
			circle_unregister_read_sock(rth1.fd);
			asd_sta_arp_listen = msg->u.arplisten.type;	
			break;
		default:
			break;
	}

}

void handle_cmd(int sock, void *circle_ctx, void *sock_ctx)
{
	int len;
	unsigned char buf[65535];
	memset(buf, 0, 65535);
	len = recvfrom(CmdSock, buf, sizeof(buf), 0, NULL, 0);
	if (len < 0) {
		perror("recv");
		return;
	}
	ASDCmdMsg *msg = (ASDCmdMsg*)buf;

	switch(msg->Type){
		case ASD_STA_TYPE :
			CMD_STA_OP(msg);
			break;
		case ASD_FD_TYPE :
			CMD_FD_OP(msg);
			break;
		case ASD_ARP_LISTEN:
			CMD_ARP_LISTEN_OP(msg);
			break;
		case ASD_SECURITY_TYPE:
			CMD_SECURITY_OP(msg);
			break;
		default:
			break;
	}
	return;
}



int asd_aWid_socket_init(){
	 struct sockaddr_un local;
	 char AWSM_PATH[PATH_LEN] = "/var/run/wcpss/asd_table";	 
	 char WSM_TABLE_PATH[PATH_LEN] = "/var/run/wcpss/wsm_table_socket";
	 char WID_PATH[PATH_LEN] = "/var/run/wcpss/wid";
	 char CHILL_PATH[PATH_LEN] = "/var/run/wcpss/portal_sta_us";
	 char DHCP_PATH[PATH_LEN] = "/var/run/dhcpsnp.unix";
	 int len;
	 int sock;
	// int ret;
	// int rcvbuf = SOCK_BUFSIZE;
#ifdef ASD_MULTI_THREAD_MODE	 
	 pthread_t ASD_TABLE;
#endif
	 InitPath(vrrid,AWSM_PATH);
	 InitPath(vrrid,WSM_TABLE_PATH);
	 InitPath(vrrid,WID_PATH);
	 InitPath(vrrid,CHILL_PATH);
	 sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	 TableSock = sock;	 
	 toWSM.addr.sun_family = PF_UNIX;
	 strcpy(toWSM.addr.sun_path,WSM_TABLE_PATH);
	 toWSM.addrlen = strlen(toWSM.addr.sun_path) + sizeof(toWSM.addr.sun_family);
	 
	 toWID.addr.sun_family = PF_UNIX;
	 strcpy(toWID.addr.sun_path,WID_PATH);
	 toWID.addrlen = strlen(toWID.addr.sun_path) + sizeof(toWID.addr.sun_family);

	 toEAG.addr.sun_family = PF_UNIX;
	 strcpy(toEAG.addr.sun_path,CHILL_PATH);
	 toEAG.addrlen = strlen(toEAG.addr.sun_path) + sizeof(toEAG.addr.sun_family);
	 
	 toDHCP.addr.sun_family = PF_UNIX;
	 strcpy(toDHCP.addr.sun_path,DHCP_PATH);
	 toDHCP.addrlen = strlen(toDHCP.addr.sun_path)+sizeof(toDHCP.addr.sun_family);
	memset(&toFASTFWD,0,sizeof(toFASTFWD));
	 toFASTFWD.addr.family = AF_TIPC;
	 toFASTFWD.addr.addrtype = TIPC_ADDR_NAME;
	 toFASTFWD.addr.addr.name.name.type = SE_AGENT_SERVER_TYPE;
	 toFASTFWD.addr.addr.name.name.instance = SE_AGENT_SERVER_LOWER_INST+slotid;
	 toFASTFWD.addr.addr.name.domain = 0;
	 toFASTFWD.addr.scope = TIPC_CLUSTER_SCOPE;
	 toFASTFWD.addrlen = sizeof(toFASTFWD.addr);
	 /*if ((ret = setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf))) < 0) {
		 asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not set sock RCVBUF\n");
		 close(sock);
		 return -1;
	 }*/

	 if (sock < 0) {
		perror("socket[PF_UNIX,PF_UNIX]");
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	 }
	 local.sun_family = PF_UNIX;
 	 strcpy(local.sun_path,AWSM_PATH);
	 unlink(local.sun_path);	
 	 len = strlen(local.sun_path) + sizeof(local.sun_family);	 
	 asd_printf(ASD_DEFAULT,MSG_DEBUG,"Trying to bind sock path [%s].\n",local.sun_path);	

	 if (bind(sock, (struct sockaddr *) &local, len) < 0) {
		 perror("bind(PF_UNIX)");
		 return -1;
	 }
	 asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_aWid_socket_init\n");
#ifndef ASD_MULTI_THREAD_MODE
	 if (circle_register_read_sock(sock, handle_msg, NULL, NULL))
	 {
		 asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
		 return -1;
	 }
#else	 
	 if(pthread_create(&ASD_TABLE, NULL, handle_msg, NULL) != 0) {
		 asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
		 return -1;
	 }
	 
	 if(pthread_create(&ASD_TABLE, NULL, handle_msg1, NULL) != 0) {
		 asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
		 return -1;
	 }
#endif
	 return sock;

}

static int asd_init_sockets_cmd(void)
{
	struct sockaddr_un local;
	char ASD_CMD_PATH[PATH_LEN] = "/var/run/wcpss/asd_cmd";	
	int len;
	int sndbuf = SOCK_BUFSIZE;
	InitPath(vrrid,ASD_CMD_PATH);
	CmdSock = socket(PF_UNIX, SOCK_DGRAM, 0);
	CmdSock_s = socket(PF_UNIX, SOCK_DGRAM, 0);	
	if ((setsockopt(CmdSock_s,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
		perror("setsockopt");
		return -1;
	}
	fcntl(CmdSock_s, F_SETFL, O_NONBLOCK);	
	toASD_C.addr.sun_family = PF_UNIX;
	strcpy(toASD_C.addr.sun_path,ASD_CMD_PATH);
	toASD_C.addrlen = strlen(toASD_C.addr.sun_path) + sizeof(toASD_C.addr.sun_family);
	if (CmdSock < 0 ) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	}
	
	local.sun_family = PF_UNIX;
	strcpy(local.sun_path,ASD_CMD_PATH);
	unlink(local.sun_path);	
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(CmdSock, (struct sockaddr *) &local, len) < 0) {
		perror("bind");
		return -1;
	}
	if (circle_register_read_sock(CmdSock, handle_cmd, NULL, NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_WARNING,"asd_init_sockets_cmd Could not register read socket\n");
		return -1;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_init_sockets_cmd finish\n");
	return 0;
}


static int asd_init_sockets(struct wid_driver_data *drv)
{
	struct ifreq ifr;
	//struct sockaddr_ll addr;
	//struct sockaddr_un remote;
	struct sockaddr_un local;
	char AWSM_PATH[PATH_LEN] = "/var/run/wcpss/asd";	
	char WSM_DATA_PATH[PATH_LEN] = "/var/run/wcpss/wsm_data_socket";
	int len;
#ifdef ASD_MULTI_THREAD_MODE
	pthread_t ASD_DATA;
#endif
	int s;
	//int ret,macs;
	//int rcvbuf = SOCK_BUFSIZE;	
	InitPath(vrrid,AWSM_PATH);
	InitPath(vrrid,WSM_DATA_PATH);
	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	toWSM_D.addr.sun_family = PF_UNIX;
	strcpy(toWSM_D.addr.sun_path,WSM_DATA_PATH);
	toWSM_D.addrlen = strlen(toWSM_D.addr.sun_path) + sizeof(toWSM_D.addr.sun_family);
	if (s < 0 ) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	}
	
	/*if ((ret = setsockopt(s,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf))) < 0) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not set sock RCVBUF\n");
		close(s);
		return -1;
	}*/
	local.sun_family = PF_UNIX;
	strcpy(local.sun_path,AWSM_PATH);
	unlink(local.sun_path);	
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *) &local, len) < 0) {
		perror("bind");
		close(s);
		s = -1;
		return -1;
	}
	drv->sock = s;
	DataSock = s;
#ifndef ASD_MULTI_THREAD_MODE
	if (circle_register_read_sock(drv->sock, handle_read, NULL, NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
		return -1;
	}
#else
		if(pthread_create(&ASD_DATA, NULL, handle_read, NULL) != 0) {
			asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
			return -1;
		}
#endif
        memset(&ifr, 0, sizeof(ifr));
        snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%sap", drv->iface);

/*	if (asd_set_iface_flags(drv, 1)) {
		return -1;
	}
*/
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_init_sockets finish\n");
	return 0;
}

/*
  *ht add 090927
  */
#ifdef ASD_MULTI_THREAD_MODE
static int asd_init_loop_socket()
{
	struct sockaddr_un local;
	char ASD_LOOP_PATH[PATH_LEN] = "/var/run/wcpss/asd_loop";	
	int len,t;
	int sock;
	int ret;
	int rcvbuf = SOCK_BUFSIZE;
	InitPath(vrrid,ASD_LOOP_PATH);
	ASD_LOOP.addr.sun_family = PF_UNIX;
	strcpy(ASD_LOOP.addr.sun_path,ASD_LOOP_PATH);
	ASD_LOOP.addrlen = strlen(ASD_LOOP.addr.sun_path) + sizeof(ASD_LOOP.addr.sun_family);

	sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0 ) {
		perror("socket[PF_UNIX,LOOP_SOCK]");
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	}
	/*if ((ret = setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf))) < 0) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not set sock RCVBUF\n");
		close(sock);
		return -1;
	}*/
	local.sun_family = PF_UNIX;
	strcpy(local.sun_path,ASD_LOOP_PATH);
	unlink(local.sun_path);	
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(sock, (struct sockaddr *) &local, len) < 0) {
		perror("bind");
		return -1;
	}

	if (circle_register_read_sock(sock, free_loop, NULL, NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
		return -1;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_init_sockets finish\n");
	return sock;
}
#endif
int asd_send_mgmt_frame_w(void *priv, const void *msg, size_t len,
				  int flags)
{
	//struct wid_driver_data *drv = priv;	
	int tlen = 0;
//	int s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if(ASD_SWITCH == 0){
		tlen = sendto(DataSend, msg, len, flags, (struct sockaddr *)&toWSM_D.addr, toWSM_D.addrlen);
	}else{
		/*ht add ,send to wifi,110303*/
		netlink_sendto_wifi(priv, msg, len, flags);
	}
//	close(s);
	return tlen;
}


static int asd_send_eapol_w(void *priv, const u8 *addr, const u8 *data,
			     size_t data_len, int encrypt, const u8 *own_addr)
{
	struct asd_data *wasd = priv;
	struct wid_driver_data *drv = wasd->drv_priv;
	struct ieee80211_hdr *hdr;
	size_t len;
	u8 *pos;
	int res;

	wasd->info->tx_auth_pkts++;
	len = sizeof(*hdr) + sizeof(rfc1042_header) + 2 + data_len;
	hdr = os_zalloc(len);
	if (hdr == NULL) {
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}

	hdr->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
	hdr->frame_control |= host_to_le16(WLAN_FC_FROMDS);
	/* Request TX callback */
	hdr->frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
	if (encrypt)
		hdr->frame_control |= host_to_le16(WLAN_FC_ISWEP);
	memcpy(hdr->IEEE80211_DA_FROMDS, addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_BSSID_FROMDS, own_addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_SA_FROMDS, own_addr, ETH_ALEN);
	hdr->seq_ctrl = seq_to_le16(wasd->seq_num++);
	if(wasd->seq_num == 4096)
		wasd->seq_num = 0;

	pos = (u8 *) (hdr + 1);
	memcpy(pos, rfc1042_header, sizeof(rfc1042_header));
	pos += sizeof(rfc1042_header);
	*((u16 *) pos) = htons(ETH_P_PAE);
	pos += 2;
	memcpy(pos, data, data_len);
	
	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, (u8 *) hdr, len, IEEE802_11_EAP);
//	CWCaptrue(msg.DataLen, msg.Data);
	res = asd_send_mgmt_frame_w(drv, &msg, msglen, 0);
	os_free(hdr);
	hdr=NULL;

	if (res < 0) {
		perror("asd_send_eapol_w: send");
		asd_printf(ASD_80211,MSG_WARNING,"asd_send_eapol_w - packet len: %lu - failed\n",
		       (unsigned long) len);
	}

	return res;
}

int asd_send_wapi(void *priv, const u8 *addr, const u8 *data,
			     size_t data_len, int encrypt, const u8 *own_addr)
{
	struct asd_data *wasd = priv;
	struct wid_driver_data *drv = wasd->drv_priv;
	struct ieee80211_hdr *hdr;
	size_t len;
	u8 *pos;
	int res;

	len = sizeof(*hdr) + sizeof(rfc1042_header) + 2 + data_len;
	hdr = os_zalloc(len);
	if (hdr == NULL) {
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}

	hdr->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
	hdr->frame_control |= host_to_le16(WLAN_FC_FROMDS);
	/* Request TX callback */
	hdr->frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
	if (encrypt)
		hdr->frame_control |= host_to_le16(WLAN_FC_ISWEP);
	memcpy(hdr->IEEE80211_DA_FROMDS, addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_BSSID_FROMDS, own_addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_SA_FROMDS, own_addr, ETH_ALEN);
	hdr->seq_ctrl = seq_to_le16(wasd->seq_num++);
	if(wasd->seq_num == 4096)
		wasd->seq_num = 0;

	pos = (u8 *) (hdr + 1);
	memcpy(pos, rfc1042_header, sizeof(rfc1042_header));
	pos += sizeof(rfc1042_header);
	*((u16 *) pos) = htons(ETH_P_WAPI);
	pos += 2;
	memcpy(pos, data, data_len);
	
	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, (u8 *) hdr, len, IEEE802_11_EAP);
//	CWCaptrue(msg.DataLen, msg.Data);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"send wapi\n");
	res = asd_send_mgmt_frame_w(drv, &msg, msglen, 0);
	os_free(hdr);
	hdr=NULL;

	if (res < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"send wapi error\n");
		perror("asd_send_eapol_w: send");
		asd_printf(ASD_DEFAULT,MSG_WARNING,"asd_send_eapol_w - packet len: %lu - failed\n",
		       (unsigned long) len);
	}

	return res;
}



static void * asd_init_w(struct asd_data *wasd)
{
	struct wid_driver_data *drv;
	unixTipc local_tipc;
	int sock;
	int sndbuf = SOCK_BUFSIZE;
	asd_printf(ASD_DEFAULT,MSG_INFO,"ASD init\n");
	drv = os_zalloc(sizeof(struct wid_driver_data));
	if (drv == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not allocate memory for asd driver data\n");
		return NULL;
	}
	TableSend = socket(PF_UNIX, SOCK_DGRAM, 0);
	DataSend = socket(PF_UNIX, SOCK_DGRAM, 0);
	TipcSend = socket(AF_TIPC,SOCK_DGRAM,0);
	if((TableSend < 0) || (DataSend < 0)||(TipcSend < 0)){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	}
	local_tipc.addr.family = AF_TIPC;
	local_tipc.addr.addrtype = TIPC_ADDR_NAME;
	local_tipc.addr.addr.name.name.type = SYN_ASD_SERVER_TYPE;
	local_tipc.addr.addr.name.name.instance =  SYN_SERVER_BASE_INST + vrrid*MAX_SLOT_NUM + slotid;;
	local_tipc.addr.addr.name.domain = 0;
	local_tipc.addr.scope = TIPC_CLUSTER_SCOPE;
	local_tipc.addrlen = sizeof(local_tipc.addr);
		
	if (bind(TipcSend, (struct sockaddr *)&local_tipc, sizeof(local_tipc)) < 0)
	{
		perror("bind(PF_TIPC)");
		os_free(drv);
		drv=NULL;
		return NULL;
	}
	if ((setsockopt(TableSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
		perror("setsockopt");
		os_free(drv);
		drv=NULL;
		return NULL;
	}
	if ((setsockopt(DataSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
		perror("setsockopt");
		os_free(drv);
		drv=NULL;
		return NULL;
	}
	if ((setsockopt(TipcSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
		perror("setsockopt");
		os_free(drv);
		drv=NULL;
		return NULL;
	}
	fcntl(TableSend, F_SETFL, O_NONBLOCK);
	fcntl(DataSend, F_SETFL, O_NONBLOCK);
	fcntl(TipcSend, F_SETFL, O_NONBLOCK);
	sock = -1;
	drv->ioctl_sock = drv->sock = -1;
#ifdef ASD_MULTI_THREAD_MODE
	LoopSend = socket(PF_UNIX, SOCK_DGRAM, 0);
	if(LoopSend < 0){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s socket create failed\n",__func__);
		exit(1);
	}
	if ((setsockopt(LoopSend,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
		perror("setsockopt");
		return -1;
	}
	fcntl(LoopSend, F_SETFL, O_NONBLOCK);
	if((sock = asd_init_loop_socket()) < 0){
		asd_printf(ASD_DEFAULT,MSG_INFO,"ASD init loop socket failed\n");
		return NULL;
	}	
#endif
	if (asd_init_sockets(drv)) {
		os_free(drv);
		drv=NULL;
		return NULL;
	}
	asd_init_sockets_cmd();
	radius_client_dm_init();

	drv->ioctl_sock = asd_aWid_socket_init();
	if(drv->ioctl_sock < 0){
		close(drv->sock);
		os_free(drv);
		drv=NULL;
		return NULL;
	}

	return drv;
}
//reinit the read socket
void asd_sock_reinit(int fd,void *handler,void *circle_data, void *user_data)
{

	asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s\n",__func__);
	if(handler ==handle_netlink)
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"handler == handle_netlink!\n");
		circle_unregister_read_sock(nl_sock);
		FD_CHANGE = 1;
		close(nl_sock);
		ASD_NETLINIK_INIT();
		asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s,netlink sock  repair!\n",__func__);
	}
	else if((handler ==do_asd_sta_arp_listen)&&(1 == asd_sta_arp_listen))
	{
		circle_unregister_read_sock(rth1.fd);
		FD_CHANGE = 1;
		if(rtnl_wilddump_request(&rth1, AF_INET, RTM_GETNEIGH) < 0)
		{
			asd_sta_arp_listen = 0;
			asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s,do_asd_sta_arp_listen sock  repair error!!\n",__func__);
		}
		else
		{
			circle_register_read_sock(rth1.fd, do_asd_sta_arp_listen, NULL, NULL);
			asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s,do_asd_sta_arp_listen sock  repair!\n",__func__);
		}
			
	}
	else	
		asd_printf(ASD_DEFAULT,MSG_INFO,"sock %d is error in poll!",fd);
}


static void asd_driver_deinit_w(void *priv)
{
	struct wid_driver_data *drv = priv;

//	(void) asd_set_iface_flags(drv, 0);
//	(void) asd_ioctl_prism2param(drv, PRISM2_PARAM_asd, 0);
//	(void) asd_ioctl_prism2param(drv, PRISM2_PARAM_asd_STA, 0);

	if (drv->ioctl_sock >= 0)
		close(drv->ioctl_sock);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"         drv->sock :%d\n",drv->sock);
	if (drv->sock >= 0)
		close(drv->sock);

	os_free(drv->generic_ie);
	drv->generic_ie=NULL;
	os_free(drv);
	drv=NULL;
}

#if 0
static int
asd_del_key(void *priv, const u8 *addr, int key_idx)
{
	if(is_secondary == 1){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"it is bak now,return\n");
		return 0;
	}

	struct asd_data *wasd = priv;
	int ret, len;
	TableMsg key;
	key.Op = WID_DEL;
	key.Type = SKEY_TYPE;
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: addr=%s key_idx=%d",
		   __func__, addr, key_idx);

	memset(&key, 0, sizeof(key));
	if (addr != NULL) {
		memcpy(key.u.KEY.StaAddr, addr, MAC_LEN);
		key.u.KEY.key_idx = ((u8) -1);
	} else {
		key.u.KEY.key_idx = key_idx;
	}

	key.u.KEY.BSSIndex = wasd->BSSIndex;
	key.u.KEY.WTPID = wasd->Radio_G_ID/L_RADIO_NUM;
	
	len = sizeof(key);
	ret = (sendto(TableSend, &key, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0);
	if(ret){
		perror("sendto(TableSock)");
		return -1;
	}
	
	if (ret < 0) {
		asd_printf(ASD_DEFAULT,MSG_WARNING, "%s: Failed to delete key (addr %s"
			   " key_idx %d)", __func__, addr,
			   key_idx);
	}

	return ret;
}
#endif

static int
asd_set_key_w(const char *ifname, void *priv, const char *alg,
		const u8 *addr, int key_idx,
		const u8 *key, size_t key_len, int txkey)
{
	struct asd_data *wasd = priv;
	TableMsg SKey;
	int ret, len;

	if(is_secondary == 1){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"it is bak now,return\n");
		return 0;
	}
//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\n\n\n\n\n\n asd_set_key_w \n\n\n\n\n");
	if (strcmp(alg, "none") == 0)
		return 0;
//		return asd_del_key(wasd, addr, key_idx);

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: alg=%s addr=%s key_idx=%d",
		   __func__, alg, addr, key_idx);
	memset(&SKey, 0, sizeof(SKey));
	SKey.Op = WID_ADD;
	SKey.Type = SKEY_TYPE;
//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"type %d  SKEY_TYPE  %d\n",SKey.Type,SKEY_TYPE);
	if (strcmp(alg, "WEP") == 0)
		SKey.u.KEY.cipher = WID_WPA_CIPHER_WEP40;
	else if (strcmp(alg, "TKIP") == 0)
		SKey.u.KEY.cipher = WID_WPA_CIPHER_TKIP;
	else if (strcmp(alg, "CCMP") == 0)
		SKey.u.KEY.cipher = WID_WPA_CIPHER_CCMP;
	else if(strcmp(alg, "SMS4") == 0)		
		SKey.u.KEY.cipher = WID_WAPI_CIPHER_SMS4;
	else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: unknown/unsupported algorithm %s\n",
			__func__, alg);
		return -1;
	}
	asd_printf(ASD_DEFAULT,MSG_INFO,"SKey.u.KEY.cipher %d\n",SKey.u.KEY.cipher);
	if (addr == NULL) {
		memset(SKey.u.KEY.StaAddr, 0xff, MAC_LEN);
		SKey.u.KEY.key_idx = key_idx;
	} else {
		memcpy(SKey.u.KEY.StaAddr, addr, MAC_LEN);
		SKey.u.KEY.key_idx = ((u8) -1);
	}
	SKey.u.KEY.key_len = key_len;
	memcpy(SKey.u.KEY.key, key, key_len);
	
	SKey.u.KEY.BSSIndex = wasd->BSSIndex;
	SKey.u.KEY.WTPID = ((wasd->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
	
	len = sizeof(SKey);
//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\n\n\n\n key send: type:%d key_idx:%d key_len:%d key:%02x %02x %02x %02x %02x\n\n\n\n",
//		SKey.Type,SKey.u.KEY.key_idx,SKey.u.KEY.key_len,SKey.u.KEY.key[0],SKey.u.KEY.key[1],SKey.u.KEY.key[2],SKey.u.KEY.key[3],SKey.u.KEY.key[4]);
	ret = (sendto(TableSend, &SKey, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0);
	if(ret){
		perror("sendto(TableSock)");
		return -1;
	}

	if (ret < 0) {
		asd_printf(ASD_DEFAULT,MSG_WARNING, "%s: Failed to set key (addr %s"
			   " key_idx %d alg '%s' key_len %lu txkey %d)",
			   __func__, SKey.u.KEY.StaAddr, key_idx,
			   alg, (unsigned long) key_len, txkey);
	}

	return ret;
}

static int
asd_get_seqnum_w(const char *ifname, void *priv, const u8 *addr, int idx,
		   u8 *seq)
{
	return 0;
}

static int
asd_set_opt_ie_w(const char *ifname, void *priv, const u8 *ie, size_t ie_len)
{
	return 0;
}

static int
asd_sta_set_flags_w(void *priv, const u8 *addr, int total_flags,
		      int flags_or, int flags_and)
{
	struct asd_data *wasd = priv;
	/* For now, only support setting Authorized flag */
	if (flags_or & WLAN_STA_AUTHORIZED)
		return AsdStaInfoToWID(wasd, addr, WID_ADD);
	if ((!(flags_and & WLAN_STA_AUTHORIZED)) && (!(total_flags & WLAN_STA_DEL)))
		return AsdStaInfoToWID(wasd, addr, WID_DEL);
	return 0;
}

static int asd_sta_remove_w(void *priv, const u8 *addr)
{
	struct asd_data *wasd = priv;
	UpdateStaInfoToWSM(wasd, addr, WID_DEL);
	return 0;
}


static int asd_sta_add_w(const char *ifname, void *priv, const u8 *addr,
			  u16 aid, u16 capability, u8 *supp_rates,
			  size_t supp_rates_len, int flags)
{	
	struct asd_data *wasd = priv;
	struct ROAMING_STAINFO * sta = NULL;
	unsigned char WLANID = wasd->WlanID;
	if((ASD_WLAN[WLANID]!=NULL)&&(ASD_WLAN[WLANID]->Roaming_Policy != 0)){
		asd_printf(ASD_80211,MSG_DEBUG,"AsdRoamingStaInfoAdd\n");
		sta = AsdRoamingStaInfoAdd(wasd,addr);
	}
	if((ASD_WLAN[WLANID] != NULL)&&(ASD_WLAN[WLANID]->AC_Roaming_Policy != 0))
	{
		ACGroupRoamingCheck(WLANID,wasd,addr);
	}else{
		UpdateStaInfoToWSM(wasd, addr, WID_ADD);	
	}
	if((ASD_WLAN[WLANID]!=NULL)&&(ASD_WLAN[WLANID]->Roaming_Policy != 0)){
		if((sta != NULL)&&(sta->PreBssIndex !=  wasd->BSSIndex)){
			asd_printf(ASD_80211,MSG_DEBUG,"send to wsm modify\n");
			RoamingStaInfoToWSM(sta,WID_MODIFY);
		}
		if((sta!=NULL)&&(sta->need_notice_wifi)){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"send to wifi add!\n");
			RoamingStaInfoToWIFI(sta,WID_ADD);
			sta->need_notice_wifi = 0;
		}
	}
	return 0;
}

static int asd_sta_deauth_w(void *priv, const u8 *addr, int reason_code)
{
	struct asd_data *wasd = priv;
	AsdStaInfoToWID(wasd, addr, WID_DEL);
	return 0;
}

static int asd_sta_disassoc_w(void *priv, const u8 *addr, int reason_code)
{
	struct asd_data *wasd = priv;
	AsdStaInfoToWID(wasd, addr, WID_DEL);
	return 0;
}

const struct wpa_driver_ops wpa_driver_asd_ops = {
	.name = "asd",
	.init = asd_init_w,
	.deinit = asd_driver_deinit_w,
	.wireless_event_init = NULL,
	.wireless_event_deinit = NULL,
	.set_ieee8021x = NULL,
	.set_privacy = NULL,
	.set_encryption = asd_set_key_w,
	.get_seqnum = asd_get_seqnum_w,
	.flush = NULL,
	.set_generic_elem = asd_set_opt_ie_w,
	.read_sta_data = NULL,
	.send_eapol = asd_send_eapol_w,
	.sta_set_flags = asd_sta_set_flags_w,
	.sta_deauth = asd_sta_deauth_w,
	.sta_disassoc = asd_sta_disassoc_w,
	.sta_remove = asd_sta_remove_w,
	.set_ssid = NULL,
	.send_mgmt_frame = asd_send_mgmt_frame_w,
	.set_assoc_ap = NULL,
	.sta_add = asd_sta_add_w,
	.get_inact_sec = NULL,
	.sta_clear_stats = NULL,
	.get_hw_feature_data = asd_get_hw_feature_data_w,
};
void ASD_STA_OPT_STR(int Op,char *msgtype)
{
	switch(Op){
        case WID_DEL:
			strcpy(msgtype,"WID_DEL");
			break;
		case WID_MODIFY:
			strcpy(msgtype,"WID_MODIFY");
			break;
		case STA_INFO:
			strcpy(msgtype,"STA_INFO");
			break;
		case WID_CONFLICT :
            strcpy(msgtype,"WID_CONFLICT");
			break;
        case STA_WAPI_INFO:
			strcpy(msgtype,"STA_WAPI_INFO");
			break;
		case  EAG_MAC_AUTH:	
			strcpy(msgtype,"EAG_MAC_AUTH");
			break;
		case EAG_MAC_DEL_AUTH:
			strcpy(msgtype,"EAG_MAC_DEL_AUTH");
			break;
		case STA_FLOW_CHECK:
			strcpy(msgtype,"STA_FLOW_CHECK");
			break;
        case EAG_AUTH:
	   	    strcpy(msgtype,"EAG_AUTH");
			break;
	    case EAG_DEL_AUTH:
		    strcpy(msgtype,"EAG_DEL_AUTH");
			break;
		case  DHCP_IP:	
		    strcpy(msgtype,"DHCP_IP");
			break;
		case STA_LEAVE_REPORT:
            strcpy(msgtype,"STA_LEAVE_REPORT");
			break;    
		case EAG_NTF_ASD_STA_INFO:
			strcpy(msgtype,"EAG_NTF_ASD_STA_INFO");
			break;
		case STA_CHECK_DEL:
			strcpy(msgtype,"STA_CHECK_DEL");
			break;
		case WID_ADD:
		case RADIO_INFO:
		case WTP_DENEY_STA:
		case STA_COME:
		case STA_LEAVE:
		case VERIFY_INFO:
		case VERIFY_FAIL_INFO:
		case WTP_DE_DENEY_STA:
		case BSS_INFO:
		case ASSOC_FAIL_INFO:
		case JIANQUAN_FAIL_INFO:
		case CHANNEL_CHANGE_INFO:
		case WID_UPDATE:
		case WID_ONE_UPDATE:
		case TRAFFIC_LIMIT:
		case WIDS_INFO:
		case WIDS_SET:
		case WAPI_INVALID_CERT:
		case WAPI_CHALLENGE_REPLAY:
		case WAPI_MIC_JUGGLE:
		case WAPI_LOW_SAFE_LEVEL:
		case WAPI_ADDR_REDIRECTION:
		case OPEN_ROAM:
		case VRRP_IF:
		case CANCEL_TRAFFIC_LIMIT:
		case WTP_STA_CHECK:
		case WID_WIFI_INFO:
		case ASD_AUTH:
		case ASD_DEL_AUTH:
		case BSS_UPDATE:
		case ASD_MAC_AUTH:
		case ASD_MAC_DEL_AUTH:
		case IDLE_STA_DEL:
		case MAC_LIST_ADD:
		case RADIUS_STA_UPDATE:		
        default:
			break;
	}
}

