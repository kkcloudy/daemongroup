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
* AsdInterACRoaming.c
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

#include   <netdb.h>   
#include   <sys/ioctl.h>   


#include "circle.h"
#include "asd.h"
#include "ASD8021XOp.h"
#include "ASD80211Op.h"
#include "ASDBeaconOp.h"
#include "ASDHWInfo.h"
#include "ASDAccounting.h"
#include "ASDEapolSM.h"
#include "ASDIappOp.h"
#include "ap.h"
#include "ASD80211AuthOp.h"
#include "ASDApList.h"
#include "ASDStaInfo.h"
#include "ASDCallback.h"
#include "ASDRadius/radius_client.h"
#include "ASDRadius/radius_server.h"
#include "ASDRadius/radius.h"
#include "ASDWPAOp.h"
#include "ASDPreauth.h"
#include "ASDWme.h"
#include "vlan_init.h"
#include "ASDCtrlIface.h"
#include "tls.h"
#include "ASDEAPMethod/eap_sim_db.h"
#include "ASDEAPMethod/eap.h"

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "ASDDbus.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ASDDbus_handler.h"
#include "ASDWPAOp.h"
#include "ASDMlme.h"

#include "Inter_AC_Roaming.h"

unsigned int AC_G_FIRST = 1;
Inter_AC_R_Group *AC_GROUP[GROUP_NUM];
unsigned int roaming_notice = 1;
int gsock = -1;
unsigned int inter_ac_roaming_count = 0;
unsigned int inter_ac_roaming_in_count = 0;
unsigned int inter_ac_roaming_out_count = 0;
extern unsigned char FD_CHANGE;
int G_Wsm_WTPOp(unsigned char group_id,unsigned char ACID,Operate op){
	TableMsg wWsm;
	int len;
	Mobility_AC_Info *vAC;
	if((AC_GROUP[group_id] == NULL)||(AC_GROUP[group_id]->Mobility_AC[ACID] == NULL))
	{
		return 0;
	}
	vAC = AC_GROUP[group_id]->Mobility_AC[ACID];
	wWsm.Op = op;
	wWsm.Type = WTP_TYPE;
	wWsm.u.DataChannel.WTPID = vAC->vWTPID;
	wWsm.u.DataChannel.WTPIP.u.ipv4_addr = vAC->ACIP;
	len = sizeof(wWsm);
	if(sendto(TableSend, &wWsm, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	return 1;
}

int G_Wsm_BSSOp(unsigned char group_id,unsigned char ACID,unsigned int BSSIndex,Operate op){
	TableMsg wASD;
	int len;
	Mobility_BSS_Info *vBSS;
	if((AC_GROUP[group_id] == NULL)\
		||(AC_GROUP[group_id]->Mobility_AC[ACID] == NULL)\
		||(AC_GROUP[group_id]->Mobility_AC[ACID]->BSS[BSSIndex]== NULL))
	{
		return 0;
	}
	vBSS = AC_GROUP[group_id]->Mobility_AC[ACID]->BSS[BSSIndex];
	wASD.Op = op;
	wASD.Type = BSS_TYPE;
	wASD.u.BSS.BSSIndex = vBSS->vBSSIndex;
	wASD.u.BSS.Radio_L_ID = vBSS->Radio_L_ID;
	wASD.u.BSS.Radio_G_ID = vBSS->vBSSIndex/L_BSS_NUM;
	wASD.u.BSS.WlanID = vBSS->WLANID;
	memcpy(wASD.u.BSS.BSSID,vBSS->BSSID, MAC_LEN);
	wASD.u.BSS.protect_type = vBSS->protect_type;
	wASD.u.BSS.wlan_ifaces_type = vBSS->wlan_ifaces_type;
	wASD.u.BSS.bss_ifaces_type = vBSS->bss_ifaces_type;
	
	len = sizeof(wASD);
	if(sendto(TableSend, &wASD, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	return 1;
}

int G_Wsm_STAOp(struct asd_data *wasd,struct sta_info *sta,Operate op,roam_type type){
	TableMsg STA;
	int len;
	STA.Op = op;
	STA.Type = STA_TYPE;
	
	STA.u.STA.BSSIndex = wasd->BSSIndex;
	STA.u.STA.WTPID = ((wasd->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
	memcpy(STA.u.STA.STAMAC, sta->addr, MAC_LEN);
	STA.u.STA.ac_roam_tag = type;
	STA.u.STA.preBSSIndex = sta->PreBssIndex;
	STA.u.STA.ac_ip.addr_family = AF_INET;
	STA.u.STA.ac_ip.u.ipv4_addr= sta->PreACIP;
	memcpy(STA.u.STA.RBSSID, sta->PreBSSID, MAC_LEN);
	len = sizeof(STA);	
	asd_printf(ASD_DEFAULT,MSG_INFO,"G_Wsm_STAOp bssindex %d wtp %d ac_roam_tag %d prebssindex %d op %d ip %x\n",wasd->BSSIndex,STA.u.STA.WTPID,type,STA.u.STA.preBSSIndex,op, sta->PreACIP);			
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	return 0;
}

int G_PreAC_STAOp(Mobility_AC_Info * Mobility_AC, G_UPDATE_STA *U_STA,Operate op){
	TableMsg STA;
	int len;
	unsigned int BSSIndex;
	STA.Op = op;
	STA.Type = STA_TYPE;
	BSSIndex = U_STA->BssIndex;
	if(Mobility_AC->BSS[BSSIndex] == NULL)
		return 0;
	STA.u.STA.BSSIndex = Mobility_AC->BSS[BSSIndex]->vBSSIndex;
	STA.u.STA.WTPID = Mobility_AC->vWTPID;
	memcpy(STA.u.STA.STAMAC, U_STA->STAMAC, MAC_LEN);
	STA.u.STA.ac_roam_tag = 1;
	STA.u.STA.preBSSIndex = U_STA->PreBssIndex;
	STA.u.STA.ac_ip.u.ipv4_addr= Mobility_AC->ACIP;
	memcpy(STA.u.STA.RBSSID, U_STA->PreBSSID, MAC_LEN);
	len = sizeof(STA);
	
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	return 0;
}

int G_O_AC_STAOp(G_Msg *msg,Operate op,roam_type type ){
	TableMsg STA;
	int len;
	STA.Op = op;
	STA.Type = STA_TYPE;
	STA.u.STA.BSSIndex = msg->Gu.U_STA.PreBssIndex;
	STA.u.STA.WTPID = msg->Gu.U_STA.PreBssIndex/L_BSS_NUM/L_RADIO_NUM;
	memcpy(STA.u.STA.STAMAC,  msg->Gu.U_STA.STAMAC, MAC_LEN);
	STA.u.STA.ac_roam_tag = type;	
	STA.u.STA.ac_ip.addr_family = AF_INET;
	STA.u.STA.ac_ip.u.ipv4_addr=  msg->acip;

	asd_printf(ASD_DEFAULT,MSG_INFO,"G_O_AC_STAOp bssindex %d wtp %d ac_roam_tag %d op %d ip %x\n",STA.u.STA.BSSIndex,STA.u.STA.WTPID,type,op, msg->acip);			

	len = sizeof(STA);
	
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
	return 0;
}



int init_asd_sctp_socket(unsigned int port){	
	int sock_tcp;	
	struct sockaddr_in my_addr;	
	int yes = 1;
	//socklen_t addr_len;	int numbytes;	
	//char buf[2048];	
	if ((sock_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
	{	printf("tcp socket create failed\n");
		return -1;
	}	
	if(setsockopt(sock_tcp, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0){  //xk debug:uncheck return
		printf("sock_tcp setsocket failed\n");
		return -1;
	}
//	setsockopt(sock_tcp, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
	my_addr.sin_family = AF_INET;	
	my_addr.sin_port = htons(port);	
	my_addr.sin_addr.s_addr = INADDR_ANY;	
	memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));	
	if (bind(sock_tcp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{	printf("udp bind failed\n");		
		//exit(1);	
	}		
	if(listen(sock_tcp, 5) != 0){   //xk debug:uncheck return
        asd_printf(ASD_DEFAULT,MSG_ERROR,"listen:%s",strerror(errno));
		return -1;
	}
	return sock_tcp;
}

int ACGroupRoamingCheck(unsigned char WLANID, struct asd_data *wasd, const unsigned char *addr){
	unsigned char group_id;
	unsigned int i = 0;
	unsigned int j = 0;
	struct sta_info *sta; 
	struct AC_ROAMING_STAINFO * rsta = NULL;
	if(ASD_WLAN[WLANID] == NULL)
		return 0;
	group_id = ASD_WLAN[WLANID]->group_id;
	if(AC_GROUP[group_id]==NULL)
		return 0;
	sta = ap_get_sta(wasd, addr);
	if(sta == NULL)
		return 0;
	for(i = 0; i < G_AC_NUM; i++){
		if(AC_GROUP[group_id]->Mobility_AC[i] != NULL){
			rsta = G_roaming_get_sta(AC_GROUP[group_id]->Mobility_AC[i],addr);
			if(rsta != NULL){
				sta->BssIndex = wasd->BSSIndex;
				sta->PreBssIndex = rsta->PreBssIndex;
				memcpy(sta->PreBSSID,rsta->PreBSSID,MAC_LEN);
				sta->PreACIP = rsta->PreACIP;				
				inter_ac_roaming_count++;
				inter_ac_roaming_in_count++;				
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
					if(ac_group_modify_sta_info(sock,group_id,sta,wasd,AC_GROUP[group_id]->Mobility_AC[i]->ACIP,rsta->BssIndex) == 0)
					{
						gThreadArg* tmp;
						tmp = (gThreadArg*)malloc(sizeof(gThreadArg));
						memset(tmp, 0, sizeof(gThreadArg));
						tmp->group_id = group_id;
						tmp->ACID = j;
						close(sock);
						AC_GROUP[group_id]->Mobility_AC[j]->sock = -1;
						AC_GROUP[group_id]->Mobility_AC[j]->is_conn = 0;
						
						if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
						{	printf("tcp socket create failed\n");
						    os_free(tmp);   //xk debug:resource leak
							return 0;
						}
						tmp->sock = sock;
						//fcntl(sock, F_SETFL, O_NONBLOCK);
						unsigned long ul = 1;
						ioctl(sock, FIONBIO, &ul);
						circle_register_timeout(0, 0,asd_synch_select, tmp, NULL);
					}
					//close(sock);
				}
				if(rsta->PreACIP != AC_GROUP[group_id]->host_ip)
					G_Wsm_STAOp(wasd,sta,WID_ADD,ROAMING_AC);
				else
					G_Wsm_STAOp(wasd,sta,WID_ADD,NO_ROAMING);
					
				G_roaming_free_sta(AC_GROUP[group_id]->Mobility_AC[i],rsta);
				break;
			}
		}
	}
	if(i == G_AC_NUM){		
		sta->BssIndex = wasd->BSSIndex;
		sta->PreBssIndex =  wasd->BSSIndex;
		memcpy(sta->PreBSSID,wasd->own_addr,MAC_LEN);
		sta->PreACIP = AC_GROUP[group_id]->host_ip;
		for(j = 0; j < G_AC_NUM; j++){
			int sock;
			if((AC_GROUP[group_id]->Mobility_AC[j] == NULL)||(AC_GROUP[group_id]->Mobility_AC[j]->is_conn == 0))
				continue;
			#if 0
			if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
			{	printf("tcp socket create failed\n");
				break;;
			}			
			unsigned long ul = 1;
			int n = 2;
			ioctl(sock, FIONBIO, &ul);
			while((connect(sock, (struct sockaddr *)&(AC_GROUP[group_id]->Mobility_AC[j]->ac_addr), sizeof(struct sockaddr)) == -1)){
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
			if(n <= 0){
				UpdateStaInfoToWSM(wasd, addr, WID_ADD);
				return;
			}
			#endif
			sock = AC_GROUP[group_id]->Mobility_AC[j]->sock;
			if(ac_group_add_del_sta_info(sock, group_id, sta, wasd, G_ADD)==0)
			{
				gThreadArg* tmp;
				tmp = (gThreadArg*)malloc(sizeof(gThreadArg));
				memset(tmp, 0, sizeof(gThreadArg));
				tmp->group_id = group_id;
				tmp->ACID = j;
				close(sock);
				AC_GROUP[group_id]->Mobility_AC[j]->sock = -1;
				AC_GROUP[group_id]->Mobility_AC[j]->is_conn = 0;
				
				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
				{	printf("tcp socket create failed\n");
					os_free(tmp);//qiuchen
					return 0;
				}
				tmp->sock = sock;
				//fcntl(sock, F_SETFL, O_NONBLOCK);
				unsigned long ul = 1;
				ioctl(sock, FIONBIO, &ul);
				circle_register_timeout(0, 0,asd_synch_select, tmp, NULL);
			}
			//close(sock);
		}
		UpdateStaInfoToWSM(wasd, addr, WID_ADD);
	}
	return 0;
}


struct AC_ROAMING_STAINFO * G_roaming_get_sta(Mobility_AC_Info * Mobility_AC, const u8 *sta)
{
	struct AC_ROAMING_STAINFO *s;

	s = Mobility_AC->r_sta_hash[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}


void G_roaming_sta_list_del(Mobility_AC_Info * Mobility_AC, struct AC_ROAMING_STAINFO * sta)
{
	struct AC_ROAMING_STAINFO *tmp;

	if (Mobility_AC->r_sta_list == sta) {
		Mobility_AC->r_sta_list = sta->next;
		return;
	}

	tmp = Mobility_AC->r_sta_list;
	while (tmp != NULL && tmp->next != sta)
		tmp = tmp->next;
	if (tmp == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not remove STA " MACSTR " from "
			   "list.", MAC2STR(sta->addr));
	} else
		tmp->next = sta->next;
}


void G_roaming_sta_hash_add(Mobility_AC_Info * Mobility_AC, struct AC_ROAMING_STAINFO * sta)
{
	sta->hnext = Mobility_AC->r_sta_hash[STA_HASH(sta->addr)];
	Mobility_AC->r_sta_hash[STA_HASH(sta->addr)] = sta;
}


static void G_roaming_sta_hash_del(Mobility_AC_Info * Mobility_AC, struct AC_ROAMING_STAINFO *sta)
{
	 struct AC_ROAMING_STAINFO *s;

	s = Mobility_AC->r_sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		Mobility_AC->r_sta_hash[STA_HASH(sta->addr)] = s->hnext;
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


void G_roaming_free_sta(Mobility_AC_Info * Mobility_AC, struct AC_ROAMING_STAINFO *sta)
{
	printf("roaming free sta\n");
	//int set_beacon = 0;
	//int i=0;
		
	G_roaming_sta_hash_del(Mobility_AC, sta);
	G_roaming_sta_list_del(Mobility_AC, sta);
	Mobility_AC->r_num_sta--;
	os_free(sta);

}


struct AC_ROAMING_STAINFO * G_roaming_sta_add(Mobility_AC_Info * Mobility_AC, G_UPDATE_STA	*U_STA)
{
	struct AC_ROAMING_STAINFO *sta;
	sta = G_roaming_get_sta(Mobility_AC, U_STA->STAMAC);
	if (sta){
		printf("roaming find sta in add\n");
		return sta;
	}
	sta = os_zalloc(sizeof(struct AC_ROAMING_STAINFO));
	if (sta == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "malloc failed");
		return NULL;
	}
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"roaming add sta in add\n");
	printf("roaming add sta in add\n");
	os_memcpy(sta->addr, U_STA->STAMAC, ETH_ALEN);
	sta->BssIndex = U_STA->BssIndex;
	sta->PreBssIndex = U_STA->PreBssIndex;
	os_memcpy(sta->PreBSSID, U_STA->PreBSSID, ETH_ALEN);
	sta->PreACIP = U_STA->PreACIP;
	sta->next = Mobility_AC->r_sta_list;
	Mobility_AC->r_sta_list = sta;
	Mobility_AC->r_num_sta++;
	G_roaming_sta_hash_add(Mobility_AC, sta);
	return sta;
	
}

void G_roaming_sta_del(Mobility_AC_Info * Mobility_AC, unsigned char *STAMAC){
	struct AC_ROAMING_STAINFO *sta;	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"roaming add sta in del\n");
	sta = G_roaming_get_sta(Mobility_AC, STAMAC);
	if(sta != NULL){
		G_roaming_free_sta(Mobility_AC, sta);
	}
}

void G_roaming_del_all_sta(Mobility_AC_Info * Mobility_AC)
{
	struct AC_ROAMING_STAINFO *sta, *prev;
	if(Mobility_AC != NULL){
		sta = Mobility_AC->r_sta_list;
		while(sta){
			prev = sta;
			sta = sta->next;
			G_roaming_free_sta(Mobility_AC,prev);
		}
	}
}

int ac_group_syn_request(int sockfd, unsigned char group_id){
	G_Msg msg;
	//unsigned int sid;
	int len;
	int ret;
	char buf[1024];
	memset(&msg,0,sizeof(msg));
	memset(buf,0,1024);
	msg.Type = G_DATA_SYNCH;
	msg.group_id = group_id;	
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
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
		ret = send(sockfd, buf, len+1, 0);
		if(ret < 0){
			perror("send");
			return 0;
			//exit(1);
		}
	}
	return 1;
}

int ac_group_syn_end(int sockfd, unsigned char group_id){
	G_Msg msg;
	memset(&msg,0,sizeof(msg));  //xk debug:using unitialized value
	//unsigned int sid;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	msg.Type = G_SYN_END;
	msg.group_id = group_id;	
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
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
		ret = send(sockfd, buf, len+1, 0);
		if(ret <= 0){
			perror("send");
			return 0;
			//exit(1);
		}
	}
	return 1;
}


int ac_group_add_del_bss_info(int sockfd, unsigned char group_id, struct asd_data *wasd, GOperate Op){
	asd_printf(ASD_DEFAULT,MSG_INFO,"%s\n",__func__);
	G_Msg msg;
	unsigned char sid;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	sid = wasd->SecurityID;
	msg.Op = Op;
	msg.Type = G_BSS_UPDATE;
	msg.group_id = group_id;
	msg.acip = AC_GROUP[group_id]->host_ip;
	msg.Gu.U_BSS.BSSIndex = wasd->BSSIndex;
	msg.Gu.U_BSS.Radio_G_ID = wasd->Radio_G_ID;
	msg.Gu.U_BSS.Radio_L_ID = wasd->Radio_L_ID;
	msg.Gu.U_BSS.WLANID = wasd->WlanID;
	memcpy(msg.Gu.U_BSS.BSSID, wasd->own_addr, MAC_LEN);
	if((ASD_SECURITY[sid] != NULL)&&(ASD_SECURITY[sid]->securityType == OPEN)&&(ASD_SECURITY[sid]->encryptionType == NONE))
		msg.Gu.U_BSS.protect_type = 0;
	else if(ASD_SECURITY[sid] != NULL)
		msg.Gu.U_BSS.protect_type = 1;
	else
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ac_group_add_del_bss_info security %d NULL!!!!!!\n",sid);		
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"ac_group_add_del_bss_info protect_type %d\n",msg.Gu.U_BSS.protect_type);
	msg.Gu.U_BSS.wlan_ifaces_type = BSS_INTERFACE;
	msg.Gu.U_BSS.bss_ifaces_type = BSS_INTERFACE;
	
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
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
		ret = send(sockfd, buf, len+1, 0);
		if(ret <= 0){
			perror("send");
			return 0;
			//exit(1);
		}
	}
	return 1;
}

int ac_group_add_del_sta_info(int sockfd, unsigned char group_id, struct sta_info *sta, struct asd_data *wasd, GOperate Op){
	asd_printf(ASD_DEFAULT,MSG_INFO,"%s\n",__func__);
	G_Msg msg;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	msg.Op = Op;
	msg.Type = G_STA_UPDATE;
	msg.group_id = group_id;
	msg.acip = AC_GROUP[group_id]->host_ip;
	msg.Gu.U_STA.BssIndex = wasd->BSSIndex;
	memcpy(msg.Gu.U_STA.STAMAC, sta->addr, MAC_LEN);
	if(sta->PreBssIndex == 0){
		sta->PreBssIndex = wasd->BSSIndex;
		memcpy(sta->PreBSSID, wasd->own_addr,MAC_LEN);
		sta->PreACIP = AC_GROUP[group_id]->host_ip;
	}
	msg.Gu.U_STA.PreBssIndex = sta->PreBssIndex;
	memcpy(msg.Gu.U_STA.PreBSSID, sta->PreBSSID, MAC_LEN);
	msg.Gu.U_STA.PreACIP = sta->PreACIP;
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
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
		ret = send(sockfd, buf, len+1, 0);
		if(ret < 0){
			perror("send");
			return 0;
			//exit(1);
		}
	}
	return 1;
}

int ac_group_modify_sta_info(int sockfd, unsigned char group_id, struct sta_info *sta,  struct asd_data *wasd, unsigned int O_IP, unsigned int O_BSSIndex){
	asd_printf(ASD_DEFAULT,MSG_INFO,"%s\n",__func__);
	G_Msg msg;
	int len;
	int ret;
	char buf[1024];
	memset(buf,0,1024);
	msg.Op = G_MODIFY;
	msg.Type = G_STA_UPDATE;
	msg.group_id = group_id;
	msg.acip = AC_GROUP[group_id]->host_ip;
	msg.Gu.U_STA.BssIndex = wasd->BSSIndex;
	memcpy(msg.Gu.U_STA.STAMAC, sta->addr, MAC_LEN);
	msg.Gu.U_STA.PreBssIndex = sta->PreBssIndex;
	memcpy(msg.Gu.U_STA.PreBSSID, sta->PreBSSID, MAC_LEN);
	msg.Gu.U_STA.PreACIP = sta->PreACIP;
	msg.Gu.U_STA.O_PreACIP = O_IP;
	msg.Gu.U_STA.O_PreBssIndex = O_BSSIndex;
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
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
		ret = send(sockfd, buf, len+1, 0);
		if(ret < 0){
			perror("send");
			return 0;
			//exit(1);
		}
	}
	return 1;
}


int G_STA_ADD(G_Msg *msg){
	unsigned char group_id;
	int i = 0;
	group_id = msg->group_id;
	if(AC_GROUP[group_id] == NULL)
		return -1;
	for(i = 0; i < G_AC_NUM; i++)
	{
		if((AC_GROUP[group_id]->Mobility_AC[i] != NULL)\
			&&(AC_GROUP[group_id]->Mobility_AC[i]->ACIP == msg->acip))
		{
			G_roaming_sta_add(AC_GROUP[group_id]->Mobility_AC[i], &(msg->Gu.U_STA));
			break;
		}
	}
	return 0;
}

int G_STA_DEL(G_Msg *msg){
	unsigned char group_id;
	int i = 0;
	group_id = msg->group_id;
	if(AC_GROUP[group_id] == NULL)
		return -1;
	for(i = 0; i < G_AC_NUM; i++)
	{
		if((AC_GROUP[group_id]->Mobility_AC[i] != NULL)\
			&&(AC_GROUP[group_id]->Mobility_AC[i]->ACIP == msg->acip))
		{
			G_roaming_sta_del(AC_GROUP[group_id]->Mobility_AC[i], (msg->Gu.U_STA.STAMAC));
			break;
		}
	}
	if(AC_GROUP[group_id]->host_ip == msg->Gu.U_STA.PreACIP){
		
		TableMsg STA;
		int len;
		STA.Op = WID_DEL;
		STA.Type = STA_TYPE;
		
		STA.u.STA.BSSIndex = msg->Gu.U_STA.PreBssIndex;
		STA.u.STA.WTPID = ((msg->Gu.U_STA.PreBssIndex)/L_BSS_NUM)/L_RADIO_NUM;
		memcpy(STA.u.STA.STAMAC, msg->Gu.U_STA.STAMAC, ETH_ALEN);
			
		len = sizeof(STA);
		
		if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWSM.addr, toWSM.addrlen) < 0){
			perror("send(wASDSocket)");
			return 0;
		}	
	}
	return 0;
}

