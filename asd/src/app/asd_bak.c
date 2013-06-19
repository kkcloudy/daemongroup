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
* Asd_bak.c
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
#include <netdb.h>   
#include <sys/ioctl.h>   
#include <net/if.h>
#include "circle.h"
#include "hmd/hmdpub.h"
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

#include "util/npd_if.h"
#include "ASDNetlinkArpOp.h"


#include "asd_bak.h"
#include "ASDWPAOp.h"			//mahz add 2010.12.29
#include "ASDDbus_handler.h"
#include "roaming_sta.h"
#include "ASDEAPAuth.h"


#define INFOSIZE  1024
int new_sock = -1;
int asd_sock = -1;
int asd_master_sock = -1;
int bak_error = 0;
int mast_bak_update = 0;
extern unsigned char FD_CHANGE;
#if 0
int send_info(const char * buf){
	int i=0;
	int ret;	
	int sockfd;
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) ==-1){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s sendto %s\n",__func__,strerror(errno));
		perror("socket create");
		exit(1);
	}
	if(connect(sockfd, (struct sockaddr *)&B_addr, sizeof(struct sockaddr)) == -1){
		close(sockfd);
		return ASD_UPDATE_ERROR;
	}
	if(buf==NULL){
		close(sockfd);
		return 0;
	}
	B_Msg msg;
	unsigned int len;
	
	msg.Op=B_MODIFY;
	msg.Type=B_INFO_TYPE;

	if(buf!=NULL){
		memset(msg.Bu.INFO.info,0,1024);
		memcpy(msg.Bu.INFO.info,buf,strlen(buf));
//		asd_printf(ASD_DEFAULT,MSG_DEBUG,"buf len %d\n",strlen(buf));
	}

	
	len=sizeof(msg);

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"len %d\nbuf:%s\n",len,msg.Bu.INFO.info);
	
	while((ret = send(sockfd, &msg, len, 0) < 0)){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s send %s\n",__func__,strerror(errno));
		perror("sendto");
		i++;
		if(i>5){
			close(sockfd);
			return ASD_UPDATE_ERROR;
		}
		continue;
		//exit(1);
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"ret = %d\n",ret);	
	close(sockfd);
	return 0;

}
#endif
int init_asd_bak_socket(){	
	int sock;	
	struct sockaddr_in my_addr;	
	int yes = 1;
	//socklen_t addr_len;	int numbytes;	
	//char buf[2048];	
	int sndbuf = 65535;
	int rcvbuf = 65535;
	if(local){
		struct sockaddr_tipc server_addr;
		server_addr.family = AF_TIPC;
		server_addr.addrtype = TIPC_ADDR_NAMESEQ;
		server_addr.addr.nameseq.type = SYN_ASD_SERVER_TYPE;
		server_addr.addr.nameseq.lower = SYN_SERVER_BASE_INST + vrrid*MAX_SLOT_NUM + slotid;;
		server_addr.addr.nameseq.upper = SYN_SERVER_BASE_INST + vrrid*MAX_SLOT_NUM + slotid;;
		server_addr.scope = TIPC_ZONE_SCOPE;
		sock = socket(AF_TIPC, SOCK_RDM, 0);
		if(sock < 0){
			asd_printf(ASD_DEFAULT,MSG_ERROR,"%s socket create failed\n",__func__);
			return -1;
		}
		if (0 != bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)))
		{
			printf("Server: failed to bind port name\n");
		}
	}else{
		#ifndef _AC_BAK_UDP_
		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
		{	asd_printf(ASD_DEFAULT,MSG_ERROR,"%s udp socket create failed\n",__func__);
			return -1;
		}	
		#else
		if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1)
		{	asd_printf(ASD_DEFAULT,MSG_ERROR,"udp socket create failed\n");
			return -1;
		}	
		#endif	
	
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		my_addr.sin_family = AF_INET;	
		my_addr.sin_port = htons(ASD_BAK_AC_PORT+local*MAX_INSTANCE+vrrid);	
		my_addr.sin_addr.s_addr = INADDR_ANY;	
		memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));	
		if (bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
		{	asd_printf(ASD_DEFAULT,MSG_ERROR,"udp bind failed\n");		
			//exit(1);	
		}
#ifndef _AC_BAK_UDP_
		listen(sock, 5);
#endif
	}
	setsockopt(sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
	setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
	return sock;
}


int bak_update_sta_ip_info(struct asd_data *wasd, struct sta_info *sta){
	B_Msg msg;
	int len;
	int ret;
	unsigned char bak_exist = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
//	int sockfd;
	msg.Op = B_MODIFY;
	msg.Type = B_STA_TYPE;
	memcpy(msg.Bu.U_STA.STAMAC, sta->addr, 6);
	msg.Bu.U_STA.WTPID = wasd->BSSIndex/L_BSS_NUM/L_RADIO_NUM;
	msg.Bu.U_STA.BSSIndex = wasd->BSSIndex;
	msg.Bu.U_STA.ipaddr = sta->ipaddr;
	msg.Bu.U_STA.gifindex = sta->gifidx;
	len = sizeof(msg);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"len %d\n",len);
#ifndef _AC_BAK_UDP_
	if((send(asd_master_sock, &msg, len, 0) < 0)){
		asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
		perror("sendto");
		//exit(1);
	}
	
#else
	if(inaddr->sin_addr.s_addr != 0){
		bak_exist = 1;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d\n",bak_exist,bak_unreach);
	if((bak_exist == 1)&&(bak_unreach == 0)){
		ret = sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
		if(ret < 0){
			bak_error = errno;
			asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
			perror("sendto");
		/*	if(errno == EBADF){
				circle_unregister_read_sock(asd_master_sock);
				close(asd_master_sock);			
				if ((asd_master_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1){
					asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
					perror("socket create");
				}			
				int sndbuf = 65535;
				int rcvbuf = 65535;
				setsockopt(asd_master_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
				setsockopt(asd_master_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
				sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
					
				if (circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL))
				{
					asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
					return -1;
				}					
			}
		*/	
			if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
				bak_unreach = 1;
			}					
		}
	}
#endif
//	close(sockfd);
	return 0;
}

