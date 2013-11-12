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
* ASDCallback_wired.c
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

#ifdef USE_KERNEL_HEADERS
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#include <linux/if_arp.h>
#include <linux/if.h>
#else /* USE_KERNEL_HEADERS */
#include <net/if_arp.h>
#include <net/if.h>
#include <netpacket/packet.h>
#endif /* USE_KERNEL_HEADERS */

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
#include "ASDMlme.h"
unixAddr toNPD_C;
unixAddr toNPD_D;


struct wired_driver_data {
	struct asd_data *wasd;

	int sock; /* raw packet socket for driver access */
	int ctl_sock;
	int dhcp_sock; /* socket for dhcp packets */
	int use_pae_group_addr;
};


#define WIRED_EAPOL_MULTICAST_GROUP	{0x01,0x80,0xc2,0x00,0x00,0x03}


/* TODO: detecting new devices should eventually be changed from using DHCP
 * snooping to trigger on any packet from a new layer 2 MAC address, e.g.,
 * based on ebtables, etc. */

struct dhcp_message {
	u_int8_t op;
	u_int8_t htype;
	u_int8_t hlen;
	u_int8_t hops;
	u_int32_t xid;
	u_int16_t secs;
	u_int16_t flags;
	u_int32_t ciaddr;
	u_int32_t yiaddr;
	u_int32_t siaddr;
	u_int32_t giaddr;
	u_int8_t chaddr[16];
	u_int8_t sname[64];
	u_int8_t file[128];
	u_int32_t cookie;
	u_int8_t options[308]; /* 312 - cookie */
};


static void wired_possible_new_sta(struct asd_data *wasd, u8 *addr)
{
	struct sta_info *sta;

	sta = ap_get_sta(wasd, addr);
	if (sta)
		return;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Data frame from unknown STA " MACSTR
		   " - adding a new STA", MAC2STR(addr));
	sta = ap_sta_add(wasd, addr, 1);
	if (sta) {
		asd_new_assoc_sta(wasd, sta, 0);
		accounting_sta_get_id(wasd, sta);
	} else {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to add STA entry for " MACSTR,
			   MAC2STR(addr));
	}
}


static void handle_data(struct asd_data *wasd, unsigned char *buf,
			size_t len)
{
	struct ieee8023_hdr *hdr;
	u8 *pos, *sa;
	size_t left;

	/* must contain at least ieee8023_hdr 6 byte source, 6 byte dest,
	 * 2 byte ethertype */
	if (len < 14) {
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP, "handle_data: too short (%lu)",
			   (unsigned long) len);
		return;
	}

	hdr = (struct ieee8023_hdr *) buf;

	switch (ntohs(hdr->ethertype)) {
		case ETH_P_PAE:
			asd_printf(ASD_DEFAULT,MSG_MSGDUMP, "Received EAPOL packet");
			sa = hdr->src;
			wired_possible_new_sta(wasd, sa);

			pos = (u8 *) (hdr + 1);
			left = len - sizeof(*hdr);

			ieee802_1x_receive(wasd, sa, pos, left);
		break;

	default:
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Unknown ethertype 0x%04x in data frame",
			   ntohs(hdr->ethertype));
		break;
	}
}


