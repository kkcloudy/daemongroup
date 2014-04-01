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
* had_uid.c
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		APIs used in vrrp part of HAD module.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.12 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <stdio.h>
#include <assert.h>
#include <net/ethernet.h>
#include <linux/rtnetlink.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/errno.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
#include <linux/if.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/filter.h>
#include <dbus/dbus.h>
#include <dirent.h>


#include <netinet/in.h>
#include <linux/un.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <strings.h>

#include "had_vrrpd.h"
#include "had_uid.h"
#include "had_dbus.h"
#include "had_log.h"

struct sockaddr_un server,client;

int npdSock;
int uidSock;
int advtime = 0;
extern hansi_s** g_hansi;
extern int service_enable[MAX_HANSI_PROFILE];
extern pthread_mutex_t PacketMutex;
extern pthread_mutex_t StateMutex;
extern int master_ipaddr_uplink[VRRP_MAX_VRID];
extern int master_ipaddr_downlink[VRRP_MAX_VRID];
extern struct sock_list_t* sock_list;
extern char* global_ht_ifname;
extern int global_ht_ip;
extern int global_ht_state;
extern int global_ht_opposite_ip;
extern int global_state_change_bit;
extern int global_multi_link_detect;
extern int time_synchronize_enable;
extern int stateFlg;
PKT_BUF 	bufNode[MAX_HANSI_PROFILE];
int vrrp_arp_send_fd = -1;
static char *state_sensitive_if[] = { \
	"eth", 	/* ethernet-port and related subif */ \
	"vlan", /* vlan interface */ \
	"ve", /* vlan interface */ \
	
};
int had_SocketInit()
{	
	memset(&client,0,sizeof(client));
	memset(&server,0,sizeof(server));
	
	if((npdSock = socket(AF_LOCAL,SOCK_DGRAM,0))!= -1)
	{       
		vrrp_syslog_dbg("vrrp packet sock is %d \n", npdSock);
		client.sun_family = AF_LOCAL;
		server.sun_family = AF_LOCAL;
		
		strcpy(client.sun_path, "/tmp/VRRP_CLIENT");
	    strcpy(server.sun_path, "/tmp/VRRP_SERVER");
		unlink(client.sun_path);

		
		if(bind(npdSock,(struct sockaddr *)&client,sizeof(client))!= -1)
		{	
			if(chmod(client.sun_path, 0777) == -1){
				vrrp_syslog_error("%s,%d,chmod error.\n",__func__,__LINE__);
				return 0;
			}
		}
		else
		{
			vrrp_syslog_dbg("vrrp packet socket bind error;");
			close (npdSock);
			return 0;
		}
	}
	else 
	{
		return 0;
		vrrp_syslog_warning("vrrp packet socket init failed\n");
	}
	return npdSock;
}


int had_SocketInit2()
{	
	int sockfd = 0;
	int ret = 0;
	/* open the socket */
	sockfd = socket(PF_PACKET, SOCK_RAW|O_NONBLOCK, htons(ETH_P_IP));
	if( sockfd < 0 ){
		vrrp_syslog_error("cannot open raw socket. error %s. (try to run it as root)\n"
						, strerror(errno));
		return -1;
	}
	/*get from tcpdump vrrp -dd */
	struct sock_filter bpf_code[] = {
		{ 0x28, 0, 0, 0x0000000c },
		{ 0x15, 0, 3, 0x00000800 },
		{ 0x30, 0, 0, 0x00000017 },
		{ 0x15, 0, 1, 0x00000070 },
		{ 0x6, 0, 0, 0x00000060 },
		{ 0x6, 0, 0, 0x00000000 }

	};
	
	  struct sock_fprog filter;
	  filter.len = sizeof(bpf_code)/sizeof(struct sock_filter);
	  filter.filter = bpf_code;
	 if(0 != (ret = setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter)))){
		vrrp_syslog_error("%s,%d,setsockopt error,sock:%d,ret:%d.\n",__func__,__LINE__,sockfd,ret);
	  }
	/* join the multicast group */
	return sockfd;	
}


vrrp_rt* had_get_vrrp_by_vrid
(
    unsigned int vrid
)
{
    int profile = 0;
	hansi_s* hansiNode = NULL;
	vrrp_rt* vrrp = NULL;
	for(profile = 0;profile < MAX_HANSI_PROFILE;profile++){
       hansiNode = g_hansi[profile];
	   if((NULL == hansiNode)||(NULL == (vrrp = hansiNode->vlist))){
           continue;
	   }
       while(vrrp &&(vrrp->vrid != vrid)){
		  	vrrp = vrrp->next;
	   }
	   return vrrp;
	} 
	return vrrp;
}
/***************************
*Func:had_mac_check
*Param: 
*   vrrp_rt* vsrv,
*   char PktMac[],
*   unsigned int flag
*Use:
*   if two mac equal,return 0;
*   else return 1;
*
***************************/
int had_mac_check
(
	vrrp_rt* vsrv,
	unsigned char PktMac[],
	unsigned int flag /*up or down link*/
)
{
	int i = 0;
  
	if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
		(0 != vsrv->no_vmac) &&
		(flag == VRRP_PKT_VIA_UPLINK))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if ((VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) &&
				(0 == memcmp(vsrv->uplink_vif[i].hwaddr, PktMac, VRRP_MAC_ADDRESS_LEN)))
			{
				return 0;
			}
		}
	}
	else if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
			 (0 != vsrv->no_vmac) &&
			 (flag == VRRP_PKT_VIA_DOWNLINK))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if ((VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) &&
				(0 == memcmp(vsrv->downlink_vif[i].hwaddr, PktMac, VRRP_MAC_ADDRESS_LEN)))
			{
				return 0;
			}
		}
	}

	return 1;
}
void had_updata
(
   int fd
)
{
	int vrid = 0;
	int hansi = 0;
	int time = 0;
	vrrp_rt* vrrp = NULL;
	int len = 0;
	unsigned char buffer[MAX_UID_MSG_SIZE] = {0};
	char *buf = NULL;
	struct iphdr *iph = NULL;
	vrrp_pkt *hd = NULL;
	struct ether_header *eth = NULL;

	/* read packet */
	len = read(fd, buffer, MAX_UID_MSG_SIZE);
	if (len <= 0) {
		return;	
	}

	/* get ether header */	
	buf = (char *)buffer;
	eth = (struct ether_header *)buffer;

	/* get ip header */
	buf += vrrp_dlt_len(vrrp);
	len -= vrrp_dlt_len(vrrp);
	iph = (struct iphdr *)buf;

	/* check protocol is VRRP */
	if (iph->protocol != IPPROTO_VRRP)
	{
		memset(buffer, 0, MAX_UID_MSG_SIZE);
		vrrp_syslog_dbg("ignore ip packet type %d none vrrp!\n",
						iph->protocol);
		return;
	}

	/* get VRRP header */
	hd = (vrrp_pkt *)((char *)iph + (iph->ihl<<2));
	had_packet_detail(buffer,len + vrrp_dlt_len(vrrp)); 
	
	vrid = hd->vrid;
	vrrp = had_get_vrrp_by_vrid(vrid);
	if (NULL == vrrp) {
		return;
	}
	else
	{	
        /* get hansi id for vrrp. 2013-06-27 */
	    hansi = vrrp->profile;		
		/*check smac if the same as my mac*/
		if (0 == had_mac_check(vrrp, eth->ether_shost, hd->updown_flag))
		{
			vrrp_syslog_dbg("vrid %d %s packet received with smac same as mine,ignore packet!\n",
							vrid, \
							(VRRP_PKT_VIA_UPLINK == hd->updown_flag) ? "uplink" : 
							(VRRP_PKT_VIA_DOWNLINK == hd->updown_flag) ? "downlink" : "otherlink");
			return;
		}

		/* receive a advertisements packet, set stateFlg is 0. */
		stateFlg = 0;
		vrrp_syslog_dbg("receive a advertisements packet, set stateflg 0\n");
		
		if (VRRP_STATE_DISABLE == vrrp->state)
		{	/* uplink && downlink are not setted. */
			if ((VRRP_LINK_NO_SETTED == vrrp->uplink_flag) &&
				(VRRP_LINK_NO_SETTED == vrrp->downlink_flag))
			{
				vrrp_syslog_error("vrid %d with no uplink and downlink,keep in disable state and ignore packet!\n",
									vrid);
				return;
			}
			/* uplink or downlink is setted, but not link-up. */
			else if (((VRRP_LINK_NO_SETTED != vrrp->uplink_flag) &&
					  (!vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_UPLINK))) || \
					  ((VRRP_LINK_NO_SETTED != vrrp->l2_uplink_flag) &&
					  (!vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_L2_UPLINK))) ||\
					 ((VRRP_LINK_NO_SETTED != vrrp->downlink_flag) &&
					  (!vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_DOWNLINK))))
			{
				vrrp_syslog_dbg("vrid %d uplink or downlink is down, disable state ignore packet!\n", \
								vrid);
				return;
			}
			/* [1] || [2]
			 * [1] uplink is not setted, and downlink is setted, and downlink link-up.
			 * [2] downlink is not setted, and uplink is setted, and uplink link-up.
			 */
			else if (((VRRP_LINK_NO_SETTED == vrrp->uplink_flag) &&
					  (VRRP_LINK_NO_SETTED != vrrp->downlink_flag) &&
					  (vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_DOWNLINK)))|| \
					 ((VRRP_LINK_NO_SETTED == vrrp->downlink_flag) &&
					  (VRRP_LINK_NO_SETTED != vrrp->uplink_flag) &&
					  (vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_UPLINK))))
			{
				master_ipaddr_uplink[vrrp->vrid] = hd->uplink_ip;
				master_ipaddr_downlink[vrrp->vrid] = hd->downlink_ip;
				if ((NULL != global_ht_ifname) &&
					(0 != global_ht_ip))
				{
					global_ht_opposite_ip = hd->heartbeatlinkip;
				}
				vrrp_syslog_dbg("vrid %d %s receive packet,update state machine from disable state!\n",
								vrid, \
								(VRRP_PKT_VIA_UPLINK == hd->updown_flag) ? "uplink" : 
								(VRRP_PKT_VIA_DOWNLINK == hd->updown_flag) ? "downlink" : "otherlink");
				had_state_from_disable(vrrp,hd);
			}
			else
			{	/* one case is : uplink and downlink are setted, and are link-up.
				 * maybe have other case.
				 */
				 
				/* save receive packet information */
				/* packet's updown_flag is uplink */
				if ((VRRP_LINK_NO_SETTED != vrrp->uplink_flag) &&
					(vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_UPLINK)) && 
					(VRRP_PKT_VIA_UPLINK == hd->updown_flag))
				{
					memset(&bufNode[hansi].uplink,0,sizeof(BUF_INFO));
					memcpy(bufNode[hansi].uplink.buf,buf,len);
					bufNode[hansi].uplink.set_flag = 1;
					bufNode[hansi].uplink.buflen = len;
					bufNode[hansi].uplink.time = had_TIMER_CLK();
				}
				/* packet's updown_flag is downlink */
				else if ((VRRP_LINK_NO_SETTED != vrrp->downlink_flag) &&
						 (vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_DOWNLINK)) && 
						 (VRRP_PKT_VIA_DOWNLINK == hd->updown_flag))
				{
					memset(&bufNode[hansi].downlink,0,sizeof(BUF_INFO));
					memcpy(bufNode[hansi].downlink.buf,buf,len);
					bufNode[hansi].downlink.set_flag = 1;
					bufNode[hansi].downlink.buflen = len;
					bufNode[hansi].downlink.time = had_TIMER_CLK();
				}
				
				if ((1 == bufNode[hansi].uplink.set_flag) &&
					(1 == bufNode[hansi].downlink.set_flag))
				{
					if ((bufNode[hansi].uplink.time > bufNode[hansi].downlink.time) && \
						((time = bufNode[hansi].uplink.time - bufNode[hansi].downlink.time) > 2 * (vrrp->adver_int)))
					{
						vrrp_syslog_dbg("time %d wait longer than two advertisement,drop the packets and dont update!\n",
										time);
						memset(&bufNode[hansi].downlink, 0, sizeof(BUF_INFO));
					}
					else if ((bufNode[hansi].downlink.time > bufNode[hansi].uplink.time)&& \
							 ((time = bufNode[hansi].downlink.time - bufNode[hansi].uplink.time) > 2 * (vrrp->adver_int)))
					{
						vrrp_syslog_dbg("time %d wait longer than two advertisement,drop the packets and dont update!\n",
										time);
						memset(&bufNode[hansi].uplink, 0, sizeof(BUF_INFO));
					}
				  
					if (time < 2 * (vrrp->adver_int))
					{
						if ((global_ht_ifname) &&
							(global_ht_ip) &&
							(!global_ht_state))
						{
							vrrp_syslog_event("vrrp %d both uplink & downlink received packet but heartbeat link down, so drop and dont update!\n",
												vrrp->vrid);
						}
						else {
							vrrp_syslog_dbg("vrrp %d both uplink & downlink received packets,to update machine!\n",
											vrrp->vrid);
							master_ipaddr_uplink[vrrp->vrid] = hd->uplink_ip;
							master_ipaddr_downlink[vrrp->vrid] = hd->downlink_ip;
							if ((NULL != global_ht_ifname) &&
								(0 != global_ht_ip))
							{
								global_ht_opposite_ip = hd->heartbeatlinkip;
							}
							had_state_from_disable(vrrp,hd);
							memset(&bufNode[hansi], 0, sizeof(PKT_BUF));
						}
					}
			   }
			   memset(buffer,0,MAX_UID_MSG_SIZE);
			}
		}
		else if ((VRRP_LINK_NO_SETTED != vrrp->uplink_flag) &&
				 (vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_UPLINK)) &&
				 (VRRP_PKT_VIA_UPLINK == hd->updown_flag) &&
				 ((VRRP_LINK_NO_SETTED == vrrp->downlink_flag) ||
				  ((VRRP_LINK_NO_SETTED != vrrp->downlink_flag) &&
				   (!had_link_auth_type_cmp(vrrp, VRRP_LINK_TYPE_DOWNLINK, VRRP_AUTH_PASS)))))
		{
			vrrp_syslog_dbg("vrrp %d uplink received packets,to update machine!\n",vrrp->vrid);
			had_update_uplink(vrrp,buf,len);
			memset(buffer,0,MAX_UID_MSG_SIZE);
		}
		else if ((VRRP_LINK_NO_SETTED != vrrp->downlink_flag) &&
				 (vrrp_get_link_state(vrrp, VRRP_LINK_TYPE_DOWNLINK)) &&
				 (VRRP_PKT_VIA_DOWNLINK == hd->updown_flag) &&
				 ((VRRP_LINK_NO_SETTED == vrrp->uplink_flag) ||
				  ((VRRP_LINK_NO_SETTED != vrrp->uplink_flag) &&
				   (!had_link_auth_type_cmp(vrrp, VRRP_LINK_TYPE_UPLINK, VRRP_AUTH_PASS)))))
		{
			vrrp_syslog_dbg("vrrp %d downlink received packets,to update machine!\n",vrrp->vrid);
			had_update_downlink(vrrp,buf,len);
			memset(buffer,0,MAX_UID_MSG_SIZE);
		}
	}
	return;
}
int had_set_fd
(
	void
)
{
	int maxfd = 0;
	int i = 0;

	//pthread_mutex_lock(&StateMutex);
	struct sock_s* fd = sock_list->sock_fd;
	if (NULL == fd) {
		//pthread_mutex_unlock(&StateMutex);
		return 0;
	}else
	{
		while (fd)
		{
			if ((NULL != global_ht_ifname) &&
				(0 != global_ht_ip) &&
				(global_ht_state))
			{
				if (fd->sockfd > maxfd) {
					maxfd = fd->sockfd;
				}
			}
			//else 
			{
				/* support mutli-interfaces in uplink & downlink */
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (fd->uplink_fd[i] > maxfd) {
						maxfd = fd->uplink_fd[i];
					}
					if (fd->downlink_fd[i] > maxfd) {
						maxfd = fd->downlink_fd[i];
					}
				}
			}
			fd = fd->next;
		}
	}

	//pthread_mutex_unlock(&StateMutex);

	/*
	vrrp_syslog_dbg("in get max fd %d\n", maxfd);
	*/
	return maxfd;
}