int bak_batch_add_sta(struct asd_data **bss, unsigned int num){
	char buf[65535] = {0};
	int i = 0;
	int j = 0;
	int k = 0; 
	int len;
	int ret;
	time_t now = 0;//qiuchen add it
	get_sysruntime(&now);
	unsigned char bak_exist = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
	struct sta_info *sta;
	_B_Msg *msg = (_B_Msg *)buf;
	msg->Op = B_ADD;
	msg->Type = B_BATCH_STA_TYPE;
	for(i = 0; i < num; i++){			
		//unsigned char sid = bss[i]->SecurityID;
		//if((ASD_SECURITY[sid]== NULL)||((ASD_SECURITY[sid]!= NULL)&&(ASD_SECURITY[sid]->securityType != OPEN)))
		//	continue;
		sta = bss[i]->sta_list;
		for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
			{
			//	int sockfd;
				memcpy(msg->U_STA[k].STAMAC, sta->addr, 6);
				msg->U_STA[k].WTPID = bss[i]->BSSIndex/L_BSS_NUM/L_RADIO_NUM;
				msg->U_STA[k].BSSIndex = bss[i]->BSSIndex;
				msg->U_STA[k].sta_add = *(sta->add_time);	
				msg->U_STA[k].sta_online_time = now - sta->add_time_sysruntime + sta->sta_online_time;
				msg->U_STA[k].ipaddr = sta->ipaddr;
				msg->U_STA[k].gifindex = sta->gifidx;
				msg->U_STA[k].total_num = bss[i]->num_sta;
				msg->U_STA[k].PreBSSIndex= sta->PreBssIndex;
				memcpy(msg->U_STA[k].PreBSSID,sta->PreBSSID,MAC_LEN);
				msg->U_STA[k].rflag = sta->rflag;
				unsigned char SID = 0;
				if(ASD_WLAN[bss[i]->WlanID])
					SID = (unsigned char)ASD_WLAN[bss[i]->WlanID]->SecurityID;
				if(sta != NULL){
					if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
						if(sta->eapol_sm != NULL){
							msg->U_STA[k].B_identity_len = sta->eapol_sm->identity_len;
							memset(msg->U_STA[k].B_identity, 0, 128);
							if(msg->U_STA[k].B_identity_len < 128){
								memcpy(msg->U_STA[k].B_identity, sta->eapol_sm->identity, msg->U_STA[k].B_identity_len);
							}else{
								memcpy(msg->U_STA[k].B_identity, sta->eapol_sm->identity, 127);
								msg->U_STA[k].B_identity_len = 127;
							}
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_batch_add_sta sta->eapol_sm->identity_len %d\n",sta->eapol_sm->identity_len);
						}
						msg->U_STA[k].B_acct_session_id_hi = sta->acct_session_id_hi;
						msg->U_STA[k].B_acct_session_id_lo = sta->acct_session_id_lo;
						msg->U_STA[k].B_acct_session_start = sta->acct_session_start;
						msg->U_STA[k].B_acct_session_started = sta->acct_session_started;
						msg->U_STA[k].B_acct_terminate_cause = sta->acct_terminate_cause;
						msg->U_STA[k].B_acct_interim_interval = sta->acct_interim_interval;
						msg->U_STA[k].B_acct_input_gigawords = sta->acct_input_gigawords; 
						msg->U_STA[k].B_acct_output_gigawords = sta->acct_output_gigawords; 
					}		
				}
				k++;
				if(k == 50){
					msg->Op = B_ADD;
					msg->Type = B_BATCH_STA_TYPE;
					msg->num = k;
					len = sizeof(_B_Msg);
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_batch_add_sta len %d\n",len);
					if(inaddr->sin_addr.s_addr != 0){
						bak_exist = 1;
					}
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d\n",bak_exist,bak_unreach);
					if((bak_exist == 1)&&(bak_unreach == 0)){
						ret = sendto(asd_master_sock, buf, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
						if(ret < 0){
							bak_error = errno;
							asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
							asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
							perror("sendto");	
						/*	if(errno == EBADF){
								circle_unregister_read_sock(asd_master_sock);
								close(asd_master_sock);			
								if ((asd_master_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1){
									asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
									perror("socket create");
								}							
								int sndbuf = 65535;
								int rcvbuf = 65535;
								setsockopt(asd_master_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
								setsockopt(asd_master_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
								sendto(asd_master_sock, buf, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
								if (circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL))
								{
									asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
									return -1;
								}	
							}
						*/
							if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
								bak_unreach = 1;
								break;
							}
							}					
					}
					k = 0;
					memset(buf, 0, 65535);
				}			
			}
			sta = sta->next;
		}		
		if(bak_unreach == 1){
			break;
		}	
	}	
	msg->Op = B_ADD;
	msg->Type = B_BATCH_STA_TYPE;
	len = sizeof(_B_Msg);	
	msg->num = k;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_batch_add_sta k %d len %d\n",k,len);
	if(inaddr->sin_addr.s_addr != 0){
		bak_exist = 1;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d\n",bak_exist,bak_unreach);
	if((bak_exist == 1)&&(bak_unreach == 0)){
		ret = sendto(asd_master_sock, buf, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
		if(ret < 0){
			bak_error = errno;
			asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
			perror("sendto");	
			/*if(errno == EBADF){
				circle_unregister_read_sock(asd_master_sock);
				close(asd_master_sock); 		
				if ((asd_master_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1){
					asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
					perror("socket create");
				}
				
				int sndbuf = 65535;
				int rcvbuf = 65535;
				setsockopt(asd_master_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
				setsockopt(asd_master_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
				sendto(asd_master_sock, buf, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));		
				if (circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL))
				{
					asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
					return -1;
				}	
			}
			*/
			if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
				bak_unreach = 1;
			}					
		}
	}
	return 0;
}
//weichao add
void bak_batch_add_roam_sta(){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func: %s\n",__func__);
	char buf[65535] = {0};
	int i = 0;
	int j = 0;
	int k = 0; 
	int len;
	int ret;
	int num = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
	unsigned char bak_exist = 0;
	struct ROAMING_STAINFO *r_sta = NULL;
	B_ROAM_Msg *msg =(B_ROAM_Msg*) buf;
	msg->Op = B_ADD;
	msg->Type = B_BATCH_ROAM_STA;
	for(i = 1 ; i < WLAN_NUM; i++){
		if(ASD_WLAN[i] == NULL)
			continue;
		
		num = ASD_WLAN[i]->r_num_sta;
		r_sta = ASD_WLAN[i]->r_sta_list;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak roam sta num is %d\n",num);
		for(j = 0 ; (j < num)&&(r_sta != NULL);j++ ){
			memcpy(msg->U_STA[k].addr,r_sta->addr,MAC_LEN);
			memcpy(msg->U_STA[k].BSSID,r_sta->BSSID,MAC_LEN);
			memcpy(msg->U_STA[k].PreBSSID,r_sta->PreBSSID,MAC_LEN);
			msg->U_STA[k].BssIndex = r_sta->BssIndex;
			msg->U_STA[k].PreBssIndex = r_sta->PreBssIndex;

			k++;
			if(k == 50){
				msg->Op = B_ADD;
				msg->Type = B_BATCH_ROAM_STA;
				msg->num = k;
				len = sizeof(B_ROAM_Msg);
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_batch_add_roam_sta len %d\n",len);
				if(inaddr->sin_addr.s_addr != 0){
					bak_exist = 1;
				}
				if((bak_exist == 1)&&(bak_unreach == 0)){
					ret = sendto(asd_master_sock, buf, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
					if(ret < 0){
						bak_error = errno;
						asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
						asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
						perror("sendto");	
						if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
							bak_unreach = 1;
							break;
						}
					}					
				}
				k = 0;
				memset(buf, 0, 65535);
			}			
			r_sta = r_sta->next;
		}
		if(bak_unreach == 1){
			break;
		}	
	}
		msg->Op = B_ADD;
		msg->Type = B_BATCH_ROAM_STA;
		len = sizeof(_B_Msg);	
		msg->num = k;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_batch_add_sta k %d len %d\n",k,len);
		if(inaddr->sin_addr.s_addr != 0){
			bak_exist = 1;
		}
		if((bak_exist == 1)&&(bak_unreach == 0)){
			ret = sendto(asd_master_sock, buf, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
			if(ret < 0){
				bak_error = errno;
				asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
				asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
				perror("sendto");	
				if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
					bak_unreach = 1;
				}					
			}
		}
	}

int bak_add_sta(struct asd_data *wasd, struct sta_info *sta){
	B_Msg msg;
	int len;
	int ret;
	unsigned char bak_exist = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
	time_t now = 0;
	get_sysruntime(&now);//qiuchen add it
//	int sockfd;
	memset(&msg,0,sizeof(msg));
	msg.Op = B_ADD;
	msg.Type = B_STA_TYPE;
	msg.Bu.U_STA.WTPID = wasd->BSSIndex/L_BSS_NUM/L_RADIO_NUM;
	msg.Bu.U_STA.BSSIndex = wasd->BSSIndex;
	msg.Bu.U_STA.total_num = wasd->num_sta;
	unsigned char SID = 0;
	if(ASD_WLAN[wasd->WlanID])
		SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
	if(sta != NULL){//qiuchen
		memcpy(msg.Bu.U_STA.STAMAC, sta->addr, 6);
		msg.Bu.U_STA.sta_add = *(sta->add_time);
		msg.Bu.U_STA.sta_online_time = now - sta->add_time_sysruntime+sta->sta_online_time;//qiuchen add it
		msg.Bu.U_STA.ipaddr = sta->ipaddr;
		msg.Bu.U_STA.gifindex = sta->gifidx;
		if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
			if(sta->eapol_sm == NULL)
				return -1;
			msg.Bu.U_STA.B_identity_len = sta->eapol_sm->identity_len;
			memset(msg.Bu.U_STA.B_identity, 0, 128);
			if(msg.Bu.U_STA.B_identity_len < 128)
				memcpy(msg.Bu.U_STA.B_identity, sta->eapol_sm->identity, msg.Bu.U_STA.B_identity_len);
			else{
				memcpy(msg.Bu.U_STA.B_identity, sta->eapol_sm->identity, 127);
				msg.Bu.U_STA.B_identity_len = 127;
			}
			msg.Bu.U_STA.B_acct_session_id_hi = sta->acct_session_id_hi;
			msg.Bu.U_STA.B_acct_session_id_lo = sta->acct_session_id_lo;
			msg.Bu.U_STA.B_acct_session_start = sta->acct_session_start;
		//	msg.Bu.U_STA.B_acct_session_started = sta->acct_session_started;
			msg.Bu.U_STA.B_acct_terminate_cause = sta->acct_terminate_cause;
			msg.Bu.U_STA.B_acct_interim_interval = sta->acct_interim_interval;
			msg.Bu.U_STA.B_acct_input_gigawords = sta->acct_input_gigawords; 
			msg.Bu.U_STA.B_acct_output_gigawords = sta->acct_output_gigawords; 
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->eapol_sm->identity_len %d\n",sta->eapol_sm->identity_len);
		}else if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P))){
		}		
	}	
	len = sizeof(msg);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"len %d\n",len);
/*	if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1){
		perror("socket create");
		exit(1);
	}
	if(connect(sockfd, (struct sockaddr *)&B_addr, sizeof(struct sockaddr)) == -1){
		close(sockfd);
		return;
	}*/
	if(asd_master_sock == -1)
		return -1;
#ifndef _AC_BAK_UDP_
	if((send(asd_master_sock, &msg, len, 0) <= 0)){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s send %s\n",__func__,strerror(errno));
		perror("sendto");	
		if(is_secondary == 0){
			close(asd_master_sock);
			//asd_master_sock = -1;
			circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
			circle_register_timeout(2, 0,asd_update_select_mode, NULL, NULL);
		}
		//exit(1);
	}
#else
	if(inaddr->sin_addr.s_addr != 0){
		bak_exist = 1;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d,asd_master_sock = %d\n",bak_exist,bak_unreach,asd_master_sock);
	if((bak_exist == 1)&&(bak_unreach == 0)){
		ret = sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
		if(ret < 0){
			bak_error = errno;
			asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
			perror("sendto");		
			/*if(errno == EBADF){
				circle_unregister_read_sock(asd_master_sock);
				close(asd_master_sock);			
				if ((asd_master_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1){
					asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
					perror("socket create");
				}			
				int sndbuf = 65535;
				int rcvbuf = 65535;
				setsockopt(asd_master_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
				setsockopt(asd_master_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
				sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
				if (circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL))
				{
					asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
					return -1;
				}
			}*/
			if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
				bak_unreach = 1;
			}
		}
	}
#endif
	return 0;
//	close(sockfd);
}

int update_end(){
	B_Msg msg;
	int len;
	int ret;
	unsigned char bak_exist = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
//	int sockfd;
	msg.Op = B_ADD;
	msg.Type = B_END;
	len = sizeof(msg);
//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"len %d\n",len);
/*	if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1){
		perror("socket create");
		exit(1);
	}
	if(connect(sockfd, (struct sockaddr *)&B_addr, sizeof(struct sockaddr)) == -1){
		close(sockfd);
		return;
	}*/
#ifndef _AC_BAK_UDP_
	if((send(asd_master_sock, &msg, len, 0) < 0)){
		asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
		perror("sendto");
		//exit(1);
	}
#else
		if(inaddr->sin_addr.s_addr != 0){
			bak_exist = 1;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d\n",bak_exist,bak_unreach);
		if((bak_exist == 1)&&(bak_unreach == 0)){
			ret = sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
			if(ret < 0){
				bak_error = errno;
				asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
				asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
				perror("sendto");	
				/*if(errno == EBADF){
					circle_unregister_read_sock(asd_master_sock);
					close(asd_master_sock); 		
					if ((asd_master_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1){
						asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
						perror("socket create");
					}
					
					int sndbuf = 65535;
					int rcvbuf = 65535;
					setsockopt(asd_master_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
					setsockopt(asd_master_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
					sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
					if (circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL))
					{
						asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
						return -1;
					}	
				}*/	
				if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
					bak_unreach = 1;
				}
				}					
			}
#endif
	return 0;
//	close(sockfd);
}

