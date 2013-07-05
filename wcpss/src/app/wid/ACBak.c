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
* ACBak.c
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "CWAC.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/npd/npd_dbus_def.h"
#include "hmd/hmdpub.h"
#include "ACBak.h"
#include "AC.h"
#define INFOSIZE  1024

int wid_sock = 0;
int set_vmac;
char v_mac[MAC_LEN];
vrrp_info vinfo;
int set_vmac_state = 0;
typedef struct sys_mac{
	unsigned char mac_add[6];
	unsigned char reserve[2];
}sys_mac_add;

#define BM_MINOR_NUM 0
#define BM_MAJOR_NUM 0xEC
#define BM_IOC_MAGIC 0xEC 
#define BM_IOC_RESET _IO(BM_IOC_MAGIC,0)
#define BM_IOC_GET_MAC _IOWR(BM_IOC_MAGIC, 16, sys_mac_add) 

#define TO_BAKUP_SECOND_PACKET_INTERVAL_MASK	3

int Get_Tunnel_Interface_Info(char * ifname, struct ifi_info *ifi){
	int sockfd;
	struct ifreq	ifr;
	struct sockaddr_in	*sinptr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));			
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
//		printf("SIOCGIFINDEX error\n");
		wid_syslog_debug_debug(WID_MB,"wtp quit reason is SIOCGIFINDEX error");
		close(sockfd);
		return -1;
	 }
	ifi->ifi_index = ifr.ifr_ifindex;
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1){
//		printf("SIOCGIFFLAGS error\n");
		wid_syslog_debug_debug(WID_MB,"wtp quit reason is SIOCGIFFLAGS error");
		close(sockfd);
		return -1;
	}
	if(ifr.ifr_flags & IFF_UP){
		//printf("interface UP\n");
		ifi->ifi_flags = ifr.ifr_flags;
	}else{
//		printf("interface DOWN\n");
		wid_syslog_debug_debug(WID_MB,"wtp quit reason is IF_DOWN error");
		close(sockfd);
		return -1;
	}
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) == -1){
//		printf("SIOCGIFADDR error\n");
		wid_syslog_debug_debug(WID_MB,"wtp quit reason is SIOCGIFADDR error");
		close(sockfd);
		return -1;
	}
	sinptr = (struct sockaddr_in *) &ifr.ifr_addr;
	ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
	memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));
//	memcpy(&(ifi->addr[0]),&(((struct sockaddr_in *) ifi->ifi_addr)->sin_addr.s_addr),sizeof(int));
	ifi->addr[0] = ((struct sockaddr_in *) ifi->ifi_addr)->sin_addr.s_addr;
	if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1){
//		 printf("SIOCGIFHWADDR error\n");
		 wid_syslog_debug_debug(WID_MB,"SIOCGIFHWADDR error");
		 free(ifi->ifi_addr);
		 close(sockfd);
		 return -1;
	 }
	memcpy(ifi->mac,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
	close(sockfd);
	return 0;
}



int set_wid_src_mac()
{
	 int ret;
	 int fd; 
	 sys_mac_add mac_add;
	 memset(&mac_add,0,sizeof(mac_add));
	 fd = open("/dev/bm0",0);
	 if(fd == -1)
	  {
	   printf("open error!");
	  }
	 ret = ioctl(fd,BM_IOC_GET_MAC,&mac_add);
//	 for(i = 0; i < 5; i++)
//	   printf("%02X:",mac_add.mac_add[i]);
//	   printf("%02X",mac_add.mac_add[5]);	 
//	   printf("\n");
	 set_wid_mac((char*)mac_add.mac_add,0);
	 set_vmac = 0;
	 close(fd);
	 return ret;;
}

int vrrp_send_pkt_arp( char* ifname, char *buffer, int buflen )
{
#if 1
	struct sockaddr from;
	int	len;
	int	fd = socket(PF_PACKET, SOCK_PACKET, 0x300); /* 0x300 is magic */
// WORK:
	if( fd < 0 ){
		wid_syslog_crit("%s socket %s",__func__,strerror(errno));
		perror( "socket" );
		return -1;
	}

	/*
	vrrp_syslog_dbg("create socket fd %d to send packet from intf %s\n",fd,ifname);
      */
	/* build the address */
	memset( &from, 0 , sizeof(from));
	strcpy( from.sa_data, ifname );

	/* send the data */
	len = sendto( fd, buffer, buflen, 0, &from, sizeof(from) );
   
	close( fd );
	return len;
#endif
}


static int send_tunnel_interface_arp( unsigned char *MAC, int addr, char* ifname )
{
	wid_syslog_debug_debug(WID_MB,"set tunnel interface %s arp \n",ifname);
struct m_arphdr
  {
    unsigned short int ar_hrd;          /* Format of hardware address.  */
    unsigned short int ar_pro;          /* Format of protocol address.  */
    unsigned char ar_hln;               /* Length of hardware address.  */
    unsigned char ar_pln;               /* Length of protocol address.  */
    unsigned short int ar_op;           /* ARP opcode (command).  */
    /* Ethernet looks like this : This bit is variable sized however...  */
    unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
    unsigned char __ar_sip[4];          /* Sender IP address.  */
    unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
    unsigned char __ar_tip[4];          /* Target IP address.  */
  };
	char	buf[sizeof(struct m_arphdr)+ETHER_HDR_LEN];
	char	buflen	= sizeof(struct m_arphdr)+ETHER_HDR_LEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct m_arphdr	*arph = (struct m_arphdr *)(buf+ETHER_HDR_LEN);
	char vrrp_hwaddr[6];
	memcpy( vrrp_hwaddr, MAC,MAC_LEN);
	unsigned char	*hwaddr	= MAC;
	int	hwlen	= ETH_ALEN;

	/* hardcoded for ethernet */
	memset( eth->ether_dhost, 0xFF, ETH_ALEN );
	memcpy( eth->ether_shost, hwaddr, hwlen );
	eth->ether_type	= htons(ETHERTYPE_ARP);

	/* build the arp payload */
	memset( arph, 0, sizeof( *arph ) );
	arph->ar_hrd	= htons(ARPHRD_ETHER);
	arph->ar_pro	= htons(ETHERTYPE_IP);
	arph->ar_hln	= 6;
	arph->ar_pln	= 4;
	arph->ar_op	= htons(ARPOP_REQUEST);
	memcpy( arph->__ar_sha, hwaddr, hwlen );
	addr = htonl(addr);
	memcpy( arph->__ar_sip, &addr, sizeof(addr) );
	memcpy( arph->__ar_tip, &addr, sizeof(addr) );
	return vrrp_send_pkt_arp( ifname, buf, buflen );
}




int AC_UPDATE_BAK_AC_WIRELESS_INFO(){
	int i = 0;
	int j = 0;
	int k = 0;
	int first = 1;
	if(bak_list == NULL)
		return 1;
	for(i = 0; i<WTP_NUM;i++){
		if((AC_WTP[i] != NULL)){
			if(AC_WTP[i]->WTPStat == 5){
				bak_add_del_wtp(bak_list->sock,B_ADD,i);
				for(j = 0; j < AC_WTP[i]->RadioCount; j++){
					if(AC_WTP[i]->WTP_Radio[j]->AdStat == 1){
						for(k = 0;k < L_BSS_NUM;k++ ){
							if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->State == 1)){
								bak_add_del_bss(bak_list->sock,B_ADD,AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex);
								first = 0;
							}
						}
					}
				}
			}else if(AC_WTP[i]->WTPStat == 7){
				bak_add_del_wtp(bak_list->sock,B_DEL,i);				
			}
		}
	}
	return 0;
}

int AC_SYNCHRONIZE_WSM_TABLE_INFO(){
	int i = 0;
	int j = 0;
	int k = 0;
	int first = 1;
	for(i = 0; i<WTP_NUM;i++){
		if((AC_WTP[i] != NULL)){
			if((AC_WTP[i]->WTPStat == 5)||(AC_WTP[i]->WTPStat == 9)){				
				AsdWsm_DataChannelOp(i, WID_ADD);
				if(first){
					sleep(1);
				}
				for(j = 0; j < AC_WTP[i]->RadioCount; j++){
					if(AC_WTP[i]->WTP_Radio[j]->AdStat == 1){
						for(k = 0;k < L_BSS_NUM;k++ ){
							if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->State == 1)){
								AsdWsm_BSSOp(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex, WID_ADD, 1);
							}
						}
					}
				}
				first = 0;
			}else if(AC_WTP[i]->WTPStat == 7){
				AsdWsm_DataChannelOp(i, WID_DEL);
			}
		}
	}
	return 0;
}