int G_STA_MODIFY(G_Msg *msg){
	unsigned char group_id;
	int i = 0;
	Mobility_AC_Info * Mobility_AC_ADD = NULL;
	Mobility_AC_Info * Mobility_AC_DEL = NULL;
	unsigned int BSSIndex;
	struct asd_data * wasd;
	struct sta_info * sta;
	group_id = msg->group_id;
	if(AC_GROUP[group_id] == NULL)
		return -1;
	for(i = 0; i < G_AC_NUM; i++)
	{
		if((AC_GROUP[group_id]->Mobility_AC[i] != NULL)\
			&&(AC_GROUP[group_id]->Mobility_AC[i]->ACIP == msg->acip))
		{
			Mobility_AC_ADD = AC_GROUP[group_id]->Mobility_AC[i];
			G_roaming_sta_add(AC_GROUP[group_id]->Mobility_AC[i], &(msg->Gu.U_STA));
		}
		else if((AC_GROUP[group_id]->Mobility_AC[i] != NULL)\
			&&(AC_GROUP[group_id]->Mobility_AC[i]->ACIP == msg->Gu.U_STA.O_PreACIP))
		{
			Mobility_AC_DEL = AC_GROUP[group_id]->Mobility_AC[i];
			G_roaming_sta_del(AC_GROUP[group_id]->Mobility_AC[i], (msg->Gu.U_STA.STAMAC));
		}
	}
	if((AC_GROUP[group_id]->host_ip == msg->Gu.U_STA.O_PreACIP)){
		inter_ac_roaming_count++;
		inter_ac_roaming_out_count++;
		BSSIndex = msg->Gu.U_STA.O_PreBssIndex;
		wasd = (struct asd_data *)AsdCheckBSSIndex(BSSIndex);
		if(wasd != NULL){
			sta = ap_get_sta(wasd, msg->Gu.U_STA.STAMAC);
			if(sta != NULL){
				sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
				unsigned char SID = 0;
				if(ASD_WLAN[wasd->WlanID])
					SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
				if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->extensible_auth == 1)&&(ASD_SECURITY[SID]->hybrid_auth == 0)))){
					wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
					mlme_deauthenticate_indication(
						wasd, sta, 0);
					sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
					ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
				}
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"G_STA_MODIFY free O_STA\n");
				if(AC_GROUP[group_id]->host_ip != msg->Gu.U_STA.PreACIP){
					roaming_notice = 0;
					ap_free_sta(wasd, sta, 0);
					roaming_notice = 1;
				}
				else
					ap_free_sta_without_wsm(wasd, sta, 0);
			}
		}
	}	
	if(AC_GROUP[group_id]->host_ip == msg->Gu.U_STA.PreACIP){
		G_O_AC_STAOp(msg,WID_MODIFY,RAW_AC);
		/*if(Mobility_AC_DEL != NULL){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"G_PreAC_STAOp del\n");
			G_PreAC_STAOp(Mobility_AC_DEL, &(msg->Gu.U_STA), WID_DEL);
		}
		if(Mobility_AC_ADD != NULL){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"G_PreAC_STAOp add\n");
			G_PreAC_STAOp(Mobility_AC_ADD, &(msg->Gu.U_STA), WID_ADD);
		}*/
	}
	return 0;
}