int bak_del_sta(struct asd_data *wasd, struct sta_info *sta){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_del_sta \n");
	B_Msg msg;
	int len;
	int ret;
	unsigned char bak_exist = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
	time_t now = 0;
	get_sysruntime(&now);//qiuchen add it
//	int sockfd;
	msg.Op = B_DEL;
	msg.Type = B_STA_TYPE;
	memcpy(msg.Bu.U_STA.STAMAC, sta->addr, 6);
	msg.Bu.U_STA.WTPID = wasd->BSSIndex/L_BSS_NUM/L_RADIO_NUM;
	msg.Bu.U_STA.BSSIndex = wasd->BSSIndex;
	msg.Bu.U_STA.sta_add = *(sta->add_time);
	msg.Bu.U_STA.sta_online_time = now - sta->add_time_sysruntime+sta->sta_online_time;//qiuchen add it
	msg.Bu.U_STA.total_num = wasd->num_sta;
	len = sizeof(msg);
/*	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) ==-1){
		perror("socket create");
		exit(1);
	}
	if(connect(sockfd, (struct sockaddr *)&B_addr, sizeof(struct sockaddr)) == -1){
		close(sockfd);
		return;
	}*/
	if(asd_master_sock == -1)
		return -1;
#ifndef _AC_BAK_UDP_
	if((send(asd_master_sock, &msg, len, 0) < 0)){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s send %s\n",__func__,strerror(errno));
		perror("sendto");		
		if(is_secondary == 0){
			close(asd_master_sock);	
			//asd_master_sock = -1;
			circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
			circle_register_timeout(2, 0,asd_update_select_mode, NULL, NULL);
		}
		//exit(1);
	}
//	close(sockfd);

#else
		if(inaddr->sin_addr.s_addr != 0){
			bak_exist = 1;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d\n",bak_exist,bak_unreach);
		if((bak_exist == 1)&&(bak_unreach == 0)){
			ret = sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
			if(ret < 0){
				bak_error = errno;
				asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
				asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
				perror("sendto");
				/*if(errno == EBADF){
					circle_unregister_read_sock(asd_master_sock);
					close(asd_master_sock);				
					int sndbuf = 65535;
					int rcvbuf = 65535;
					setsockopt(asd_master_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
					setsockopt(asd_master_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
					if ((asd_master_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1){
						asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
						perror("socket create");
					}
					sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
					if (circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL))
					{
						asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
						return -1;
					}	
				}*/
				if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
					bak_unreach = 1;
				}
			}
		}
#endif
	return 0;
}
void bak_new_station_wpa(struct asd_data *wasd, struct sta_info *sta){
	if(wasd->wpa_auth == NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s wasd->wpa_auth == NULL\n",__func__);
		return;
	}
	sta->wpa_sm = wpa_auth_sta_init(wasd->wpa_auth,
					sta->addr);
	if(sta->wpa_sm == NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s WPA Init failed\n",__func__);
		return;
	}	
	sta->flags |= WLAN_STA_AUTHORIZED;
	return;
}

void bak_batch_new_station(struct asd_data *wasd, struct sta_info *sta, B_UPDATE_STA	*U_STA)
{
	//struct rsn_pmksa_cache_entry *pmksa;
	int reassoc = 1;
	//int force_1x = 0;
	if (sta->eapol_sm == NULL) {
		sta->eapol_sm = eapol_auth_alloc(wasd->eapol_auth, sta->addr,
						 sta->flags & WLAN_STA_PREAUTH,
						 sta);
		if (sta->eapol_sm == NULL) {
			return;
		}
		reassoc = 0;
	}

	sta->eapol_sm->eap_if->portEnabled = TRUE;
	sta->eapol_sm->authAbort = FALSE;
	sta->eapol_sm->authFail = FALSE;
	sta->eapol_sm->authPortStatus = Authorized;
	sta->eapol_sm->authStart = TRUE;
	sta->eapol_sm->authTimeout = FALSE;
	sta->eapol_sm->authSuccess = TRUE;
	sta->eapol_sm->eapolEap = FALSE;
	sta->eapol_sm->initialize = FALSE;
	sta->eapol_sm->keyDone = TRUE;
	sta->eapol_sm->keyRun = TRUE;
	sta->eapol_sm->keyTxEnabled = TRUE;
	sta->eapol_sm->portControl = Auto;
	sta->eapol_sm->portValid = TRUE;
	sta->eapol_sm->reAuthenticate = FALSE;	
	sta->eapol_sm->eapolLogoff = FALSE;
	sta->eapol_sm->eapolStart = FALSE;
	sta->eapol_sm->portMode = Auto;
	sta->eapol_sm->reAuthEnabled = TRUE;
	sta->eapol_sm->rxKey = FALSE;
	sta->eapol_sm->operEdge = FALSE;
	sta->eapol_sm->initializing = FALSE;
	
	sta->eapol_sm->auth_pae_state = 4;
	sta->eapol_sm->be_auth_state = 2;
	sta->eapol_sm->reauth_timer_state = 0;
	sta->eapol_sm->auth_key_tx_state = 1;
	sta->eapol_sm->key_rx_state = 0;
	sta->eapol_sm->ctrl_dir_state = 0;
	sta->eapol_sm->identity_len = U_STA->B_identity_len;
	sta->eapol_sm->identity = malloc(U_STA->B_identity_len+1);
	if(sta->eapol_sm->identity != NULL){
		memset(sta->eapol_sm->identity, 0, U_STA->B_identity_len+1);
		memcpy(sta->eapol_sm->identity, U_STA->B_identity, U_STA->B_identity_len);
	}
	sta->acct_session_id_hi = U_STA->B_acct_session_id_hi;
	sta->acct_session_id_lo = U_STA->B_acct_session_id_lo;
	sta->acct_session_start = U_STA->B_acct_session_start;
	//sta->acct_session_started = U_STA->B_acct_session_started;
	//sta->acct_session_started  = 0;
	sta->acct_terminate_cause = U_STA->B_acct_terminate_cause;
	sta->acct_interim_interval = U_STA->B_acct_interim_interval;
	sta->acct_input_gigawords = U_STA->B_acct_input_gigawords; 
	sta->acct_output_gigawords = U_STA->B_acct_output_gigawords; 	
	sta->flags |= WLAN_STA_AUTHORIZED;
}
void bak_new_station_1x(struct asd_data *wasd, struct sta_info *sta, B_Msg *msg)
{
	//struct rsn_pmksa_cache_entry *pmksa;
	int reassoc = 1;
	//int force_1x = 0;
	if (sta->eapol_sm == NULL) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
				   asd_LEVEL_DEBUG, "start authentication");
		sta->eapol_sm = eapol_auth_alloc(wasd->eapol_auth, sta->addr,
						 sta->flags & WLAN_STA_PREAUTH,
						 sta);
		if (sta->eapol_sm == NULL) {
			asd_logger(wasd, sta->addr,
					   asd_MODULE_IEEE8021X,
					   asd_LEVEL_INFO,
					   "failed to allocate state machine");
			return;
		}
		reassoc = 0;
	}

	sta->eapol_sm->eap_if->portEnabled = TRUE;
	sta->eapol_sm->authAbort = FALSE;
	sta->eapol_sm->authFail = FALSE;
	sta->eapol_sm->authPortStatus = Authorized;
	sta->eapol_sm->authStart = TRUE;
	sta->eapol_sm->authTimeout = FALSE;
	sta->eapol_sm->authSuccess = TRUE;
	sta->eapol_sm->eapolEap = FALSE;
	sta->eapol_sm->initialize = FALSE;
	sta->eapol_sm->keyDone = TRUE;
	sta->eapol_sm->keyRun = TRUE;
	sta->eapol_sm->keyTxEnabled = TRUE;
	sta->eapol_sm->portControl = Auto;
	sta->eapol_sm->portValid = TRUE;
	sta->eapol_sm->reAuthenticate = FALSE;	
	sta->eapol_sm->eapolLogoff = FALSE;
	sta->eapol_sm->eapolStart = FALSE;
	sta->eapol_sm->portMode = Auto;
	sta->eapol_sm->reAuthEnabled = TRUE;
	sta->eapol_sm->rxKey = FALSE;
	sta->eapol_sm->operEdge = FALSE;
	sta->eapol_sm->initializing = FALSE;
	
	sta->eapol_sm->auth_pae_state = 4;
	sta->eapol_sm->be_auth_state = 2;
	sta->eapol_sm->reauth_timer_state = 0;
	sta->eapol_sm->auth_key_tx_state = 1;
	sta->eapol_sm->key_rx_state = 0;
	sta->eapol_sm->ctrl_dir_state = 0;
	sta->eapol_sm->identity_len = msg->Bu.U_STA.B_identity_len;
	sta->eapol_sm->identity = os_zalloc(msg->Bu.U_STA.B_identity_len+1);
	if(sta->eapol_sm->identity!=NULL){
		memset(sta->eapol_sm->identity, 0, msg->Bu.U_STA.B_identity_len+1);
		memcpy(sta->eapol_sm->identity, msg->Bu.U_STA.B_identity, msg->Bu.U_STA.B_identity_len);
	}
	
	sta->acct_session_id_hi = msg->Bu.U_STA.B_acct_session_id_hi;
	sta->acct_session_id_lo = msg->Bu.U_STA.B_acct_session_id_lo;
	sta->acct_session_start = msg->Bu.U_STA.B_acct_session_start;
//	sta->acct_session_started = msg->Bu.U_STA.B_acct_session_started;
	//sta->acct_session_started = 0;
	sta->acct_terminate_cause = msg->Bu.U_STA.B_acct_terminate_cause;
	sta->acct_interim_interval = msg->Bu.U_STA.B_acct_interim_interval;
	sta->acct_input_gigawords = msg->Bu.U_STA.B_acct_input_gigawords; 
	sta->acct_output_gigawords = msg->Bu.U_STA.B_acct_output_gigawords; 	
	sta->flags |= WLAN_STA_AUTHORIZED;
}
#if 0
void B_INFO_OP(B_Msg *msg){
	char buf[1024];
	memset(buf, 0, 1024);	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"buf:%s\n",msg->Bu.INFO.info);
	sprintf(buf,"/opt/bin/vtysh -c \"con t\n %s\"\n",msg->Bu.INFO.info);	
	system(buf);
}
#endif
//qiuchen
int update_radius_state_to_bakAC(unsigned char SID,unsigned char auth,unsigned char acct){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);
	B_Msg msg;
	unsigned char bak_exist = 0;
	int len = 0;
	int ret = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;

	msg.Type = B_BSS_RADIUS;
	msg.Op = B_UPDATE;
	msg.Bu.U_BSS_RADIUS.sid_num = 1;
	msg.Bu.U_BSS_RADIUS.sArray[0].SID = SID;
	msg.Bu.U_BSS_RADIUS.sArray[0].auth_server = auth;
	msg.Bu.U_BSS_RADIUS.sArray[0].acct_server = acct;

	len = sizeof(msg);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"len %d\n",len);
	
	if(asd_master_sock == -1)
		return -1;