void had_read_sock_list
(
	fd_set readfds
)
{
	int i = 0;
	struct sock_s *sockfd = sock_list->sock_fd;

	if (NULL == sockfd) {
		vrrp_syslog_dbg("no sock fd in sock list!\n");
		return;
	}
	else
	{
		while (sockfd)
		{
			vrrp_syslog_dbg("check vrrp %d if receive packets!\n",
							sockfd->vrid);
			if ((NULL != global_ht_ifname) &&
				(0 != global_ht_ip) &&
				(global_ht_state))
			{
				if (FD_ISSET(sockfd->sockfd, &readfds)) {
					vrrp_syslog_dbg("updata vrrp %d, heartbeat link receive packet!\n",
									sockfd->vrid);
					had_updata(sockfd->sockfd);
				}
			}			
			/*no matter heartbeat is down or not, try to receive packet from uplink and downlink also*/
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (FD_ISSET(sockfd->uplink_fd[i], &readfds)) {
					vrrp_syslog_dbg("updata vrrp %d uplink receive packet!\n",
									sockfd->vrid);
					had_updata(sockfd->uplink_fd[i]);
				}
				if (FD_ISSET(sockfd->downlink_fd[i], &readfds)) {
					vrrp_syslog_dbg("updata vrrp %d downlink receive packet!\n",
									sockfd->vrid);
					had_updata(sockfd->downlink_fd[i]);
				}
			}
			sockfd = sockfd->next;
		}
	}

	return;
}

#if 0
/* delete by jinpc@autelan.com
 * for not in use
 */
int vrrp_rx_packet
(
	vrrp_rt* vrrp,
	unsigned int ifindex,
	char* packet,
	int   len
)
{
	char* buffer = packet;
	int pcklen = len;
	if((NULL == buffer)|| (0 == pcklen)){
	   vrrp_syslog_error("receive vrrp packet null or length zero error!\n");
	   return -1;
	}	
	if((ifindex == vrrp->uplink_vif.ifindex)&&( vrrp_in_chk_uplink( vrrp, (struct iphdr *)buffer )) ){
		vrrp_syslog_packet_receive("bogus packet!\n");
		return -1;
	}
	else if((ifindex == vrrp->downlink_vif.ifindex)&&( vrrp_in_chk_downlink( vrrp, (struct iphdr *)buffer )) ){
		vrrp_syslog_packet_receive("bogus packet!\n");
		return -1;
	}
	
	return 0;
}
#endif

void had_packet_detail(unsigned char *buffer, int buffLen)
{
	unsigned int i;
	unsigned char lineBuffer[64] = {0}, *bufPtr = NULL;
	unsigned int curLen = 0;

	if(!buffer){
		return;
	}
	vrrp_syslog_packet_receive(".......................RX.......................%d\n",buffLen);

	bufPtr = lineBuffer;
	curLen = 0;
	for(i = 0;i < buffLen ; i++)
	{
		curLen += sprintf((char*)bufPtr,"%02x ",buffer[i]);
		bufPtr = lineBuffer + curLen;
		
		if(0==(i+1)%16) {
			vrrp_syslog_packet_receive("%s\n",lineBuffer);			
			memset(lineBuffer,0,sizeof(lineBuffer));
			curLen = 0;
			bufPtr = lineBuffer;
		}
	}
	
	if((buffLen%16)!=0)
	{
		vrrp_syslog_packet_receive("%s\n",lineBuffer);
	}
	
	vrrp_syslog_packet_receive(".......................RX.......................\n");
}

void had_packet_thread2_main(void)
{	
	struct timeval	timeout;
	int i = 0;
	int sockfd = 0;

    memset(bufNode, 0,MAX_HANSI_PROFILE * sizeof(PKT_BUF));
	
	vrrp_debug_tell_whoami("packetRx", 0);
#if 0
	uidSock = had_SocketInit2();
	if(uidSock <= 0){
	   vrrp_syslog_error("init uid sock failed!\n");
	   return -1;
	}
#endif
  	while(1){
#if 1  
		fd_set		readfds;
        struct sock_s* fd = NULL;

		FD_ZERO(&readfds);
		sockfd = had_set_fd();
		/*
		vrrp_syslog_dbg("get max fd %d\n", sockfd);
		*/
		
		//pthread_mutex_lock(&StateMutex);
		if(sock_list) {
			fd = sock_list->sock_fd;  
		}

		/*
		vrrp_syslog_dbg("get sockfd %p\n", fd);
		*/
		
		for(;fd;fd = fd->next){
			if((NULL != global_ht_ifname) && (0 != global_ht_ip)&&(global_ht_state)){
				if(0 != fd->sockfd)
               		FD_SET(fd->sockfd,&readfds);
			}
			//else
			 {
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if(0 != fd->uplink_fd[i]){
						FD_SET(fd->uplink_fd[i],&readfds);
					}
					if(0 != fd->downlink_fd[i]){
					    FD_SET(fd->downlink_fd[i],&readfds);
					}
				}
			}
		} 
		//pthread_mutex_unlock(&StateMutex);

		timeout.tv_sec	= 1;
		timeout.tv_usec = 0;
		if( select( sockfd + 1, &readfds, NULL, NULL, &timeout ) > 0 ){
			#if 1			
			pthread_mutex_lock(&StateMutex);				
			#endif
	        had_read_sock_list(readfds); 
			#if 1
			/* delete jinpc */
			pthread_mutex_unlock(&StateMutex);	
			#endif
		}
#endif
  	}

	return;
}