int AC_SYNCHRONIZE_ASD_TABLE_INFO(){
	int i = 0;
	int j = 0;
	int k = 0;
	int first = 1;
	for(i = 0; i<WTP_NUM;i++){
		if((AC_WTP[i] != NULL)){
			if((AC_WTP[i]->WTPStat == 5)||(AC_WTP[i]->WTPStat == 9)){				
				AsdWsm_WTPOp(i,WID_MODIFY);
				if(first){
					sleep(1);
				}
				for(j = 0; j < AC_WTP[i]->RadioCount; j++){
					if(AC_WTP[i]->WTP_Radio[j]->AdStat == 1){
						for(k = 0;k < L_BSS_NUM;k++ ){
							if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->State == 1)){
								AsdWsm_BSSOp(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex, WID_ADD, 1);
								first = 0;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

#if 0
int send_info(const char * buf){
	int i=0;
	int ret;	
/*	int sockfd;
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) ==-1){
		perror("socket create");
		exit(1);
	}
	if(connect(sockfd, (struct sockaddr *)&B_addr, sizeof(struct sockaddr)) == -1){
		close(sockfd);
		return UPDATE_CONFIG_FAIL;
	}*/
	if(buf==NULL){
		return 0;
	}
	B_Msg msg;
	unsigned int len;
	
	msg.Op=B_MODIFY;
	msg.Type=B_INFO_TYPE;

	if(buf!=NULL){
		memset(msg.Bu.INFO.info,0,1024);
		memcpy(msg.Bu.INFO.info,buf,strlen(buf));
//		printf("buf len %d\n",strlen(buf));
	}

	
	len=sizeof(msg);

//	printf("len %d\nbuf:%s\n",len,msg.Bu.INFO.info);
	
	while((ret = send(bak_list->sock, &msg, len, 0) < 0)){
		wid_syslog_crit("%s sendto %s",__func__,strerror(errno));
		perror("sendto");
		i++;
		if(i>5){
			return UPDATE_CONFIG_FAIL;
		}
		continue;
		//exit(1);
	}
//	printf("ret = %d\n",ret);	
	return 0;

}
#endif
int hwaddr_set( char *ifname, char *addr, int addrlen )
{
	struct ifreq	ifr;
	int		fd	= socket(AF_INET, SOCK_DGRAM, 0);
	int		ret;
	unsigned long	flags;
	if (fd < 0) 	return (-1);
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	/* get the flags */
	ret = ioctl(fd, SIOCGIFFLAGS, (char *)&ifr);
	if( ret )	goto end;
	flags = ifr.ifr_flags;
	/* set the interface down */
	ifr.ifr_flags &= ~IFF_UP;
	ret = ioctl(fd, SIOCSIFFLAGS, (char *)&ifr);
	if( ret )	goto end;
	/* change the hwaddr */
	memcpy( ifr.ifr_hwaddr.sa_data, addr, addrlen );
	ifr.ifr_hwaddr.sa_family = AF_UNIX;
	ret = ioctl(fd, SIOCSIFHWADDR, (char *)&ifr);
	if( ret )	goto end;
	/* set the interface up */
	ifr.ifr_flags = flags;
	ret = ioctl(fd, SIOCSIFFLAGS, (char *)&ifr);
	if( ret )	goto end;
end:;
if( ret )	printf("error errno=%d\n",errno);

	close(fd);
	return ret;
}

int set_wid_mac(char * addr, int is_vmac){
	int i = 0;
	int j = 0;
	int k = 0;
	for(i = 0; i<WTP_NUM;i++){
		if((AC_WTP[i] != NULL)){
			for(j = 0; j < AC_WTP[i]->RadioCount; j++){
				for(k = 0;k < L_BSS_NUM;k++ ){
					if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)){
						if((AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSS_IF_POLICY != NO_INTERFACE)){
							if((is_vmac==1)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->vMAC_STATE == 0)){
								hwaddr_set(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSS_IF_NAME,addr,MAC_LEN);
								AC_WTP[i]->WTP_Radio[j]->BSS[k]->vMAC_STATE = 1;
							}else if((is_vmac==0)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->vMAC_STATE == 1)){
								hwaddr_set(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSS_IF_NAME,addr,MAC_LEN);
								AC_WTP[i]->WTP_Radio[j]->BSS[k]->vMAC_STATE = 0;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

int send_all_tunnel_interface_arp(){
	int i = 0;
	int j = 0;
	int k = 0;
	int ret = 0;
	struct ifi_info ifinfo;
	char name[ETH_IF_NAME_LEN];
//	printf("1\n");
	for(i = 0; i<WLAN_NUM;i++){
		if((AC_WLAN[i]!=NULL)&&(AC_WLAN[i]->Status == 0)){
			
//			printf("WLANID %d\n",i);
			if(AC_WLAN[i]->wlan_if_policy == WLAN_INTERFACE){
				
//				printf("3\n");
				memset(name,0,ETH_IF_NAME_LEN);
				if(local)
					sprintf(name,"wlanl%d-%d-%d",slotid,vrrid,i);
				else
					sprintf(name,"wlan%d-%d-%d",slotid,vrrid,i);					
//				printf("name¡¡%s\n",name);
				memset(&ifinfo,0,sizeof(struct ifi_info));
				ret = Get_Tunnel_Interface_Info(name,&ifinfo);				
//				printf("ret %d\n",ret);
//				printf("ip %x\n",ifinfo.addr[0]);
//				printf("mac %02x:%02x:%02x:%02x:%02x:%02x\n",ifinfo.mac[0],ifinfo.mac[1],ifinfo.mac[2],ifinfo.mac[3],ifinfo.mac[4],ifinfo.mac[5]);
				if(ret == 0){
					send_tunnel_interface_arp(ifinfo.mac,ifinfo.addr[0],name);
					free(ifinfo.ifi_addr);
				}
			}
		}
	}
	for(i = 0; i<WTP_NUM;i++){
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat != 7)){
			for(j = 0; j < AC_WTP[i]->RadioCount; j++){
				for(k = 0;k < L_BSS_NUM;k++ ){
					if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)){
						if((AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSS_IF_POLICY == BSS_INTERFACE)){
							memset(name,0,ETH_IF_NAME_LEN);
							if(local)
								sprintf(name,"r%d-%d-%d.%d",vrrid,i,j,AC_WTP[i]->WTP_Radio[j]->BSS[k]->WlanID);
							else
								sprintf(name,"r%d-%d-%d-%d.%d",slotid,vrrid,i,j,AC_WTP[i]->WTP_Radio[j]->BSS[k]->WlanID);							
//							printf("name¡¡%s\n",name);
							memset(&ifinfo,0,sizeof(struct ifi_info));
							ret = Get_Tunnel_Interface_Info(name,&ifinfo);
							if(ret == 0)
								send_tunnel_interface_arp(ifinfo.mac,ifinfo.addr[0],name);
						}
					}
				}
			}
		}
	}
	return 0;
}

int UpdateWifiHansiState(){
		int ret = -1;	
		struct HANSI_INFO hansiinfo;
		int fd = open("/dev/wifi0", O_RDWR);
		wid_syslog_info("***%s fd:%d ***\n",__func__,fd);
		CWThreadMutexLock(&MasterBak);
		if(fd < 0)
		{
			CWThreadMutexUnlock(&MasterBak);
			return -1;//create failure
		}
		memset(&hansiinfo, 0, sizeof(struct HANSI_INFO));
		hansiinfo.instId = local*MAX_INSTANCE + vrrid;
		hansiinfo.hstate = is_secondary;
		hansiinfo.vlanSwitch = vlanSwitch;
		hansiinfo.dhcpoption82 = DhcpOption82Switch;
		wid_syslog_info("%s,line=%d,hansiinfo.instId=%d,hansiinfo.hstate=%d.\n",__func__,__LINE__,hansiinfo.instId,hansiinfo.hstate);
		ret = ioctl(fd, WIFI_IOC_HANSISTATE_UPDATE, &hansiinfo);
		
		wid_syslog_info("*** %s, ret:%d ***\n",__func__,ret);
		close(fd);
		wid_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		if(ret < 0)
		{
			wid_syslog_info("%s,line=%d.\n",__func__,__LINE__);
			wid_syslog_err("%s fail,ret=%d.\n",__func__,ret);
			CWThreadMutexUnlock(&MasterBak);
			return -1;
		}
		CWThreadMutexUnlock(&MasterBak);
		wid_syslog_info("%s,line=%d.\n",__func__,__LINE__);
		return 0;
}

int init_wid_bak_socket(){
	int sock;
	int yes = 1;
	int sndbuf = 65525;
	int rcvbuf = 65525;
	if(local){
		struct sockaddr_tipc server_addr;
		server_addr.family = AF_TIPC;
		server_addr.addrtype = TIPC_ADDR_NAMESEQ;
		server_addr.addr.nameseq.type = SYN_WID_SERVER_TYPE;
		server_addr.addr.nameseq.lower = SYN_SERVER_BASE_INST + vrrid*MAX_SLOT_NUM + slotid;;
		server_addr.addr.nameseq.upper = SYN_SERVER_BASE_INST + vrrid*MAX_SLOT_NUM + slotid;;
		server_addr.scope = TIPC_ZONE_SCOPE;
		sock = socket(AF_TIPC, SOCK_RDM, 0);
		if (0 != bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)))
		{
			printf("Server: failed to bind port name\n");
			//exit(1);
		}
	}else{
		struct sockaddr_in my_addr;
#ifndef _AC_BAK_UDP_ 
		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
		{	printf("udp socket create failed\n");		
			exit(1);	
		}
#else
		if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1)
		{	printf("udp socket create failed\n");		
			exit(1);	
		}
#endif
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		my_addr.sin_family = AF_INET;	
		my_addr.sin_port = htons(WID_BAK_AC_PORT+ local*MAX_INSTANCE +vrrid);	
		my_addr.sin_addr.s_addr = INADDR_ANY;	
		memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));	
		if (bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
		{	
			wid_syslog_info("udp bind failed\n");	
			exit(1);
			return -1;
		}	
	}
	setsockopt(sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
	setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
	return sock;
}

void bak_add_del_wtp(int sockfd,BakOperate Op,unsigned int WTPID){
	B_Msg msg;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	time_t time;
	if(AC_WTP[WTPID] == NULL){
		wid_syslog_info("%s AC_WTP[%d] is NULL.\n",__func__,WTPID);
		return;
	}
	if(AC_WTP[WTPID]->add_time == NULL)
	{
		time = 0;
	}
	else
	{
		time = *AC_WTP[WTPID]->add_time;
		
	}
	wid_syslog_debug_debug(WID_MB,"update WTP %d info\n",WTPID);
	memset(&msg,0,sizeof(B_Msg));
	msg.Op = Op;
	msg.Type = B_WTP_TYPE;
	msg.Bu.WTP.WTPID = WTPID;
	if(Op == B_ADD){
		memcpy(msg.Bu.WTP.SN,AC_WTP[WTPID]->WTPSN,strlen(AC_WTP[WTPID]->WTPSN));
		memcpy(msg.Bu.WTP.WTP_IP,AC_WTP[WTPID]->WTPIP,strlen(AC_WTP[WTPID]->WTPIP));
		memcpy(msg.Bu.WTP.WTP_MAC,AC_WTP[WTPID]->WTPMAC,MAC_LEN);	
		memcpy((struct sockaddr*)&(msg.Bu.WTP.addr), (struct sockaddr*)&(gWTPs[WTPID].address), sizeof(struct sockaddr));

		if((AC_WTP[WTPID]->codever)&&(strlen(AC_WTP[WTPID]->codever) <= NAME_LEN))
		{
			memcpy(msg.Bu.WTP.WTP_CODEVER,AC_WTP[WTPID]->codever,strlen(AC_WTP[WTPID]->codever));
		}else{
			memcpy(msg.Bu.WTP.WTP_CODEVER,"none",5);
			if(AC_WTP[WTPID]->codever)
			{	wid_syslog_err(" strlen(AC_WTP[%d]->codever)  =  %d  \n ",WTPID,strlen(AC_WTP[WTPID]->codever));}
			else
			{  wid_syslog_err(" AC_WTP[%d]->codever  =	%p	\n ",WTPID,AC_WTP[WTPID]->codever);}
		}
		
		if((AC_WTP[WTPID]->sysver)&&(strlen(AC_WTP[WTPID]->sysver) <= NAME_LEN))
		{
			memcpy(msg.Bu.WTP.WTP_SYSVER,AC_WTP[WTPID]->sysver,strlen(AC_WTP[WTPID]->sysver));
		}else{
			memcpy(msg.Bu.WTP.WTP_CODEVER,"none",5);
			if(AC_WTP[WTPID]->sysver)
			{	wid_syslog_err(" strlen(AC_WTP[%d]->sysver)  =	%d	\n ",WTPID,strlen(AC_WTP[WTPID]->sysver));}
			else
			{  wid_syslog_err(" AC_WTP[%d]->sysver	=  %p  \n ",WTPID,AC_WTP[WTPID]->sysver);}			
		}
		
		if((AC_WTP[WTPID]->ver)&&(strlen(AC_WTP[WTPID]->ver) <= NAME_LEN))
		{
			memcpy(msg.Bu.WTP.WTP_VER,AC_WTP[WTPID]->ver,strlen(AC_WTP[WTPID]->ver));
		}else{
			memcpy(msg.Bu.WTP.WTP_CODEVER,"none",5);
			if(AC_WTP[WTPID]->ver)
			{	wid_syslog_err(" strlen(AC_WTP[%d]->ver)  =  %d  \n ",WTPID,strlen(AC_WTP[WTPID]->ver));}
			else
			{  wid_syslog_err(" AC_WTP[%d]->ver  =	%p	\n ",WTPID,AC_WTP[WTPID]->ver);}			
		}

		msg.Bu.WTP.oemoption = gWTPs[WTPID].oemoption;
		msg.Bu.WTP.add_time = time;
		msg.Bu.WTP.imagedata_time = AC_WTP[WTPID]->imagedata_time;
		msg.Bu.WTP.config_update_time = AC_WTP[WTPID]->config_update_time;
		msg.Bu.WTP.ElectrifyRegisterCircle = AC_WTP[WTPID]->ElectrifyRegisterCircle;
	}
	len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		wid_syslog_info("select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
#endif
		wid_syslog_debug_debug(WID_MB,"ap ret %d\n",ret);
		if( ret < 0){
			wid_syslog_info("%s send %s",__func__,strerror(errno));
			perror("send");		
			wid_syslog_info("bak_add_del_wtp send error\n");
			//exit(1);
		}
	}
}

void bak_add_del_bss(int sockfd,BakOperate Op,unsigned int BSSIndex){
	char __str[128];			
	memset(__str,0,128);
	char *str = "lo";	
	B_Msg msg;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	wid_syslog_debug_debug(WID_MB,"update BSS %d info \n",BSSIndex);
	int radioID = BSSIndex/L_BSS_NUM;
	int Bid = BSSIndex%L_BSS_NUM;
	int WTPIndex = BSSIndex/L_BSS_NUM/L_RADIO_NUM;
	msg.Op = Op;
	msg.Type = B_BSS_TYPE;
	msg.Bu.BSS.BSSIndex = BSSIndex;
	msg.Bu.BSS.WLANID = AC_BSS[BSSIndex]->WlanID;
	//msg.Bu.BSS.WTPID = BSSIndex/L_BSS_NUM/L_RADIO_NUM;
	str = sock_ntop_r(((struct sockaddr*)&(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr)), __str);
	wid_syslog_info("WTP %d on Interface %s (%d)\n",WTPIndex, str, gWTPs[WTPIndex].interfaceIndex);
	struct sockaddr_in	*sin = (struct sockaddr_in *) &(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr);
	unsigned int v_ip = sin->sin_addr.s_addr;
	wid_syslog_info("v_ip %d.%d.%d.%d.\n", \
				(v_ip >> 24) & 0xFF, (v_ip >> 16) & 0xFF,(v_ip >> 8) & 0xFF, v_ip & 0xFF);
	msg.Bu.BSS.WTPID = v_ip;
	if(Op == B_ADD){
		memcpy(msg.Bu.BSS.BSSID,AC_RADIO[radioID]->BSS[Bid]->BSSID,MAC_LEN);
	}
	len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		wid_syslog_info("select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
#endif
		wid_syslog_debug_debug(WID_MB,"bss ret %d\n",ret);
		if(ret < 0){
			wid_syslog_info("%s send %s",__func__,strerror(errno));
			perror("send");
			wid_syslog_info("bak_add_del_wtp send error\n");
			//exit(1);
		}
	}
}

void B_WID_WTP_INIT(void *arg){
		int i = ((CWACThreadArg_clone*)arg)->index;
		CWSocket sock = ((CWACThreadArg_clone*)arg)->sock;
//		int interfaceIndex = ((CWACThreadArg_clone*)arg)->interfaceIndex;
		gWTPs[i].BAK_FLAG = 1;
		if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
			wid_syslog_crit("can't lock threadmutex");
			exit(1);
			}
		gActiveWTPs++;
		CWThreadMutexLock(&ACLicense);
		(g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count)++;
		/*xiaodawei modify, 20101111*/
		if(g_wtp_count[gWTPs[i].oemoption]->flag!=0){
			(g_wtp_binding_count[(g_wtp_count[gWTPs[i].oemoption]->flag)]->gcurrent_wtp_count)++;
		}
		CWThreadMutexUnlock(&ACLicense);	
//		gInterfaces[interfaceIndex].WTPCount++;
//		CWUseSockNtop(((struct sockaddr*) &(gInterfaces[interfaceIndex].addr)),
//				wid_syslog_debug_debug("One more WTP on %s (%d)", str, interfaceIndex);
//				);
	
		CWThreadMutexUnlock(&gActiveWTPsMutex);
	
		CWACInitBinding(i);
		
//		gWTPs[i].interfaceIndex = interfaceIndex;
		gWTPs[i].socket = sock;
	#ifdef CW_NO_DTLS
		AC_WTP[i]->CTR_ID = sock;
	#endif
		gWTPs[i].fragmentsList = NULL;
		gWTPs[i].subState = CW_DTLS_HANDSHAKE_IN_PROGRESS;
			
		/**** ACInterface ****/
		gWTPs[i].interfaceCommandProgress = CW_FALSE;
		gWTPs[i].interfaceCommand = NO_CMD;
		CWDestroyThreadMutex(&gWTPs[i].interfaceMutex); 
		CWCreateThreadMutex(&gWTPs[i].interfaceMutex);
		CWDestroyThreadMutex(&gWTPs[i].WTPThreadMutex); 
		CWCreateThreadMutex(&gWTPs[i].WTPThreadMutex);
		CWDestroyThreadMutex(&gWTPs[i].RRMThreadMutex); 
		CWCreateThreadMutex(&gWTPs[i].RRMThreadMutex);
		CWDestroyThreadMutex(&gWTPs[i].WIDSThreadMutex); 
		CWCreateThreadMutex(&gWTPs[i].WIDSThreadMutex);
		CWDestroyThreadMutex(&gWTPs[i].interfaceSingleton); 
		CWCreateThreadMutex(&gWTPs[i].interfaceSingleton);
		CWDestroyThreadCondition(&gWTPs[i].interfaceWait);	
		CWCreateThreadCondition(&gWTPs[i].interfaceWait);
		CWDestroyThreadCondition(&gWTPs[i].interfaceComplete);	
		CWCreateThreadCondition(&gWTPs[i].interfaceComplete);
		CWDestroyThreadMutex(&gWTPs[i].WTPThreadControllistMutex); 
		CWCreateThreadMutex(&gWTPs[i].WTPThreadControllistMutex);
		CWDestroyThreadMutex(&gWTPs[i].mutex_controlList);
		CWCreateThreadMutex(&gWTPs[i].mutex_controlList);
		gWTPs[i].qosValues=NULL;
		/**** ACInterface ****/
	
		gWTPs[i].messages = NULL;
		gWTPs[i].messagesCount = 0;
		gWTPs[i].isRetransmitting = CW_FALSE;
		gWTPs[i].retransmissionCount = 0;
		
		CWResetWTPProtocolManager(&(gWTPs[i].WTPProtocolManager));
		wid_syslog_debug_debug(WID_MB,"New Session");	
#ifndef CW_NO_DTLS
		wid_syslog_debug_debug(WID_MB,"Init DTLS Session");
	
		if(!CWErr(CWSecurityInitSessionServer(&gWTPs[i], sock, gACSecurityContext, &((gWTPs[i]).session), &(gWTPs[i].pathMTU) )  ) ) { // error joining
			CWTimerCancel(&(gWTPs[i].currentTimer),1);
			_CWCloseThread(i);
		}
#endif
		(gWTPs[i]).subState = CW_WAITING_REQUEST;
		
		if(gCWForceMTU > 0) gWTPs[i].pathMTU = gCWForceMTU;
		wid_syslog_debug_debug(WID_MB,"Path MTU for this Session: %d",  gWTPs[i].pathMTU);
	


}

void B_WTP_ADD_OP(B_Msg *msg){
	unsigned int WTPID;
	unsigned int flag = 0;	/*xiaodawei add, 20101109*/
	WTPID = msg->Bu.WTP.WTPID;
	if(WTPID >= WTP_NUM)
		return;
	if((AC_WTP[WTPID]!=NULL)&&((AC_WTP[WTPID]->WTPStat != 5)&&(AC_WTP[WTPID]->WTPStat != 9))){
		/*AXSSZFI-1204*/
		if(memcmp(AC_WTP[WTPID]->WTPMAC,msg->Bu.WTP.WTP_MAC,MAC_LEN) ==0){
			if(AC_WTP[WTPID]->WTPSN != NULL){
				free(AC_WTP[WTPID]->WTPSN);
				AC_WTP[WTPID]->WTPSN = NULL;
			}
			AC_WTP[WTPID]->WTPSN = (char*)malloc(NAS_IDENTIFIER_NAME);
			memset(AC_WTP[WTPID]->WTPSN,0,NAS_IDENTIFIER_NAME);
			memcpy(AC_WTP[WTPID]->WTPSN,msg->Bu.WTP.SN,strlen((char *)(msg->Bu.WTP.SN)));
			memset(AC_WTP[WTPID]->WTPMAC,0,7);
			memcpy(AC_WTP[WTPID]->WTPMAC,msg->Bu.WTP.WTP_MAC,MAC_LEN);
			memset(AC_WTP[WTPID]->WTPIP,0,DEFAULT_LEN);
			memcpy(AC_WTP[WTPID]->WTPIP,msg->Bu.WTP.WTP_IP,strlen((char *)(msg->Bu.WTP.WTP_IP)));
			memcpy((struct sockaddr*)&(gWTPs[WTPID].address),&(msg->Bu.WTP.addr),sizeof(struct sockaddr));		
			CW_FREE_OBJECT(AC_WTP[WTPID]->codever);
			AC_WTP[WTPID]->codever = (char*)malloc(strlen((char *)(msg->Bu.WTP.WTP_CODEVER))+1);
			memset(AC_WTP[WTPID]->codever,0,strlen((char *)(msg->Bu.WTP.WTP_CODEVER))+1);
			memcpy(AC_WTP[WTPID]->codever,msg->Bu.WTP.WTP_CODEVER,strlen((char *)(msg->Bu.WTP.WTP_CODEVER)));
			wid_syslog_debug_debug(WID_MB,"AC_WTP[%d]->codever =  %s ",WTPID,AC_WTP[WTPID]->codever);	

			CW_FREE_OBJECT(AC_WTP[WTPID]->ver);	
			AC_WTP[WTPID]->ver = (char*)malloc(strlen((char *)(msg->Bu.WTP.WTP_VER))+1);
			memset(AC_WTP[WTPID]->ver,0,strlen((char *)(msg->Bu.WTP.WTP_VER))+1);
			memcpy(AC_WTP[WTPID]->ver,msg->Bu.WTP.WTP_VER,strlen((char *)(msg->Bu.WTP.WTP_VER)));
			wid_syslog_debug_debug(WID_MB,"AC_WTP[%d]->ver =  %s ",WTPID,AC_WTP[WTPID]->ver);	

			CW_FREE_OBJECT(AC_WTP[WTPID]->sysver);
			AC_WTP[WTPID]->sysver = (char*)malloc(strlen((char *)(msg->Bu.WTP.WTP_SYSVER))+1);
			memset(AC_WTP[WTPID]->sysver,0,strlen((char *)(msg->Bu.WTP.WTP_SYSVER))+1);
			memcpy(AC_WTP[WTPID]->sysver,msg->Bu.WTP.WTP_SYSVER,strlen((char *)(msg->Bu.WTP.WTP_SYSVER)));
			wid_syslog_debug_debug(WID_MB,"AC_WTP[%d]->sysver =  %s ",WTPID,AC_WTP[WTPID]->sysver);	

			if(msg->Bu.WTP.oemoption > (glicensecount-1))
				gWTPs[WTPID].oemoption = 0;
			else
				gWTPs[WTPID].oemoption = msg->Bu.WTP.oemoption;		
			/*xiaodawei modify, 20101109*/
			CWThreadMutexLock(&ACLicense);
			if(g_wtp_count[gWTPs[WTPID].oemoption]->flag==0){
				if(g_wtp_count[gWTPs[WTPID].oemoption]->gcurrent_wtp_count >= g_wtp_count[gWTPs[WTPID].oemoption]->gmax_wtp_count){
					printf("###can not access ap type %d access count over type count\n",gWTPs[WTPID].oemoption);
					wid_syslog_debug_debug(WID_WTPINFO,"B_WTP_ADD_OP wtp %d ###can not access ap type %d access count over type count",WTPID,gWTPs[WTPID].oemoption);	
					CWThreadMutexUnlock(&ACLicense);
					return ;
				}
			}
			else{
				flag = g_wtp_count[gWTPs[WTPID].oemoption]->flag;
				if(g_wtp_binding_count[flag]->gcurrent_wtp_count >= g_wtp_binding_count[flag]->gmax_wtp_count){
					printf("###can not access ap type %d access count over type count\n",gWTPs[WTPID].oemoption);
					wid_syslog_debug_debug(WID_WTPINFO,"B_WTP_ADD_OP wtp %d ###can not access ap type %d access count over type count",WTPID,gWTPs[WTPID].oemoption);	
					CWThreadMutexUnlock(&ACLicense);
					return ;
				}
			}
			CWThreadMutexUnlock(&ACLicense);
			wid_syslog_debug_debug(WID_MB,"%s,%d,msg->Bu.WTP.add_time=%d.\n",__func__,__LINE__,msg->Bu.WTP.add_time);
			/*END*/
			if(AC_WTP[WTPID]->add_time != NULL){
				
				*(AC_WTP[WTPID]->add_time) = msg->Bu.WTP.add_time;
			}else{

				AC_WTP[WTPID]->add_time = (time_t *)malloc(sizeof(time_t));
				*(AC_WTP[WTPID]->add_time) = msg->Bu.WTP.add_time;
			}
			AC_WTP[WTPID]->imagedata_time = msg->Bu.WTP.imagedata_time;
			AC_WTP[WTPID]->config_update_time = msg->Bu.WTP.config_update_time;
			if(0 != msg->Bu.WTP.ElectrifyRegisterCircle)
				AC_WTP[WTPID]->ElectrifyRegisterCircle = msg->Bu.WTP.ElectrifyRegisterCircle;
			wid_syslog_debug_debug(WID_MB,"%s,%d,msg->Bu.WTP.ElectrifyRegisterCircle=%d.\n",__func__,__LINE__,msg->Bu.WTP.ElectrifyRegisterCircle);
			
			gWTPs[WTPID].isNotFree = CW_TRUE;
			gWTPs[WTPID].isRequestClose = CW_FALSE;	
			AC_WTP[WTPID]->WTPStat = 9;
			gWTPs[WTPID].currentState = CW_BAK_RUN;
			BakArgPtr.index = WTPID;
			CWThreadMutexLock(&(gWTPs[WTPID].WTPThreadControllistMutex));
			if(AC_WTP[WTPID]->ControlWait != NULL){
				free(AC_WTP[WTPID]->ControlWait);
				AC_WTP[WTPID]->ControlWait = NULL;
			}
			CWThreadMutexUnlock(&(gWTPs[WTPID].WTPThreadControllistMutex));
			B_WID_WTP_INIT(&BakArgPtr);		
			AsdWsm_DataChannelOp(WTPID, WID_ADD);
		}
		else{
			wid_syslog_info("config a different wtp(different mac) in slave from master\n");
		}
	}	
	else if((AC_WTP[WTPID]!=NULL)&&(AC_WTP[WTPID]->WTPStat == 5)){
		gWTPs[WTPID].isRequestClose = CW_FALSE;	
		AC_WTP[WTPID]->WTPStat = 9;
		gWTPs[WTPID].currentState = CW_BAK_RUN;
	}
	return;
}

void B_WTP_DEL_OP(B_Msg *msg){
	
	if(msg->Bu.WTP.WTPID >= WTP_NUM)
		return;
	if((AC_WTP[msg->Bu.WTP.WTPID]!=NULL)&&(AC_WTP[msg->Bu.WTP.WTPID]->WTPStat != 7))
	_CWCloseThread(msg->Bu.WTP.WTPID);
}

void B_BSS_ADD_OP(B_Msg *msg){
	unsigned int BSSIndex;
	unsigned int RadioID;
	unsigned int WtpID;
	unsigned int local_radioid;
	unsigned char WlanId; 
	BSSIndex = msg->Bu.BSS.BSSIndex;
	RadioID = BSSIndex/L_BSS_NUM;
	WtpID = RadioID/L_RADIO_NUM;
	local_radioid = RadioID%L_RADIO_NUM; 	
	WlanId =  msg->Bu.BSS.WLANID;
	if(WlanId >= WLAN_NUM)
		return;
	if((AC_WLAN[WlanId]!=NULL)&&(AC_WLAN[WlanId]->Status == 1)){
		wid_syslog_info("%s WLAN %d is disable, bss %d can't backup\n",__func__,WlanId,BSSIndex);
		return;		
	}
	if(BSSIndex >= BSS_NUM)
		return;
	if((AC_BSS[BSSIndex]!=NULL)&&(AC_BSS[BSSIndex]->State != 1)){
		memcpy(AC_BSS[BSSIndex]->BSSID,msg->Bu.BSS.BSSID,MAC_LEN);
		AC_BSS[BSSIndex]->State = 1;
		if(AC_RADIO[RadioID]->AdStat == 2){
			AC_RADIO[RadioID]->AdStat = 1;
			AC_RADIO[RadioID]->OpStat = 1;
		}
		WlanId = AC_BSS[BSSIndex]->WlanID;
		AC_WTP[WtpID]->CMD->radiowlanid[local_radioid][WlanId] = 2;
		if(AsdWsm_BSSOp(BSSIndex, WID_ADD, 1)){
			//if((AC_BSS[BSSIndex]->BSS_IF_POLICY == BSS_INTERFACE)||(AC_BSS[BSSIndex]->BSS_IF_POLICY == WLAN_INTERFACE))
			{
				struct sockaddr *sa;				
				struct sockaddr* sa2;
				int ret = -1;									
				unsigned char wlan_id = 0;
				IF_info ifinfo;
				int fd = open("/dev/wifi0", O_RDWR);
				wid_syslog_debug_debug(WID_DEFAULT,"*** fd:%d ***\n",fd);
			
				if(fd < 0)
				{
					return ;//create failure
				}
				sa = (struct sockaddr *)&gWTPs[WtpID].address;
				sa2 = (struct sockaddr*)&(gInterfaces[gWTPs[WtpID].interfaceIndex].addr);
				memset(&ifinfo, 0, sizeof(IF_info));
				ifinfo.acport = htons(5247);
				ifinfo.BSSIndex = BSSIndex;
				ifinfo.WLANID =0; /*AC_BSS[BSSIndex]->WlanID;*/  //fengwenchao modify 20111221
				wlan_id = AC_BSS[BSSIndex]->WlanID; 
				if((AC_WLAN[wlan_id] != NULL)&&(AC_WLAN[wlan_id]->SecurityType == OPEN)&&(AC_WLAN[wlan_id]->EncryptionType == NONE))
					ifinfo.protect_type = 0;
				else
					ifinfo.protect_type = 1;
				
				if((AC_BSS[BSSIndex]->BSS_IF_POLICY == BSS_INTERFACE)||(AC_BSS[BSSIndex]->BSS_IF_POLICY == WLAN_INTERFACE))
					ifinfo.if_policy = 1;
				else
					ifinfo.if_policy = 0;
				
				if(AC_BSS[BSSIndex]->vlanid != 0){
					ifinfo.vlanid = AC_BSS[BSSIndex]->vlanid;
				}else if(AC_BSS[BSSIndex]->wlan_vlanid != 0){
					ifinfo.vlanid = AC_BSS[BSSIndex]->wlan_vlanid;
				}else{
					ifinfo.vlanid = 0;
				}
				ifinfo.vrid = local*MAX_INSTANCE + vrrid;
				if(AC_BSS[BSSIndex]->BSS_TUNNEL_POLICY == CW_802_DOT_3_TUNNEL){
						ifinfo.f802_3 = 1;
					wid_syslog_info("%s,ifinfo.f802_3 = %d.\n",__func__,ifinfo.f802_3);
				}else{
					ifinfo.f802_3 = 0;
					wid_syslog_info("%s,AC_BSS[%d]->BSS_TUNNEL_POLICY =%d, not dot3,set ifinfo.f802_3 = %d.\n",__func__,BSSIndex,AC_BSS[BSSIndex]->BSS_TUNNEL_POLICY,ifinfo.f802_3);
				}
				ifinfo.wsmswitch = wsmswitch;
				ifinfo.vlanSwitch = vlanSwitch;
				memcpy(ifinfo.apmac, AC_WTP[WtpID]->WTPMAC, MAC_LEN);
				memcpy(ifinfo.bssid,  AC_BSS[BSSIndex]->BSSID, MAC_LEN);
				memcpy(ifinfo.ifname, AC_WTP[WtpID]->BindingIFName,strlen(AC_WTP[WtpID]->BindingIFName));				
				memcpy(ifinfo.apname,AC_WTP[WtpID]->WTPNAME,strlen(AC_WTP[WtpID]->WTPNAME));
				if(AC_WLAN[wlan_id] != NULL){
					memcpy(ifinfo.essid ,AC_WLAN[wlan_id]->ESSID ,strlen(AC_WLAN[wlan_id]->ESSID));
					ifinfo.Eap1XServerSwitch = AC_WLAN[wlan_id]->eap_mac_switch;
					memset(ifinfo.Eap1XServerMac,0,MAC_LEN);
					memcpy(ifinfo.Eap1XServerMac,AC_WLAN[wlan_id]->eap_mac2,MAC_LEN);
				}
				char __str[128];			
				memset(__str,0,128);
				char *str = "lo";	//fengwenchao modify 20110525
				str = sock_ntop_r(((struct sockaddr*)&(gInterfaces[gWTPs[WtpID].interfaceIndex].addr)), __str);
				wid_syslog_info("WTP %d on Interface %s (%d)\n",WtpID, str, gWTPs[WtpID].interfaceIndex);
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
				
				close(fd);
				if(ret < 0)
				{
					return ;
				}
			
			}

		}
	}	
	return;
}

int B_BSS_DEL_OP(B_Msg *msg){
	unsigned int RadioId = msg->Bu.BSS.BSSIndex/L_BSS_NUM;
	unsigned char WlanId = msg->Bu.BSS.WLANID;
	int WtpID = RadioId/L_RADIO_NUM;
	int local_radioid = RadioId%L_RADIO_NUM;
	
	if(WtpID >= WTP_NUM)
		return WLAN_ID_NOT_EXIST;
	if(AC_WLAN[WlanId] == NULL)
	{
		return WLAN_ID_NOT_EXIST;
	}

	if(AC_BSS[msg->Bu.BSS.BSSIndex]==NULL){
		return 0;
	}
	AsdWsm_BSSOp(msg->Bu.BSS.BSSIndex, WID_DEL, 1);
	if((AC_BSS[msg->Bu.BSS.BSSIndex]!=NULL)&&(AC_BSS[msg->Bu.BSS.BSSIndex]->State == 0))
		return 0;
	
	AC_BSS[msg->Bu.BSS.BSSIndex]->State = 0;	
	memset(AC_BSS[msg->Bu.BSS.BSSIndex]->BSSID, 0, 6);
	AC_WTP[WtpID]->CMD->radiowlanid[local_radioid][WlanId] = 0;
	return 0;
}
#if 0
void B_INFO_OP(B_Msg *msg){
	char buf[1024];
	memset(buf, 0, 1024);	
//	printf("buf:%s\n",msg->Bu.INFO.info);
	sprintf(buf,"/opt/bin/vtysh -c \"con t\n %s\"\n",msg->Bu.INFO.info);	
	system(buf);
}
#endif
#if 0
void B_CHK_RES(int sockfd, int wtp_count, int bss_count){
	B_Msg msg;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	msg.Type = B_CHECK_RES;
	msg.Bu.CHK.wtp_count = wtp_count;
	msg.Bu.CHK.bss_count = bss_count;
	len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		wid_syslog_info("select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr));
#endif
		wid_syslog_debug_debug(WID_MB,"bss ret %d\n",ret);
		if(ret < 0){			
			wid_syslog_info("%s send %s",__func__,strerror(errno));
			perror("send");
			wid_syslog_info("bak_add_del_wtp send error\n");
			//exit(1);
		}
	}
}
#endif

void B_WTP_OP(B_Msg *msg){
	switch(msg->Op){
		case B_ADD:
			B_WTP_ADD_OP(msg);
			break;
		case B_DEL:			
			B_WTP_DEL_OP(msg);
			break;
		default :
			break;
	}
	return;
}	
void B_BSS_OP(B_Msg *msg){
	switch(msg->Op){
		case B_ADD:			
			B_BSS_ADD_OP(msg);
			break;
		case B_DEL:
			B_BSS_DEL_OP(msg);
			break;
		default:
			break;
	}
	return;
}	

int GetBakIFInfo(char *name, unsigned int ip){
	CWMultiHomedSocket *sockPtr;
	sockPtr = &gACSocket;	
	struct CWMultiHomedInterface *inf;
	inf = sockPtr->interfaces;
	int i;
	for(i = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		if(inf){
			//if((memcmp(inf->ifname,name,strlen(name))==0)&&(memcmp(&((struct sockaddr_in *)&(inf->addr))->sin_addr,&ip,sizeof(struct in_addr))))
			if((memcmp(inf->ifname,name,strlen(name))==0)&&(((struct sockaddr_in *)&(inf->addr))->sin_addr.s_addr == ip))
			{
				BakArgPtr.interfaceIndex = inf->gIf_Index;
				BakArgPtr.sock = inf->sock;
				return 1;
			}		
			if(inf->if_next == NULL)
				return 0;
			inf = inf->if_next;
		}
	}
	return 0;

}

#ifndef _AC_BAK_UDP_
void *wid_master_thread()
{
	wid_pid_write_v2("wid_master_thread,(not define _AC_BAK_UDP_)",0,vrrid);
	socklen_t addr_len;
	listen(wid_sock, 5);
	struct bak_sock *tmp;
	send_all_tunnel_interface_arp();
	while(is_secondary == 0){		
		int new_sock;
		struct bak_sock *bsock;
		struct sockaddr_in ac_addr; 
		unsigned int i,j,k;
		addr_len = sizeof(struct sockaddr);
		wid_syslog_debug_debug(WID_MB,"wait bak wtp\n");
		//printf("wait bak wtp\n");
		new_sock = accept(wid_sock, (struct sockaddr *)&ac_addr, &addr_len);
		wid_syslog_debug_debug(WID_MB,"get bak wtp sock:%d\n",new_sock);
		//printf("get bak wtp sock:%d\n",new_sock);
		if(new_sock == -1){
			wid_syslog_info("accept failed\n");
			break;
		}
		CWThreadMutexLock(&MasterBak);
		if(bak_list == NULL){
			bsock = (struct bak_sock*)malloc(sizeof(struct bak_sock));
			memset(bsock,0,sizeof(struct bak_sock));
			bsock->sock = new_sock;
			//memcpy(&(bsock->ip),&(ac_addr.sin_addr.s_addr),sizeof(int));
			//bsock->ip = ac_addr.sin_addr.s_addr;
			bak_list = bsock;
		}else{
			tmp = bak_list;
			while(tmp != NULL){
				//if(memcmp(&(tmp->ip),&(ac_addr.sin_addr.s_addr),sizeof(int))==0){		
				if(tmp->ip == ac_addr.sin_addr.s_addr){
					if(tmp->sock != new_sock){
						close(tmp->sock);
						tmp->sock = new_sock;
						break;
					}
				}
				tmp = tmp->next;
			}
			if(tmp == NULL){
				bsock = (struct bak_sock*)malloc(sizeof(struct bak_sock));
				memset(bsock,0,sizeof(struct bak_sock));
				bsock->sock = new_sock;
				//memcpy(&(bsock->ip),&(ac_addr.sin_addr.s_addr),sizeof(int));
				//bsock->ip = ac_addr.sin_addr.s_addr;
				bsock->next = bak_list;
				bak_list = bsock;
				wid_syslog_debug_debug(WID_MB,"get sock %d\n",bak_list->sock);
			}
		}
		CWThreadMutexUnlock(&MasterBak);
		for(i = 0; i<WTP_NUM;i++){
			if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5)){
				bak_add_del_wtp(new_sock,B_ADD,i);
				
				for(j = 0; j < AC_WTP[i]->RadioCount; j++){
					if(AC_WTP[i]->WTP_Radio[j]->AdStat == 1){
						for(k = 0;k < L_BSS_NUM;k++ ){
							if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->State == 1)){
								bak_add_del_bss(new_sock,B_ADD,AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex);
								
							}
						}
					}
				}
			}
		}
		sleep(1);
		WIDBAKInfoToASD(is_secondary,&M_addr,ac_addr.sin_addr.s_addr,0,"update",WID_UPDATE);			
	}