#ifndef _AC_BAK_UDP_
		if((send(asd_master_sock, &msg, len, 0) <= 0)){
			asd_printf(ASD_DEFAULT,MSG_CRIT,"%s send %s\n",__func__,strerror(errno));
			perror("sendto");	
			if(is_secondary == 0){
				close(asd_master_sock);
				//asd_master_sock = -1;
				circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
				circle_register_timeout(2, 0,asd_update_select_mode, NULL, NULL);
			}
			//exit(1);
		}
#else
	if(inaddr->sin_addr.s_addr != 0){
		bak_exist = 1;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d,asd_master_sock = %d\n",bak_exist,bak_unreach,asd_master_sock);
		if((bak_exist == 1)&&(bak_unreach == 0)){
			ret = sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
			if(ret < 0){
				bak_error = errno;
				asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
				asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
				perror("sendto");		
				if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
					bak_unreach = 1;
				}
			}
		}
#endif
		return 0;

}

void B_RADIUS_UPDATE(B_Msg *msg){
	unsigned char SID = 0;
	unsigned char acct_use;
	unsigned char auth_use;
	int ret = 0;
	int j = 0;
	unsigned char sid_num;
	switch(msg->Op){
		case B_UPDATE:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_RADIUS_UPDATE B_UPDATE\n");
			sid_num = msg->Bu.U_BSS_RADIUS.sid_num;
			acct_use = msg->Bu.U_BSS_RADIUS.sArray[j].acct_server;
			auth_use = msg->Bu.U_BSS_RADIUS.sArray[j].auth_server;
			for(j=0;j<sid_num;j++){
				SID = msg->Bu.U_BSS_RADIUS.sArray[j].SID;
				if(ASD_SECURITY[SID] == NULL)
					continue;
				if(acct_use == RADIUS_HOLD_ON)
					acct_use = ASD_SECURITY[SID]->acct_server_current_use;
				if(auth_use == RADIUS_HOLD_ON)
					auth_use = ASD_SECURITY[SID]->auth_server_current_use;
				if(ASD_SECURITY[SID]->acct_server_current_use != acct_use)
					ret |= radius_change_server_acct(SID,acct_use);
				if(ASD_SECURITY[SID]->auth_server_current_use != auth_use)
					ret |= radius_change_server_auth(SID,auth_use);
				if(ret){
					asd_printf(ASD_80211,MSG_DEBUG,"<error>! Bak AC change radius server failed!SID = %d\n",SID);
				}
				else{
					ASD_SECURITY[SID]->acct_server_current_use = acct_use;
					ASD_SECURITY[SID]->auth_server_current_use = auth_use;
				}
			}
			
			break;
		default:
			break;
	}
}
//end
void B_STA_OP(B_Msg *msg){
	int i = 0;
	//int ret;	
	//int len;
	struct sta_info *sta = NULL;
	unsigned int BSSIndex;
	unsigned int Radio_ID;	
	struct asd_data *wasd = NULL;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	switch(msg->Op){
		case B_ADD:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_STA_OP M_ADD\n");
			BSSIndex = msg->Bu.U_STA.BSSIndex;
			Radio_ID = msg->Bu.U_STA.BSSIndex/L_BSS_NUM;
			i = msg->Bu.U_STA.BSSIndex%L_BSS_NUM;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSSIndex %d Radio_ID %d\n",BSSIndex,Radio_ID);			
			if((Radio_ID >= G_RADIO_NUM)||(BSSIndex >= BSS_NUM))
				return;
			if((interfaces != NULL)&&(interfaces->iface[Radio_ID] != NULL))
				wasd = interfaces->iface[Radio_ID]->bss[i];
			else
				return;
			if(wasd != NULL){
				sta = ap_sta_add(wasd, msg->Bu.U_STA.STAMAC, 1);			
				if(sta == NULL){
					asd_printf(ASD_DEFAULT,MSG_ERROR,"B_STA_OP sta add failed!!!!!!\n");
					return;
				}
				if(msg->Bu.U_STA.ipaddr != 0)
				{
					if(sta->ipaddr == msg->Bu.U_STA.ipaddr){
						break;
					}else{
						char mac[20];
						char ifname[ETH_IF_NAME_LEN];
						unsigned char *ip;						
						//struct if_nameindex *ifin = NULL;
						//int gindex = 0;
						memset(mac,0,20);
						memset(ifname,0,ETH_IF_NAME_LEN);
						sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(sta->addr));
						#if 0
						ifin = if_nameindex();
						
						if(ifin) {
							i = 0;
							while(ifin[i].if_name && ifin[i].if_index) {
								ret = if_name2gindex(ifin[i].if_name, &gindex);
								if((!ret)&&(gindex == msg->Bu.U_STA.gifindex)) {
									strcpy(ifname,ifin[i].if_name);
									break;
								}
								i++;
							}
							if_freenameindex(ifin);
						}
						#endif
						if(ASD_ARP_GROUP[msg->Bu.U_STA.gifindex] != NULL){
							strcpy(ifname,ASD_ARP_GROUP[msg->Bu.U_STA.gifindex]);
						}
						if(sta->ipaddr != 0){
							ipneigh_modify(RTM_DELNEIGH, 0,sta->in_addr, mac,sta->arpifname);
							 if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL == sta->security_type)){
								 if(asd_ipset_switch)
									 eap_connect_down(sta->ipaddr);
								 else
									 AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
							}
						}
						sta->ipaddr = msg->Bu.U_STA.ipaddr;
						sta->gifidx = msg->Bu.U_STA.gifindex;
						memset(sta->arpifname,0,16);
						memset(sta->in_addr, 0, 16);
						strcpy(sta->arpifname,ifname);
						ip = (unsigned char *)&(sta->ipaddr);
						sprintf(sta->in_addr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
						inet_aton(sta->in_addr,&(sta->ip_addr));
						if(sta->ip_addr.s_addr != 0){
							 if((sta->security_type == NO_NEED_AUTH) ||(HYBRID_AUTH_EAPOL == sta->security_type)){
								 if(asd_ipset_switch)
									 eap_connect_up(sta->ipaddr);
								 else
									 AsdStaInfoToEAG(wasd,sta,ASD_AUTH);
								}
						}
						if(asd_sta_static_arp)
							ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,sta->in_addr, mac,ifname);
					}
				}
			}
			else{
				asd_printf(ASD_DEFAULT,MSG_ERROR,"B_STA_OP sta add :wasd NULL!!!!!!\n");
				return;
			}
		/*	#ifdef _AC_BAK_UDP_
			if(wasd->num_sta < msg->Bu.U_STA.total_num){
				asd_printf(ASD_DEFAULT,MSG_INFO,"B_STA_OP sta add :need reupdate\n");
				B_UPDATE_REQUEST(asd_sock, BSSIndex);
			}
			#endif				
		*/
			*(sta->add_time) = msg->Bu.U_STA.sta_add;
			//qiuchen add it 2012.10.31
			if(mast_bak_update == 1){
				sta->sta_online_time = msg->Bu.U_STA.sta_online_time;
			}
			get_sysruntime(&sta->add_time_sysruntime);
			sta->flags |= WLAN_STA_AUTH;
			sta->flags |= WLAN_STA_ASSOC;
			unsigned char SID = 0;
			if(ASD_WLAN[wasd->WlanID])
				SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
			if(sta != NULL){
				if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
					bak_new_station_1x(wasd,sta,msg);
					if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)){
						bak_new_station_wpa(wasd,sta);
					}
				}else if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P))){
					bak_new_station_wpa(wasd,sta);
				}					
				if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
					if(sta->ipaddr != 0)
						accounting_sta_start(wasd, sta);	
					else if(ASD_SECURITY[SID]->account_after_authorize)
						accounting_sta_start(wasd,sta);
				}
			}
			
			break;
		case B_DEL:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_STA_OP M_DEL\n");
			//pthread_mutex_lock(&asd_g_sta_mutex); 
			BSSIndex = msg->Bu.U_STA.BSSIndex;
			Radio_ID = msg->Bu.U_STA.BSSIndex/L_BSS_NUM;
			if((Radio_ID >= G_RADIO_NUM)||(BSSIndex >= BSS_NUM))
				return;
			i = msg->Bu.U_STA.BSSIndex%L_BSS_NUM;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSSIndex %d Radio_ID %d\n",BSSIndex,Radio_ID);
			if((interfaces != NULL)&&(interfaces->iface[Radio_ID] != NULL) && ( interfaces->iface[Radio_ID]->bss[i] != NULL )) {
				wasd = interfaces->iface[Radio_ID]->bss[i];
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_lock(&(wasd->asd_sta_mutex));   
#endif
			/*	if(wasd->num_sta < (msg->Bu.U_STA.total_num)){
					asd_printf(ASD_DEFAULT,MSG_INFO,"B_STA_OP sta del :need reupdate\n");
					B_UPDATE_REQUEST(asd_sock, BSSIndex);
				}
			*/
				sta = ap_get_sta(wasd, msg->Bu.U_STA.STAMAC);
				if(sta != NULL)
					ap_free_sta(wasd, sta, 0);	
#ifdef ASD_USE_PERBSS_LOCK
				pthread_mutex_unlock(&(wasd->asd_sta_mutex)); 
#endif
			}
			
			//pthread_mutex_unlock(&asd_g_sta_mutex); 
			return;
	
			break;
		case B_MODIFY:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_STA_OP MODIFY\n");
			//pthread_mutex_lock(&asd_g_sta_mutex); 
			BSSIndex = msg->Bu.U_STA.BSSIndex;
			Radio_ID = msg->Bu.U_STA.BSSIndex/L_BSS_NUM;
			if((Radio_ID >= G_RADIO_NUM)||(BSSIndex >= BSS_NUM))
				return;
			i = msg->Bu.U_STA.BSSIndex%L_BSS_NUM;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSSIndex %d Radio_ID %d\n",BSSIndex,Radio_ID);
			if((interfaces != NULL)&&(interfaces->iface[Radio_ID] != NULL) && ( interfaces->iface[Radio_ID]->bss[i] != NULL )) {
				wasd = interfaces->iface[Radio_ID]->bss[i];
				sta = ap_get_sta(wasd, msg->Bu.U_STA.STAMAC);
				if(sta != NULL){
					if(sta->ipaddr == msg->Bu.U_STA.ipaddr){
						break;
					}else{
						char mac[20];
						char ifname[ETH_IF_NAME_LEN];
						unsigned char *ip;						
						//struct if_nameindex *ifin = NULL;
						//int gindex = 0;
						memset(mac,0,20);
						memset(ifname,0,ETH_IF_NAME_LEN);
						sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(sta->addr));
						#if 0
						ifin = if_nameindex();
						
						if(ifin) {
							i = 0;
							while(ifin[i].if_name && ifin[i].if_index) {
								ret = if_name2gindex(ifin[i].if_name, &gindex);
								if((!ret)&&(gindex == msg->Bu.U_STA.gifindex)) {
									strcpy(ifname,ifin[i].if_name);
									break;
								}
								i++;
							}
							if_freenameindex(ifin);
						}	
						#endif						
						if(ASD_ARP_GROUP[msg->Bu.U_STA.gifindex] != NULL){
							strcpy(ifname,ASD_ARP_GROUP[msg->Bu.U_STA.gifindex]);
						}
						if(sta->ipaddr != 0){
							ipneigh_modify(RTM_DELNEIGH, 0,sta->in_addr, mac,sta->arpifname);
							 if(sta->security_type == NO_NEED_AUTH){
								 if(asd_ipset_switch)
									 eap_connect_down(sta->ipaddr);
								 else
									 AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
							}
						}
						sta->ipaddr = msg->Bu.U_STA.ipaddr;
						sta->gifidx = msg->Bu.U_STA.gifindex;
						memset(sta->arpifname,0,16);
						memset(sta->in_addr, 0, 16);
						strcpy(sta->arpifname,ifname);
						ip = (unsigned char *)&(sta->ipaddr);
						sprintf(sta->in_addr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
						inet_aton(sta->in_addr,&(sta->ip_addr));
						if(sta->ip_addr.s_addr != 0){
							if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL == sta->security_type)){
								if(asd_ipset_switch)
									eap_connect_down(sta->ipaddr);
								else
									AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
							}
						}
						unsigned char SID = 0;
						if(ASD_WLAN[wasd->WlanID])
							SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
						if((ASD_SECURITY[SID])&&( 0 ==ASD_SECURITY[SID]->account_after_authorize))
						{
							if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
								accounting_sta_start(wasd, sta);	
							}
						}	
						if(asd_sta_static_arp)
							ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,sta->in_addr, mac,ifname);
					}
				}
			}
			break;
		case B_UPDATE:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"case B_UPDATE\n");
			break;
	}
}	