#define EBR_PATH "/sys/class/net/%s/brif/%s"
#define EBR_NAME "/sys/class/net/%s/brif/"
#define EBR_UPLINK_NAME "/sys/class/net/%s/bridge/uplink_port"


int had_check_br_muster
(
   char* brname,
   int*  radio_ifCount,
   int*  eth_ifCount
)
{
   if(NULL == brname){
      return 0;
   }

   DIR* dp = NULL;
   struct dirent* dir = NULL;
   char path[128] = {0};
   int radio_count = 0,eth_count = 0,count = 0;
   sprintf(path,EBR_NAME,brname);
   vrrp_syslog_dbg("ebr brif in path name %s\n",path);
   if(NULL ==(dp = opendir(path))){
      vrrp_syslog_error("can not open %s when check br interface\n",brname);
	  return 0;
   }
   else{
   	  /*loop for member of ebr interface to get status*/
      while(NULL != (dir = readdir(dp))){
	  	 if(!strcmp(".",dir->d_name) || !strcmp("..",dir->d_name)){
            vrrp_syslog_dbg("name . or .. in ebr1 should be ignored!\n");
			continue;
		 }
         else if((!strncmp("radio",dir->d_name,5))||(!strncmp("r",dir->d_name,1))){
            vrrp_syslog_dbg("radio interface %s in %s\n",dir->d_name,brname);
			radio_count++;
		 }
         else if(!strncmp("eth",dir->d_name,3)){
            vrrp_syslog_dbg("eth interface %s in %s\n",dir->d_name,brname);
			eth_count++;
		 } 
	  }
   }
   *radio_ifCount = radio_count;
   *eth_ifCount = eth_count;
   count = radio_count+eth_count;
   closedir(dp);
   dp = NULL;
   return count;
}

/*
  *******************************************************************************
  * had_state_sensitive_intf_check
  *
  *  DESCRIPTION:
  * 		check if the interface given is VRRP state sensitive.
  *
  *  INPUTS:
  *		ifname	- name of the interface
  *
  *  OUTPUTS:
  *		attr 		- 1 for VRRP state-sensitive interface,
  *				   0 for VRRP state non-sensitive inteface.
  *
  *  RETURN VALUE:
  *		0	- success
  *		1	- fail
  *
  *******************************************************************************
  */
int had_state_sensitive_intf_check
(
	char *ifname,
	unsigned int *attr
)
{
	int i = 0, round = 0, clen = 0;

	if(!ifname || !attr) {
		return -1;
	}

	round = sizeof(state_sensitive_if)/sizeof(char*);
	clen = strlen(ifname);
	
	for(i = 0; i < round; i++) {
		if(!strncmp(ifname, state_sensitive_if[i], strlen(state_sensitive_if[i]))) {
			*attr = 1;
			return 0;
		}
	}

	*attr = 0;
	return 0;
}

/*
 *******************************************************************************
 *had_check_ebr_state()
 *
 *  DESCRIPTION:
 *		get link-status of the special ebr
 *  INPUTS:
 *		char *ebrname		- name of the special ebr
 *
 *  OUTPUTS:
 *		char link_status			- INTERFACE_DOWN link-down,
 *							  INTERFACE_UP link-up
 *
 *  RETURN VALUE:
 *		0	- success
 *		1	- fail
 *
 *******************************************************************************
 */
int had_check_ebr_state
(
   char *ebrname,
   char *link_status
)
{
   int ret = 0;
   DIR* dp = NULL;
   struct dirent* dir = NULL;
   char path[128] = {0};
   int status = 0;
   unsigned int tmp_status = INTERFACE_DOWN, sensitive = 0;
   int member = 0;

   if(NULL == ebrname){
   	  vrrp_syslog_error("check ebr state with null name error!\n");
      return 1;
   }

   sprintf(path,EBR_NAME,ebrname);
   vrrp_syslog_dbg("ebr brif in path name %s\n",path);
   if(NULL ==(dp = opendir(path))){
      vrrp_syslog_error("can not open %s when check ebr state\n",ebrname);
	  return 1;
   }
   else{
   	  /*loop for member of ebr interface to get status*/
      while(NULL != (dir = readdir(dp))){
	  	 if(!strcmp(".",dir->d_name) || !strcmp("..",dir->d_name)){
            vrrp_syslog_dbg("name . or .. in ebr1 should be ignored!\n");
			continue;
		 }
		 
		 if(!had_state_sensitive_intf_check(dir->d_name, &sensitive)) {
		 	 
             if(sensitive)
			 {	
			    /* vrrp_syslog_dbg("interface %s belong to ebr interface %s is sensitive ",dir->d_name,ebrname);*/
			     ret = had_ifname_to_status_byIoctl(dir->d_name, &tmp_status);
				 if (ret != 0) {
				     vrrp_syslog_error("get interface %s link status error when check ebr %s state!\n", dir->d_name, ebrname);
				 }
		     }
			 else{
			 	/* vrrp_syslog_dbg("interface %s belong to ebr interface %s is not sensitive ",dir->d_name,ebrname);*/
	             tmp_status = INTERFACE_UP;
			 }
		 }
	  	 status += tmp_status;
         /*vrrp_syslog_dbg("interface %s(%s) belong to ebr interface %s, status %d\n",\
					 	dir->d_name, (INTERFACE_UP == tmp_status) ? "U":"D", ebrname, status);*/
		 tmp_status = INTERFACE_DOWN;
		 sensitive = 0;
		 member++;
		 if(0 != member){/*eixst members*/
			if(global_multi_link_detect ){/*turn on multi link detect , only all up,return up*/
				if(status < member){/*some(one or all ) member interfaces down*/
					vrrp_syslog_dbg("multi detect on,member %d,status %d,return INTERFACE DOWN!\n",
					            member,status);
					*link_status = INTERFACE_DOWN;
					closedir(dp);
					dp = NULL;
					return 0;
				}	
				else{
				}
			}
			else{/*turn off multi link detect, only all interfaces down,return down*/
				if(status){
					vrrp_syslog_dbg("multi detect off,member %d,status %d,return INTERFACE UP!\n",
					            member,status);
					*link_status = INTERFACE_UP;
					closedir(dp);
					dp = NULL;
					return 0;
				}
			}
		  }	  
	  }

	  /* add by jinpc 20091012 for close fd */
	  closedir(dp);
	  dp = NULL;
	  
	  if(0 != member){/*eixst members*/
         if(global_multi_link_detect ){/*turn on multi link detect , only all up,return up*/
             if(status < member){/*some(one or all ) member interfaces down*/
			 	vrrp_syslog_dbg("multi detect on,member %d,status %d,return INTERFACE DOWN!\n",
					            member,status);
				*link_status = INTERFACE_DOWN;
                return 0;
			 }	
			 else if(status == member){/*all interfaces up*/
				 vrrp_syslog_dbg("multi detect on,member %d,status %d,return INTERFACE UP!\n",
								 member,status);			 	
				*link_status = INTERFACE_UP;
                return 0;
			 }
		 }
		 else{/*turn off multi link detect, only all interfaces down,return down*/
             if(status){
			 	vrrp_syslog_dbg("multi detect off,member %d,status %d,return INTERFACE UP!\n",
					            member,status);
				*link_status = INTERFACE_UP;
                return 0;
			 }
			 vrrp_syslog_dbg("multi detect off,member %d,status %d,return INTERFACE DOWN!\n",
					            member,status);			 
			 *link_status = INTERFACE_DOWN;
			 return 0;
		 }
	  }
	  /*if ebr interface no members,return down*/
	  vrrp_syslog_dbg("ebr interface %s no members,return down!\n",ebrname);
	  *link_status = INTERFACE_DOWN;
	  return 0;
   }
}

/*
 *******************************************************************************
 *had_check_l2_ebr_state()
 *
 *  DESCRIPTION:
 *		get link-status of the special ebr
 *  INPUTS:
 *		char *ebrname		- name of the special ebr
 *
 *  OUTPUTS:
 *		char link_status			- INTERFACE_DOWN link-down,
 *							  INTERFACE_UP link-up
 *
 *  RETURN VALUE:
 *		0	- success
 *		1	- fail
 *
 *******************************************************************************
 */
int had_check_l2_ebr_state
(
   char *ebrname,
   char *link_status
)
{
   int ret = 0;
   int uplinkfd = 0;
   char path[128] = {0};
   char strbuf[256];
   int count = 0;
   int getCount = 0;
   int status = 0;
   unsigned char * ifname = NULL;
   unsigned int tmp_status = INTERFACE_DOWN, sensitive = 0;
   int member = 0;

   if(NULL == ebrname){
   	  vrrp_syslog_error("check ebr state with null name error!\n");
      return 1;
   }

   sprintf(path,EBR_UPLINK_NAME,ebrname);
   vrrp_syslog_dbg("ebr brif in path name %s\n",path);
   
   if( 0 > (uplinkfd = open(path, O_RDONLY))){
      vrrp_syslog_error("can not open %s when check ebr state\n",ebrname);
	  return 1;
   }
   
   if(0 > read(uplinkfd, strbuf, 256)){
   	vrrp_syslog_error("read failed when get uplink interfaces of %s,err: %s \n", ebrname,strerror(errno));
	close(uplinkfd);
	return -1;
   	}
   vrrp_syslog_dbg("read string %s from file %s success\n",strbuf, path);
   close(uplinkfd);
   
   ifname = strtok(strbuf, " \r\n\t");	  	
	  while(count<4){
		  if(ifname == NULL){
		  	break;
			}
		  
		  count ++ ;
		  getCount ++;
		  if(!had_state_sensitive_intf_check(ifname, &sensitive)){	  	
			if(sensitive)
			{  
				member++;
				vrrp_syslog_dbg("interface %s belong to l2-uplink interface %s is sensitive ",ifname,ebrname);
				ret = had_ifname_to_status_byIoctl(ifname, &tmp_status);
				if (ret != 0) {
					vrrp_syslog_error("get interface %s link status error when check l2-uplink %s state!\n", ifname, ebrname);
				}
			}
			status += tmp_status;
			vrrp_syslog_dbg("interface %s(%s) belong to l2-uplink interface %s, status %d\n",\
					 	ifname, (INTERFACE_UP == tmp_status) ? "U":"D", ebrname, status);
			}
		   tmp_status = INTERFACE_DOWN;
		   sensitive = 0;
		  ifname = strtok(NULL, " \r\n\t");
	  }
  
	  if(0 != member){/*eixst members*/
             if(status < member){/*some(one or all ) member interfaces down*/
			 	vrrp_syslog_dbg("multi detect on,member %d,status %d,return INTERFACE DOWN!\n",
					            member,status);
				*link_status = INTERFACE_DOWN;
                return 0;
			 }	
			 else if(status == member){/*all interfaces up*/
				 vrrp_syslog_dbg("multi detect on,member %d,status %d,return INTERFACE UP!\n",
								 member,status);			 	
				*link_status = INTERFACE_UP;
                return 0;
			 }		 
	  }
	  /*if ebr interface no members,return down*/
	  vrrp_syslog_dbg("ebr interface %s no members,return down!\n",ebrname);
	  *link_status = INTERFACE_DOWN;
	  return 0; 
}

