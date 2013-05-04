#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/uio.h>
#include "board/netlink.h"
#include "sem/product.h"
#include "board/board_define.h"
#include "hmd/hmdpub.h"
#include "hmd.h"
#include "HmdStateListen.h"
#include "HmdLog.h"
#include "HmdThread.h"
#include "HmdDbus.h"
#include "HmdMonitor.h"
int sock_n_fd = 0;
struct msghdr n_msg;
struct nlmsghdr *n_nlh = NULL;
struct sockaddr_nl n_src_addr, n_dest_addr;
struct iovec n_iov;

int sock_ks_fd = 0;
struct msghdr ks_msg;
struct nlmsghdr *ks_nlh = NULL;
struct sockaddr_nl ks_src_addr, ks_dest_addr;
struct iovec ks_iov;

#define nl_perror(str) perror(str)


void * hmd_netlink_recv_thread(void)
{
    int i = 0;
	int profile = 0;
	int j = 0;
	fd_set read_fds;
	int maxfd = 0;
	int ret = 0;
	int preAM = 0;
	int slotid = 0;
	int neighbor_slotid = 0;
	char *ifname = NULL;
	int vip = 0;
	int mask = 0;
	char mac[MAC_LEN];
	int isSem = 0;
	int type = 0;
	//int HmdTimerID = 0;
    char command[128] = {0};
	unsigned int msg_len = 0;
	unsigned int msg_offset = 0;
	hmd_pid_write_v2("HMDStateListen",HOST_SLOT_NO);
	hmd_netlink_init();
	hmd_netlink_init_for_ksem();
	while( 1 )
	{
		nl_msg_head_t* head = NULL;
		netlink_msg_t* nl_msg = NULL;
		msg_len = 0;
		msg_offset = 0;
		FD_ZERO(&read_fds);
		FD_SET(sock_n_fd, &read_fds);
		FD_SET(sock_ks_fd, &read_fds);
        maxfd = sock_n_fd > sock_ks_fd ? sock_n_fd : sock_ks_fd;
		
		ret = select(maxfd+1, &read_fds, NULL, NULL, NULL);
		if(ret < 0)
		{
			continue;		
		}
		else if(ret == 0)
		{
			continue;
		}
		else{
			
			if(FD_ISSET(sock_n_fd, &read_fds))
			{
				if(recvmsg(sock_n_fd, &n_msg, 0) < 0)
				{
					hmd_syslog_info("Failed hmd netlink recv : %s\n", strerror(errno));
				}
        		head = (nl_msg_head_t*)NLMSG_DATA(n_nlh);
				nl_msg= (netlink_msg_t*)(NLMSG_DATA(n_nlh) + sizeof(nl_msg_head_t));
				isSem = 1;
				msg_len = n_nlh->nlmsg_len;
				hmd_syslog_info("%s,%d,n_nlh->nlmsg_len: %d.\n",__func__,__LINE__,msg_len);
			}
			else if(FD_ISSET(sock_ks_fd, &read_fds)){
                if(recvmsg(sock_ks_fd, &ks_msg, 0) < 0)
                {
					hmd_syslog_info("Failed hmd knetlink recv : %s\n", strerror(errno));
        		}
				head = (nl_msg_head_t*)NLMSG_DATA(ks_nlh);
				nl_msg= (netlink_msg_t*)(NLMSG_DATA(ks_nlh) + sizeof(nl_msg_head_t));	
				isSem = 0;
				msg_len = ks_nlh->nlmsg_len;
				hmd_syslog_info("%s,%d,ks_nlh->nlmsg_len: %d.\n",__func__,__LINE__,msg_len);
			}		        
			if(head->object == NPD_MODULE || head->object == COMMON_MODULE)
			{
	            
	            for(i = 0; i < head->count; i++)
	            {
	    			if(nl_msg)
	    			{
	        			hmd_syslog_info("Npd netlink msgType(%d)\n", nl_msg->msgType);	
	        			switch(nl_msg->msgType)
	        			{
	        				case SYSTEM_STATE_NOTIFIER_EVENT:
	        					
	        					if(nl_msg->msgData.productInfo.action_type == SYSTEM_READY)
	        					{
									HmdStateReInit();
									hmd_syslog_info("Product has been ready.\n"); 	
	        					}
	        					break;
	        				case BOARD_STATE_NOTIFIER_EVENT:

	    						if(nl_msg->msgData.boardInfo.action_type == BOARD_INSERTED)
	    						{
									hmd_syslog_info("New board %d inserted.\n",nl_msg->msgData.boardInfo.slotid);
	    							slotid = nl_msg->msgData.boardInfo.slotid;
									if((slotid > 0) && (slotid < MAX_SLOT_NUM))
										HmdStateReInitSlot(slotid);
									else
										break;
									
									if(isSem){
										if(isActive){
											
											//HMDTimerRequest(HMD_CHECKING_TIMER,&(HmdTimerID), HMD_CONFIG_LOAD, slotid, 0);
											hmd_syslog_info(" %s  %d BOARD_INSERTED  isActive master board ,timerequest HMD_CONFIG_LOAD \n",__func__,__LINE__);
											#if 0
											memset(ConfigFileNew, 0, DEFAULT_LEN);
											sprintf(ConfigFileNew,"%s%d",ConfigFile,slotid);
											hmd_load_slot_config(ConfigPath, ConfigFileNew,0);
											#endif
										}
									}
									hmd_syslog_info("New board %d inserted.\n",nl_msg->msgData.boardInfo.slotid);
	    						}
	    						else if(nl_msg->msgData.boardInfo.action_type == BOARD_REMOVED)
	    						{
									hmd_syslog_info("New board %d remove.\n",nl_msg->msgData.boardInfo.slotid);
									if(isSem){
										if(isMaster)
										{
											slotid = nl_msg->msgData.boardInfo.slotid;
											if(MASTER_BACKUP_SLOT_NO == slotid){
												MASTER_BACKUP_SLOT_NO = 0;
											}
											hmd_syslog_info("%s HOST_SLOT_NO %d\n",__func__,HOST_SLOT_NO);
											{
												if(HMD_BOARD[slotid] != NULL){
													hmd_syslog_info("%s 3\n",__func__);
													for(profile = 1; profile < MAX_INSTANCE; profile++){
														if(HMD_BOARD[slotid]->Hmd_Inst[profile] != NULL){
															if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slotid)){
																syn_hansi_info_to_backup(slotid, profile, MASTER_BACKUP_SLOT_NO,0,HMD_HANSI_INFO_SYN_DEL,0);
															}											
															memset(command, 0, 128);
															sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
																		0,slotid,profile, ".pid");
															system(command);
															for(type = 0; type <  LicenseCount; type++){
																if(LICENSE_MGMT[type].r_assigned_num[slotid][profile] != 0){
																	LICENSE_MGMT[type].free_num += LICENSE_MGMT[type].r_assigned_num[slotid][profile];
																	LICENSE_MGMT[type].r_assigned_num[slotid][profile] = 0;
																	if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slotid)){
																		syn_hansi_info_to_backup(slotid, profile, MASTER_BACKUP_SLOT_NO,0,HMD_HANSI_INFO_SYN_LICENSE,type);
																	}
																}
															}
															free(HMD_BOARD[slotid]->Hmd_Inst[profile]);
															HMD_BOARD[slotid]->Hmd_Inst[profile] = NULL;
														}
														hmd_syslog_info("%s profile %d\n",__func__,profile);
														if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile] != NULL){
														{
															hmd_syslog_info("%s HMD_BOARD[preAM]->Hmd_Local_Inst[profile]->slot_num %d\n",__func__,HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num);
																if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num == 2){
																	hmd_syslog_info("%s 6\n",__func__);
																	if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no == slotid){
																		hmd_syslog_info("%s 7\n",__func__);
																		neighbor_slotid = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no1;
																	}
																	hmd_syslog_info("%s neighbor_slotid %d\n",__func__,neighbor_slotid);
																	if((HMD_BOARD[neighbor_slotid] != NULL)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]!= NULL)){
																		hmd_syslog_info("%s 17\n",__func__);
																		if((HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->InstState == 1)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive == INST_BACKUP)){
																			hmd_syslog_info("%s 18\n",__func__);
																			HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
																			HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive1 = 0;
																			HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->slot_no1 = 0;
																			HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->slot_num = 1;
																			if(HMD_L_HANSI[profile] != NULL){
																				if(HMD_L_HANSI[profile]->slot_no == slotid){
																					HMD_L_HANSI[profile]->isActive1 = INST_ACTIVE;
																					HMD_L_HANSI[profile]->slot_no = 0;
																					HMD_L_HANSI[profile]->InstState = 0;
																					HMD_L_HANSI[profile]->isActive = 0;
																				}else if(HMD_L_HANSI[profile]->slot_no == slotid){
																					HMD_L_HANSI[profile]->isActive = INST_ACTIVE;
																					HMD_L_HANSI[profile]->slot_no1 = 0;
																					HMD_L_HANSI[profile]->InstState1 = 0;
																					HMD_L_HANSI[profile]->isActive1 = 0;
																				}
																				HMD_L_HANSI[profile]->slot_num = 1;
																			}
																			
																			if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slotid)){
																				syn_hansi_info_to_backup(slotid, profile, MASTER_BACKUP_SLOT_NO,1,HMD_HANSI_INFO_SYN_DEL,0);
																			}
																			memset(command, 0, 128);
																			sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
																						1,slotid,profile, ".pid");
																			system(command);
																			if(neighbor_slotid != HOST_SLOT_NO){
																				hmd_syslog_info("%s 19\n",__func__);
																				ret = configuration_server_to_client(neighbor_slotid, profile,slotid);
																			}
																			else{								
																				hmd_syslog_info("%s 20\n",__func__);
																				
																				for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_DNum; i++){
																					ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].ifname;
																					vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].vir_ip;
																					mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mask;
																					hmd_ifname_to_mac(ifname,mac);
																					memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mac,mac,MAC_LEN);
																					hmd_ipaddr_op_withmask(ifname,vip,mask,1);
																					send_tunnel_interface_arp(mac,vip,ifname);	
																				}
																				for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_UNum; i++){
																					ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].ifname;
																					vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].vir_ip;
																					mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mask;
																					hmd_ifname_to_mac(ifname,mac);
																					memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mac,mac,MAC_LEN);
																					hmd_ipaddr_op_withmask(ifname,vip,mask,1);
																					send_tunnel_interface_arp(mac,vip,ifname);	
																				}
																				for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_GNum; i++){
																					ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].ifname;
																					vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].vir_ip;
																					mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mask;
																					hmd_ifname_to_mac(ifname,mac);
																					memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mac,mac,MAC_LEN);
																					hmd_ipaddr_op_withmask(ifname,vip,mask,1);
																					send_tunnel_interface_arp(mac,vip,ifname);	
																				}
																				notice_wid_local_hansi_service_change_state(profile, slotid);
																				hmd_syslog_info("%s 21\n",__func__);
																			}
																		}						
																	}
																}
																else{
																	if(HMD_L_HANSI[profile] != NULL){
																		free(HMD_L_HANSI[profile]);
																		HMD_L_HANSI[profile] = NULL;
																	}
																}
																free(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]);
																HMD_BOARD[slotid]->Hmd_Local_Inst[profile] = NULL;											
																for(type = 0; type <  LicenseCount; type++){
																	if(LICENSE_MGMT[type].l_assigned_num[slotid][profile] != 0){
																		LICENSE_MGMT[type].free_num += LICENSE_MGMT[type].l_assigned_num[slotid][profile];
																		LICENSE_MGMT[type].l_assigned_num[slotid][profile] = 0;									
																		if((MASTER_BACKUP_SLOT_NO != 0)&&(MASTER_BACKUP_SLOT_NO != slotid)){
																			syn_hansi_info_to_backup(slotid, profile, MASTER_BACKUP_SLOT_NO,1,HMD_HANSI_INFO_SYN_LICENSE,type);
																		}
																	}
																}
															}
														}
													}
													for(j = 0; j < MAX_INSTANCE; j++){
														if(HMD_BOARD[slotid]->L_LicenseNum[j] != NULL){
															free(HMD_BOARD[slotid]->L_LicenseNum[j]);
															HMD_BOARD[slotid]->L_LicenseNum[j] = NULL;
														}
														if(HMD_BOARD[slotid]->R_LicenseNum[j] != NULL){
															free(HMD_BOARD[slotid]->R_LicenseNum[j]);
															HMD_BOARD[slotid]->R_LicenseNum[j] = NULL;
														}														
													}
													free(HMD_BOARD[slotid]);
													HMD_BOARD[slotid] = NULL;
												}
											}
										}
									}
	    						}
								break;								
							case ACTIVE_STDBY_SWITCH_EVENT:
								hmd_syslog_info("Active master board %d inserted.\n",nl_msg->msgData.boardInfo.slotid);
								preAM = MASTER_SLOT_NO;
								slotid = MASTER_SLOT_NO;
								MASTER_SLOT_NO = nl_msg->msgData.boardInfo.slotid;
								HOST_BOARD->tipcaddr.addr.name.name.instance = SERVER_BASE_INST+MASTER_SLOT_NO;
								hmd_syslog_info("%s isMaster %d\n",__func__,isMaster);
								if(isMaster){
									hmd_syslog_info("%s HOST_SLOT_NO %d\n",__func__,HOST_SLOT_NO);
									if(HOST_SLOT_NO == nl_msg->msgData.boardInfo.slotid){
										isActive = 1;
										hmd_syslog_info("%s preAM %d\n",__func__,preAM);
										if(HMD_BOARD[preAM] != NULL){
											hmd_syslog_info("%s 3\n",__func__);
											for(profile = 1; profile < MAX_INSTANCE; profile++){
												if(HMD_BOARD[preAM]->Hmd_Inst[profile] != NULL){
													
													memset(command, 0, 128);
													sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
																0,preAM,profile, ".pid");
													system(command);
													for(type = 0; type <  LicenseCount; type++){
														if(LICENSE_MGMT[type].r_assigned_num[preAM][profile] != 0){
															LICENSE_MGMT[type].free_num += LICENSE_MGMT[type].r_assigned_num[preAM][profile];
															LICENSE_MGMT[type].r_assigned_num[preAM][profile] = 0;
														}
													}
													free(HMD_BOARD[preAM]->Hmd_Inst[profile]);
													HMD_BOARD[preAM]->Hmd_Inst[profile] = NULL;
												}
												hmd_syslog_info("%s profile %d\n",__func__,profile);
												if(HMD_BOARD[preAM]->Hmd_Local_Inst[profile] != NULL){
												{
													hmd_syslog_info("%s HMD_BOARD[preAM]->Hmd_Local_Inst[profile]->slot_num %d\n",__func__,HMD_BOARD[preAM]->Hmd_Local_Inst[profile]->slot_num);
														if(HMD_BOARD[preAM]->Hmd_Local_Inst[profile]->slot_num == 2){
															hmd_syslog_info("%s 6\n",__func__);
															if(HMD_BOARD[preAM]->Hmd_Local_Inst[profile]->slot_no == preAM){
																hmd_syslog_info("%s 7\n",__func__);
																neighbor_slotid = HMD_BOARD[preAM]->Hmd_Local_Inst[profile]->slot_no1;
															}
															hmd_syslog_info("%s neighbor_slotid %d\n",__func__,neighbor_slotid);
															if((HMD_BOARD[neighbor_slotid] != NULL)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]!= NULL)){
																hmd_syslog_info("%s 17\n",__func__);
																if((HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->InstState == 1)&&(HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive == INST_BACKUP)){
																	hmd_syslog_info("%s 18\n",__func__);
																	HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive = INST_ACTIVE;
																	HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->isActive1 = 0;
																	HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->slot_no1 = 0;
																	HMD_BOARD[neighbor_slotid]->Hmd_Local_Inst[profile]->slot_num = 1;
																	if(HMD_L_HANSI[profile] != NULL){
																		if(HMD_L_HANSI[profile]->slot_no == slotid){
																			HMD_L_HANSI[profile]->isActive1 = INST_ACTIVE;
																			HMD_L_HANSI[profile]->slot_no = 0;
																			HMD_L_HANSI[profile]->InstState = 0;
																			HMD_L_HANSI[profile]->isActive = 0;
																		}else if(HMD_L_HANSI[profile]->slot_no == slotid){
																			HMD_L_HANSI[profile]->isActive = INST_ACTIVE;
																			HMD_L_HANSI[profile]->slot_no1 = 0;
																			HMD_L_HANSI[profile]->InstState1 = 0;
																			HMD_L_HANSI[profile]->isActive1 = 0;
																		}
																		HMD_L_HANSI[profile]->slot_num = 1;
																	}
																	if(neighbor_slotid != HOST_SLOT_NO){
																		hmd_syslog_info("%s 19\n",__func__);
																		ret = configuration_server_to_client(neighbor_slotid, profile,slotid);
																	}
																	else{								
																		hmd_syslog_info("%s 20\n",__func__);
																		
																		for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_DNum; i++){
																			ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].ifname;
																			vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].vir_ip;
																			mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mask;
																			hmd_ifname_to_mac(ifname,mac);
																			memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Downlink[i].mac,mac,MAC_LEN);
																			hmd_ipaddr_op_withmask(ifname,vip,mask,1);
																			send_tunnel_interface_arp(mac,vip,ifname);	
																		}
																		for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_UNum; i++){
																			ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].ifname;
																			vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].vir_ip;
																			mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mask;
																			hmd_ifname_to_mac(ifname,mac);
																			memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Uplink[i].mac,mac,MAC_LEN);
																			hmd_ipaddr_op_withmask(ifname,vip,mask,1);
																			send_tunnel_interface_arp(mac,vip,ifname);	
																		}
																		for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[profile]->Inst_GNum; i++){
																			ifname = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].ifname;
																			vip = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].vir_ip;
																			mask = HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mask;
																			hmd_ifname_to_mac(ifname,mac);
																			memcpy(HOST_BOARD->Hmd_Local_Inst[profile]->Inst_Gateway[i].mac,mac,MAC_LEN);
																			hmd_ipaddr_op_withmask(ifname,vip,mask,1);
																			send_tunnel_interface_arp(mac,vip,ifname);	
																		}
																		notice_wid_local_hansi_service_change_state(profile, slotid);
																		notice_eag_local_hansi_service_change_state(profile, slotid);
																		hmd_syslog_info("%s 21\n",__func__);
																	}
																}						
															}
														}
														else{
															if(HMD_L_HANSI[profile] != NULL){
																free(HMD_L_HANSI[profile]);
																HMD_L_HANSI[profile] = NULL;
															}
														}
														
														memset(command, 0, 128);
														sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
																	1,preAM,profile, ".pid");
														system(command);
														for(type = 0; type <  LicenseCount; type++){
															if(LICENSE_MGMT[type].l_assigned_num[preAM][profile] != 0){
																LICENSE_MGMT[type].free_num += LICENSE_MGMT[type].l_assigned_num[preAM][profile];
																LICENSE_MGMT[type].l_assigned_num[preAM][profile] = 0;
															}
														}
														free(HMD_BOARD[preAM]->Hmd_Local_Inst[profile]);
														HMD_BOARD[preAM]->Hmd_Local_Inst[profile] = NULL;
													}
												}
											}
											
											for(j = 0; j < MAX_INSTANCE; j++){
												if(HMD_BOARD[preAM]->L_LicenseNum[j] != NULL){
													free(HMD_BOARD[preAM]->L_LicenseNum[j]);
													HMD_BOARD[preAM]->L_LicenseNum[j] = NULL;
												}
												if(HMD_BOARD[preAM]->R_LicenseNum[j] != NULL){
													free(HMD_BOARD[preAM]->R_LicenseNum[j]);
													HMD_BOARD[preAM]->R_LicenseNum[j] = NULL;
												}														
											}
											free(HMD_BOARD[preAM]);
											HMD_BOARD[preAM] = NULL;
										}
									}
								}
								break;
	        				default:
	        					hmd_syslog_info("Error:hmd recv an error message type\n");
	        					break;
	        			}
						if((msg_offset + sizeof(netlink_msg_t)) <= msg_len){
	        			nl_msg = nl_msg + sizeof(netlink_msg_t);
							msg_offset += sizeof(netlink_msg_t);
						}else{
							hmd_syslog_err("%s,%d,msg_len=%d,msg_offset=%d,nl_msg->msgType=%d.\n",__func__,__LINE__,msg_len,msg_offset,nl_msg->msgType);	
							break;
						}
	    			}
	            }
			}			
		}
	}
	

}