void B_BATCH_STA_OP(_B_Msg *msg){
	int i = 0;
	//int ret = 0;	
	//int len = 0;
	int num = 0;
	int j = 0;
	struct sta_info *sta = NULL;
	unsigned int BSSIndex;
	unsigned int Radio_ID;	
	struct asd_data *wasd = NULL;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	switch(msg->Op){
		case B_ADD:
			num = msg->num;
			for(j = 0; j < num; j++){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_BATCH_STA_OP M_ADD\n");
				BSSIndex = msg->U_STA[j].BSSIndex;
				Radio_ID = msg->U_STA[j].BSSIndex/L_BSS_NUM;
				i = msg->U_STA[j].BSSIndex%L_BSS_NUM;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_BATCH_STA_OP BSSIndex %d Radio_ID %d\n",BSSIndex,Radio_ID);
				if((Radio_ID >= G_RADIO_NUM)||(BSSIndex >= BSS_NUM))
					continue;
				if((interfaces != NULL)&&(interfaces->iface[Radio_ID] != NULL))
					wasd = interfaces->iface[Radio_ID]->bss[i];
				else
					continue;
				if(wasd != NULL){
					sta = ap_sta_add(wasd, msg->U_STA[j].STAMAC, 1);					
					if(sta == NULL){
						asd_printf(ASD_DEFAULT,MSG_ERROR,"B_BATCH_STA_OP sta add failed!!!!!!\n");
						return;
					}
					if(msg->U_STA[j].ipaddr != 0)
					{
						if(sta->ipaddr == msg->U_STA[j].ipaddr){
							continue;
						}else{
							char mac[20];
							char ifname[ETH_IF_NAME_LEN];
							unsigned char *ip;						
							//struct if_nameindex *ifin = NULL;
							//int gindex = 0;
							memset(mac,0,20);
							memset(ifname,0,ETH_IF_NAME_LEN);
							sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(sta->addr));
							#if 0
							ifin = if_nameindex();
							
							if(ifin) {
								i = 0;
								while(ifin[i].if_name && ifin[i].if_index) {
									ret = if_name2gindex(ifin[i].if_name, &gindex);
									if((!ret)&&(gindex == msg->Bu.U_STA.gifindex)) {
										strcpy(ifname,ifin[i].if_name);
										break;
									}
									i++;
								}
								if_freenameindex(ifin);
							}
							#endif
							if(ASD_ARP_GROUP[msg->U_STA[j].gifindex] != NULL){
								strcpy(ifname,ASD_ARP_GROUP[msg->U_STA[j].gifindex]);
							}
							if(sta->ipaddr != 0){
								ipneigh_modify(RTM_DELNEIGH, 0,sta->in_addr, mac,sta->arpifname);
								if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL == sta->security_type)){
									if(asd_ipset_switch)
										eap_connect_down(sta->ip_addr.s_addr);
									else
										AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
								}
							}
							sta->ipaddr = msg->U_STA[j].ipaddr;
							sta->gifidx = msg->U_STA[j].gifindex;
							memset(sta->arpifname,0,16);
							memset(sta->in_addr, 0, 16);
							strcpy(sta->arpifname,ifname);
							ip = (unsigned char *)&(sta->ipaddr);
							sprintf(sta->in_addr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
							inet_aton(sta->in_addr,&(sta->ip_addr));
							if(sta->ip_addr.s_addr != 0){
								if((sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL == sta->security_type)){
									if(asd_ipset_switch)
										eap_connect_up(sta->ip_addr.s_addr);
									else
										AsdStaInfoToEAG(wasd,sta,ASD_AUTH);
								}
							}							
							if(asd_sta_static_arp)
								ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,sta->in_addr, mac,ifname);
						}
					}
				}
				else{
					asd_printf(ASD_DEFAULT,MSG_ERROR,"B_BATCH_STA_OP sta add :wasd NULL!!!!!!\n");
					continue;
				}
				*(sta->add_time) = msg->U_STA[j].sta_add;
				sta->sta_online_time = msg->U_STA[j].sta_online_time;//qiuchen add it
				sta->flags |= WLAN_STA_AUTH;
				sta->flags |= WLAN_STA_ASSOC;
				unsigned char SID = 0;
				if(ASD_WLAN[wasd->WlanID])
					SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
				if(sta != NULL){
					if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
						bak_batch_new_station(wasd,sta,&(msg->U_STA[j]));						
						if(sta->ipaddr != 0)
						{
								accounting_sta_start(wasd, sta);	
						}	
						else if(ASD_SECURITY[SID]->account_after_authorize == 1)
							accounting_sta_start(wasd,sta);
					}
				}
				if(mast_bak_update == 1){
					sta->PreBssIndex = msg->U_STA[j].PreBSSIndex;
					sta->rflag = msg->U_STA[j].rflag;
					memcpy(sta->PreBSSID,msg->U_STA[j].PreBSSID,MAC_LEN);
					if(sta->PreBssIndex != wasd->BSSIndex && ASD_WLAN[wasd->WlanID] && ASD_WLAN[wasd->WlanID]->Roaming_Policy){
						RoamingStaInfoToWSM_1(sta,WID_MODIFY);
						RoamingStaInfoToWIFI_1(sta,WID_ADD);
					}
				}	
			}
			break;
		case B_DEL:
		case B_MODIFY:
		case B_UPDATE:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"not case B_ADD\n");
			break;
	}
}	
void B_BATCH_ROAM_STA_OP(B_ROAM_Msg  *msg){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"in fun %s\n",__func__);
	unsigned int BSSIndex_before = 0;
	unsigned int Radio_ID_before  = 0;;	
	unsigned int num = 0 ; 
	unsigned int i ,j;
	struct ROAMING_STAINFO *r_sta;
	struct asd_data *wasd_before = NULL;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->Op = %d\n",msg->Op);
	switch(msg->Op){
		case B_ADD:
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s :in case B_ADD\n",__func__);
			num = msg->num;
			for( j = 0 ; j < num ; j++)
			{
				BSSIndex_before = msg->U_STA[j].PreBssIndex;
				Radio_ID_before = BSSIndex_before/L_BSS_NUM;				
				i = msg->U_STA[j].PreBssIndex%L_BSS_NUM;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_BATCH_STA_OP BSSIndex %d Radio_ID %d\n",BSSIndex_before,Radio_ID_before);
				if((Radio_ID_before>= G_RADIO_NUM)||(BSSIndex_before >= BSS_NUM))
					continue;
				if((interfaces != NULL)&&(interfaces->iface[Radio_ID_before] != NULL))
					wasd_before= interfaces->iface[Radio_ID_before]->bss[i];
				else{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func :%s :B_ADD:next!\n",__func__);
					continue;
				}	
				if(wasd_before!=NULL)
				{
					r_sta = roaming_get_sta(ASD_WLAN[wasd_before->WlanID],msg->U_STA[j].addr);
					if(r_sta == NULL){
						r_sta = roaming_sta_add(ASD_WLAN[wasd_before->WlanID],msg->U_STA[j].addr,wasd_before->BSSIndex,wasd_before->own_addr);
						if(r_sta == NULL){
							asd_printf(ASD_DEFAULT,MSG_ERROR,"in asd_bak,roaming sta add error!\n");
							continue;
						}
						r_sta->BssIndex = msg->U_STA[j].BssIndex;
						memcpy(r_sta->BSSID,msg->U_STA[j].BSSID,MAC_LEN);
						memcpy(r_sta->PreBSSID,msg->U_STA[j].PreBSSID,MAC_LEN);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"ASD_WLAN[%d]->Roaming_Policy  = %d\n",wasd_before->WlanID,ASD_WLAN[wasd_before->WlanID]->Roaming_Policy);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"r_sta->BssIndex = %d\n",r_sta->BssIndex);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"r_sta->PreBssIndex = %d\n",r_sta->PreBssIndex);
						if((ASD_WLAN[wasd_before->WlanID]!=NULL)&&(ASD_WLAN[wasd_before->WlanID]->Roaming_Policy != 0)){
							if((r_sta != NULL)&&(r_sta->BssIndex != r_sta->PreBssIndex)){
								RoamingStaInfoToWSM(r_sta,WID_MODIFY);
								RoamingStaInfoToWIFI(r_sta,WID_ADD);
								
							}
						}
					}
					else
					{
						if(r_sta->BssIndex == msg->U_STA[j].BssIndex){
							asd_printf(ASD_DEFAULT,MSG_DEBUG,"roam sta already add!\n");
							continue;
						}
						else
						{
							r_sta->BssIndex = msg->U_STA[j].BssIndex;
							r_sta->PreBssIndex = msg->U_STA[j].PreBssIndex;
							memcpy(r_sta->BSSID,msg->U_STA[j].BSSID,MAC_LEN);
							memcpy(r_sta->PreBSSID,msg->U_STA[j].PreBSSID,MAC_LEN);
							
							if((ASD_WLAN[wasd_before->WlanID]!=NULL)&&(ASD_WLAN[wasd_before->WlanID]->Roaming_Policy != 0)){
								if((r_sta != NULL)&&(r_sta->BssIndex != r_sta->PreBssIndex)){
									RoamingStaInfoToWSM(r_sta,WID_MODIFY);
									RoamingStaInfoToWIFI(r_sta,WID_ADD);
									
								}
							}
							
						}
					}
					
				}
				else{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func %s:wasd_before = NULL\n",__func__);
					continue;
				}
			}
			break;
			default:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func %s: go to default\n",__func__);
	}

}