/*
 *******************************************************************************
 *had_set_link_state()
 *
 *  DESCRIPTION:
 *		set uplink/downlink/vgateway interface's link state.
 *  INPUTS:
 *		char* ifname,
 *		int ifi_flags,
 *		vrrp_rt* vsrv
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
void had_set_link_state
(
    char* ifname,
    int   ifi_flags,
    vrrp_rt* vsrv
)
{
   char path[128] = {0};
   int ret = 0;
   int i = 0;
   int j = 0;
   int fd = -1;
   vrrp_rt* vrrp = NULL;
   
   if(NULL != vsrv){
   	  vrrp = vsrv;
      goto do_vrrp;
   }
   for(;i<MAX_HANSI_PROFILE;i++){				
	  if((NULL != g_hansi[i])&&(NULL != (vrrp = g_hansi[i]->vlist))){ 
		  /*check ifname if in vrrp*/
do_vrrp:

			if (VRRP_LINK_SETTED == vrrp->uplink_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->uplink_vif[j].set_flg) &&
						(!strncmp("ebr",vrrp->uplink_vif[j].ifname,3)))
					{
						/*uplink is ebr interface,check if ifname belong to ebr*/
						vrrp_syslog_dbg("uplink if name %s belong to ebr interface!\n",
										vrrp->uplink_vif[j].ifname);
						sprintf(path,EBR_PATH,vrrp->uplink_vif[j].ifname,ifname);
						if((fd = open(path,O_RDONLY,0))<=0){
							vrrp_syslog_dbg("%s can not open,%s not in %s\n",
											path,ifname,vrrp->uplink_vif[j].ifname);
						}
						else{
							/* add by jinpc, 20091012, for close fd */
							close(fd);

							/*if link changed ifname in ebr interface,check all members link state*/
							ret = had_check_ebr_state(vrrp->uplink_vif[j].ifname, &(vrrp->uplink_vif[j].linkstate));
							if (ret != 0) {
					vrrp_syslog_error("get uplink ebr %s link-status error %d!\n",
												vrrp->uplink_vif[j].ifname, ret);
							}
							vrrp_syslog_dbg("vrrp set uplink %s link state %s\n",
											vrrp->uplink_vif[j].ifname,\
											vrrp->uplink_vif[j].linkstate ? "up" : "down");
							return;
						}
					}
				}
			}
			if (VRRP_LINK_SETTED == vrrp->l2_uplink_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->l2_uplink_vif[j].set_flg) &&
						(!strncmp("ebr",vrrp->l2_uplink_vif[j].ifname,3)))
					{
						/*uplink is ebr interface,check if ifname belong to ebr*/
						vrrp_syslog_dbg("l2-uplink if name %s belong to ebr interface!\n",
										vrrp->l2_uplink_vif[j].ifname);
						sprintf(path,EBR_PATH,vrrp->l2_uplink_vif[j].ifname,ifname);
						if((fd = open(path,O_RDONLY,0))<=0){
							vrrp_syslog_dbg("l2-uplink %s can not open,%s not in %s\n",
											path,ifname,vrrp->l2_uplink_vif[j].ifname);
						}
						else{
							
							close(fd);

							/*if link changed ifname in ebr interface,check all members link state*/
							ret = had_check_l2_ebr_state(vrrp->l2_uplink_vif[j].ifname, &(vrrp->l2_uplink_vif[j].linkstate));
							if (ret != 0) {
								vrrp_syslog_error("l2-uplink get ebr %s link-status error %d!\n",
												vrrp->l2_uplink_vif[j].ifname, ret);
							}
							vrrp_syslog_dbg("l2-uplink set %s link state %s\n",
											vrrp->l2_uplink_vif[j].ifname,\
											vrrp->l2_uplink_vif[j].linkstate ? "up" : "down");
							return;
						}
					}
				}
			}
			
			if (VRRP_LINK_SETTED == vrrp->downlink_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->downlink_vif[j].set_flg) &&
						(!strncmp("ebr",vrrp->downlink_vif[j].ifname,3)))
					{
						/*downlink is ebr interface,check if ifname belong to ebr*/
						memset(path,0,128);			  
						vrrp_syslog_dbg("downlink if name %s belong to ebr interface!\n",
										vrrp->downlink_vif[j].ifname);
						sprintf(path,EBR_PATH,vrrp->downlink_vif[j].ifname,ifname);
						if((fd = open(path,O_RDONLY,0))<=0){
							vrrp_syslog_dbg("%s can not open,%s not in %s\n",
											path,ifname,vrrp->downlink_vif[j].ifname);
						}
						else {
							/* add by jinpc, 20091012, for close fd */
							close(fd);

							/*if link changed ifname in ebr interface,check all members link state*/
							ret = had_check_ebr_state(vrrp->downlink_vif[j].ifname, &(vrrp->downlink_vif[j].linkstate));
							if (ret != 0) {
								vrrp_syslog_error("get downlink ebr %s link-status error!\n",
												vrrp->downlink_vif[j].ifname);
							}
							vrrp_syslog_dbg("vrrp set downlink %s link state %s\n",vrrp->downlink_vif[j].ifname,\
											vrrp->downlink_vif[j].linkstate ? "up" : "down");
							return;
						}
					}
				}
			}

			#if 1
			/* add by jinpc@autelan.com 2009.11.17
			 * for get vgateway interface's link state.
			 */
			if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED ==vrrp->vgateway_vif[j].set_flg) &&
							(!strncmp("ebr", vrrp->vgateway_vif[j].ifname, 3)))
					{
						vrrp_syslog_dbg("vgateway %d if name %s belong to ebr interface!\n",
										j, vrrp->vgateway_vif[j].ifname);

						/* vgateway interface is ebr interface, check if ifname belong to ebr */
						memset(path, 0, 128);
						sprintf(path, EBR_PATH, vrrp->vgateway_vif[j].ifname, ifname);
						if ((fd = open(path, O_RDONLY, 0)) <= 0) {
							vrrp_syslog_dbg("%s can not open,%s not in %s\n",
											path, ifname, vrrp->vgateway_vif[j].ifname);
						}else {
							close(fd);

							/* if link changed ifname in ebr interface,
							 * check all members link state
							 */
							ret = had_check_ebr_state(vrrp->vgateway_vif[j].ifname, &(vrrp->vgateway_vif[j].linkstate));
							if (ret != 0) {
								vrrp_syslog_error("get vgateway ebr %s link-status error!\n",
													vrrp->vgateway_vif[j].ifname);
							}
							vrrp_syslog_dbg("vrrp set vgateway %d %s link state %s\n",
											vrrp->vgateway_vif[j].ifname,
											vrrp->vgateway_vif[j].linkstate ? "up" : "down");

							return;
						}
					}
				}
			}
			#endif
		  
			/*if uplink/downlink not ebr interface  , or ifname not ebr member ,just judging the interface status to set uplink or downlink state
			  if its uplink or downlink*/
			int uplink_index = 0;
			int l2_uplink_index = 0;
			int downlink_index = 0;
			unsigned int linkState = INTERFACE_DOWN;
			ret = had_get_link_index_by_ifname(vrrp, ifname, VRRP_LINK_TYPE_UPLINK, &uplink_index);
			if (VRRP_RETURN_CODE_IF_EXIST == ret)
			{
				if ((VRRP_LINK_SETTED == vrrp->uplink_flag) &&
					(!strcmp(vrrp->uplink_vif[uplink_index].ifname,ifname)))
				{
					if(ifi_flags & IFF_RUNNING){
						vrrp->uplink_vif[uplink_index].linkstate = INTERFACE_UP;
						vrrp_syslog_dbg("vrrp %d uplink interface %s link-up!\n",
										vrrp->vrid,
										vrrp->uplink_vif[uplink_index].ifname); 		   
					}
					else{
						vrrp->uplink_vif[uplink_index].linkstate = INTERFACE_DOWN;
						vrrp_syslog_dbg("vrrp %d uplink interface %s link-down!\n",
										vrrp->vrid,
										vrrp->uplink_vif[uplink_index].ifname); 
					}
				}
			}

			ret = had_get_link_index_by_ifname(vrrp, ifname, VRRP_LINK_TYPE_DOWNLINK, &downlink_index);
			if (VRRP_RETURN_CODE_IF_EXIST == ret)
			{
				if ((VRRP_LINK_SETTED == vrrp->downlink_flag) &&
					(!strcmp(vrrp->downlink_vif[downlink_index].ifname, ifname)))
				{
					if(ifi_flags & IFF_RUNNING){
						vrrp->downlink_vif[downlink_index].linkstate = INTERFACE_UP;
						vrrp_syslog_dbg("vrrp %d downlink interface %s link-up!\n",
										vrrp->vrid,
										vrrp->downlink_vif[downlink_index].ifname);
					}
					else{
						vrrp->downlink_vif[downlink_index].linkstate = INTERFACE_DOWN;
						vrrp_syslog_dbg("vrrp %d downlink interface %s link-down!\n",
										vrrp->vrid,
										vrrp->downlink_vif[downlink_index].ifname);
					}				 
				}	
			}
			#if 1
			/* add by jinpc@autelan.com 2009.11.17
			 * for get vgateway interface's link state.
			 */
			if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->vgateway_vif[j].set_flg) &&
							(!strcmp(vrrp->vgateway_vif[j].ifname, ifname)))
					{
						if (ifi_flags & IFF_RUNNING) {
							vrrp->vgateway_vif[j].linkstate = INTERFACE_UP;
						}else {
							vrrp->vgateway_vif[j].linkstate = INTERFACE_DOWN;
						}
						vrrp_syslog_dbg("vrrp %d vgateway %d interface %s %s!\n",
										vrrp->vrid, j, ifname, (ifi_flags & IFF_RUNNING) ? "up":"down");
					}
				}
			}
			#endif
		   
		  /*if vsrv argument is sure,break;*/
		  if(NULL != vsrv){
             return;
		  }
	  }
   }
   /*ifname not in vrrp instance,not care*/
   return;
}