int hmd_netlink_init(void)
{
	/* Initialize data field */
	memset(&n_src_addr, 0, sizeof(n_src_addr));
	memset(&n_dest_addr, 0, sizeof(n_dest_addr));
	memset(&n_iov, 0, sizeof(n_iov));
	memset(&n_msg, 0, sizeof(n_msg));
	
	/* Create netlink socket use NETLINK_SELINUX(7) */
	if ((sock_n_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_DISTRIBUTED)) < 0) {
		hmd_syslog_info("Failed socket\n");
		return -1;
	}

	/* Fill in src_addr */
	n_src_addr.nl_family = AF_NETLINK;
	n_src_addr.nl_pid = getpid();
	/* Focus */
	n_src_addr.nl_groups = 1;

	if (bind(sock_n_fd, (struct sockaddr*)&n_src_addr, sizeof(n_src_addr)) < 0) {
		hmd_syslog_info("Failed bind\n");
		return -1;
	}

	/* Fill in dest_addr */
	n_dest_addr.nl_pid = 0;
	n_dest_addr.nl_family = AF_NETLINK;
	/* Focus */
	n_dest_addr.nl_groups = 1;

	/* Initialize buffer */
	if((n_nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL) {
		hmd_syslog_info("Failed malloc\n");
		return -1;
	}


	memset(n_nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	n_iov.iov_base = (void *)n_nlh;
	n_iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	n_msg.msg_name = (void *)&n_dest_addr;
	n_msg.msg_namelen = sizeof(n_dest_addr);
	n_msg.msg_iov = &n_iov;
	n_msg.msg_iovlen = 1;	
	return 0;
}

int hmd_netlink_init_for_ksem(void)
{
	/* Initialize data field */
	memset(&ks_src_addr, 0, sizeof(ks_src_addr));
	memset(&ks_dest_addr, 0, sizeof(ks_dest_addr));
	memset(&ks_iov, 0, sizeof(ks_iov));
	memset(&ks_msg, 0, sizeof(ks_msg));
	
	/* Create netlink socket use NETLINK_KSEM(19) */
	if ((sock_ks_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KSEM)) < 0) {
		hmd_syslog_info("Failed socket\n");
		return -1;
	}

	/* Fill in src_addr */
	ks_src_addr.nl_family = AF_NETLINK;
	ks_src_addr.nl_pid = getpid();
	/* Focus */
	ks_src_addr.nl_groups = 1;

	if (bind(sock_ks_fd, (struct sockaddr*)&ks_src_addr, sizeof(ks_src_addr)) < 0) {
		hmd_syslog_info("Failed bind\n");
		return -1;
	}

	/* Fill in dest_addr */
	ks_dest_addr.nl_pid = 0;
	ks_dest_addr.nl_family = AF_NETLINK;
	/* Focus */
	ks_dest_addr.nl_groups = 1;

	/* Initialize buffer */
	if((ks_nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL) {
		hmd_syslog_info("Failed malloc\n");
		return -1;
	}

	memset(ks_nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	ks_iov.iov_base = (void *)ks_nlh;
	ks_iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	ks_msg.msg_name = (void *)&ks_dest_addr;
	ks_msg.msg_namelen = sizeof(ks_dest_addr);
	ks_msg.msg_iov = &ks_iov;
	ks_msg.msg_iovlen = 1;
	
	return 0;
}



int hmd_rtnl_open(struct rtnl_handle *rth, unsigned subscriptions)
{
	unsigned int addr_len;

	memset(rth, 0, sizeof(struct rtnl_handle));

	rth->fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (rth->fd < 0) {
		nl_perror("Cannot open netlink socket");
		return -1;
	}

	memset(&rth->local, 0, sizeof(rth->local));
	rth->local.nl_family = AF_NETLINK;
	rth->local.nl_groups = subscriptions;

	if (bind(rth->fd, (struct sockaddr*)&rth->local, sizeof(rth->local)) < 0) {
		nl_perror("Cannot bind netlink socket");
		close(rth->fd);
		return -1;
	}
	addr_len = sizeof(rth->local);
	if (getsockname(rth->fd, (struct sockaddr*)&rth->local, (socklen_t *)&addr_len) < 0) {
		nl_perror("Cannot getsockname");
		close(rth->fd);
		return -1;
	}
	if (addr_len != sizeof(rth->local)) {
		fprintf(stderr, "Wrong address length %d\n", addr_len);
		close(rth->fd);
		return -1;
	}
	if (rth->local.nl_family != AF_NETLINK) {
		fprintf(stderr, "Wrong address family %d\n", rth->local.nl_family);
		close(rth->fd);
		return -1;
	}
	rth->seq = time(NULL);
	return 0;
}


int hmd_rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, pid_t peer,
	      unsigned groups, struct nlmsghdr *answer,
	      int (*junk)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
	      void *jarg)
{
	int status;
	struct nlmsghdr *h;
	struct sockaddr_nl nladdr;
	struct iovec iov = { (void*)n, n->nlmsg_len };
	char   buf[8192];
	struct msghdr msg = {
		(void*)&nladdr, sizeof(nladdr),
		&iov,	1,
		NULL,	0,
		0
	};

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = peer;
	nladdr.nl_groups = groups;

	n->nlmsg_seq = ++rtnl->seq;
	if (answer == NULL)
		n->nlmsg_flags |= NLM_F_ACK;

	status = sendmsg(rtnl->fd, &msg, 0);

	if (status < 0) {
		nl_perror("Cannot talk to rtnetlink");
		return -1;
	}

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	while (1) {
		status = recvmsg(rtnl->fd, &msg, 0);

		if (status < 0) {
			if (errno == EINTR)
				continue;
			nl_perror("OVERRUN");
			continue;
		}
		if (status == 0) {
			fprintf(stderr, "EOF on netlink\n");
			return -1;
		}
		if (msg.msg_namelen != sizeof(nladdr)) {
			fprintf(stderr, "sender address length == %d\n", msg.msg_namelen);
			exit(1);
		}
		for (h = (struct nlmsghdr*)buf; status >= sizeof(*h); ) {
			int err;
			int len = h->nlmsg_len;
			pid_t pid=h->nlmsg_pid;
			int l = len - sizeof(*h);
			unsigned seq=h->nlmsg_seq;

			if (l<0 || len>status) {
				if (msg.msg_flags & MSG_TRUNC) {
					fprintf(stderr, "Truncated message\n");
					return -1;
				}
				fprintf(stderr, "!!!malformed message: len=%d\n", len);
				exit(1);
			}

			if (h->nlmsg_pid != pid || h->nlmsg_seq != seq) {
				if (junk) {
					err = junk(&nladdr, h, jarg);
					if (err < 0)
						return err;
				}
				continue;
			}

			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
				if (l < sizeof(struct nlmsgerr)) {
					fprintf(stderr, "ERROR truncated\n");
				} else {
					errno = -err->error;
					if (errno == 0) {
						if (answer)
							memcpy(answer, h, h->nlmsg_len);
						return 0;
					}
					/* Scott checked for EEXIST and return 0! 9-4-02*/
					if (errno != EEXIST)
						nl_perror("RTNETLINK answers");
					else return 0;
				}
				return -1;
			}
			if (answer) {
				memcpy(answer, h, h->nlmsg_len);
				return 0;
			}

			fprintf(stderr, "Unexpected reply!!!\n");

			status -= NLMSG_ALIGN(len);
			h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
		}
		if (msg.msg_flags & MSG_TRUNC) {
			fprintf(stderr, "Message truncated\n");
			continue;
		}
		if (status) {
			fprintf(stderr, "!!!Remnant of size %d\n", status);
			exit(1);
		}
	}
}