//mahz add 2011.8.2
int ASD_SEARCH_ALL_BSS(struct asd_data **bss, B_Msg *msg){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0, num = 0;
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else if(interfaces->iface[i]->bss[j] != NULL){
					bss[num] = interfaces->iface[i]->bss[j];
					msg->Bu.U_ALL_BSS.bssindex_array[num].bssindex = bss[num]->BSSIndex;
					msg->Bu.U_ALL_BSS.bssindex_array[num].sta_num = bss[num]->num_sta;
					num += 1;	
					continue;
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
	}
	return num;
}

void bak_update_bss_req(void *circle_ctx, void *timeout_ctx){
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s\n",__func__);

	int bss_num=0;
	int len=0, ret=0;
	char buf[40960] = {0};
	B_Msg *msg = (B_Msg*)buf;
	//struct asd_data *bss[BSS_NUM];
	struct asd_data **bss = NULL;
	bss = os_zalloc(BSS_NUM*(sizeof(struct asd_data *)));
	if( bss == NULL){
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		circle_register_timeout(asd_bak_sta_update_time*60, 0, bak_update_bss_req, NULL, NULL);
		return;
	}
	//qiuchen change it 2012.11.27
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	int i = 0; //num = 0;
	for(i = 0; i < G_RADIO_NUM; ){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			int j = 0 ;
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else if(interfaces->iface[i]->bss[j] != NULL){
					bss[bss_num] = interfaces->iface[i]->bss[j];
					msg->Bu.U_ALL_BSS.bssindex_array[bss_num].bssindex = bss[bss_num]->BSSIndex;
					msg->Bu.U_ALL_BSS.bssindex_array[bss_num].sta_num = bss[bss_num]->num_sta;
					bss_num += 1;	
					if(bss_num == 4096){
						msg->Bu.U_ALL_BSS.count = bss_num;
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"************bss_num = %d**********\n",msg->Bu.U_ALL_BSS.count);
						msg->Type = B_BSS_TYPE;
						msg->Op = B_UPDATE;
						len = sizeof(B_Msg);
						
						ret = sendto(asd_sock, buf, len+1,0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr));
						if(ret < 0){			
							asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
							asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s",__func__,strerror(errno));
						}
						bss_num = 0;
						memset(buf,0,sizeof(buf));
						memset(bss,0,BSS_NUM*(sizeof(struct asd_data *)));
					}
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
	}
	//end
	//bss_num= ASD_SEARCH_ALL_BSS(bss,&msg);
	if(bss_num == 0){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"the last bss_num is 0 \n");	//for test
		circle_register_timeout(asd_bak_sta_update_time*60, 0, bak_update_bss_req, NULL, NULL);
		if(bss){
			free(bss);
			bss = NULL;
		}
		return;
	}

	msg->Bu.U_ALL_BSS.count = bss_num;//qiuchen change it
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"************the last bss_num = %d**********\n",msg->Bu.U_ALL_BSS.count);
	msg->Type = B_BSS_TYPE;
	msg->Op = B_UPDATE;

	len = sizeof(B_Msg);

	ret = sendto(asd_sock, buf, len+1,0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr));
	if(ret < 0){			
		asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
		asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s",__func__,strerror(errno));
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"succeed to send bss update request\n");
	circle_register_timeout(asd_bak_sta_update_time*60, 0, bak_update_bss_req, NULL, NULL);
	if(bss){
		free(bss);
		bss = NULL;
	}
}

#ifdef ASD_MULTI_THREAD_MODE

void *asd_bak_thread()
{
	int numbytes;
	char buf[2048];
	socklen_t addr_len;
	struct sockaddr_in ac_addr;	
//	if(asd_sock == -1)
//	listen(asd_sock, 5);//zhanglei move to socket init

	while(is_secondary == 1){		
		addr_len = sizeof(struct sockaddr);
		new_sock = accept(asd_sock, (struct sockaddr *)&ac_addr, &addr_len);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"new_sock %d\n",new_sock);
		if(new_sock == -1){
			asd_printf(ASD_DEFAULT,MSG_ERROR,"accept failed\n");
			continue;
		}
	
		while(is_secondary == 1){
			memset(buf, 0, 2048);
			numbytes = recv(new_sock, buf, 2047, 0);
			if(numbytes <= 0){
				asd_printf(ASD_DEFAULT,MSG_ERROR,"bak recv error\n");
				close(new_sock);
				break;
			}
			if(is_secondary == 1){
			B_Msg *msg = (B_Msg*) buf;
				switch(msg->Type){
					case B_STA_TYPE:
						B_STA_OP(msg);	
						break;	
					case B_INFO_TYPE:
						B_INFO_OP(msg);
						break;
					case B_END:
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"notice vrrp %d\n",vrrid);
						notice_vrrp_state_change(vrrid,0);
						break;
					case B_BSS_RADIUS:
						B_RADIUS_UPDATE(msg);
						break;
					default:
						break;
				}
			}
			//close(new_sock);
		}
	}
	ASD_BAK = 0;
	pthread_exit((void *) 0);
}

void *asd_update_thread()
{
	struct asd_data **bss;
	struct asd_data *wasd = NULL;
	unsigned int num;
	unsigned int i;
	unsigned int j;
	unsigned int k = 0;
	struct sta_info *sta;
	if ((asd_master_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s socket %s\n",__func__,strerror(errno));
		perror("socket create");
		exit(1);
	}
	while((is_secondary == 0)&&(connect(asd_master_sock, (struct sockaddr *)&B_addr, sizeof(struct sockaddr)) == -1)){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s connect %s\n",__func__,strerror(errno));
		perror("connect");
		k++;
		if(k >= 20){
			asd_printf(ASD_DEFAULT,MSG_INFO,"master AC connect bak AC failed\n");
			update_pass = 0;
			break;
		}	
		sleep(1);
		continue;
	}

	if(is_secondary == 0){				
		bss = os_zalloc(BSS_NUM*sizeof(struct asd_data *));
		if(bss==NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}	
		memset(bss,0,BSS_NUM*sizeof(struct asd_data *));
		num = ASD_SEARCH_ALL_STA(bss);
		sleep(2);
		for(i=0;i<num;i++){
			unsigned char sid = bss[i]->SecurityID;
			if((ASD_SECURITY[sid]== NULL)||((ASD_SECURITY[sid]!= NULL)&&(ASD_SECURITY[sid]->securityType != OPEN)))
				continue;
			sta = bss[i]->sta_list;
			for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
				bak_add_sta(bss[i],sta);
				sta = sta->next;
			}				
		}
		update_end();
		free(bss);
	}
	update_pass = 0;
	pthread_exit((void *) 0);
}
#else

#ifdef _AC_BAK_UDP_

int bak_check(int sockfd, int sta_count){
	B_Msg msg;
	B_Msg *tmp;
	int len;
	int ret;
	char buf[65535];
	int n = 0;
	struct sockaddr_in addr;
	socklen_t addr_len;
	memset(buf,0,65535);
	msg.Type = B_CHECK_REQ;
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
		asd_printf(ASD_DEFAULT,MSG_INFO,"select err!\n");
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
#endif
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss ret %d\n",ret);
		if(ret < 0){			
			asd_printf(ASD_DEFAULT,MSG_INFO,"%s send %s",__func__,strerror(errno));
			perror("send");
			//exit(1);
		}
	}

	
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 3;
	tv.tv_usec = 5000;
	tmp = (B_Msg *)buf;
	tmp->Type = B_CHECK_ReREQ;
	while((select(sockfd + 1,(fd_set *) &fdset,(fd_set *) 0,(fd_set *) 0,&tv) <= 0)&&(n < 3))
	{
		sendto(sockfd, buf, len+1,0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
		n++;
	}
	if(n == 3){		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s n = 3\n",__func__);
		return 0;
	}
	memset(buf,0,65535);
	if (FD_ISSET(sockfd, &fdset)){		
		len = recvfrom(sockfd,buf, 65535,0, (struct sockaddr *)&addr, &addr_len);
		if(len > 0){
			tmp = (B_Msg *)buf;
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s tmp->Bu.CHK.sta_count %d sta_count %d\n",__func__,tmp->Bu.CHK.sta_count,sta_count);			
			if(tmp->Type == B_CHECK_RES){
				if((tmp->Bu.CHK.sta_count >= sta_count)){
					return 1;
				}
			}
		}
	}
	return 0;
}