void had_read_link(void* ptr,int totlemsglen)
{
	
	struct	ifinfomsg *rtEntry = NULL;
	struct nlmsghdr *nlp = (struct nlmsghdr *)ptr;
	int payloadoff = 0,ret = 0;
	struct rtattr *rtattrptr = NULL;
	unsigned char* ifaddr = NULL;
	char* ifname = NULL;
	unsigned int ifi_flags = 0, ifi_index = ~0UI;
	unsigned int status = 0;
	unsigned int isAdd = 0;
	struct ifaddrmsg *ifa;
	struct rtattr *tb[IFA_MAX + 1];
	static char buf[BUFLENGTH] = { 0 };
	unsigned char ifnameArray[MAX_IFNAME_LEN] = {0};
	unsigned int ipaddr = 0;

	for(;NLMSG_OK(nlp, totlemsglen);nlp=NLMSG_NEXT(nlp, totlemsglen)) {
		rtEntry = (struct ifinfomsg *) NLMSG_DATA(nlp);
		payloadoff = RTM_PAYLOAD(nlp);
		isAdd = 0;
		switch( nlp->nlmsg_type ) {
		  case RTM_NEWLINK:
			ifi_flags = rtEntry->ifi_flags;  /* get interface */
			ifi_index = rtEntry->ifi_index; /* ifindex for kernel*/
			rtattrptr = (struct rtattr *)IFLA_RTA(rtEntry);
			for(;RTA_OK(rtattrptr, payloadoff);rtattrptr=RTA_NEXT(rtattrptr,payloadoff)) {
				switch(rtattrptr->rta_type) {
					case IFLA_ADDRESS:
						ifaddr = (unsigned char*)RTA_DATA(rtattrptr);
						break;
					case IFLA_IFNAME:
						ifname = RTA_DATA(rtattrptr);
						/*npd_syslog_dbg("ifindex %d,vid %d\n",ifindex,vid);*/
						break;
					default:
						/*npd_syslog_dbg("other value ignore %d\n",nlp->nlmsg_type);*/
						break;
				}
			}
			/* check whether this message is for L3 interface mac address update*/
			if(ifaddr && ifname) {
				/*care hearbeatlink interface*/
				if((NULL != global_ht_ifname)&&(!strcmp(ifname,global_ht_ifname))){
					if(ifi_flags & IFF_RUNNING){
					   had_sleep_seconds(1);
					   ret = had_ifname_to_status_byIoctl( global_ht_ifname, &status);
					   if ((ret == 0) &&
						   (status == INTERFACE_UP)) {
					       global_ht_state = 1;
			               vrrp_syslog_dbg("interface %s up!\n",ifname);
					   }
					   else if(0 == ret){
						   vrrp_syslog_dbg("get heartbeat interface %s link status %s!\n", \
						   				ifname, (INTERFACE_UP == status) ? "up":"down");
					   }
					   else {
						   vrrp_syslog_error("get heartbeat interface %s link status error!\n", ifname);
					   }
					}
					else{
					   had_sleep_seconds(1);
					   ret  = had_ifname_to_status_byIoctl( global_ht_ifname, &status);
					   if ((ret == 0) &&
						   (status == INTERFACE_DOWN)) {
						   global_ht_state = 0;
						   vrrp_syslog_dbg("heartbeat interface %s down!\n",ifname);
					   }else if(0 == ret) {
						   vrrp_syslog_dbg("get heartbeat interface %s link status %s!\n",  \
						   				ifname, (INTERFACE_UP == status) ? "up":"down");
					   }
					   else {
						   vrrp_syslog_error("get heartbeat interface %s link status error!\n", ifname);
					   }
					}
				}
				vrrp_syslog_dbg("%s,%d,interface %s!\n",__func__,__LINE__,ifname);
				/*judge uplink & downlink & vgateway */	
				had_set_link_state(ifname,ifi_flags,NULL);
			}	
			break;
		  case 	RTM_NEWADDR:
		  	    isAdd = 1;
		  case  RTM_DELADDR:
				memset (tb, 0, sizeof tb);
				ifa = NLMSG_DATA (nlp);
				int len = nlp->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifaddrmsg));
				if(len < 0){
					continue;
				}
				struct rtattr *rta = IFA_RTA (ifa);
				while (RTA_OK (rta, len))
				{
					if (rta->rta_type <= IFA_MAX){
						tb[rta->rta_type] = rta;
				  	}
					rta = RTA_NEXT (rta, len);
				}
				ifname = if_indextoname(ifa->ifa_index,ifnameArray);
				if (tb[IFA_LOCAL]){
					memcpy(&ipaddr,RTA_DATA (tb[IFA_LOCAL]),sizeof(unsigned int));
				}							
				vrrp_syslog_info("%s ipaddr %#x ifname %s\n",isAdd? "add":"delete",ipaddr,ifname);
				if(ifname){
					had_real_ipaddr_syn(ifname,ipaddr,isAdd);
				}
				else{
					vrrp_syslog_info("get ifname null ifindex %#x when %s ip address %#x\n", 
						ifa->ifa_index, isAdd ? "add":"delete", ipaddr);
				}
				break;
			default:
			  break;
		}
	}	
	return;
}
/*****************************************************************************
 * DESCRIPTION:
 * 	synchronize the real ip for hansi uplink/downlink/vgateway   when interface ip's deleted or new ip added
 * INPUT:
 *	ifname    --   the ifname which ip address deleted or added
 *	ifaddr	--   the ip address is deleted or added
 *	isAdd	--   add (!0) or delete (0) 
 * OUTPUT:
 *	NONE
 * RETURN:
 *	return 0
 *****************************************************************************/
unsigned int had_real_ipaddr_syn(unsigned char * ifname, unsigned int ifaddr, unsigned int isAdd){

	int i = 0,j = 0, k = 0;
	struct vrrp_t * vrrp = NULL;
	unsigned int ipAddrs[MAX_IP_COUNT] = { 0 };
	unsigned int masks[MAX_IP_COUNT] = { 0 };
	for(i = 0; i < MAX_HANSI_PROFILE; i++)
	{				
		if((NULL != g_hansi[i])&&(NULL != (vrrp = g_hansi[i]->vlist)))
		{ 
			had_intf_netlink_get_ip_addrs(ifname, ipAddrs, masks);
			if (VRRP_LINK_SETTED == vrrp->uplink_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->uplink_vif[j].set_flg) &&
						(!strcmp(ifname,vrrp->uplink_vif[j].ifname)))
					{
						if(!isAdd)
						{//delete an ip addr
							if(vrrp->uplink_vif[j].ipaddr == ifaddr)
							{// if delete real ip address , try to find a new ip as real ip
								vrrp->uplink_vif[j].ipaddr = 0;
								for(k = 0; k < MAX_IP_COUNT; k++)
								{
									if(vrrp->uplink_vaddr[j].addr != ipAddrs[k] && \
										INVALID_HOST_IP != ipAddrs[k] && ipAddrs[k])
									{
										vrrp->uplink_vif[j].ipaddr = ipAddrs[k];
										vrrp_syslog_info("set another ip address %#x as uplink real ip, when old real ip %#x delete\n",
											ifaddr, ipAddrs[k]);
										break;
									}
								}
							}						
						}
						else{//Add a new ip for interface,  if the real ip not in the interface , set new ip as real ip
							int ipfind = FALSE;
							for(k = 0; k < MAX_IP_COUNT; k++)
							{
								if((vrrp->uplink_vif[j].ipaddr == ipAddrs[k])&& \
										INVALID_HOST_IP != ipAddrs[k] && ipAddrs[k])
								{
									ipfind = TRUE;
								}
							}
							if((FALSE == ipfind) && (ifaddr, vrrp->uplink_vaddr[j].addr != ifaddr))
							{
								vrrp->uplink_vif[j].ipaddr = ifaddr;
								vrrp_syslog_info("can't find the uplink real ip in the interface, set the new added ip %#x as the new real ip\n",
											ifaddr );
							}
						}
					}
				}
			}
			if (VRRP_LINK_SETTED == vrrp->downlink_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->downlink_vif[j].set_flg) &&
						(!strcmp(ifname,vrrp->downlink_vif[j].ifname)))
					{
						if(!isAdd)
						{//delete an ip addr		
							if(vrrp->downlink_vif[j].ipaddr == ifaddr)
							{// if delete real ip address , try to find a new ip as real ip
								vrrp->downlink_vif[j].ipaddr = 0;
								for(k = 0; k < MAX_IP_COUNT; k++)
								{
									if((vrrp->downlink_vaddr[j].addr != ipAddrs[k]) && \
										INVALID_HOST_IP != ipAddrs[k] && ipAddrs[k])
									{
										vrrp->downlink_vif[j].ipaddr = ipAddrs[k];
										vrrp_syslog_info("set another ip address %#x as downlink real ip, when old real ip %#x delete\n",
											ifaddr, ipAddrs[k]);
										break;
									}
								}
							}						
						}
						else{//Add a new ip for interface,  if the real ip not in the interface , set new ip as real ip
							int ipfind = FALSE;
							for(k = 0; k < MAX_IP_COUNT; k++)
							{
								if(vrrp->downlink_vif[j].ipaddr == ipAddrs[k] && \
										INVALID_HOST_IP != ipAddrs[k] && ipAddrs[k])
								{
									ipfind = TRUE;
								}
							}
							if((FALSE == ipfind) && (ifaddr, vrrp->downlink_vaddr[j].addr != ifaddr))
							{
								vrrp->downlink_vif[j].ipaddr = ifaddr;
								vrrp_syslog_info("can't find the downlink real ip in the interface, set the new added ip %#x as the new real ip\n",
											ifaddr );
							}
						}
					}
				}
			}
			if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->vgateway_vif[j].set_flg) &&
						(!strcmp(ifname,vrrp->vgateway_vif[j].ifname)))
					{
						if(!isAdd)
						{//delete an ip addr		
							if(vrrp->vgateway_vif[j].ipaddr == ifaddr)
							{// if delete real ip address , try to find a new ip as real ip
								vrrp->vgateway_vif[j].ipaddr = 0;
								for(k = 0; k < MAX_IP_COUNT; k++)
								{
									if((vrrp->vgateway_vaddr[j].addr != ipAddrs[k]) && \
										INVALID_HOST_IP != ipAddrs[k] && ipAddrs[k])
									{
										vrrp->vgateway_vif[j].ipaddr = ipAddrs[k];
										vrrp_syslog_info("set another ip address %#x as vgateway real ip, when old real ip %#x delete\n",
											ifaddr, ipAddrs[k]);
										break;
									}
								}
							}						
						}
						else{//Add a new ip for interface,  if the real ip not in the interface , set new ip as real ip
							int ipfind = FALSE;
							for(k = 0; k < MAX_IP_COUNT; k++)
							{
								if(vrrp->vgateway_vif[j].ipaddr == ipAddrs[k]&& \
										INVALID_HOST_IP != ipAddrs[k] && ipAddrs[k])
								{
									ipfind = TRUE;
								}
							}
							if((FALSE == ipfind) && (vrrp->vgateway_vaddr[j].addr != ifaddr))
							{
								vrrp->vgateway_vif[j].ipaddr = ifaddr;
								vrrp_syslog_info("can't find the vgateway real ip in the interface, set the new added ip %#x as the new real ip\n",
											ifaddr );
							}
						}
					}
				}
			}
			
		}
	}
	return 0;
}
/***********************************************************************
 * DESCRIPTION:
 *		get ip addresses of a interface 
 * INPUT:
 *		ifname       --   the name of the interface which we want to get the ip addresses
 * OUTPUT:
 *		ipAddrs      --   ip addresses array
 *		masks	   --   mask array
 * RETURN:
 *		VRRP_RETURN_CODE_ERR   --   some error occurred 
 *		VRRP_RETURN_CODE_SUCCESS    -- get success
 ***********************************************************************/