int ac_group_sta_op(G_Msg *msg){
	
	switch(msg->Op){
		case G_ADD:{			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"ac_group_sta_op add\n");
			G_STA_ADD(msg);
			break;
		}
		case G_DEL:{			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"ac_group_sta_op del\n");
			G_STA_DEL(msg);
			break;
		}
		case G_MODIFY:{			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"ac_group_sta_op modify\n");
			G_STA_MODIFY(msg);
			break;
		}
		//default:
		//	break;
	}
	return 0;
}

int G_BSS_ADD(G_Msg *msg){
	unsigned char group_id;
	unsigned int BSSIndex;
	int i = 0;
	//int j = 0;
	Mobility_BSS_Info *bss;
	group_id = msg->group_id;
	if(AC_GROUP[group_id] == NULL)
		return -1;
	for(i = 0; i < G_AC_NUM; i++)
	{
		if((AC_GROUP[group_id]->Mobility_AC[i] != NULL)\
			&&(AC_GROUP[group_id]->Mobility_AC[i]->ACIP == msg->acip))
		{
			BSSIndex = msg->Gu.U_BSS.BSSIndex;
			if(AC_GROUP[group_id]->Mobility_AC[i]->BSS[BSSIndex] == NULL){
				bss = AC_GROUP[group_id]->Mobility_AC[i]->BSS[BSSIndex] = (Mobility_BSS_Info*)malloc(sizeof(Mobility_BSS_Info));
				/*for(j = 0; j < L_BSS_NUM; j++){
					if(AC_GROUP[group_id]->Mobility_AC[i]->L_BSS_INDEX[j] == 0)
						break;
				}
				if(j == L_BSS_NUM){
					free(bss);
					return;
				}
				AC_GROUP[group_id]->Mobility_AC[i]->L_BSS_INDEX[j] = msg->Gu.U_BSS.BSSIndex;*/
//				bss->vBSSIndex = (AC_GROUP[group_id]->Mobility_AC[i]->vWTPID)*L_RADIO_NUM*L_BSS_NUM + j;
//				bss->L_BSSIndex = j;
				bss->ACID = i;
				bss->BSSIndex = msg->Gu.U_BSS.BSSIndex;
				memcpy(bss->BSSID, msg->Gu.U_BSS.BSSID, MAC_LEN);
				bss->Radio_G_ID = msg->Gu.U_BSS.Radio_G_ID;
				bss->Radio_L_ID = msg->Gu.U_BSS.Radio_L_ID;
				bss->WLANID = msg->Gu.U_BSS.WLANID;
				bss->bss_ifaces_type = msg->Gu.U_BSS.bss_ifaces_type;
				bss->wlan_ifaces_type = msg->Gu.U_BSS.wlan_ifaces_type;
				bss->protect_type = msg->Gu.U_BSS.protect_type;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"G_Wsm_BSSOp WID_ADD group id %d acid %d bssindex %d\n",group_id,i,bss->BSSIndex);
//				G_Wsm_BSSOp(group_id,i,bss->BSSIndex,WID_ADD);
			}
			else 
			{			
//				asd_printf(ASD_DEFAULT,MSG_DEBUG,"G_Wsm_BSSOp WID_DEL group id %d acid %d bssindex %d\n",group_id,i,BSSIndex);
//				G_Wsm_BSSOp(group_id,i,BSSIndex,WID_DEL);
				bss = AC_GROUP[group_id]->Mobility_AC[i]->BSS[BSSIndex];
				bss->ACID = i;
				bss->BSSIndex = msg->Gu.U_BSS.BSSIndex;
				memcpy(bss->BSSID, msg->Gu.U_BSS.BSSID, MAC_LEN);
				bss->Radio_G_ID = msg->Gu.U_BSS.Radio_G_ID;
				bss->Radio_L_ID = msg->Gu.U_BSS.Radio_L_ID;
				bss->WLANID = msg->Gu.U_BSS.WLANID;
				bss->bss_ifaces_type = msg->Gu.U_BSS.bss_ifaces_type;
				bss->wlan_ifaces_type = msg->Gu.U_BSS.wlan_ifaces_type;
				bss->protect_type = msg->Gu.U_BSS.protect_type;	
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"G_Wsm_BSSOp WID_ADD group id %d acid %d bssindex %d\n",group_id,i,bss->BSSIndex);
//				G_Wsm_BSSOp(group_id,i,BSSIndex,WID_ADD);
			}
			break;
		}
	}
	return 0;
}