void B_CHK_RES(int sockfd, int sta_count){
	B_Msg msg;
	int len;
	int ret;
	char buf[65535];
	memset(buf,0,65535);
	msg.Type = B_CHECK_RES;
	msg.Bu.CHK.sta_count = sta_count;
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
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr));
#endif
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss ret %d\n",ret);
		if(ret < 0){			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s send %s",__func__,strerror(errno));
			perror("send");
			//exit(1);
		}
	}
}

void B_UPDATE_REQUEST(int sockfd, int bssindex){
	B_Msg msg;
	int len;
	int ret;
	char buf[65535];
	memset(buf,0,65535);
	msg.Type = B_UPDATE_REQ;
	msg.Bu.U_BSS_STA.bss_index = bssindex;
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
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, buf, len+1, 0);
#else
		ret = sendto(sockfd, buf, len+1,0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr));
#endif
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss ret %d\n",ret);
		if(ret < 0){			
			asd_printf(ASD_DEFAULT,MSG_CRIT,"%s send %s",__func__,strerror(errno));
			perror("send");
			//exit(1);
		}
	}
}

void asd_bak_select_mode(int sock, void *circle_ctx, void *sock_ctx){
	
	int numbytes;
	char buf[65535];	
	//struct sockaddr_in ac_addr;
	socklen_t addr_len;
	int sta_count = 0;
	int re_sta_count = 0;
	memset(buf, 0, 65535);

	addr_len = sizeof(struct sockaddr);
	numbytes = recvfrom(asd_sock, buf, 65534, 0, (struct sockaddr *)&M_addr, &addr_len);
	if(numbytes <= 0){
		asd_printf(ASD_DEFAULT,MSG_ERROR,"errno = %d\n",errno);
		asd_printf(ASD_DEFAULT,MSG_ERROR,"%s recvfrom %s",__func__,strerror(errno));
		asd_printf(ASD_DEFAULT,MSG_ERROR,"bak recv error\n");
		return;
	}
	if(numbytes == sizeof(B_Msg))
		mast_bak_update = 1;
	else
		mast_bak_update = 0;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"mast_bak_update = %d\n",mast_bak_update);
	if(is_secondary == 1){
	B_Msg *msg = (B_Msg*) buf;
		_B_Msg *msg1 = (_B_Msg*) buf;
		B_ROAM_Msg *msg2 = (B_ROAM_Msg *)buf;
		switch(msg->Type){
			case B_STA_TYPE:
				B_STA_OP(msg);
				sta_count++;
				break;	
			case B_INFO_TYPE:
				//B_INFO_OP(msg);
				break;
			case B_END:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"notice vrrp %d\n",vrrid);
				notice_vrrp_state_change(vrrid,0);
				break;			
			case B_CHECK_REQ:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_CHECK_REQ sta_count %d\n",sta_count);
				B_CHK_RES(asd_sock,sta_count);
				re_sta_count = sta_count;
				sta_count = 0;
				break;
			case B_CHECK_ReREQ:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_CHECK_REQ re_sta_count %d\n",re_sta_count);
				B_CHK_RES(asd_sock,re_sta_count);
				sta_count = 0;
				break;
			case B_BATCH_STA_TYPE:				
				if(numbytes == sizeof(_B_Msg))
					mast_bak_update = 1;
				else
					mast_bak_update = 0;
				asd_printf(ASD_DEFAULT,MSG_INFO,"%s B_BATCH_STA_TYPE\n",__func__);
				B_BATCH_STA_OP(msg1);
				break;
			case B_BATCH_ROAM_STA:
				asd_printf(ASD_DEFAULT,MSG_INFO,"%s: B_BATCH_ROAM_STA\n",__func__);
				B_BATCH_ROAM_STA_OP(msg2);
			case B_BSS_RADIUS:
				asd_printf(ASD_DEFAULT,MSG_INFO,"%s: B_RADIUS_UPDATE\n",__func__);
				B_RADIUS_UPDATE(msg);
				break;
			default:
				asd_printf(ASD_DEFAULT,MSG_INFO,"%s: error msg->Type:%d\n ",__func__,msg->Type);
				break;
		}
	}
}

void asd_master_select_mode(int sock, void *eloop_ctx, void *sock_ctx){
	
	int numbytes;
	char buf[65535];	
	struct sockaddr_in ac_addr;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
	socklen_t addr_len;
	struct asd_data *bss = NULL;
	struct asd_data *wasd = NULL;
	unsigned int bssindex = 0;
	unsigned int radioid = 0;
	unsigned char bss_id = 0;	
	unsigned int i,j;
	unsigned int bss_count;
	unsigned int sta_num;
	struct sta_info *sta;
	memset(buf, 0, 65535);
	numbytes = recvfrom(asd_master_sock, buf, 65534, 0, (struct sockaddr *)&ac_addr, &addr_len);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_master_select_mode 1\n");

	if(numbytes <= 0){
		asd_printf(ASD_DEFAULT,MSG_ERROR,"asd_master_select_mode recv error\n");
		asd_printf(ASD_DEFAULT,MSG_ERROR,"errno = %d\n",errno);
		asd_printf(ASD_DEFAULT,MSG_ERROR,"%s recvfrom %s",__func__,strerror(errno));
		return;
	}
	if(is_secondary == 0){
	B_Msg *msg = (B_Msg*) buf;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_master_select_mode 2\n");
		switch(msg->Type){
			case B_UPDATE_REQ:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_master_select_mode 3\n");
				bssindex = msg->Bu.U_BSS_STA.bss_index;
				radioid = bssindex/L_BSS_NUM;
				bss_id = bssindex%L_BSS_NUM + 1;
				ASD_SEARCH_BSS_BY_RADIO_BSS(radioid, bss_id, &bss);
				if(bss == NULL)
					break;
				sta = bss->sta_list;
				for(j = 0; (j < bss->num_sta)&&(sta!=NULL); j++){
					bak_add_sta(bss,sta);
					sta = sta->next;
				}				
				break;	
			case B_BSS_TYPE:{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"case B_BSS_TYPE\n");	//for test
				if(bak_unreach == 1){
					bak_unreach = 0;
					inaddr->sin_family = AF_INET;	
					inaddr->sin_port = htons(ASD_BAK_AC_PORT+local*MAX_INSTANCE+vrrid);	
					inaddr->sin_addr.s_addr = ac_addr.sin_addr.s_addr;	
					memset(inaddr->sin_zero, '\0', sizeof(inaddr->sin_zero)); 			
				}
				bss_count = msg->Bu.U_ALL_BSS.count;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"************bss_count = %d**********\n",bss_count);
				for(i=0; i<bss_count&&bss_count <=4096; i++){//qiuchen change it
					wasd = (struct asd_data *)AsdCheckBSSIndex(msg->Bu.U_ALL_BSS.bssindex_array[i].bssindex);				
					if(wasd == NULL){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"wasd is null\n");	//for test
						continue;
					}
					sta_num = msg->Bu.U_ALL_BSS.bssindex_array[i].sta_num;
					if(sta_num == wasd->num_sta){
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"there is no need to update sta info\n");	//for test
						continue;
					}
					sta = wasd->sta_list;
					for(j = 0; (j < wasd->num_sta)&&(sta!=NULL); j++){
						bak_add_sta(wasd,sta);
						sta = sta->next;
					}				
				}
				break;
			}
			case B_CHECK_STA_REQ:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"case B_CHECK_STA_REQ:");
				u8 mac[MAC_LEN];
				memset(mac,0,MAC_LEN);
				bssindex = msg->Bu.U_STA.BSSIndex;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bssindex = %d\n",bssindex);
				memcpy(mac,msg->Bu.U_STA.STAMAC,MAC_LEN);				
				pthread_mutex_lock(&(asd_g_sta_mutex)); 		
				wasd = AsdCheckBSSIndex(bssindex);
				if(wasd == NULL)
				{
					asd_printf(ASD_DEFAULT,MSG_INFO,"bak request sta"MACSTR";wasd is not exist!\n",MAC2STR(mac));
					pthread_mutex_unlock(&(asd_g_sta_mutex));
					break;
				}
				sta = ap_get_sta(wasd,mac);
				if(sta == NULL)
				{
					asd_printf(ASD_DEFAULT,MSG_INFO,"bak request sta"MACSTR";sta  is not exist!\n",MAC2STR(mac));
					pthread_mutex_unlock(&(asd_g_sta_mutex));
					break;
					
				}
				bak_add_sta(wasd,sta);
				pthread_mutex_unlock(&(asd_g_sta_mutex));
				break;
			default:
				break;
		}
	}
}