int had_intf_netlink_get_ip_addrs
(
	unsigned char* ifName,
	unsigned int* ipAddrs,
	unsigned int* masks
) {
#define _16K 16384
	int fd = -1;
	int status = 0;
	int err = 0;
	int len = 0;
	int i = 0;
	int ret = VRRP_RETURN_CODE_SUCCESS;
	int sndbuf = _16K*2;
	int rcvbuf = _16K*2;
	int seq = time(NULL);
	static char buf[_16K] = {0};
	char abuf[256] = {0};
	socklen_t addr_len;
	unsigned int ifIndex = 0;
	struct nlmsghdr *h = NULL;
	struct ifaddrmsg *ifa = NULL;
	struct rtattr * rta_tb[IFA_MAX+1] = {NULL};
	struct rtattr *rta = NULL;
	struct sockaddr_nl	local;
	struct sockaddr_nl nladdr;
	struct sockaddr_nl nladdr2;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;
	memset(&nladdr, 0, sizeof(struct sockaddr_nl));
	memset(&iov, 0, sizeof(struct iovec));
	if ((NULL == ifName) || (NULL == ipAddrs) || (NULL == masks)) {
		return VRRP_RETURN_CODE_ERR;
	}
	ifIndex = if_nametoindex(ifName);
	if (!ifIndex) {
		return VRRP_RETURN_CODE_ERR;
	}
	for (i = 0; i < MAX_IP_COUNT; i++) {
		ipAddrs[i] = INVALID_HOST_IP;
		masks[i] = 0;
	}
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	h = (struct nlmsghdr*)buf;
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd < 0) {
		vrrp_syslog_error(" socket error %d \n", fd);
		return VRRP_RETURN_CODE_ERR;
	}
	if ((ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf))) < 0) {
		vrrp_syslog_error("setsocketopt SO_SNDBUF error %d \n", ret);
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}

	if ((ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf))) < 0) {
		vrrp_syslog_error("setsockopt SO_RCVBUF error %d\n", ret);
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_IPV4_IFADDR;

	if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0) {
		vrrp_syslog_error("Cannot bind netlink socket\n");
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}
	addr_len = sizeof(local);
	if (getsockname(fd, (struct sockaddr*)&local, &addr_len) < 0) {
		vrrp_syslog_error("Cannot getsockname");
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}
	if (addr_len != sizeof(local)) {
		vrrp_syslog_error("Wrong address length %d\n", addr_len);
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}
	if (local.nl_family != AF_NETLINK) {
		vrrp_syslog_error("Wrong address family %d\n", local.nl_family);
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}

	memset(&nladdr2, 0, sizeof(nladdr2));
	nladdr2.nl_family = AF_NETLINK;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = RTM_GETADDR;
	req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = ++seq;
	req.g.rtgen_family = AF_NETLINK;

	if ((ret = sendto(fd, (void*) & req, sizeof(req), 0,
					  (struct sockaddr*) & nladdr2, sizeof(nladdr2))) < 0) {
		vrrp_syslog_error("Send request error %d\n", ret);
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}
	status = recvmsg(fd, &msg, 0);
	if (status < 0) {
		vrrp_syslog_error("get msg error when get ip address,status %d \n", status);
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}

	if (status == 0) {
		vrrp_syslog_error("EOF on netlink\n");
		close(fd);
		return VRRP_RETURN_CODE_ERR;
	}
	i = 0;
	vrrp_syslog_dbg("had_intf_get_intf_ip_addrs::get if %#0x ip address:\n", ifIndex);
	while ((NLMSG_OK(h, status)) && (i < MAX_IP_COUNT)) {
		if ((h->nlmsg_type != RTM_NEWADDR) && (h->nlmsg_type != RTM_DELADDR)) {
			close(fd);
			return VRRP_RETURN_CODE_SUCCESS;
		}
		if ((len = (h->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa)))) < 0) {
			vrrp_syslog_error("wrong nlmsg len %d\n", len);
		}
		ifa = NLMSG_DATA(h);
		if (ifIndex == ifa->ifa_index) {
			memset(rta_tb, 0, sizeof(struct rtattr *) *(IFA_MAX + 1));
			rta = IFA_RTA(ifa);
			while (RTA_OK(rta, len)) {
				if (rta->rta_type <= IFA_MAX) {
					rta_tb[rta->rta_type] = rta;
				}
				rta = RTA_NEXT(rta, len);
			}
			ipAddrs[i] = (unsigned int)(*((unsigned int *)RTA_DATA(rta_tb[IFA_LOCAL])));
			ipAddrs[i] = htonl(ipAddrs[i]);
			masks[i] = (unsigned int)(-(1 << (32 - ifa->ifa_prefixlen)));
			masks[i] = htonl(masks[i]);
			if (INVALID_HOST_IP != ipAddrs[i]) {
				vrrp_syslog_dbg("\t%d.%d.%d.%d %d.%d.%d.%d\n",  \
								   (ipAddrs[i] >> 24)&0xFF, (ipAddrs[i] >> 16)&0xFF, (ipAddrs[i] >> 8)&0xFF, (ipAddrs[i])&0xFF, \
								   (masks[i] >> 24)&0xFF, (masks[i] >> 16)&0xFF, (masks[i] >> 8)&0xFF, (masks[i])&0xFF);
				i++;
			}
		}
		h = NLMSG_NEXT(h, status);
	}
	close(fd);
	return VRRP_RETURN_CODE_SUCCESS;
}

/*
 *******************************************************************************
 *had_update_vlink_state()
 *
 *  DESCRIPTION:
 *		update uplink/downlink/vgateway interface's link state.
 *  INPUTS:
 *		vrrp - hansi insante control block info
 *		type - virtual link type which maybe uplink/downlink/vgateway
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_BAD_PARAM  - null pointer error
 *
 *******************************************************************************
 */