//	close(wid_sock);
	pthread_exit((void *) 0);
}



void *wid_bak_thread()
{
	wid_pid_write_v2("wid_bak_thread,no define",0,vrrid);
	int numbytes;
	char buf[2048];
	socklen_t addr_len;
	int ret;
	fd_set fdset;
	struct timeval tv;						

//	printf("WID_BAK_THREAD\n");
	while(is_secondary == 1){
		int wid_bak_sock;
		if ((wid_bak_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
		{	wid_syslog_info("udp socket create failed\n");		
			exit(1);	
		}	
		while(connect(wid_bak_sock, (struct sockaddr *)&M_addr, sizeof(struct sockaddr)) == -1){
			wid_syslog_debug_debug(WID_MB,"connet master ac\n");
			sleep(1);
			continue;
		}
		addr_len = sizeof(struct sockaddr);
		while(is_secondary == 1){		
			memset(buf, 0, 2048);
			FD_ZERO(&fdset);
			FD_SET(wid_bak_sock,&fdset);
			
			tv.tv_sec = 1;
			tv.tv_usec = 5000;
			ret = select(wid_bak_sock + 1,&fdset,(fd_set *) 0,(fd_set *) 0,&tv);

			if(ret == -1){
				break;
			}else if(ret ==0){
				continue;
			}
			else if (FD_ISSET(wid_bak_sock, &fdset)){
				numbytes = recv(wid_bak_sock, buf, 2047, 0);
				if(numbytes < 0){
					wid_syslog_info("bak recv error\n");
					break;
				}
			}
//			printf("numbytes %d\n",numbytes);
			if(is_secondary == 1){
			B_Msg *msg = (B_Msg*) buf;
				switch(msg->Type){
					case B_WTP_TYPE:
//						printf("wtp op\n");
						B_WTP_OP(msg);
						break;	
					case B_BSS_TYPE:
//						printf("bss op\n");
						B_BSS_OP(msg);
						break;	
					case B_INFO_TYPE:
//						printf("break;");
						break;
						//B_INFO_OP(msg);
//						break;
					default:
						break;
				}
			}
		}
		close(wid_bak_sock);
	}
	pthread_exit((void *) 0);
}
#else

int check_license(B_Msg *tmp){
	int i = 0; 
	if(glicensecount >= AC_LICENSE_COUNT){
		tmp->Bu.LICENSE.count = AC_LICENSE_COUNT;
	}else{
		tmp->Bu.LICENSE.count = glicensecount;
	}
	for(i = 0; i < tmp->Bu.LICENSE.count; i++){
		if(g_wtp_count[i] != NULL)
			tmp->Bu.LICENSE.license[i] = g_wtp_count[i]->gmax_wtp_count;
	}
	return 0;
}
int notice_wid_update_to_hmd(B_Msg *msg){
	wid_syslog_info("%s,%d,vrrid:%d.\n",__func__,__LINE__,vrrid);
	int h_times =0;
	while(h_times < 3){
		if(-1 == notice_hmd_update_state_change(vrrid,0)){
			h_times ++;
			sleep(2);
			wid_syslog_info("%s,%d,vrrid:%d,h_times:%d,sleep(2),send again.\n",__func__,__LINE__,vrrid,h_times);
		}else{
			h_times = 4;
			wid_syslog_info("%s,%d,vrrid:%d,h_times:%d,break,send successfull.\n",__func__,__LINE__,vrrid,h_times);
			break;
		}
	}
	wid_syslog_info("%s,%d,vrrid:%d,ret:%d.\n",__func__,__LINE__,vrrid);
	return 0;
}
int compare_license(B_Msg *tmp){
	int count = 0;
	int i = 0;
	int flag = 0;
	int num1 = 0;
	if(glicensecount >= tmp->Bu.LICENSE.count){
		count = tmp->Bu.LICENSE.count;
	}else{
		count = glicensecount;
	}
	CWThreadMutexLock(&ACLicense);
	for(i = 0; i < count; i++){
		if(g_wtp_count[i] != NULL){
			if(tmp->Bu.LICENSE.license[i] > g_wtp_count[i]->gmax_wtp_count){
				wid_syslog_info("%s,%d,shared lic,change type %d from %d to %d.\n",__func__,__LINE__,i+1,g_wtp_count[i]->gmax_wtp_count,tmp->Bu.LICENSE.license[i]);
				if(g_wtp_count[i]->flag != 0){
					flag = g_wtp_count[i]->flag;
					g_wtp_binding_count[flag]->gmax_wtp_count += (tmp->Bu.LICENSE.license[i] - g_wtp_count[i]->gmax_wtp_count);
				}
				g_wtp_count[i]->gmax_wtp_count = tmp->Bu.LICENSE.license[i];
				g_wtp_count[i]->isShm = 1;
			}else{
				if(g_wtp_count[i]->isShm == 1){
					if(tmp->Bu.LICENSE.license[i] < g_wtp_count[i]->gmax_wtp_count_assign){
						if(g_wtp_count[i]->gmax_wtp_count >= g_wtp_count[i]->gmax_wtp_count_assign){
							wid_syslog_info("%s,%d,reduce shared lic,1change to lic assign,type %d from %d to %d.\n",__func__,__LINE__,i+1,g_wtp_count[i]->gmax_wtp_count,g_wtp_count[i]->gmax_wtp_count_assign);
							num1 = g_wtp_count[i]->gmax_wtp_count - g_wtp_count[i]->gmax_wtp_count_assign;
							if((g_wtp_count[i]->flag != 0)){
								flag = g_wtp_count[i]->flag;
								g_wtp_binding_count[flag]->gmax_wtp_count -= num1;
							}
							g_wtp_count[i]->gmax_wtp_count = g_wtp_count[i]->gmax_wtp_count_assign;
						}else{//normal ,this case not exsit
							wid_syslog_info("%s,%d,reduce shared lic,2change to lic assign,type %d from %d to %d.\n",__func__,__LINE__,i+1,g_wtp_count[i]->gmax_wtp_count,g_wtp_count[i]->gmax_wtp_count_assign);
							num1 = g_wtp_count[i]->gmax_wtp_count_assign - g_wtp_count[i]->gmax_wtp_count;
							if((g_wtp_count[i]->flag != 0)){
								flag = g_wtp_count[i]->flag;
								g_wtp_binding_count[flag]->gmax_wtp_count += num1;
							}
							g_wtp_count[i]->gmax_wtp_count = g_wtp_count[i]->gmax_wtp_count_assign;
						}
						g_wtp_count[i]->isShm = 0;
					}else{
						wid_syslog_info("%s,%d,reduce shared lic,new shared greater than assign,change type %d from %d to %d,assign lic is %d.\n",__func__,__LINE__,i+1,g_wtp_count[i]->gmax_wtp_count,tmp->Bu.LICENSE.license[i],g_wtp_count[i]->gmax_wtp_count_assign);
						num1 = g_wtp_count[i]->gmax_wtp_count - tmp->Bu.LICENSE.license[i];
						if((g_wtp_count[i]->flag != 0)){
							flag = g_wtp_count[i]->flag;
							g_wtp_binding_count[flag]->gmax_wtp_count -= num1;
						}
						g_wtp_count[i]->gmax_wtp_count = tmp->Bu.LICENSE.license[i];
					}
				}else{
					wid_syslog_info("%s,%d g_wtp_count[%d]->isShm != 1.\n",__func__,__LINE__, i);
				}
			}
		}
	}
	CWThreadMutexUnlock(&ACLicense);
	return 0;
}

int update_license(int sockfd,struct sockaddr_in *addr){
	B_Msg msg;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	msg.Type = B_CHECK_LICENSE;
	check_license(&msg);
	len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	int ip = addr->sin_addr.s_addr;
	wid_syslog_debug_debug(WID_DBUS,"%s,%d,sockfd=%d,ip=%d.%d.%d.%d.\n",__func__,__LINE__,sockfd,(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		wid_syslog_info("select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)addr, sizeof(struct sockaddr));
#endif
		wid_syslog_info("%s,%d,send update license ret %d\n",__func__,__LINE__,ret);
		if( ret < 0){
			wid_syslog_info("%s send %s",__func__,strerror(errno));
			perror("send");		
			wid_syslog_info("bak_add_del_wtp send error\n");
			//exit(1);
		}
	}
	return 0;
}

int update_license_req(int sockfd,struct sockaddr_in *addr){
	B_Msg msg;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	msg.Type = B_LICENSE_REQUEST;
	check_license(&msg);
	len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		wid_syslog_info("select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)addr, sizeof(struct sockaddr));
#endif
		wid_syslog_info("%s,%d,send update license req ret %d\n",__func__,__LINE__,ret);
		if( ret < 0){
			wid_syslog_info("%s send %s",__func__,strerror(errno));
			perror("send");		
			wid_syslog_info("bak_add_del_wtp send error\n");
			//exit(1);
		}
	}
	return 0;
}


int bak_check_campare_license(int sockfd,B_Msg *tmp){
	B_Msg msg;
	int len;
	int ret;
	char buf[1024];
	compare_license(tmp);
	memset(buf,0,1024);
	msg.Type = B_CHECK_LICENSE;
	check_license(&msg);
	len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		wid_syslog_info("select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
#endif
		wid_syslog_debug_debug(WID_MB,"ap ret %d\n",ret);
		if( ret < 0){
			wid_syslog_info("%s send %s",__func__,strerror(errno));
			perror("send");		
			wid_syslog_info("bak_add_del_wtp send error\n");
			//exit(1);
		}
	}
	return 0;
}

int bak_check_req(int sockfd){
	B_Check_Msg msg;
	int len;
	int ret;
	int i = 0;
	char buf[4096+1024];
	memset(buf,0,(4096+1024));
	memset(&msg,0,sizeof(msg));
	msg.Type = B_CHECK_REQ;
	for(i = 1; i < WTP_NUM; i++){
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 9)){
			if(i <= 4096){
				msg.Bu.WTP_CHECK.wtp_check[i] = 1;
				msg.Bu.WTP_CHECK.wtp_count++;
			}else{
				wid_syslog_warning("%s,%d,invlid wtpid:%d for WTP_CHECK.wtp_check.\n",__func__,__LINE__,i);
			}
		}
	}
	len = sizeof(msg);
	if(len < (4096+1024)){
		memcpy(buf,(char*)&msg,len);
	}else{
		wid_syslog_warning("%s,%d,invlid len:%d.\n",__func__,__LINE__,len);
	}
	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		wid_syslog_info("select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr));
#endif
		wid_syslog_debug_debug(WID_MB,"bss ret %d\n",ret);
		if(ret < 0){			
			wid_syslog_info("%s send %s",__func__,strerror(errno));
			perror("send");
			wid_syslog_info("bak_check_req send error\n");
			//exit(1);
		}
	}

	
	return 0;
}

void *wid_master_thread()
{
	wid_pid_write_v2("wid_master_thread",0,vrrid);
	socklen_t addr_len;
	char buf[4096+1024];
	int len;
	int wtp_count = 0;
	int bss_count = 0;
	unsigned int count = 0;
	unsigned int bssindex[4096] = {0};
	send_all_tunnel_interface_arp();
	while(is_secondary == 0){		
		struct bak_sock *bsock;
		unsigned int i,j,k;
		addr_len = sizeof(struct sockaddr);
		wid_syslog_debug_debug(WID_MB,"wait bak wtp\n");
		//printf("wait bak wtp\n");
		memset(buf, 0, (4096+1024));
		fd_set fdset;
		struct timeval tv;			
		FD_ZERO(&fdset);
		FD_SET(wid_sock,&fdset);
		
		tv.tv_sec = 3;
		tv.tv_usec = 5000;
		
		if (select(wid_sock + 1,&fdset,(fd_set *) 0,(fd_set *) 0,&tv) == -1)
		{
			if((is_secondary == 0)){
				wid_syslog_debug_debug(WID_MB,"wid_master_thread wait continue\n");
				continue;
			}else{				
				wid_syslog_debug_debug(WID_MB,"wid_master_thread quit\n");
				break;
			}
		}

		if (FD_ISSET(wid_sock, &fdset)){		
			len = recvfrom(wid_sock,buf, (4095+1024),0, (struct sockaddr *)&B_addr, &addr_len);
			//printf("get bak wtp sock:%d\n",new_sock);
			
			wid_syslog_info("%s len %d\n",__func__, len);
			if(len <= 0){
				wid_syslog_info("recvfrom failed\n");
				continue;
			}
			
			CWThreadMutexLock(&MasterBak);
			if(bak_list == NULL){
				bsock = (struct bak_sock*)malloc(sizeof(struct bak_sock));
				memset(bsock,0,sizeof(struct bak_sock));
				bsock->sock = wid_sock;
				//memcpy(&(bsock->ip),&(ac_addr.sin_addr.s_addr),sizeof(int));
				//bsock->ip = B_addr.sin_addr.s_addr;
				bak_list = bsock;
				wid_syslog_debug_debug(WID_MB,"get sock %d\n",bak_list->sock);
			}/*else{
				tmp = bak_list;
				if((tmp != NULL)&&(tmp->sock != wid_sock)){
					memset(tmp,0,sizeof(struct bak_sock));
					tmp->sock = wid_sock;
					//memcpy(&(bsock->ip),&(ac_addr.sin_addr.s_addr),sizeof(int));
					tmp->ip = B_addr.sin_addr.s_addr;
					wid_syslog_debug_debug(WID_MB,"get sock2 %d\n",bak_list->sock);
				}
			}
			*/
			B_Msg *msg = (B_Msg*) buf;			
			B_Check_Msg *msg1 = (B_Check_Msg*) buf;
			switch(msg->Type){
				case B_AC_REQUEST:					
					wid_syslog_info("%s _AC_REQUEST\n",__func__);
					#if 1
					neighbor_slotid = msg->Op;
					wid_syslog_info("%s _AC_REQUEST  neighbor_slotid %d\n",__func__,neighbor_slotid);
					bak_check_campare_license(wid_sock,msg);
					for(i = 1; i<WTP_NUM;i++){
						if((AC_WTP[i] != NULL)&&((AC_WTP[i]->WTPStat == 5) || (AC_WTP[i]->WTPStat == 9))){
							bak_add_del_wtp(wid_sock,B_ADD,i);
							wtp_count++;
							for(j = 0; j < AC_WTP[i]->RadioCount; j++){
								if(AC_WTP[i]->WTP_Radio[j]->AdStat == 1){
									for(k = 0;k < L_BSS_NUM;k++ ){
										if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->State == 1)){
											bak_add_del_bss(wid_sock,B_ADD,AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex);
											bss_count++;	
										}
									}
								}
							}
						}
						if((bss_count/30 > 0)||(wtp_count/30 > 0)){
							sleep(1);
							bss_count = 0;
							wtp_count = 0;
							#if 0
							if(bak_check(wid_sock, wtp_count, bss_count)){
								wid_syslog_debug_debug(WID_MB,"gogogogo\n");
								;
							}else{
								i = i - 100 + 1;
								if(i <= 0){
									i = 1;
								}
							}
							wtp_count = 0;
							bss_count = 0;
							#endif
						}
					}
					#endif
					sleep(1);
					if(local)
						WIDLocalBAKInfoToASD(is_secondary,neighbor_slotid,vrrid,WID_UPDATE);	
					else{
						struct sockaddr_in * temp = (struct sockaddr_in *)&B_addr;
						WIDBAKInfoToASD(is_secondary,(struct sockaddr_in *)&M_addr,temp->sin_addr.s_addr,0,"update",WID_UPDATE);	
					}
					notice_wid_update_to_hmd(NULL);
					break;
				case B_CHECK_REQ:					
					wid_syslog_info("%s B_CHECK_REQ\n",__func__);
					for(i = 1; i < WTP_NUM; i++){
						if((AC_WTP[i] != NULL)&&((AC_WTP[i]->WTPStat == 5) || (AC_WTP[i]->WTPStat == 9))){
							if(i <= 4096){
								if(msg1->Bu.WTP_CHECK.wtp_check[i] == 0){
									bak_add_del_wtp(wid_sock,B_ADD,i);
									for(j = 0; j < AC_WTP[i]->RadioCount; j++){
										if(AC_WTP[i]->WTP_Radio[j]->AdStat == 1){
											for(k = 0;k < L_BSS_NUM;k++ ){
												if((AC_WTP[i]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[i]->WTP_Radio[j]->BSS[k]->State == 1)){
													bak_add_del_bss(wid_sock,B_ADD,AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex);

													if((AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex < L_RADIO_NUM*L_BSS_NUM)
														||(AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex > G_RADIO_NUM*L_BSS_NUM)){
														wid_syslog_info("bssindex is error! Bssindex = %d\n",
															AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex);
														continue;
													}
													bssindex[count++] = AC_WTP[i]->WTP_Radio[j]->BSS[k]->BSSIndex;
													if(count >= 4096){
														sleep(1);
														wid_syslog_debug_debug(WID_MB,"count = %d\n",count);	//for test
														WIDBAKBSSInfoToASD(0, bssindex, count, BSS_UPDATE);
														memset(bssindex,0,4096*sizeof(int));
														count = 0;
													}
												}
											}
										}
									}
								}
							}else{
								wid_syslog_warning("%s,%d,invlid wtpid:%d for WTP_CHECK.wtp_check.\n",__func__,__LINE__,i);
							}
						}
					}
					//mahz add 2011.6.22
					if(count > 0){
						sleep(1);
						wid_syslog_debug_debug(WID_MB,"count = %d\n",count);	//for test
						WIDBAKBSSInfoToASD(0, bssindex, count, BSS_UPDATE);
					}
					count = 0;
					break;				
				case B_CHECK_LICENSE:
					compare_license(msg);
					break;
				case B_LICENSE_REQUEST:
					wid_syslog_info("%s B_LICENSE_REQUEST\n",__func__);
					compare_license(msg);
					update_license(wid_sock , (struct sockaddr_in *)&B_addr);
					break;
				default:
					wid_syslog_info("%s msg->Type %d break\n",__func__,msg->Type);
					break;
			}				
			CWThreadMutexUnlock(&MasterBak);
		}
	}
//	close(wid_sock);
	pthread_exit((void *) 0);
}