int G_BSS_DEL(G_Msg *msg){
	unsigned char group_id;
	unsigned int BSSIndex;
	int i = 0;
	//Mobility_BSS_Info *bss;
	group_id = msg->group_id;
	if(AC_GROUP[group_id] == NULL)
		return -1;
	for(i = 0; i < G_AC_NUM; i++)
	{
		if((AC_GROUP[group_id]->Mobility_AC[i] != NULL)\
			&&(AC_GROUP[group_id]->Mobility_AC[i]->ACIP == msg->acip))
		{
			BSSIndex = msg->Gu.U_BSS.BSSIndex;			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"G_Wsm_BSSOp WID_DEL group id %d acid %d bssindex %d\n",group_id,i,BSSIndex);
//			G_Wsm_BSSOp(group_id,i,BSSIndex,WID_DEL);
			if(AC_GROUP[group_id]->Mobility_AC[i]->BSS[BSSIndex] != NULL){
				free(AC_GROUP[group_id]->Mobility_AC[i]->BSS[BSSIndex]);
				AC_GROUP[group_id]->Mobility_AC[i]->BSS[BSSIndex] = NULL;
			} 
			break;
		}
	}
	return 0;
}

int ac_group_bss_op(G_Msg *msg){
	
	switch(msg->Op){
		case G_ADD:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"ac_group_bss_op add\n");
			G_BSS_ADD(msg);
			break;
		}
		case G_DEL:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"ac_group_bss_op del\n");			
			G_BSS_DEL(msg);
			break;
		}
		default:
			break;
	}
	return 0;
}