int had_update_vlink_state
(
	vrrp_rt* vrrp,
	VRRP_LINK_TYPE	type
)
{
	int ret = VRRP_RETURN_CODE_OK, j = 0;
	unsigned int set_flag = VRRP_LINK_NO_SETTED, tmp_status = INTERFACE_DOWN;
	vrrp_if *vif = NULL;
	int linktype = 0;

	if(!vrrp) {
		vrrp_syslog_error("update %s interface state null pointer error!\n", VRRP_LINK_TYPE_DESCANT(type));
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	
	switch(type)
	{
		default:
			vrrp_syslog_error("update interface state with unknown link type %d!\n", type);
			return VRRP_RETURN_CODE_BAD_PARAM;
			break;
		case VRRP_LINK_TYPE_UPLINK:
			set_flag = vrrp->uplink_flag;
			vif = &(vrrp->uplink_vif[0]);
			linktype = 0;
			break;
		case VRRP_LINK_TYPE_DOWNLINK:
			set_flag = vrrp->downlink_flag;
			vif = &(vrrp->downlink_vif[0]);
			linktype = 0;
			break;
		case VRRP_LINK_TYPE_VGATEWAY:
			set_flag = vrrp->vgateway_flag;
			vif = &(vrrp->vgateway_vif[0]);
			linktype = 0;
			break;			
		case VRRP_LINK_TYPE_L2_UPLINK:
			set_flag = vrrp->l2_uplink_flag;
			vif = &(vrrp->l2_uplink_vif[0]);
			linktype = 1;
			break;
	}

	if (VRRP_LINK_SETTED == set_flag)
	{
		if(linktype == 0)
		{
		for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
		{
			if(VRRP_LINK_NO_SETTED == vif[j].set_flg) {
				continue;
			}
			
			if (!strncmp("ebr", vif[j].ifname, 3))
			{
				/*if link changed ifname in ebr interface,check all members link state*/
				ret = had_check_ebr_state(vif[j].ifname, &(vif[j].linkstate));
				if (ret != 0) {
					vrrp_syslog_error("vrrp %d get %s %s link-status error!\n",
									vrrp->vrid,VRRP_LINK_TYPE_DESCANT(type), vif[j].ifname);
				} 
				else {
					vrrp_syslog_dbg("vrrp %d get %s %s link state %s\n",
									vrrp->vrid, VRRP_LINK_TYPE_DESCANT(type),vif[j].ifname,
									(vif[j].linkstate == INTERFACE_UP) ? "up" : "down");
				}
			}
			else {
				ret = had_ifname_to_status_byIoctl(vif[j].ifname, &tmp_status);
				if (ret != 0) {
					vrrp_syslog_error("vrrp %d update %s %s status error!\n",
										vrrp->vrid,VRRP_LINK_TYPE_DESCANT(type),vif[j].ifname);
				}else {
					vif[j].linkstate = (char)tmp_status;
					vrrp_syslog_dbg("vrrp %d update %s %s status %s!\n",
									vrrp->vrid, VRRP_LINK_TYPE_DESCANT(type),
									vif[j].ifname, (vif[j].linkstate == INTERFACE_UP) ?  "up" : "down");
				}
				tmp_status = INTERFACE_DOWN;
				}
			}
		}
		else
		{
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
				if(VRRP_LINK_NO_SETTED == vif[j].set_flg) {
					continue;
				}
				
				if (!strncmp("ebr", vif[j].ifname, 3))
				{
					/*if link changed ifname in ebr interface,check all members link state*/
					ret = had_check_l2_ebr_state(vif[j].ifname, &(vif[j].linkstate));
					
					if (ret != 0) {
						vrrp_syslog_error("vrrp %d get %s %s link-status error!\n",
										vrrp->vrid,VRRP_LINK_TYPE_DESCANT(type), vif[j].ifname);
					} 
					else {
						vrrp_syslog_dbg("vrrp %d get %s %s link state %s\n",
										vrrp->vrid, VRRP_LINK_TYPE_DESCANT(type),vif[j].ifname,
										(vif[j].linkstate == INTERFACE_UP) ? "up" : "down");
					}
				}
				else {
					ret = had_ifname_to_status_byIoctl(vif[j].ifname, &tmp_status);
					vrrp_syslog_dbg("vrrp %d update %s %s tmp_status %s!\n",
									vrrp->vrid, VRRP_LINK_TYPE_DESCANT(type),
									vif[j].ifname, (tmp_status == INTERFACE_UP) ?  "up" : "down");
					if (ret != 0) {
						vrrp_syslog_error("vrrp %d update %s %s status error!\n",
											vrrp->vrid,VRRP_LINK_TYPE_DESCANT(type),vif[j].ifname);
					}else {
						vif[j].linkstate = (char)tmp_status;
						vrrp_syslog_dbg("vrrp %d update %s %s status %s!\n",
										vrrp->vrid, VRRP_LINK_TYPE_DESCANT(type),
										vif[j].ifname, (vif[j].linkstate == INTERFACE_UP) ?  "up" : "down");
					}
					tmp_status = INTERFACE_DOWN;
				}
			}
		}
	}

	return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 *had_update_uplink_downlink_state()
 *
 *  DESCRIPTION:
 *		update uplink/downlink/vgateway interface's link state.
 *  INPUTS:
 *		NULL
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
void had_update_uplink_downlink_state
(
	void
)
{
	int ret = 0;
	int i = 0;
	vrrp_rt* vrrp = NULL;

	for (; i < MAX_HANSI_PROFILE; i++)
	{				
		if ((NULL != g_hansi[i]) &&
			(NULL != (vrrp = g_hansi[i]->vlist)))
		{ 
			/*check ifname if in vrrp*/
			if (VRRP_LINK_SETTED == vrrp->uplink_flag)
			{
				ret = had_update_vlink_state(vrrp, VRRP_LINK_TYPE_UPLINK);
				if(VRRP_RETURN_CODE_OK != ret) {
					vrrp_syslog_error("vrrp %d update uplink state error %d\n", ret);
				}
				#if 0
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if(VRRP_LINK_NO_SETTED == vrrp->uplink_vif[j].set_flg) {
						continue;
					}
					
					if (!strncmp("ebr", vrrp->uplink_vif[j].ifname, 3))
					{
						/*uplink is ebr interface,check if ifname belong to ebr*/
						vrrp_syslog_dbg("uplink if name %s belong to ebr interface!\n",
										vrrp->uplink_vif[j].ifname);
						/*if link changed ifname in ebr interface,check all members link state*/
						ret = had_check_ebr_state(vrrp->uplink_vif[j].ifname, &(vrrp->uplink_vif[j].linkstate));
						if (ret != 0) {
							vrrp_syslog_error("vrrp %d get uplink %s link-status error %d!\n",
											vrrp->vrid, vrrp->uplink_vif[j].ifname, ret);
						} 
						else {
							vrrp_syslog_dbg("vrrp %d get uplink %s link state %s\n",
											vrrp->vrid, vrrp->uplink_vif[j].ifname,
											(vrrp->uplink_vif[j].linkstate == INTERFACE_UP) ? "up" : "down");
						}
					}
					else {
						ret = had_ifname_to_status_byIoctl(vrrp->uplink_vif[j].ifname, &tmp_status);
						if (ret != 0) {
							vrrp_syslog_error("vrrp %d update uplink %s status error %d!\n",
												vrrp->vrid,
												vrrp->uplink_vif[j].ifname, ret);
						}else {
							vrrp->uplink_vif[j].linkstate = (char)tmp_status;
							vrrp_syslog_dbg("vrrp %d update uplink %s status %s!\n",
											vrrp->vrid,
											vrrp->uplink_vif[j].ifname,
											(vrrp->uplink_vif[j].linkstate == INTERFACE_UP) ?  "up" : "down");
						}
						tmp_status = INTERFACE_DOWN;
					}
				}
				#endif
			}
			/*check ifname if in vrrp*/
			if (VRRP_LINK_SETTED == vrrp->l2_uplink_flag)
			{
				ret = had_update_vlink_state(vrrp, VRRP_LINK_TYPE_L2_UPLINK);
				if(VRRP_RETURN_CODE_OK != ret) {
					vrrp_syslog_error("vrrp %d update uplink state error %d\n", ret);
				}
			}
			
			
			if (VRRP_LINK_SETTED == vrrp->downlink_flag)
			{
				ret = had_update_vlink_state(vrrp, VRRP_LINK_TYPE_DOWNLINK);
				if(VRRP_RETURN_CODE_OK != ret) {
					vrrp_syslog_error("vrrp %d update uplink state error %d\n", ret);
				}
				#if 0
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->downlink_vif[j].set_flg) &&
						(!strncmp("ebr", vrrp->downlink_vif[j].ifname, 3)))
					{
						/*downlink is ebr interface,check if ifname belong to ebr*/
						/*if link changed ifname in ebr interface,check all members link state*/
						ret = had_check_ebr_state(vrrp->downlink_vif[j].ifname, &(vrrp->downlink_vif[j].linkstate));
						if (ret != 0) {
							vrrp_syslog_error("vrrp %d get downlink %s link-status error %d!\n",
												vrrp->vrid, vrrp->downlink_vif[j].ifname ,ret);
						}
						else {
							vrrp_syslog_dbg("vrrp %d get downlink %s link state %s\n",
											vrrp->vrid, vrrp->downlink_vif[j].ifname,
											(vrrp->downlink_vif[j].linkstate == INTERFACE_UP) ? "up" : "down");
						}
					}
				}
				#endif
			}

			/* add by jinpc@autelan.com 2009.11.17
			 * for get vgateway interface's link state.
			 */
			if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
			{
				ret = had_update_vlink_state(vrrp, VRRP_LINK_TYPE_VGATEWAY);
				if(VRRP_RETURN_CODE_OK != ret) {
					vrrp_syslog_error("vrrp %d update uplink state error %d\n", ret);
				}
				#if 0
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->vgateway_vif[j].set_flg) &&
						(!strncmp("ebr", vrrp->vgateway_vif[j].ifname, 3)))
					{
						/* vgateway interface is ebr interface,check if ifname belong to ebr*/
						/*if link changed ifname in ebr interface,check all members link state*/
						ret = had_check_ebr_state(vrrp->vgateway_vif[j].ifname, &(vrrp->vgateway_vif[j].linkstate));
						if (ret != 0) {
							vrrp_syslog_error("vrrp %d get vgateway ebr %s link-status error!\n",
												vrrp->vrid, vrrp->vgateway_vif[j].ifname);
						}
						else {
							vrrp_syslog_dbg("vrrp %d get vgateway %s link state %s\n",
											vrrp->vrid, vrrp->vgateway_vif.ifname,
											(vrrp->vgateway_vif.linkstate == INTERFACE_UP) ? "up" : "down");
						}
					}
				}
				#endif
			}

			#if 0
			/*  if uplink/downlink not ebr interface, or ifname not ebr member,
				just judging the interface status to set uplink or downlink state
				if its uplink or downlink
			*/
			if (VRRP_LINK_SETTED == vrrp->uplink_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->uplink_vif[j].set_flg) &&
						(strncmp("ebr", vrrp->uplink_vif[j].ifname, 3)))
					{
						ret = had_ifname_to_status_byIoctl(vrrp->uplink_vif[j].ifname, &tmp_status);
						if (ret != 0) {
							vrrp_syslog_error("vrrp %d update uplink %s status error!\n",
												vrrp->vrid,
												vrrp->uplink_vif[j].ifname);
						}else {
							vrrp->uplink_vif[j].linkstate = (char)tmp_status;
							vrrp_syslog_dbg("vrrp %d update uplink %s status %s!\n",
											vrrp->vrid,
											vrrp->uplink_vif[j].ifname,
											(vrrp->uplink_vif[j].linkstate == INTERFACE_UP) ?  "up" : "down");
						}
						tmp_status = INTERFACE_DOWN;
					}
				}
			}

			if (VRRP_LINK_SETTED == vrrp->downlink_flag)
			{
				for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
				{
					if ((VRRP_LINK_SETTED == vrrp->downlink_vif[j].set_flg) &&
						(strncmp("ebr", vrrp->downlink_vif[j].ifname, 3)))
					{
						ret = had_ifname_to_status_byIoctl(vrrp->downlink_vif[j].ifname, &tmp_status);
						if (ret != 0) {
							vrrp_syslog_error("vrrp %d update downlink %s status error!\n",
												vrrp->vrid,
												vrrp->downlink_vif[j].ifname);
						}else {
							vrrp->downlink_vif[j].linkstate = (char)tmp_status;
							vrrp_syslog_dbg("vrrp %d update downlink %s status %s!\n",
											vrrp->vrid,
											vrrp->downlink_vif[j].ifname,
											(vrrp->downlink_vif[j].linkstate == INTERFACE_UP) ?  "up" : "down");
						}
						tmp_status = INTERFACE_DOWN;
					}	
				}
			}

			/* add by jinpc@autelan.com 2009.11.17
			* for get vgateway interface's link state.
			*/
			if ((1 == vrrp->vgateway_flag) &&
				(strncmp("ebr", vrrp->vgateway_vif.ifname, 3)))
			{
				ret = had_ifname_to_status_byIoctl(vrrp->vgateway_vif.ifname, &tmp_status);
				if (ret != 0) {
					vrrp_syslog_error("vrrp %d update vgateway interface status error!\n",
									vrrp->vrid);
				}else {
					vrrp->vgateway_vif.linkstate = (char)tmp_status;
					vrrp_syslog_dbg("vrrp %d update vgateway interface status %s!\n",
									vrrp->vrid,
									(vrrp->vgateway_vif.linkstate == INTERFACE_UP) ?  "up" : "down");
				}
				tmp_status = INTERFACE_DOWN;
			}
			#endif
		}
	}

	return;
}