static void handle_read(int sock, void *circle_ctx, void *sock_ctx)
{
	struct asd_data *wasd ;//= (struct asd_data *) circle_ctx;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int len;
	unsigned char buf[3000];

	len = recv(sock, buf, sizeof(buf), 0);
	if (len < 0) {
		perror("recv");
		return;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"recv packet\n");
	if(interfaces->iface_wired[24]!=NULL)
		if(interfaces->iface_wired[24]->bss[100]!=NULL)
			wasd = interfaces->iface_wired[24]->bss[100];
		else
			return;
	else
		return;
	handle_data(wasd, buf, len);
}
void WIRED_STA_OP(WIRED_TableMsg *msg){
	struct asd_data *bss;	
	struct sta_info *sta;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	switch(msg->wOp){
		case WID_DEL:{
			unsigned int PortID = msg->u.wSTA.PORTID;
			unsigned int VlanID = msg->u.wSTA.VLANID;
			if((interfaces->iface_wired[PortID]!=NULL)&&(interfaces->iface_wired[PortID]->bss[VlanID]!=NULL)){				
				bss = interfaces->iface_wired[PortID]->bss[VlanID];
				sta = ap_get_sta(bss, msg->u.wSTA.STAMAC);
				if(sta != NULL){
					sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
					wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
					mlme_deauthenticate_indication(
						bss, sta, 0);
					sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
					ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
					ap_free_sta(bss, sta,0);
				}
			}
			break;
		}
		case WID_ADD:
		case WID_MODIFY:
		case STA_INFO:
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
		case STA_WAPI_INFO:
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
        default:
			break;
	}
}

void WIRED_PORT_OP(WIRED_TableMsg *msg){
	struct asd_data *wasd;		
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	
	switch(msg->wOp){
		case WID_DEL :{
			unsigned int PortID = msg->u.wPORT.PORTID;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"WID_DEL\n");
			
			if(interfaces->iface_wired[PortID]!= NULL){		
				
				int num,i;
				for(num=0;num<msg->u.wPORT.vlan_num;num++){
					i = msg->u.wPORT.VLANID[num];
					if(interfaces->iface_wired[PortID]->bss[i] != NULL){
						wasd = interfaces->iface_wired[PortID]->bss[i];
						asd_free_stas(wasd);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_free_stas finish\n");		
						asd_cleanup(wasd);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_cleanup finish\n");
						if(wasd->conf->radius->acct_server != NULL){
							if(wasd->conf->radius->acct_server->shared_secret != NULL)
							{	
								os_free(wasd->conf->radius->acct_server->shared_secret);
								wasd->conf->radius->acct_server->shared_secret = NULL;
							}
							os_free(wasd->conf->radius->acct_server);
							wasd->conf->radius->acct_server = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf->radius->acct_server) finish\n");
						if(wasd->conf->radius->auth_server != NULL){
							
							if(wasd->conf->radius->auth_server->shared_secret != NULL)
							{	
								os_free(wasd->conf->radius->auth_server->shared_secret);
								wasd->conf->radius->auth_server->shared_secret = NULL;
							}
							os_free(wasd->conf->radius->auth_server);
							wasd->conf->radius->auth_server = NULL;
						}
						
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf->radius->auth_server) finish\n");
						if(wasd->conf->radius != NULL){
							os_free(wasd->conf->radius);
							wasd->conf->radius = NULL;
						}
						
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf->radius) finish\n");
						if(wasd->info != NULL) {
							os_free(wasd->info);
							wasd->info = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->info) finish\n");
						if(wasd->conf != NULL){
							os_free(wasd->conf);
							wasd->conf = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf) finish\n");
						if(wasd->iconf != NULL){
							os_free(wasd->iconf);
							wasd->iconf = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->iconf) finish\n");
						if(wasd != NULL){						
							wasd->driver = NULL;
							wasd->drv_priv = NULL;
							os_free(wasd);
							wasd = NULL;
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd) finish\n");
							interfaces->iface_wired[PortID]->bss[i] = NULL;
							interfaces->iface_wired[PortID]->num_bss--;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(ASD_BSS[msg->u.BSS.BSSIndex]) finish\n");
						return;	
					}		
				}
			}		
		}
		case WID_ADD:
		case WID_MODIFY:
		case STA_INFO:
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
		case STA_WAPI_INFO:
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
        default:
			break;
	}
}