int ac_group_del(G_Msg *msg){
	unsigned char group_id;
	int i = 0;
	//int j = 0;
	//Mobility_BSS_Info *bss;
	group_id = msg->group_id;
	if(AC_GROUP[group_id] == NULL)
		return -1;
	for(i = 0; i < G_AC_NUM; i++)
	{
		if((AC_GROUP[group_id]->Mobility_AC[i] != NULL)\
			&&(AC_GROUP[group_id]->Mobility_AC[i]->ACIP == msg->acip))
		{
			AC_GROUP[group_id]->Mobility_AC[i]->is_conn = 0;
		}
	}
	return 0;
}

#if 0
void *asd_synch_thread(void *arg){
	unsigned char group_id = ((gThreadArg*)arg)->group_id;
	unsigned char ACID = ((gThreadArg*)arg)->ACID;
	int sock;
	int numbytes;
	char buf[2048];
	int ret;
	struct timeval timeo;	
	socklen_t len = sizeof(timeo);
	asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread 1\n");
	if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){		
		asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread 2\n");

		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
		{	printf("tcp socket create failed\n");
			return;
		}
		//fcntl(sock, F_SETFL, O_NONBLOCK);
		unsigned long ul = 1;
     	ioctl(sock, FIONBIO, &ul);
//		timeo.tv_sec = 5;
		
//		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
		asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread sock %d\n",sock);
		while(((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL))\
			&&(ret = connect(sock, (struct sockaddr *)&(AC_GROUP[group_id]->Mobility_AC[ACID]->ac_addr), sizeof(struct sockaddr)) == -1)){
			perror("connect");
			asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread connect ret : %d\n",ret);
//			if(ul == 1){
//				ul = 0;
//				ioctl(sock, FIONBIO, &ul);
//			}
			while((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
				fd_set fdset;
				struct timeval tv;			
				FD_ZERO(&fdset);
				FD_SET(sock,&fdset);
				
				tv.tv_sec = 5;
				tv.tv_usec = 0;
				
				if(select(sock + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
				{
					continue;
				}
				
				if (FD_ISSET(sock, &fdset)){
					ul = 0;
					ioctl(sock, FIONBIO, &ul);
					if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL))
						AC_GROUP[group_id]->Mobility_AC[ACID]->is_conn = 1;
					break;			
				}
			}
			break;
		}
		asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread sock2 %d\n",sock);
		
		ac_group_syn_request(sock,group_id);

		asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread 4\n");
		while(((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL))){
			memset(buf, 0, 2048);
			
			asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread 5\n");
			numbytes = recv(sock, buf, 2047, 0);
			asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread numbytes %d\n",numbytes);

			if(numbytes <= 0){
				asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread recv3\n");
				close(sock);
				break;
			}
			printf("1\n");
			G_Msg *msg = (G_Msg*) buf;
			switch(msg->Type){
				case G_BSS_UPDATE:{					
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread G_BSS_UPDATE\n");
					ac_group_bss_op(msg);
					break;
				}
				case G_STA_UPDATE:{					
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread G_STA_UPDATE\n");
					ac_group_sta_op(msg);
					break;
				}
				case G_SYN_END:{					
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread G_SYN_END\n");
					close(sock);
					pthread_exit((void *) 0);
					return;
				}
				default:
					break;
			}
		}
	}
	close(sock);
	pthread_exit((void *) 0);
}