void had_get_netlink(int sockfd)
{
	int msglen=0;
	char buf[4096] = { 0 };
	unsigned int checktimes = 0;
	struct iovec iov = { buf, sizeof buf };
	struct sockaddr_nl snl ;
	struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
	
	char* p = buf;
	struct nlmsghdr *nlp = (struct nlmsghdr *)p,*tmp=NULL;

#if 1
/* add by jinpc 20091010 */
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	memset(&tv, 0, sizeof(struct timeval));

	while (1)
	{
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);

		/* select with 10s timeout */
		tv.tv_usec = 0;
		tv.tv_sec = 10;

		switch(select(sockfd + 1, &rfds, NULL, NULL, &tv))
		{
			case -1:
				vrrp_syslog_error("vrrp get link thread select return -1\n");
				break;
			case 0:
				vrrp_syslog_dbg("vrrp get link thread select return 0.\n");
				/* break; */
			default:
				if (FD_ISSET(sockfd, &rfds))
				{
					msglen = recvmsg(sockfd, &msg, 0);
					if(msglen <= 0)
					{
						continue;
					}
					
					tmp = (struct nlmsghdr *) p;
					if(tmp->nlmsg_flags == NLM_F_MULTI && tmp->nlmsg_type == NLMSG_DONE  ) 
					{
						vrrp_syslog_dbg("%s,%d,receive netlink message,but is NLM_F_MULTI,continue!\n",__func__,__LINE__);
						continue;
					}
					vrrp_syslog_dbg("%s,%d,receive netlink message!\n",__func__,__LINE__);
					had_read_link(nlp,msglen);
					/* update uplink & downlink & vgateway interface */ 
					checktimes ++;
					if(checktimes >= 15){/*1,reduce check times;2,make sure check if state within 15*10s*/
						had_update_uplink_downlink_state();
						checktimes = 0;
					}
				}
				else { /* update heartbeat/uplink/downlink if state */
					unsigned int status = INTERFACE_DOWN;
					unsigned int ret = 0;

					vrrp_syslog_dbg("select timeout when no netlink message received!\n");
					/* update hearbeatlink interface */
					if (NULL != global_ht_ifname) {
					   ret = had_ifname_to_status_byIoctl(global_ht_ifname, &status);
					   if ((ret == 0) &&
						   (status == INTERFACE_UP)) {
						   global_ht_state = 1;
						   vrrp_syslog_dbg("get heartbeat interface %s up!\n", global_ht_ifname);
					   }else {
						   vrrp_syslog_dbg("get heartbeat interface %s link status %s!\n",
						   					global_ht_ifname, ret ? "error" :"down");
					   }
					}

					/* update uplink & downlink & vgateway interface */			
					had_update_uplink_downlink_state();
				}
		}
	}
#endif
}
void had_link_thread_main(void){
	int fd;
	struct sockaddr_nl la;
	
	vrrp_debug_tell_whoami("linkMonitor", 0);	
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(fd<0)
	{
		vrrp_syslog_error("create socket error when get link from netlink!\n");
		return;
	}
	
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
	la.nl_groups = RTMGRP_IPV4_IFADDR|RTMGRP_IPV4_ROUTE |RTMGRP_LINK| RTMGRP_NOTIFY ;

	if(bind(fd, (struct sockaddr*) &la, sizeof(la)))
	{
		vrrp_syslog_error("bind socket %d error when get link from netlink\n", fd);
		close(fd);
		return;
	}
	had_get_netlink(fd);

	return;     
}

void had_arp_thread_main(void){
    unsigned int profile = 0;
	int i = 0;
	hansi_s* hanode = NULL;
	vrrp_rt* vrrp = NULL;
	static int count = 0;
	static int vCount = 0;
	int ret = 0,radioCount = 0,ethCount = 0;
    int j = 0;
	vrrp_arp_send_fd = socket(PF_PACKET, SOCK_PACKET, 0x300); /* 0x300 is magic */

	if( vrrp_arp_send_fd < 0 ){
		vrrp_syslog_error("send arp thread create socket failed!\n");
		return ;
	}
	
	vrrp_syslog_event("send arp thread create global socket success! fd %d\n", vrrp_arp_send_fd);

	vrrp_debug_tell_whoami("arpEcho", 0);
	while(1){
		/*sleep 5 seconds*/
		if(global_state_change_bit){
			had_sleep_seconds(1);
			count++;
			if(count > 20){
			   count = 0;
               global_state_change_bit =0;
			}
			vCount++;
		}
		else{
            while((j++ < 10)&&!(global_state_change_bit)){
		        had_sleep_seconds(1);                
            }
            if(global_state_change_bit){
                count++;
            }
			vCount++;
            j = 0;
		}
		pthread_mutex_lock(&StateMutex);	
		for (profile = 0; profile < MAX_HANSI_PROFILE; profile++) {
           hanode = had_get_profile_node(profile);
		   if((NULL != hanode)&&(NULL != ( vrrp = hanode->vlist))&& \
		   	  (VRRP_STATE_MAST == vrrp->state)){
              /*send arp*/
			  if(VRRP_LINK_NO_SETTED != vrrp->uplink_flag){
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vrrp->uplink_vif[i].set_flg) {
						had_send_uplink_gratuitous_arp(vrrp,vrrp->uplink_vif[i].ipaddr, i, 1);
						had_send_uplink_gratuitous_arp(vrrp,vrrp->uplink_vaddr[i].addr, i, 1);
					}
				}
			  }
			  if(VRRP_LINK_NO_SETTED != vrrp->downlink_flag){
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vrrp->downlink_vif[i].set_flg) {
						had_send_downlink_gratuitous_arp(vrrp,vrrp->downlink_vif[i].ipaddr, i, 1);
						had_send_downlink_gratuitous_arp(vrrp,vrrp->downlink_vaddr[i].addr, i, 1);
					}
				}
			  }   
			  if(VRRP_LINK_NO_SETTED != vrrp->vgateway_flag){
				  for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				  {
				  	  if(VRRP_LINK_NO_SETTED == vrrp->vgateway_vif[i].set_flg) {
						continue;
				  	  }
					  
				  	  /*check if is bridge interface*/
					  if((!strncmp(vrrp->vgateway_vif[i].ifname,"ebr",3))||
					  		(!strncmp(vrrp->vgateway_vif[i].ifname,"wlan",4))){
	                      /*if br interface, check if radio muster */
						  ret = had_check_br_muster(vrrp->vgateway_vif[i].ifname,&radioCount,&ethCount);
						  if(0 != radioCount){/*no eth interfaces,do not need gratuitous arp*/
	                          ;/*continue;*/
						  }
					  }
					  
					  if(vCount){
						  had_send_vgateway_gratuitous_arp(vrrp,vrrp->vgateway_vif[i].ipaddr,i,1);
						  had_send_vgateway_gratuitous_arp(vrrp,vrrp->vgateway_vaddr[i].addr,i,1);
					  }
				  }
				  vCount = 0;
			  }
		   }
		}
		pthread_mutex_unlock(&StateMutex);	
	}
	close(vrrp_arp_send_fd);
	return;     
}
#if 0
/* cut off timesyn */
void had_timesyn_thread_main(void){
    int profile = 0;
	hansi_s* hanode = NULL;
	vrrp_rt* vrrp = NULL;

	vrrp_debug_tell_whoami("timeSync", 1);
	while(1){
		/*sleep 5 minutes*/
		had_sleep_seconds(300);
		pthread_mutex_lock(&StateMutex);	
		for (profile = 0; profile < MAX_HANSI_PROFILE; profile++) {
           hanode = had_get_profile_node(profile);
		   if((NULL != hanode)&&(NULL != ( vrrp = hanode->vlist))&& \
		   	  (VRRP_STATE_MAST == vrrp->state)){
		   	  /*send time syning packets*/
	            struct timeval tv;
				gettimeofday(&tv,NULL);
				vrrp->que_f = 1;
				vrrp_send_adv(vrrp,vrrp->priority);
				vrrp->que_f = 0;	
		   }
		}
		pthread_mutex_unlock(&StateMutex);			
	}
	return;     
}
#endif
/* set timer with this thread after cut off timesyn*/
void vrrp_timer_thread_main(void){

	int profile = 0;
  	int	result = 0, count[MAX_HANSI_PROFILE] = {0};
	hansi_s* hanode = NULL;
	vrrp_rt* vrrp = NULL;
	
	vrrp_debug_tell_whoami("hansiTimer", 0);	
	while(1){
		/*sleep 1 seconds*/
		had_sleep_seconds(1);
		pthread_mutex_lock(&StateMutex);
		for (profile = 1; profile < MAX_HANSI_PROFILE; profile++) {
           hanode = had_get_profile_node(profile);
		   char dbgStr[128] = {0};
		   int len = 0;
		   static int notify_sec_count = 0;
		   if((NULL != hanode)&&(NULL != ( vrrp = hanode->vlist))){
					if(g_boot_check_flag) {
						++count[profile];
						result = had_check_boot_status(profile);
						if(result < 0) { /* error condition */
							if(0 == (count[profile] % HAD_BOOT_CHECK_MSG_HZ)) {
								vrrp_syslog_error("inst%d timer thread check %d times in boot stage error %d\n", \
													profile, count[profile] ,result);
							}
							continue;
						}
						else if(1 == result) { /* in boot stage */
							vrrp_syslog_info("inst%d timer thread wait %d times in boot stage!\n", profile, count[profile]);
							continue;
						}
					}
				
					len += sprintf(dbgStr, " decrease timer hansi profile %d", profile);
			   		VRRP_TIMER_DECREASE(vrrp->uplink_adver_timer);
			   		VRRP_TIMER_DECREASE(vrrp->downlink_adver_timer);
					if(VRRP_STATE_MAST == vrrp->state){
						len += sprintf(dbgStr + len, " upadvtimer %d.%d downadvtimer %d.%d", \
							vrrp->uplink_adver_timer/VRRP_TIMER_HZ,vrrp->uplink_adver_timer%VRRP_TIMER_HZ,\
							vrrp->downlink_adver_timer/VRRP_TIMER_HZ,vrrp->downlink_adver_timer%VRRP_TIMER_HZ);
					}
		   		if(!global_notifying_flag){
			   		VRRP_TIMER_DECREASE(vrrp->uplink_ms_down_timer);
			   		VRRP_TIMER_DECREASE(vrrp->downlink_ms_down_timer);
					if(VRRP_STATE_MAST != vrrp->state){
						len += sprintf(dbgStr + len, " upmsdowntimer %d.%d downmsdowntimer %d.%d", \
							vrrp->uplink_ms_down_timer/VRRP_TIMER_HZ,vrrp->uplink_ms_down_timer%VRRP_TIMER_HZ,\
							vrrp->downlink_ms_down_timer/VRRP_TIMER_HZ,vrrp->downlink_ms_down_timer%VRRP_TIMER_HZ);
					}
					notify_sec_count = 0;					
					vrrp_syslog_dbg("%s\n", dbgStr);
		   		}
				else{
					vrrp_syslog_dbg("%s\n", dbgStr);
					if(!(notify_sec_count % NOTIFY_DEBUG_OUT_INTERVAL)){
						vrrp_syslog_info("notifying to %s process..., for %d seconds already\n", \
							global_notify_obj[global_notifying_flag], global_notify_count[global_notifying_flag]);
					}
					global_notify_count[global_notifying_flag]++;
					notify_sec_count++;
				}		   
		   }
		}
		pthread_mutex_unlock(&StateMutex);			
	}
	return; 
  
}

#ifdef __cplusplus
}
#endif