void *wid_bak_thread()
{
	wid_pid_write_v2("wid_bak_thread",0,vrrid);
	int numbytes;
	char buf[2048];
	socklen_t addr_len;
	B_Msg tmp;	
	int wtp_count = 0;
	int bss_count = 0;	
	int sndbuf = 65525;
	int rcvbuf = 65525;
	struct sockaddr tmp_addr;
	memset(&tmp, 0, sizeof(B_Msg));
	tmp.Type = B_AC_REQUEST;
	tmp.Op = slotid;
	check_license(&tmp);
	int b_ac_request_only_one_flag = 1;  //fengwenchao add for axsszfi-1770
//	printf("WID_BAK_THREAD\n");
	while(is_secondary == 1){
		if(local){
			if ((wid_bak_sock = socket(AF_TIPC, SOCK_RDM, 0)) ==-1)
			{	wid_syslog_info("udp socket create failed\n");		
				exit(1);	
			}	
		}else{
			if ((wid_bak_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1)
			{	wid_syslog_info("udp socket create failed\n");		
				exit(1);	
			}	
		}	
		setsockopt(wid_bak_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
		setsockopt(wid_bak_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
		if(!CWErr(CWTimerRequest(BakCheckInterval / TO_BAKUP_SECOND_PACKET_INTERVAL_MASK, NULL, &(bak_check_timer), 900, 0))) { 
			return CW_FALSE;
		}
		/*
		while(sendto(wid_bak_sock,(char *)&tmp, sizeof(B_Msg),0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr)) <= 0){
			wid_syslog_info("sendto master ac\n");
			sleep(1);
			continue;
		}*/
		
		int n = 0;
		int ret = 0;
		while((n <= 1)&&(b_ac_request_only_one_flag == 1)){  //fengwenchao add for axsszfi-1770. we conside B_AC_REQUEST can only send one,even though happened bak recv erro.
			wid_syslog_info("sendto master ac,n:%d\n",n);
			while((ret = sendto(wid_bak_sock,(char *)&tmp, sizeof(B_Msg),0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr))) <= 0){
				wid_syslog_info("sendto master ac error,resend\n");
				
				sleep(1);
				continue;
			}
			if( ret > 0){		
				b_ac_request_only_one_flag = 0; //fengwenchao add for axsszfi-1770
				break;
			}	
			sleep(1);
			n++;
			continue;
		}

		while(is_secondary == 1){		
			memset(buf, 0, 2048);
			
			fd_set fdset;
			struct timeval tv;			
			FD_ZERO(&fdset);
			FD_SET(wid_bak_sock,&fdset);
			
			tv.tv_sec = 2;
			tv.tv_usec = 5000;
			
			if (select(wid_bak_sock + 1,&fdset,(fd_set *) 0,(fd_set *) 0,&tv) == -1)
			{
				if((is_secondary == 1))
					continue;
				else
					break;
			}

			if (FD_ISSET(wid_bak_sock, &fdset)){
			numbytes = recvfrom(wid_bak_sock, buf, 2047, 0,&tmp_addr,&addr_len);
			wid_syslog_info("bak recv numbytes %d.\n",numbytes);
			if(numbytes < 0){
				wid_syslog_info("bak recv error\n");
				break;
			}
//			printf("numbytes %d\n",numbytes);
			if(is_secondary == 1){
			B_Msg *msg = (B_Msg*) buf;
				switch(msg->Type){
					case B_WTP_TYPE:
//						printf("wtp op\n");
						B_WTP_OP(msg);
						wtp_count++;
						break;	
					case B_BSS_TYPE:
//						printf("bss op\n");
						B_BSS_OP(msg);
						bss_count++;
						break;	
					case B_INFO_TYPE:
//						printf("break;");
						break;
						//B_INFO_OP(msg);
					case B_CHECK_LICENSE:
						compare_license(msg);
						break;
					case B_LICENSE_REQUEST:
						wid_syslog_info("%s B_LICENSE_REQUEST\n",__func__);
						compare_license(msg);
						CWThreadMutexLock(&MasterBak);
						update_license(wid_bak_sock , (struct sockaddr_in *)&M_addr);
						CWThreadMutexUnlock(&MasterBak);
						break;
					default:
						break;
				}
			}
		}
		}
		close(wid_bak_sock);
	}
	pthread_exit((void *) 0);
}

#endif