void WIRED_VLAN_OP(WIRED_TableMsg *msg){
	struct asd_data *wasd;		
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	
	switch(msg->wOp){
		case WID_DEL :{
			unsigned int VlanID = msg->u.wVLAN.VLANID;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"WID_DEL\n");
			int num,i;
			for(num=0;num<msg->u.wVLAN.port_num;num++){	
				i = msg->u.wVLAN.PORTID[num];
				if(interfaces->iface_wired[i]!= NULL){ 
					if(interfaces->iface_wired[i]->bss[VlanID] != NULL){
						wasd = interfaces->iface_wired[i]->bss[VlanID];
						asd_free_stas(wasd);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_free_stas finish\n");						
						asd_cleanup(wasd);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_cleanup finish\n");
						if(wasd->conf->radius->acct_server != NULL){
							if(wasd->conf->radius->acct_server->shared_secret != NULL)
							{	
								os_free(wasd->conf->radius->acct_server->shared_secret);
								wasd->conf->radius->acct_server->shared_secret = NULL;
							}
							os_free(wasd->conf->radius->acct_server);
							wasd->conf->radius->acct_server = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf->radius->acct_server) finish\n");
						if(wasd->conf->radius->auth_server != NULL){
							
							if(wasd->conf->radius->auth_server->shared_secret != NULL)
							{	
								os_free(wasd->conf->radius->auth_server->shared_secret);
								wasd->conf->radius->auth_server->shared_secret = NULL;
							}
							os_free(wasd->conf->radius->auth_server);
							wasd->conf->radius->auth_server = NULL;
						}
						
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf->radius->auth_server) finish\n");
						if(wasd->conf->radius != NULL){
							os_free(wasd->conf->radius);
							wasd->conf->radius = NULL;
						}
						
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf->radius) finish\n");
						if(wasd->info != NULL) {
							os_free(wasd->info);
							wasd->info = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->info) finish\n");
						if(wasd->conf != NULL){
							os_free(wasd->conf);
							wasd->conf = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->conf) finish\n");
						if(wasd->iconf != NULL){
							os_free(wasd->iconf);
							wasd->iconf = NULL;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd->iconf) finish\n");
						if(wasd != NULL){						
							wasd->driver = NULL;
							wasd->drv_priv = NULL;
							os_free(wasd);
							wasd = NULL;
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(wasd) finish\n");
							interfaces->iface_wired[i]->bss[VlanID] = NULL;
							interfaces->iface_wired[i]->num_bss--;
						}
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"free(ASD_BSS[msg->u.BSS.BSSIndex]) finish\n");
						return;	
					}		
				}
			}		
		}
		case WID_ADD:
		case WID_MODIFY:
		case STA_INFO:
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
		case STA_WAPI_INFO:
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
		default:
			break;	
	}
}


static void handle_buf(struct wasd_interfaces *interfaces, u8 *buf, size_t len)
{
	WIRED_TableMsg *msg = (WIRED_TableMsg*)buf;
	switch(msg->wType){
		case PORT_TYPE :{			
			WIRED_PORT_OP(msg);
			break;
		}		
		case VLAN_TYPE :{			
			WIRED_VLAN_OP(msg);
			break;
		}
		case STA_TYPE :{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA_TYPE\n");
			WIRED_STA_OP(msg);
			break;
		}
		default:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->Type %d\n",msg->wType);
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"default\n");
			break;
	}
	return;
}

static void handle_msg(int sock, void *circle_ctx, void *sock_ctx)
{
	
	int len;
	unsigned char buf[3000];
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	len = recvfrom(sock, buf, sizeof(buf), 0, NULL, 0);
	if (len < 0) {
		perror("recv");
		return;
	}
	
	handle_buf(interfaces, buf, len);
}