int  asd_update_select_mode(void *circle_ctx, void *timeout_ctx){
	int sndbuf = 65535;
	int rcvbuf = 65535;
	//struct sta_info *sta;
	
	
	//asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_update_select_mode 11\n");//mahz test 2010.12.29
	if(local){
		if ((asd_master_sock = socket(AF_TIPC, SOCK_RDM, 0)) ==-1){
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
			perror("socket create");
		}		
	}else{
		if ((asd_master_sock = socket(PF_INET, SOCK_DGRAM, 0)) ==-1){
			asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
			perror("socket create");
			exit(1);
		}
	}
	setsockopt(asd_master_sock,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf));
	setsockopt(asd_master_sock,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf));
	if (circle_register_read_sock(asd_master_sock, asd_master_select_mode, NULL, NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
		asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
		asd_printf(ASD_DEFAULT,MSG_WARNING,"%s socket %s\n",__func__,strerror(errno));
		return -1;
	}		
	return 1;
}
//qiuchen add it for master_bak radius server 2012.12.17
void asd_update_radius_state()
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func:%s\n",__func__);
	int len;
	int ret;
	int i = 0;
	unsigned char bak_exist = 0;
	struct sockaddr_in *inaddr = (struct sockaddr_in *)&B_addr;
	int j = 1;
	
	B_Msg msg;
	msg.Type = B_BSS_RADIUS;
	msg.Op = B_UPDATE;
	for(j=1;j<129;j++){
		if((ASD_SECURITY[j] != NULL) && (ASD_SECURITY[j]->heart_test_on == 1)){
			msg.Bu.U_BSS_RADIUS.sArray[i].SID = (unsigned char)j;
			msg.Bu.U_BSS_RADIUS.sArray[i].acct_server = ASD_SECURITY[j]->acct_server_current_use;
			msg.Bu.U_BSS_RADIUS.sArray[i].auth_server = ASD_SECURITY[j]->auth_server_current_use;
			i++;
		}
	}
	if(i == 0){
		asd_printf(ASD_80211,MSG_DEBUG,"#######There is no radius server state to update!#######\n");
		return;
	}
	asd_printf(ASD_80211,MSG_DEBUG,"#######There are %d radius servers state to update!#######\n",i);
	msg.Bu.U_BSS_RADIUS.sid_num = (unsigned char)i;
	
		len = sizeof(msg);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s,len %d\n",__func__,len);
		
		if(asd_master_sock == -1)
			return;
#ifndef _AC_BAK_UDP_
			if((send(asd_master_sock, &msg, len, 0) <= 0)){
				asd_printf(ASD_DEFAULT,MSG_CRIT,"%s send %s\n",__func__,strerror(errno));
				perror("sendto");	
				if(is_secondary == 0){
					close(asd_master_sock);
					//asd_master_sock = -1;
					circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
					circle_register_timeout(2, 0,asd_update_select_mode, NULL, NULL);
				}
				//exit(1);
			}
#else
		if(inaddr->sin_addr.s_addr != 0){
			bak_exist = 1;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_exist = %d,bak_unreach = %d,asd_master_sock = %d\n",bak_exist,bak_unreach,asd_master_sock);
			if((bak_exist == 1)&&(bak_unreach == 0)){
				ret = sendto(asd_master_sock, &msg, len, 0,(struct sockaddr *)&B_addr, sizeof(struct sockaddr));
				if(ret < 0){
					bak_error = errno;
					asd_printf(ASD_DEFAULT,MSG_WARNING,"errno = %d\n",errno);
					asd_printf(ASD_DEFAULT,MSG_WARNING,"%s send %s\n",__func__,strerror(errno));
					perror("sendto");		
					if((bak_error == EBADF)||(bak_error == ENETUNREACH)){
						bak_unreach = 1;
					}
				}
			}
#endif
			return;

}

//end
void asd_update_batch_sta()
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func:%s\n",__func__);
	struct asd_data **bss;
	unsigned int num;
	unsigned int i;
	struct asd_data *wasd = NULL;			//mahz add 2010.12.29
	if(is_secondary == 0){				
		bss = malloc(BSS_NUM*sizeof(struct asd_data *));				
		if(bss==NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}	
		memset(bss,0,BSS_NUM*sizeof(struct asd_data *));
		sleep(2);
		num = ASD_SEARCH_ALL_STA(bss);
		if(num > 0)
			bak_batch_add_sta(bss, num);
		//weichao add
		sleep(2);
		//bak_batch_add_roam_sta();
		update_end();

		//mahz modified 2010.12.29
		if(num > 0){
			for(i=0;i<num;i++){ 			
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bss->index :%d\n",bss[i]->BSSIndex);

				wasd = AsdCheckBSSIndex(bss[i]->BSSIndex);
				if(wasd != NULL){//qiuchen
					unsigned char sid = wasd->SecurityID;
					if((ASD_SECURITY[sid]!= NULL)&&((ASD_SECURITY[sid]->securityType == WPA_P)||(ASD_SECURITY[sid]->securityType == WPA2_P)||(ASD_SECURITY[sid]->securityType == WPA_E)||(ASD_SECURITY[sid]->securityType == WPA2_E))){
						circle_cancel_timeout(wpa_rekey_gtk,wasd->wpa_auth,NULL);
						circle_register_timeout(ASD_SECURITY[sid]->wpa_once_group_rekey_time, 0,wpa_rekey_gtk, wasd->wpa_auth, NULL);
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"wasd->wpa_auth p:%p\n",wasd->wpa_auth);
					}
				}
			}
		}
		//
		if(bss)
		{
			free(bss);
			bss = NULL;
		}
	}
	update_pass = 0;
}
void bak_check_sta_req(int sockfd, unsigned int BSSIndex,u8* mac){
	B_Msg msg;
	int len;
	int ret;
	memset(&msg,0,sizeof(msg));
	msg.Type = B_CHECK_STA_REQ;
	memcpy(msg.Bu.U_STA.STAMAC,mac,MAC_LEN);
	msg.Bu.U_STA.BSSIndex = BSSIndex;
	len = sizeof(msg);
	
	fd_set fdset;
	struct timeval tv;			
	FD_ZERO(&fdset);
	FD_SET(sockfd,&fdset);
	
	tv.tv_sec = 1;
	tv.tv_usec = 5000;
	
	if (select(sockfd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"select err!\n");
		
	}
	
	if (FD_ISSET(sockfd, &fdset)){
#ifndef _AC_BAK_UDP_
		ret = send(sockfd, &msg, len+1, 0);
#else
		ret = sendto(sockfd, &msg, len+1,0,(struct sockaddr *)&M_addr, sizeof(struct sockaddr));
#endif
		if(ret < 0){			
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s send %s",__func__,strerror(errno));
			perror("send");
			//exit(1);
		}
	}
}
#else
void asd_bak_select_mode(int sock, void *circle_ctx, void *sock_ctx){
	
	socklen_t addr_len;
	struct sockaddr_in ac_addr; 
	int tsock = -1;
	addr_len = sizeof(struct sockaddr);
	tsock = accept(asd_sock, (struct sockaddr *)&ac_addr, &addr_len);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"new_sock %d\n",new_sock);
	if(tsock < 0){
		asd_printf(ASD_DEFAULT,MSG_ERROR,"accept failed\n");
		return;
	}
	circle_unregister_read_sock(new_sock);
	FD_CHANGE = 1;
	close(new_sock);
	new_sock = -1;
	new_sock = tsock;
	if (circle_register_read_sock(new_sock, asd_bak_select_mode2, NULL, NULL))
	{
		asd_printf(ASD_DEFAULT,MSG_WARNING,"Could not register read socket\n");
		return;
	}
//	circle_unregister_read_sock(asd_sock);

}
void asd_bak_select_mode2(int sock, void *circle_ctx, void *sock_ctx){
	
	int numbytes;
	char buf[2048];
	memset(buf, 0, 2048);
	numbytes = recv(new_sock, buf, 2047, 0);
	if(numbytes <= 0){
		asd_printf(ASD_DEFAULT,MSG_ERROR,"bak recv error\n");		
		circle_unregister_read_sock(new_sock);
		FD_CHANGE = 1;
		close(new_sock);
		new_sock = -1;
		return;
	}
	if(is_secondary == 1){
	B_Msg *msg = (B_Msg*) buf;
		switch(msg->Type){
			case B_STA_TYPE:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_STA_TYPE\n");
				B_STA_OP(msg);	
				break;	
			case B_INFO_TYPE:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"B_INFO_TYPE\n");
			//	B_INFO_OP(msg);
				break;
			case B_END:
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"notice vrrp %d\n",vrrid);
				notice_vrrp_state_change(vrrid,0);
				break;
			default:
				break;
		}
	}
}
void asd_update_select_mode(void *circle_ctx, void *timeout_ctx){
	struct asd_data **bss;
	struct asd_data *wasd = NULL;
	unsigned int num;
	unsigned int i;
	unsigned int j;
	unsigned int k = 0;
	struct sta_info *sta;
	if ((asd_master_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s socket %s\n",__func__,strerror(errno));
		perror("socket create");
		exit(1);
	}
	
	unsigned long ul = 1;
//	ioctl(asd_master_sock, FIONBIO, &ul);
//	fcntl(asd_master_sock, F_SETFL, O_NONBLOCK);
	if((is_secondary == 0)&&(connect(asd_master_sock, (struct sockaddr *)&B_addr, sizeof(struct sockaddr)) == -1)){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s connect %s\n",__func__,strerror(errno));
		perror("connect");
		int n = 3;
		while(n > 0){
			int ret = 0;
			fd_set fdset;
			struct timeval tv;			
			FD_ZERO(&fdset);
			FD_SET(asd_master_sock,&fdset);
			
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			ret = select(asd_master_sock + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv);
			if(ret == -1)
			{
				n--;
				continue;
			}else if(ret == 0){
				n--;
				continue;
			}else{			
				if (FD_ISSET(asd_master_sock, &fdset)){
					ul = 0;
		//			ioctl(asd_master_sock, FIONBIO, &ul);
					break;			
				}
			}
			n--;
		}
		if(n <= 0){
			close(asd_master_sock);	
			asd_master_sock = -1;
			circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
			circle_register_timeout(2, 0,asd_update_select_mode, NULL, NULL);
			return;
		}
	}

	if(is_secondary == 0){				
		bss = malloc(BSS_NUM*sizeof(struct asd_data *));				
		if(bss==NULL){
			asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
			exit(1);
		}	
		memset(bss,0,BSS_NUM*sizeof(struct asd_data *));
		num = ASD_SEARCH_ALL_STA(bss);
		for(i=0;i<num;i++){ 			
			sta = bss[i]->sta_list;
			for(j = 0; (j < bss[i]->num_sta)&&(sta!=NULL); j++){
				bak_add_sta(bss[i],sta);
				sta = sta->next;
			}				
		}
		update_end();
		free(bss);
	}	
	circle_cancel_timeout(asd_update_select_mode,NULL,NULL);
	update_pass = 0;
}
	#endif
#endif