int hmd_rtnl_close(struct rtnl_handle *rth)
{
	/* close the fd */
	close( rth->fd );
	return(0);
}


int hmd_addattr_l(struct nlmsghdr *n, int maxlen, int type, void *data, int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;
	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
	return 0;
}


int hmd_ifname_to_idx( char *ifname )
{
	struct ifreq	ifr;	
	int		fd	= socket(AF_INET, SOCK_DGRAM, 0);
	int		ifindex = -1;
	if (fd < 0) 	return (-1);
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFINDEX, (char *)&ifr) == 0){
		ifindex = ifr.ifr_ifindex;
	}
	close(fd);
	return ifindex;
}
int hmd_ifname_to_mac( char *ifname, char * mac)
{
	struct ifreq	ifr;	
	int		fd	= socket(AF_INET, SOCK_DGRAM, 0);
	int		ifindex = -1;
	if (fd < 0) 	return (-1);
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if(ioctl(fd, SIOCGIFHWADDR, &ifr) == -1){
		close(fd);
		return -1;
	}
	memcpy(mac,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
	close(fd);
	return ifindex;
}


int hmd_ipaddr_op_withmask( char *ifname , unsigned int addr, int mask,int addF )
{
	struct rtnl_handle	rth;
	struct {
		struct nlmsghdr 	n;
		struct ifaddrmsg 	ifa;
		char   			buf[256];
	} req;
	char name[IF_NAMESIZE] = {0};
	int		ifindex = hmd_ifname_to_idx(ifname);
	if(ifindex == -1){
		return -1;
	}
	
	hmd_syslog_info("start ip op: ifindex %d,ipaddr %02x,%s\n",ifindex,addr,addF ? "RTM_NEWADDR" : "RTM_DELADDR");
	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len		= NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags	= NLM_F_REQUEST;
	req.n.nlmsg_type	= addF ? RTM_NEWADDR : RTM_DELADDR;
	req.ifa.ifa_family	= AF_INET;
	req.ifa.ifa_index	= ifindex;
	req.ifa.ifa_prefixlen	= mask;
	
	addr = htonl( addr );
	hmd_addattr_l(&req.n, sizeof(req), IFA_LOCAL, &addr, sizeof(addr) );

	if (hmd_rtnl_open(&rth, 0) < 0){
        hmd_syslog_info("open netlink socket failed when %s ip %#x mask %d on %s!\n", \
						addF ? "add" : "delete", addr, mask, if_indextoname(ifindex, name) ? name : "nil");
		return -1;
	}
	if (hmd_rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0){
        hmd_syslog_info("netlink talk failed when %s ip %#x mask %d on %s!\n", \
						addF ? "add" : "delete", addr, mask, if_indextoname(ifindex, name) ? name : "nil");
		return -1;
    }
	
	/* to close the clocket */
 	hmd_rtnl_close( &rth );

	return(0);
}