static void handle_dhcp(int sock, void *circle_ctx, void *sock_ctx)
{
	struct asd_data *wasd ;//= (struct asd_data *) circle_ctx;
	int len;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned char buf[3000];
	struct dhcp_message *msg;
	u8 *mac_address;

	len = recv(sock, buf, sizeof(buf), 0);
	if (len < 0) {
		perror("recv"); 
		return;
	}

	/* must contain at least dhcp_message->chaddr */
	if (len < 44) {
		asd_printf(ASD_DEFAULT,MSG_MSGDUMP, "handle_dhcp: too short (%d)", len);
		return;
	}
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"recv DHCP\n");
	if(interfaces->iface_wired[24]!=NULL)
		if(interfaces->iface_wired[24]->bss[100]!=NULL)
			wasd = interfaces->iface_wired[24]->bss[100];
		else
			return;
	else
		return;
	msg = (struct dhcp_message *) buf;
	mac_address = (u8 *) &(msg->chaddr);
	
	asd_printf(ASD_DEFAULT,MSG_MSGDUMP, "Got DHCP broadcast packet from " MACSTR,
		   MAC2STR(mac_address));

	wired_possible_new_sta(wasd, mac_address);
}

int asd_npd_ctl_socket_init(){
	 struct sockaddr_un local;
	 char *ASD_CTL_PATH = "/var/run/wcpss/asd_ctl";	 
	 char *NPD_CTL_PATH = "/var/run/wcpss/npd_ctl";
	 int len;
	 int sock;
	 sock = socket(PF_UNIX, SOCK_DGRAM, 0);	 
	 toNPD_C.addr.sun_family = PF_UNIX;
	 strcpy(toNPD_C.addr.sun_path,NPD_CTL_PATH);
	 toNPD_C.addrlen = strlen(toNPD_C.addr.sun_path) + sizeof(toNPD_C.addr.sun_family);
	 if (sock < 0) {
		perror("socket[PF_UNIX,PF_UNIX]");
		return -1;
	 }
	 local.sun_family = PF_UNIX;
 	 strcpy(local.sun_path,ASD_CTL_PATH);
	 unlink(local.sun_path);	
 	 len = strlen(local.sun_path) + sizeof(local.sun_family);	 
	 asd_printf(ASD_DEFAULT,MSG_DEBUG,"Trying to bind sock path [%s].\n",local.sun_path);	
	 if (bind(sock, (struct sockaddr *) &local, len) < 0) {
		 perror("bind(PF_UNIX)");
		 close(sock);//qiuchen
		 return -1;
	 }
	 asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_npd_ctl_socket_init\n");
	 if (circle_register_read_sock(sock, handle_msg, NULL, NULL))
	 {
		 asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not register read socket\n");
		 return -1;
	 }
	 return sock;

}