void *asd_ac_group_thread()
{
	int gsock;
	int tsock;
	int numbytes;
	char buf[2048];
	socklen_t addr_len;
	struct sockaddr_in ac_addr; 
	gsock = init_asd_sctp_socket(60086);
	listen(gsock, 5);
	unsigned char group_id;
	unsigned char WLANID;
	struct asd_data **bss;
	unsigned int bss_num;
	unsigned int i = 0;
	unsigned int j = 0;
	struct sta_info *sta;
	
	while(AC_G_FIRST == 0){		
		addr_len = sizeof(struct sockaddr);
		tsock = accept(gsock, (struct sockaddr *)&ac_addr, &addr_len);
		printf("new_sock %d\n",tsock);
		if(tsock == -1){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"accept failed\n");
			continue;
		}
	
		memset(buf, 0, 2048);
		numbytes = recv(tsock, buf, 2047, 0);
		if(numbytes <= 0){
			printf("group recv error\n");
			close(tsock);
			continue;
		}
		printf("1\n");
		if(AC_G_FIRST == 0){
			G_Msg *msg = (G_Msg*) buf;
			switch(msg->Type){
				case G_DATA_SYNCH:{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_DATA_SYNCH\n");
					group_id = msg->group_id;
					if(AC_GROUP[group_id] == NULL){
						break;
					}
					WLANID = AC_GROUP[group_id]->WLANID;
					bss = malloc(BSS_NUM*sizeof(struct asd_data *));
					bss_num = ASD_SEARCH_WLAN_STA(WLANID, bss);
					for(j = 0; j < bss_num; j++){
						ac_group_add_del_bss_info(tsock,group_id,bss[j],G_ADD);
						sta = bss[j]->sta_list;
						for(i = 0; i < bss[j]->num_sta; i++){
							ac_group_add_del_sta_info(tsock,group_id,sta,bss[j],G_ADD);
							sta = sta->next;
						}
					}
					free(bss);
					ac_group_syn_end(tsock, group_id);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_DATA_SYNCH end\n");
					break;
				}
				case G_BSS_UPDATE:{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_BSS_UPDATE\n");
					ac_group_bss_op(msg);
					break;
				}
				case G_STA_UPDATE:{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_STA_UPDATE\n");	
					ac_group_sta_op(msg);
					break;
				}
				case G_GROUP_DEL:{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_GROUP_DEL\n");	
					ac_group_del(msg);
				}
				default:
					break;
			}
		}
		close(tsock);
	}
	close(gsock);
	pthread_exit((void *) 0);
}
#endif
void asd_synch_recv_select(int sock, void *circle_ctx, void *sock_ctx)
{
	int numbytes;
	char buf[2048];
	memset(buf, 0, 2048);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread 5\n");
	numbytes = recv(sock, buf, 2047, 0);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread numbytes %d\n",numbytes);

	if(numbytes <= 0){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread recv3\n");
		circle_unregister_read_sock(sock);
		FD_CHANGE = 1;
		close(sock);
		
		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
		{	printf("tcp socket create failed\n");
			return;
		}
		((gThreadArg*)circle_ctx)->sock = sock;
		//fcntl(sock, F_SETFL, O_NONBLOCK);
		unsigned long ul = 1;
		ioctl(sock, FIONBIO, &ul);
		circle_register_timeout(0, 0,asd_synch_select, circle_ctx, NULL);
//		close(sock);
		return;
	}
	printf("1\n");
	G_Msg *msg = (G_Msg*) buf;
	switch(msg->Type){
		case G_BSS_UPDATE:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread G_BSS_UPDATE\n");
			ac_group_bss_op(msg);
			break;
		}
		case G_STA_UPDATE:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread G_STA_UPDATE\n");
			ac_group_sta_op(msg);
			break;
		}
		case G_SYN_END:{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread G_SYN_END\n");
			free(circle_ctx);
			circle_ctx = NULL;
			circle_unregister_read_sock(sock);
			FD_CHANGE = 1;
			return;
		}
		default:
			break;
	}
}

void asd_connect_select(void *circle_ctx, void *timeout_ctx){
	unsigned char group_id = ((gThreadArg*)circle_ctx)->group_id;
	unsigned char ACID = ((gThreadArg*)circle_ctx)->ACID;
	int sock = ((gThreadArg*)circle_ctx)->sock;
	int ret;
	if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
		fd_set fdset;
		struct timeval tv;			
		FD_ZERO(&fdset);
		FD_SET(sock,&fdset);
		
		tv.tv_sec = 0;
		tv.tv_usec = 500;
		
		ret =  select(sock + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_connect_select ret %d\n",ret);
		
		if (FD_ISSET(sock, &fdset)){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_connect_select we get sock %d\n",sock);
			circle_cancel_timeout(asd_connect_select,circle_ctx,NULL);
			unsigned long ul = 0;
			ioctl(sock, FIONBIO, &ul);
			if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
				AC_GROUP[group_id]->Mobility_AC[ACID]->is_conn = 1;
				AC_GROUP[group_id]->Mobility_AC[ACID]->sock = sock;				
			}
//				break;
		}else{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_connect_select circle_register_timeout\n");
			circle_cancel_timeout(asd_connect_select,circle_ctx,NULL);
			circle_register_timeout(2,0,asd_connect_select, circle_ctx, NULL);
			return;
		}
	}else{
		circle_cancel_timeout(asd_connect_select,circle_ctx,NULL);
		close(sock);
		free(circle_ctx);
	}
	
}

void asd_synch_select2(void *circle_ctx, void *timeout_ctx){
	unsigned char group_id = ((gThreadArg*)circle_ctx)->group_id;
	unsigned char ACID = ((gThreadArg*)circle_ctx)->ACID;
	int sock = ((gThreadArg*)circle_ctx)->sock;
	int ret;
	if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
		fd_set fdset;
		struct timeval tv;			
		FD_ZERO(&fdset);
		FD_SET(sock,&fdset);
		
		tv.tv_sec = 0;
		tv.tv_usec = 500;
		
		ret =  select(sock + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_select2 ret %d\n",ret);
		
		if (FD_ISSET(sock, &fdset)){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_select2 we get sock %d\n",sock);
			unsigned long ul = 0;
			ioctl(sock, FIONBIO, &ul);
			if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
				AC_GROUP[group_id]->Mobility_AC[ACID]->is_conn = 1;
				AC_GROUP[group_id]->Mobility_AC[ACID]->sock = sock;				
				if (circle_register_read_sock(sock, asd_synch_recv_select, circle_ctx, NULL))
				{
					asd_printf(ASD_DEFAULT,MSG_WARNING,"%s Could not register read socket\n",__func__);
					return;
				}
				ac_group_syn_request(sock,group_id);
			}
//				break;
		}else{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_select2 circle_register_timeout\n");
			circle_cancel_timeout(asd_synch_select2,circle_ctx,NULL);
			circle_register_timeout(2,0,asd_synch_select2, circle_ctx, NULL);
			return;
		}
	}else{
		circle_cancel_timeout(asd_synch_select2,circle_ctx,NULL);
		close(sock);
		free(circle_ctx);
	}
	
}
void asd_synch_select(void *circle_ctx, void *timeout_ctx){
	unsigned char group_id = ((gThreadArg*)circle_ctx)->group_id;
	unsigned char ACID = ((gThreadArg*)circle_ctx)->ACID;
	int sock= ((gThreadArg*)circle_ctx)->sock;
	/*int numbytes;
	char buf[2048];
	int ret;
	int n = 10;
	struct timeval timeo;	
	socklen_t len = sizeof(timeo);*/
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread 1\n");
	if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread 2\n");

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread sock %d\n",sock);
		if(((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL))\
			&&(connect(sock, (struct sockaddr *)&(AC_GROUP[group_id]->Mobility_AC[ACID]->ac_addr), sizeof(struct sockaddr)) == -1)){
			perror("connect");
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread connect failed\n");
			#if 0
			if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
				fd_set fdset;
				struct timeval tv;			
				FD_ZERO(&fdset);
				FD_SET(sock,&fdset);
				
				tv.tv_sec = 5;
				tv.tv_usec = 0;
				
				ret =  select(sock + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv);
				asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread ret %d\n",ret);
				/*n--;
				asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread n %d\n",n);
				if(n <= 0){
					close(sock);
					circle_cancel_timeout(asd_synch_select,circle_ctx,NULL);
					asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread nnnn %d\n",n);
					circle_register_timeout(5, 0,asd_synch_select, circle_ctx, NULL);
					return;
				}*/
				
				if (FD_ISSET(sock, &fdset)){
					asd_printf(ASD_DEFAULT,MSG_INFO,"asd_synch_thread we get sock %d\n",sock);
					unsigned long ul = 0;
					ioctl(sock, FIONBIO, &ul);
					if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){
						AC_GROUP[group_id]->Mobility_AC[ACID]->is_conn = 1;
						AC_GROUP[group_id]->Mobility_AC[ACID]->sock = sock;
					}
	//				break;
				}else{
					circle_cancel_timeout(asd_synch_select,circle_ctx,NULL);
					circle_register_read_sock(sock,asd_synch_select2, circle_ctx, NULL);
					return;
				}
			}
			#endif
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread circle_register_timeout\n");
			circle_cancel_timeout(asd_synch_select,circle_ctx,NULL);
			circle_register_timeout(2,0,asd_synch_select2, circle_ctx, NULL);
			return;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread sock2 %d\n",sock);
		circle_cancel_timeout(asd_synch_select,circle_ctx,NULL);
		if((AC_GROUP[group_id]!=NULL)&&(AC_GROUP[group_id]->Mobility_AC[ACID]!=NULL)){			
			unsigned long ul = 0;
			ioctl(sock, FIONBIO, &ul);
			AC_GROUP[group_id]->Mobility_AC[ACID]->is_conn = 1;
			AC_GROUP[group_id]->Mobility_AC[ACID]->sock = sock;
			if (circle_register_read_sock(sock, asd_synch_recv_select, circle_ctx, NULL))
			{
				asd_printf(ASD_DEFAULT,MSG_WARNING,"%s Could not register read socket\n",__func__);
				return;
			}
			ac_group_syn_request(sock,group_id);
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_synch_thread 4\n");
	}	
	else{
		circle_cancel_timeout(asd_synch_select,circle_ctx,NULL);
		close(sock);
		free(circle_ctx);
	}
}