#if 0
static int asd_npd_data_init_sockets(struct wired_driver_data *drv)
{
	//struct ifreq ifr;
	//struct sockaddr_ll addr;
	//struct sockaddr_un remote;
	struct sockaddr_un local;
	char *ASD_DATA_PATH = "/var/run/wcpss/asd_data";	
	char *NPD_DATA_PATH = "/var/run/wcpss/npd_data";
	int len;
	int s;
	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	toNPD_D.addr.sun_family = PF_UNIX;
	strcpy(toNPD_D.addr.sun_path,NPD_DATA_PATH);
	toNPD_D.addrlen = strlen(toNPD_D.addr.sun_path) + sizeof(toNPD_D.addr.sun_family);
	if (s < 0 ) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		return -1;
	}
	local.sun_family = PF_UNIX;
	strcpy(local.sun_path,ASD_DATA_PATH);
	unlink(local.sun_path);	
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *) &local, len) < 0) {
		perror("bind");
		return -1;
	}
	drv->sock = s;
	if (circle_register_read_sock(drv->sock, handle_read, NULL, NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not register read socket\n");
		return -1;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_npd_data_init_sockets finish\n");
	return 0;
}
#endif


static int wired_init_sockets(struct wired_driver_data *drv)
{
	struct asd_data *wasd = drv->wasd;
	struct ifreq ifr;
	struct sockaddr_ll addr;
	struct sockaddr_in addr2;
	struct packet_mreq mreq;
	u8 multicastgroup_eapol[6] = WIRED_EAPOL_MULTICAST_GROUP;
	int n = 1;

	drv->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_PAE));
	if (drv->sock < 0) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		return -1;
	}

	if (circle_register_read_sock(drv->sock, handle_read, wasd, NULL)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not register read socket\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, "eth.p.0-3", sizeof(ifr.ifr_name));
	if (ioctl(drv->sock, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)");
		return -1;
	}

	
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Opening raw packet socket for ifindex %d",
		   addr.sll_ifindex);

	if (bind(drv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		return -1;
	}

	/* filter multicast address */
	memset(&mreq, 0, sizeof(mreq));
	mreq.mr_ifindex = ifr.ifr_ifindex;
	mreq.mr_type = PACKET_MR_MULTICAST;
	mreq.mr_alen = 6;
	memcpy(mreq.mr_address, multicastgroup_eapol, mreq.mr_alen);

	if (setsockopt(drv->sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq,
		       sizeof(mreq)) < 0) {
		perror("setsockopt[SOL_SOCKET,PACKET_ADD_MEMBERSHIP]");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, "eth.p.0-3", sizeof(ifr.ifr_name));
	if (ioctl(drv->sock, SIOCGIFHWADDR, &ifr) != 0) {
		perror("ioctl(SIOCGIFHWADDR)");
		return -1;
	}

	if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Invalid HW-addr family 0x%04x\n",
		       ifr.ifr_hwaddr.sa_family);
		return -1;
	}
	memcpy(wasd->own_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	/* setup dhcp listen socket for sta detection */
	if ((drv->dhcp_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket call failed for dhcp");
		return -1;
	}

	if (circle_register_read_sock(drv->dhcp_sock, handle_dhcp, wasd, NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"Could not register read socket\n");
		return -1;
	}
	
	memset(&addr2, 0, sizeof(addr2));
	addr2.sin_family = AF_INET;
	addr2.sin_port = htons(67);
	addr2.sin_addr.s_addr = INADDR_ANY;

	if (setsockopt(drv->dhcp_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &n,
		       sizeof(n)) == -1) {
		perror("setsockopt[SOL_SOCKET,SO_REUSEADDR]");
		return -1;
	}
	if (setsockopt(drv->dhcp_sock, SOL_SOCKET, SO_BROADCAST, (char *) &n,
		       sizeof(n)) == -1) {
		perror("setsockopt[SOL_SOCKET,SO_BROADCAST]");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_ifrn.ifrn_name, "eth.p.0-3", IFNAMSIZ);
	if (setsockopt(drv->dhcp_sock, SOL_SOCKET, SO_BINDTODEVICE,
		       (char *) &ifr, sizeof(ifr)) < 0) {
		perror("setsockopt[SOL_SOCKET,SO_BINDTODEVICE]");
		return -1;
	}

	if (bind(drv->dhcp_sock, (struct sockaddr *) &addr2,
		 sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return -1;
	}

	return 0;
}

/*
static int wired_send_eapol(void *priv, const u8 *addr,
			    const u8 *data, size_t data_len, int encrypt,
			    const u8 *own_addr)
{
	struct asd_data *wasd = priv;
	struct wired_driver_data *drv = wasd->drv_priv;
	u8 pae_group_addr[ETH_ALEN] = WIRED_EAPOL_MULTICAST_GROUP;
	struct ieee8023_hdr *hdr;
	size_t len;
	u8 *pos;
	int res;

	len = sizeof(*hdr) + data_len;
	hdr = os_zalloc(len);
	if (hdr == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"malloc() failed for wired_send_eapol(len=%lu)\n",
		       (unsigned long) len);
		return -1;
	}

	memcpy(hdr->dest, drv->use_pae_group_addr ? pae_group_addr : addr,
	       ETH_ALEN);
	memcpy(hdr->src, own_addr, ETH_ALEN);
	hdr->ethertype = htons(ETH_P_PAE);

	pos = (u8 *) (hdr + 1);
	memcpy(pos, data, data_len);

	res = sendto(drv->sock, (u8 *)hdr, len, 0, (struct sockaddr *)&toNPD_D.addr, toNPD_D.addrlen);
	free(hdr);

	if (res < 0) {
		perror("wired_send_eapol: send");
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"wired_send_eapol - packet len: %lu - failed\n",
		       (unsigned long) len);
	}

	return res;
}
*/
static int wired_send_eapol(void *priv, const u8 *addr,
					const u8 *data, size_t data_len, int encrypt,
					const u8 *own_addr)
	{
	struct asd_data *wasd = priv;
	struct wired_driver_data *drv = wasd->drv_priv;
		u8 pae_group_addr[ETH_ALEN] = WIRED_EAPOL_MULTICAST_GROUP;
		struct ieee8023_hdr *hdr;
		size_t len;
		u8 *pos;
		int res;
	
		len = sizeof(*hdr) + data_len;
		hdr = os_zalloc(len);
		if (hdr == NULL) {
			asd_printf(ASD_DEFAULT,MSG_WARNING,"malloc() failed for wired_send_eapol(len=%lu)\n",
				   (unsigned long) len);
			return -1;
		}
	
		memcpy(hdr->dest, drv->use_pae_group_addr ? pae_group_addr : addr,
			   ETH_ALEN);
		memcpy(hdr->src, own_addr, ETH_ALEN);
		hdr->ethertype = htons(ETH_P_PAE);
	
		pos = (u8 *) (hdr + 1);
		memcpy(pos, data, data_len);
	
		res = send(drv->sock, (u8 *) hdr, len, 0);
		os_free(hdr);
		hdr=NULL;
	
		if (res < 0) {
			perror("wired_send_eapol: send");
			asd_printf(ASD_DEFAULT,MSG_WARNING,"wired_send_eapol - packet len: %lu - failed\n",
				   (unsigned long) len);
		}
	
		return res;
	}

/*
static void * wired_driver_init(struct asd_data *wasd)
{
	struct wired_driver_data *drv;

	drv = os_zalloc(sizeof(struct wired_driver_data));
	if (drv == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not allocate memory for wired driver data\n");
		return NULL;
	}

	drv->use_pae_group_addr = 1;
	
	drv->ctl_sock = drv->sock = -1;
	
	drv->ctl_sock = asd_npd_ctl_socket_init();
	if(drv->ctl_sock < 0){
		free(drv);
		return NULL;
	}

	if (asd_npd_data_init_sockets(drv)) {
		close(drv->ctl_sock);
		free(drv);
		return NULL;
	}

	return drv;
}
*/

static void * wired_driver_init(struct asd_data *wasd)
{
	struct wired_driver_data *drv;

	drv = os_zalloc(sizeof(struct wired_driver_data));
	if (drv == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not allocate memory for wired driver data\n");
		return NULL;
	}

	drv->wasd = wasd;
	drv->use_pae_group_addr = 0;

	if (wired_init_sockets(drv)) {
		os_free(drv);
		drv=NULL;
		return NULL;
	}

	return drv;
}



static void wired_driver_deinit(void *priv)
{
	struct wired_driver_data *drv = priv;

	if (drv->sock >= 0)
		close(drv->sock);
	
	if (drv->ctl_sock >= 0)
		close(drv->ctl_sock);

	os_free(drv);
	drv=NULL;
}

static int
wired_sta_set_flags(void *priv, const u8 *addr, int total_flags,
		      int flags_or, int flags_and)
{
	/* For now, only support setting Authorized flag */
	if (flags_or & WLAN_STA_AUTHORIZED){
		asd_printf(ASD_DEFAULT,MSG_INFO,"STA_AUTHORIZED\n");
		return 0;
	}
	if (!(flags_and & WLAN_STA_AUTHORIZED)){
		asd_printf(ASD_DEFAULT,MSG_NOTICE,"STA_UNAUTHORIZED\n");
		return 0;
	}
	return 0;
}



const struct wpa_driver_ops wpa_driver_wired_ops = {
	.name = "wired",
	.init = wired_driver_init,
	.deinit = wired_driver_deinit,
	.send_eapol = wired_send_eapol,
	.sta_set_flags = wired_sta_set_flags,
};