void asd_ac_group_recv_select(int sock, void *circle_ctx, void *sock_ctx)
{		
	int numbytes;
	char buf[2048];
	unsigned char group_id;
	unsigned char WLANID;
	struct asd_data **bss;
	unsigned int bss_num;
	unsigned int i = 0;
	unsigned int j = 0;
	struct sta_info *sta;

	memset(buf, 0, 2048);
	numbytes = recv(sock, buf, 2047, 0);
	if(numbytes <= 0){
		printf("group recv error\n");		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_recv_select error\n");
		circle_unregister_read_sock(sock);
		FD_CHANGE = 1;
		close(sock);
		return;
	}
	printf("1\n");
	if(AC_G_FIRST == 0){
		G_Msg *msg = (G_Msg*) buf;
		switch(msg->Type){
			case G_DATA_SYNCH:{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_DATA_SYNCH\n");
				group_id = msg->group_id;
				if(AC_GROUP[group_id] == NULL){
					break;
				}
				WLANID = AC_GROUP[group_id]->WLANID;
				bss = malloc(BSS_NUM*sizeof(struct asd_data *));
				bss_num = ASD_SEARCH_WLAN_STA(WLANID, bss);
				for(j = 0; j < bss_num; j++){
					ac_group_add_del_bss_info(sock,group_id,bss[j],G_ADD);
					sta = bss[j]->sta_list;
					for(i = 0; i < bss[j]->num_sta; i++){
						if(ac_group_add_del_sta_info(sock,group_id,sta,bss[j],G_ADD) == 0){
							circle_unregister_read_sock(sock);
							FD_CHANGE = 1;
							close(sock);
							free(bss);//qiuchen
							return;	
						}
						sta = sta->next;
					}
				}
				free(bss);
				ac_group_syn_end(sock, group_id);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_DATA_SYNCH end\n");
				break;
			}
			case G_BSS_UPDATE:{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_BSS_UPDATE\n");
				ac_group_bss_op(msg);
				break;
			}
			case G_STA_UPDATE:{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_STA_UPDATE\n"); 
				ac_group_sta_op(msg);
				break;
			}
			case G_GROUP_DEL:{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_thread G_GROUP_DEL\n");	
				ac_group_del(msg);
			}
			default:
				break;
		}
	}
}

void asd_ac_group_accept_select(int sock, void *circle_ctx, void *sock_ctx)
{
	int tsock;
	//int numbytes;
	//char buf[2048];
	socklen_t addr_len;
	struct sockaddr_in ac_addr; 
	/*unsigned char group_id;
	unsigned char WLANID;
	struct asd_data **bss;
	unsigned int bss_num;
	unsigned int i = 0;
	unsigned int j = 0;
	struct sta_info *sta;*/
	
	if(AC_G_FIRST == 0){		
		addr_len = sizeof(struct sockaddr);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_accept_select sock %d\n",sock);
		tsock = accept(gsock, (struct sockaddr *)&ac_addr, &addr_len);
		printf("new_sock %d\n",tsock);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_ac_group_accept_select tsock %d\n",tsock);
		if(tsock == -1){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"accept failed\n");
			return;
		}
		
		if (circle_register_read_sock(tsock, asd_ac_group_recv_select, NULL, NULL))
		{
			asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
			return;
		}			
	}
}



int init_ac_group_info(unsigned char group_id, char *name, char *ESSID){
	unsigned int i = 0;
	for(i=0; i<WLAN_NUM; i++){
		if(ASD_WLAN[i] != NULL){
			if(strcmp(ASD_WLAN[i]->ESSID,ESSID) == 0){
				break;
			}
		}
	}
	if(i == WLAN_NUM){
		return ASD_AC_GROUP_ESSID_NOT_EXIST;
	}
	if(AC_GROUP[group_id] == NULL){
		ASD_WLAN[i]->AC_Roaming_Policy = 1;
		ASD_WLAN[i]->group_id = group_id;
		AC_GROUP[group_id] = (Inter_AC_R_Group*)malloc(sizeof(Inter_AC_R_Group));
		memset(AC_GROUP[group_id], 0, sizeof(Inter_AC_R_Group));
		AC_GROUP[group_id]->GroupID = group_id;
		AC_GROUP[group_id]->WLANID = i;
		AC_GROUP[group_id]->name = (unsigned char*)malloc(strlen(name)+1);
		memset(AC_GROUP[group_id]->name,0,strlen(name)+1);
		memcpy(AC_GROUP[group_id]->name,name,strlen(name));
		AC_GROUP[group_id]->ESSID = (unsigned char*)malloc(strlen(ESSID)+1);
		memset(AC_GROUP[group_id]->ESSID,0,strlen(ESSID)+1);
		memcpy(AC_GROUP[group_id]->ESSID,ESSID,strlen(ESSID));
		
	}
	return 0;
}


int del_ac_group_member(unsigned char group_id,unsigned char ac_id, int flag){
	Mobility_AC_Info *tmp;
	int i = 0;
	if(AC_GROUP[group_id] == NULL)
		return ASD_AC_GROUP_ID_NOT_EXIST;
	if(AC_GROUP[group_id]->Mobility_AC[ac_id] == NULL)
		return ASD_AC_GROUP_MEMBER_NOT_EXIST;
	tmp = AC_GROUP[group_id]->Mobility_AC[ac_id];
	AC_GROUP[group_id]->Mobility_AC[ac_id] = NULL;
	G_roaming_del_all_sta(tmp);
	for(i = 0; i < BSS_NUM; i++){
		if(tmp->BSS[i] != NULL){
			free(tmp->BSS[i]);
			tmp->BSS[i] = NULL;
		}
	}
	free(tmp->BSS);
	tmp->BSS = NULL;
	free(tmp);
	tmp = NULL;
	return 0;
}

int del_ac_group_info(unsigned char group_id){
	unsigned char i = 0;
	unsigned char ID = 0;
	if(AC_GROUP[group_id] == NULL)
		return ASD_AC_GROUP_ID_NOT_EXIST;
	for(i = 0; i < G_AC_NUM; i++){
		del_ac_group_member(group_id, i,1);
	}
	ID = AC_GROUP[group_id]->WLANID;
	if(ASD_WLAN[ID] != NULL){		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"del_ac_group_info ASD_WLAN[ID]->AC_Roaming_Policy = 0\n");
		ASD_WLAN[ID]->AC_Roaming_Policy = 0;
	}
	free(AC_GROUP[group_id]->ESSID);
	AC_GROUP[group_id]->ESSID = NULL;
	free(AC_GROUP[group_id]);
	AC_GROUP[group_id] = NULL;
	return 0;
}


