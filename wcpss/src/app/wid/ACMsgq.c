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
* ACMsgq.c
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
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "CWAC.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "CWCommon.h"
#include "ACDbus.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "ACDbus_handler.h"



void WID_INSERT_CONTROL_LIST(unsigned int WTPID, struct msgqlist *elem)
{
	if(AC_WTP[WTPID] == NULL)
	{
		return;
	}
	CWThreadMutexLock(&(gWTPs[WTPID].mutex_controlList));
	struct msgqlist *tmp = NULL;
	if(AC_WTP[WTPID]->elem_num >= 500)//qiuchen change it 2012.10.26
	{
		CWThreadMutexUnlock(&(gWTPs[WTPID].mutex_controlList));
		WID_CLEAN_CONTROL_LIST(WTPID);
		WID_CONFIG_SAVE(WTPID);
		CWThreadMutexLock(&(gWTPs[WTPID].mutex_controlList)); 
	}	
	if(AC_WTP[WTPID]->ControlList == NULL){
		AC_WTP[WTPID]->ControlList = elem;
		elem->next = NULL;
		AC_WTP[WTPID]->elem_num++;
		goto out;
	}
	tmp = AC_WTP[WTPID]->ControlList;
	while(tmp->next != NULL){
		tmp = tmp->next;
	}
	tmp->next = elem;
	elem->next = NULL;
	AC_WTP[WTPID]->elem_num++;
out:
	CWThreadMutexUnlock(&(gWTPs[WTPID].mutex_controlList));
	return;
}
/*fengwenchao add 20121214 for new config save*/
void WID_WLAN_CONFIG_SAVE(unsigned int WTPIndex,unsigned int local_radio)
{
	int k =0;
	msgq msg;
	struct msgqlist *elem =NULL;	
	unsigned int bssindex =0;	
	struct wds_bssid *wds = NULL;
	msgq msg2;
	struct msgqlist *elem2;	
	msgq msg4 ;
	struct msgqlist *elem4 = NULL; 						
	char *command = NULL;
	char *ath_str = NULL;
	unsigned char pcy = 0;
	char buf[DEFAULT_LEN];
	if(AC_WTP[WTPIndex] == NULL)
	{
		wid_syslog_err("%s WTPIndex %d is NULL\n",__func__,WTPIndex);
		return;
	}

	for(k = 0;k < WLAN_NUM; k++)
	{
		{
		AC_WTP[WTPIndex]->CMD->radiowlanid[local_radio][k] = 0;
		if((AC_WLAN[k] != NULL)&&(AC_WLAN[k]->Status == 0) && (AC_WLAN[k]->want_to_delete == 0))	// Huangleilei add it for AXSSZFI-1622
		{
			wid_syslog_debug_debug(WID_DEFAULT,"***wtp index is %d\n",AC_WTP[WTPIndex]->BindingSystemIndex);
			{
	#ifdef _CheckBindingIf_
			if(AC_WLAN[k]->Wlan_Ifi != NULL)
			{
				struct ifi * wlan_ifi = AC_WLAN[k]->Wlan_Ifi;
				while(wlan_ifi != NULL)
				{
					wid_syslog_debug_debug(WID_DEFAULT,"*** wlan index is %d\n",wlan_ifi->ifi_index);
					if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->BindingSystemIndex == wlan_ifi->ifi_index))
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
						if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->WTP_Radio[local_radio])
							&&(AC_WTP[WTPIndex]->WTP_Radio[local_radio]->isBinddingWlan== 1))
						{
							struct wlanid *wlan_id = AC_WTP[WTPIndex]->WTP_Radio[local_radio]->Wlan_Id;
							while(wlan_id != NULL)
							{	
								if(wlan_id->wlanid == k)
								{
									break;
								}
								wlan_id = wlan_id->next;
							}
							if((wlan_id != NULL)&&(AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->isused == 1))
							{

								wid_syslog_debug_debug(WID_DEFAULT,"*** wtp binding wlan id match success**\n");
								memset((char*)&msg, 0, sizeof(msg));
								msg.mqid = WTPIndex%THREAD_NUM+1;
								msg.mqinfo.WTPID = WTPIndex;
								msg.mqinfo.type = CONTROL_TYPE;
								msg.mqinfo.subtype = WLAN_S_TYPE;
								msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_ADD;
								msg.mqinfo.u.WlanInfo.WLANID = k;
								msg.mqinfo.u.WlanInfo.Radio_L_ID = local_radio;//zhanglei wait for M-radio

								msg.mqinfo.u.WlanInfo.HideESSid = AC_WLAN[k]->HideESSid;
								memset(msg.mqinfo.u.WlanInfo.WlanKey,0,DEFAULT_LEN);
								memcpy(msg.mqinfo.u.WlanInfo.WlanKey,AC_WLAN[k]->WlanKey,DEFAULT_LEN);
								msg.mqinfo.u.WlanInfo.KeyLen = AC_WLAN[k]->KeyLen;
								msg.mqinfo.u.WlanInfo.SecurityType = AC_WLAN[k]->SecurityType;
								msg.mqinfo.u.WlanInfo.SecurityIndex = AC_WLAN[k]->SecurityIndex;
								msg.mqinfo.u.WlanInfo.asic_hex = AC_WLAN[k]->asic_hex;/* 0 asic; 1 hex*/
								msg.mqinfo.u.WlanInfo.Roaming_Policy = AC_WLAN[k]->Roaming_Policy;		/*Roaming (1 enable /0 disable)*/
								memset(msg.mqinfo.u.WlanInfo.WlanEssid,0,ESSID_LENGTH);
								//memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[k]->ESSID,ESSID_LENGTH);
								if(wlan_id->ESSID)
								{
									wid_syslog_debug_debug(WID_DEFAULT,"$$$$$$$$$$ wtp online !\n");
									wid_syslog_debug_debug(WID_DEFAULT,"ESSID = %s\n",wlan_id->ESSID);
									memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,wlan_id->ESSID,strlen(wlan_id->ESSID));
								}
								else
								{
									memcpy(msg.mqinfo.u.WlanInfo.WlanEssid,AC_WLAN[k]->ESSID,strlen(AC_WLAN[k]->ESSID));
								}
								msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio];
								
								elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
								if(elem == NULL){
									perror("malloc");
									return ;
								}
								memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
								elem->next = NULL;
								memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
								WID_INSERT_CONTROL_LIST(WTPIndex, elem);
								/*xm addd 09.5.13*/
								if((AC_WLAN[wlan_id->wlanid]!=NULL)&&(AC_WLAN[wlan_id->wlanid]->balance_switch==1)){

									char *command=NULL;
									command = (char *)malloc(sizeof(char)*50);
									memset(command,0,50);
									strncpy(command,"echo 1 > /proc/sys/dev/wifi0/traffic_balance",44);
									set_balance_probe_extension_command(WTPIndex,command);
									free(command);
								}
								//wds state 
								if((check_bssid_func(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]))
									&&(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->WDSStat == WDS_ANY))
								{
									msg2.mqid = WTPIndex%THREAD_NUM +1;
									msg2.mqinfo.WTPID = WTPIndex;
									msg2.mqinfo.type = CONTROL_TYPE;
									msg2.mqinfo.subtype = WDS_S_TYPE;
									msg2.mqinfo.u.WlanInfo.Wlan_Op = WLAN_WDS_ENABLE;
									msg2.mqinfo.u.WlanInfo.WLANID = k;
									msg2.mqinfo.u.WlanInfo.Radio_L_ID = local_radio;
									
									elem2 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
									if(elem2 == NULL){
										perror("malloc");
										return;
									}
									memset((char*)&(elem2->mqinfo), 0, sizeof(msgqdetail));
									elem2->next = NULL;
									memcpy((char*)&(elem2->mqinfo),(char*)&(msg2.mqinfo),sizeof(msg2.mqinfo));
									WID_INSERT_CONTROL_LIST(WTPIndex, elem2);
								}	
								//wds state end
								else{
									bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio];
									if((check_bssid_func(bssindex))&&(AC_BSS[bssindex]!=NULL) && (AC_BSS[bssindex]->WDSStat == WDS_SOME)){
										wds = AC_BSS[bssindex]->wds_bss_list;
										while(wds != NULL){
											memset(buf,0,DEFAULT_LEN);
											sprintf(buf,"/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",AC_BSS[bssindex]->Radio_L_ID,k,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
											printf("/usr/sbin/wds_add ath.%d-%d %02X:%02X:%02X:%02X:%02X:%02X\n",AC_BSS[bssindex]->Radio_L_ID,k,wds->BSSID[0],wds->BSSID[1],wds->BSSID[2],wds->BSSID[3],wds->BSSID[4],wds->BSSID[5]);
											wid_radio_set_extension_command(WTPIndex,buf);
											wds = wds->next;
										}
									}
								}
								if((check_bssid_func(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]))
									&&(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->ath_l2_isolation == 1))
								{
									bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio];
									memset(buf,0,DEFAULT_LEN);
									sprintf(buf,"iwpriv ath.%d-%d ap_bridge 0\n",AC_BSS[bssindex]->Radio_L_ID,AC_BSS[bssindex]->WlanID);
									wid_syslog_debug_debug(WID_DEFAULT,"iwpriv ath.%d-%d ap_bridge 0\n",AC_BSS[bssindex]->Radio_L_ID,AC_BSS[bssindex]->WlanID);
									wid_radio_set_extension_command(WTPIndex,buf);
								}
								
								/*traffic limit*/
								if((check_bssid_func(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]))
									&&(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->traffic_limit_able == 1))
								{
//									unsigned int value = 0;
									bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio];
									if(!check_bssid_func(bssindex)){
										wid_syslog_err("<error> %s\n",__func__);
										//return ;
									}else{
										WID_Save_Traffic_Limit(bssindex, WTPIndex);
										if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->multi_user_optimize_switch == 1))
										{
											char wlanid =AC_BSS[bssindex]->WlanID;
											int radioid = AC_BSS[bssindex]->Radio_G_ID;
											muti_user_optimize_switch(wlanid,radioid,1);											
										}
									}
								}

								/*ip mac binding*/
								if((check_bssid_func(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]))
									&&(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->ip_mac_binding == 1))
								{
									//unsigned int value = 0;
									
									memset(buf,0,DEFAULT_LEN);
									bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]; //fengwenchao add 20120517 for autelan-2970
									sprintf(buf,"/usr/sbin/set_ip_enable ath.%d-%d %d",AC_BSS[bssindex]->Radio_L_ID,AC_BSS[bssindex]->WlanID,1);
									
									wid_syslog_debug_debug(WID_DEFAULT,"wid_set_sta_ip_mac_binding apcmd %s\n",buf);
									wid_radio_set_extension_command(WTPIndex,buf);
								}
								if((check_bssid_func(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]))
									&&((AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->unicast_sw == 1)\
									||(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->muti_bro_cast_sw == 1)
									||(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->muti_rate != 10))){
								
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
									setWtpUniMutiBroCastIsolation(WTPIndex,local_radio,k,pcy);
									wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,WTPIndex,local_radio,k,(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]));
								}
								if((check_bssid_func(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]))
									&&(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->noResToStaProReqSW == 1))
								{
									setWtpNoRespToStaProReq(WTPIndex,local_radio,k,AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->noResToStaProReqSW);
									wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,WTPIndex,local_radio,k,(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]));
								}
								if((check_bssid_func(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]))
									&&(AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->muti_rate != 10))
								{
									setWtpUniMutiBroCastRate(WTPIndex,local_radio,k,AC_BSS[(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio])]->muti_rate);
									wid_syslog_debug_debug(WID_DBUS,"%s,%d,radio %d-%d wlan:%d,bssindex=%d.config save.\n",__func__,__LINE__,WTPIndex,local_radio,k,(AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio]));
								}

								//wei chao add flow check 2011.11.03
								
								if((AC_WLAN[k]->SecurityType == IEEE8021X)||(AC_WLAN[k]->SecurityType ==WPA_E)
									||(AC_WLAN[k]->SecurityType ==WPA2_E)||(AC_WLAN[k]->SecurityType ==MD5))
								{
									msgq msg3;
									struct msgqlist *elem3;
									
									msg2.mqinfo.u.WlanInfo.WLANID = k;
									msg2.mqinfo.u.WlanInfo.Radio_L_ID = local_radio;

									msg3.mqid = WTPIndex%THREAD_NUM +1;
									msg3.mqinfo.WTPID = WTPIndex;			
									msg3.mqinfo.type = CONTROL_TYPE;
									msg3.mqinfo.subtype = WTP_S_TYPE;
									msg3.mqinfo.u.WtpInfo.Wtp_Op = WTP_FLOW_CHECK;
									msg3.mqinfo.u.WlanInfo.WLANID = k;
									msg3.mqinfo.u.WlanInfo.Radio_L_ID = local_radio;
									msg3.mqinfo.u.WlanInfo.flow_check = AC_WLAN[k]->flow_check;
									msg3.mqinfo.u.WlanInfo.no_flow_time = AC_WLAN[k]->no_flow_time;
									msg3.mqinfo.u.WlanInfo.limit_flow = AC_WLAN[k]->limit_flow;
								//weichao add 
								elem3 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
								if(elem3 == NULL){			
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									perror("malloc");
									return ;
								}
								
								memset((char*)&(elem3->mqinfo), 0, sizeof(msgqdetail));
								elem3->next = NULL;
								memcpy((char*)&(elem3->mqinfo),(char*)&(msg3.mqinfo),sizeof(msg3.mqinfo));
								WID_INSERT_CONTROL_LIST(WTPIndex, elem3);
								}
								
								//weichao add 
								if(AC_WLAN[k]->Status == 0){
								command = (char *)malloc(sizeof(char)*100);
								if(NULL == command)
								{							
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									return ;
								}
								ath_str = (char *)malloc(sizeof(char)*20);
								if(NULL == ath_str)
								{
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									return ;
								}
								memset(command,0,100);
								memset(ath_str,0,20);
								memset(&msg4,0,sizeof(msg4));
								msg4.mqid = WTPIndex%THREAD_NUM +1;
								msg4.mqinfo.WTPID = WTPIndex;			
								msg4.mqinfo.type = CONTROL_TYPE;
								msg4.mqinfo.subtype = WTP_S_TYPE;
								msg4.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;
								
								sprintf(ath_str,"ath.%d-%d",local_radio,k);
								sprintf(command,"ifconfig %s down;iwpriv %s inact %u;ifconfig %s up",ath_str,ath_str,AC_WLAN[k]->ap_max_inactivity,ath_str);
								memcpy(msg4.mqinfo.u.WtpInfo.value, command, strlen(command));
								elem4 = (struct msgqlist*)malloc(sizeof(struct msgqlist));
								if(elem4 == NULL){			
									wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
									perror("malloc");
									free(command);
									command = NULL;
									free(ath_str);
									ath_str = NULL;
									return ;
								}
								
								memset((char*)&(elem4->mqinfo), 0, sizeof(msgqdetail));
								elem4->next = NULL;
								memcpy((char*)&(elem4->mqinfo),(char*)&(msg4.mqinfo),sizeof(msg4.mqinfo));
								WID_INSERT_CONTROL_LIST(WTPIndex, elem4);
								free(command);
								command = NULL;
								free(ath_str);
								ath_str = NULL;
								}
								/* send eap switch & mac to ap,zhangshu add 2010-10-22 */
								bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio];
								char apcmd[WID_SYSTEM_CMD_LENTH];
								memset(apcmd,0,WID_SYSTEM_CMD_LENTH);

								if((AC_WLAN[k]->eap_mac_switch==1)&&(AC_WLAN[k]->wlan_if_policy==NO_INTERFACE)
									&&(AC_BSS[bssindex]!=NULL)&&(AC_BSS[bssindex]->BSS_IF_POLICY==NO_INTERFACE))
								{
									sprintf(apcmd,"set_eap_mac ath.%d-%d %s",local_radio,k,AC_WLAN[k]->eap_mac);
									wid_syslog_debug_debug(WID_DEFAULT,"Enable Wlan: set eap mac cmd %s\n",apcmd);
									wid_radio_set_extension_command(WTPIndex,apcmd);
								}
								else
								{
								   // sprintf(apcmd,"set_eap_mac ath.%d-%d 0",m,k);
								}

								int bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio];
								if((AC_BSS[bssindex])&&(AC_BSS[bssindex]->BSS_IF_POLICY != NO_INTERFACE)
									&&(AC_BSS[bssindex]->BSS_TUNNEL_POLICY != CW_802_DOT_11_TUNNEL)){
									msgq msg;
									struct msgqlist *elem;
									memset((char*)&msg, 0, sizeof(msg));
									wid_syslog_debug_debug(WID_DEFAULT,"*** %s,%d.**\n",__func__,__LINE__);
									msg.mqid = WTPIndex%THREAD_NUM+1;
									msg.mqinfo.WTPID = WTPIndex;
									msg.mqinfo.type = CONTROL_TYPE;
									msg.mqinfo.subtype = WLAN_S_TYPE;
									msg.mqinfo.u.WlanInfo.Wlan_Op = WLAN_CHANGE_TUNNEL;
									msg.mqinfo.u.WlanInfo.WLANID = k;
									msg.mqinfo.u.WlanInfo.Radio_L_ID = local_radio;
									
									msg.mqinfo.u.WlanInfo.bssindex = AC_WLAN[k]->S_WTP_BSS_List[WTPIndex][local_radio];
									
									if((AC_WTP[WTPIndex]->WTPStat == 5)){ 
										if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
											wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
											perror("msgsnd");
										}
									}
									else{
										elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
										if(elem == NULL){
											wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
											perror("malloc");
											return ;
										}
										memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
										elem->next = NULL;
										memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
										WID_INSERT_CONTROL_LIST(WTPIndex, elem);
									}
								}						
								/* end */
							}

						}
						else
						{
							continue;
						}
					}
				}
			}
		}
	}
		
	}
	  
}
		
/*fengwenchao add end*/
void WID_CONFIG_SAVE(unsigned int WTPIndex){
	int i = 0;
	char buf[DEFAULT_LEN];
	unsigned char eth_index = 0;
	msgq msg;
	struct msgqlist *elem;	

	for(i=0; i<AC_WTP[WTPIndex]->RadioCount; i++){		

		WID_WLAN_CONFIG_SAVE(WTPIndex,i);//fengwenchao add 20121214 for new config save
		
		if(AC_WTP[WTPIndex]->WTP_Radio[i]->REFlag == 1){
			wid_wtp_radio_extern_command_check(WTPIndex,i);
		}
		if(1 == AC_WTP[WTPIndex]->radio_5g_sw){
			if(AC_WTP[WTPIndex]->RadioCount >= 2){
				set_wtp_5g_switch(WTPIndex,1);
			}else{
				wid_syslog_warning("%s,%d,WTP[%d] need set 5g enable,but it is single radio,will not send.\n",__func__,__LINE__,WTPIndex);
			}
		}
		
		if (strcmp((char *)AC_WTP[WTPIndex]->longitude, "") != 0 || strcmp((char *)AC_WTP[WTPIndex]->latitude, "") != 0 ) {
			wid_set_ap_longitude_latitude(WTPIndex, AC_WTP[WTPIndex]->longitude, AC_WTP[WTPIndex]->latitude);
		}
		
		/*11n parameter set,zhangshu modify 2010-11-25*/
		if((AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Type & IEEE80211_11N) == IEEE80211_11N){
            unsigned int Radio_ID = WTPIndex * L_RADIO_NUM + i;
		    if(AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.Able != 1){
    			wid_radio_set_ampdu_able(Radio_ID, 1);
    		}
    		if((AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.Able == 1)&&(AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.AmpduLimit != 65535)){
    			wid_radio_set_ampdu_limit(Radio_ID, 1);
    		}
    		if((AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.Able == 1)&&(AC_WTP[WTPIndex]->WTP_Radio[i]->Ampdu.subframe!= 32)){
    			wid_radio_set_ampdu_subframe(Radio_ID, 1);
    		}
    		if(AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.Able != 0){
    			wid_radio_set_ampdu_able(Radio_ID, 2);
    		}
    		if((AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.Able == 1)&&(AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.AmsduLimit != 4000)){
    			wid_radio_set_ampdu_limit(Radio_ID, 2);
    		}
    		if((AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.Able == 1)&&(AC_WTP[WTPIndex]->WTP_Radio[i]->Amsdu.subframe!= 32)){
    			wid_radio_set_ampdu_subframe(Radio_ID, 2);
    		}
    		if((AC_WTP[WTPIndex]->WTP_Radio[i]->MixedGreenfield.Mixed_Greenfield != 0)
		&&((AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Type != 10)&&(AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Type !=12))) //fengwenchao modify 20120716 for autelan-3057
		{
    			wid_radio_set_mixed_puren_switch(Radio_ID);
    		}
    		if(AC_WTP[WTPIndex]->WTP_Radio[i]->channel_offset != 0){
    			wid_radio_set_channel_Extoffset(Radio_ID);
    		}
    		/* zhangshu add for set chainmask, 2010-11-25 */
			if(((AC_WTP[WTPIndex]->WTP_Radio[i]->chainmask_num == 1) \
			&& (AC_WTP[WTPIndex]->WTP_Radio[i]->tx_chainmask_state_value != 1)) || 
			((AC_WTP[WTPIndex]->WTP_Radio[i]->chainmask_num == 2) \
			&& (AC_WTP[WTPIndex]->WTP_Radio[i]->tx_chainmask_state_value != 3)) ||
			((AC_WTP[WTPIndex]->WTP_Radio[i]->chainmask_num == 3) \
			&& (AC_WTP[WTPIndex]->WTP_Radio[i]->tx_chainmask_state_value != 7))) {
			    wid_radio_set_chainmask(Radio_ID,1);
			}
			if(((AC_WTP[WTPIndex]->WTP_Radio[i]->chainmask_num == 1) \
			&& (AC_WTP[WTPIndex]->WTP_Radio[i]->rx_chainmask_state_value != 1)) || 
			((AC_WTP[WTPIndex]->WTP_Radio[i]->chainmask_num == 2) \
			&& (AC_WTP[WTPIndex]->WTP_Radio[i]->rx_chainmask_state_value != 3)) ||
			((AC_WTP[WTPIndex]->WTP_Radio[i]->chainmask_num == 3) \
			&& (AC_WTP[WTPIndex]->WTP_Radio[i]->rx_chainmask_state_value != 7))) {
			    wid_radio_set_chainmask(Radio_ID,2);
			}

			if(AC_WTP[WTPIndex]->WTP_Radio[i]->guardinterval != 1){
				wid_radio_set_guard_interval(WTPIndex*L_RADIO_NUM+i);
			}
			if(AC_WTP[WTPIndex]->WTP_Radio[i]->cwmode != 0){
				wid_radio_set_cmmode(WTPIndex*L_RADIO_NUM+i);
			}
			/*if(AC_WTP[WTPIndex]->WTP_Radio[i]->mcs != 0){
				wid_radio_set_mcs(WTPIndex*L_RADIO_NUM+i);
			}*/
			/*fengwenchao add 20120314 for requirements-407*/
			if(check_ac_whether_or_not_set_mcs_list(WTPIndex,i) == 1)
			{
				wid_radio_set_mcs_list	(WTPIndex*L_RADIO_NUM+i);
			}
			/*fengwenchao add end*/
			
        } 		
		if(AC_WTP[WTPIndex]->WTP_Radio[i]->ack.distance != 0){
				wid_radio_set_acktimeout_distance(AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_G_ID);
		}/*wcl add for RDIR-33*/
		
		if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->WTP_Radio[i] != NULL))
		  {
			  if(AC_WTP[WTPIndex]->WTP_Radio[i]->auto_channel != 0)
			  {
				  memset(buf,0,DEFAULT_LEN);
				  sprintf(buf,"echo 1 > /proc/sys/dev/wifi%d/nonoverlapping",i);
				  wid_syslog_debug_debug(WID_DEFAULT,"cmd buf %s\n",buf);
				  wid_radio_set_extension_command(WTPIndex,buf);
			  }
		  }	
		/*fengwenchao add 20110920 for radio disable config save*/
		if(AC_WTP[WTPIndex]->WTP_Radio[i]->radio_disable_flag == 1)
		{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = Radio_S_TYPE;
			msg.mqinfo.u.RadioInfo.Radio_Op = Radio_STATUS;
			msg.mqinfo.u.RadioInfo.Radio_L_ID = i;
			msg.mqinfo.u.RadioInfo.Radio_G_ID = AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_G_ID;
			struct msgqlist *elem;
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
			elem = NULL;			
		}
		/*fengwenchao add end*/
	}

	  if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->ntp_state != 1))
	  {
	  	wid_set_wtp_ntp(WTPIndex);
	 }

	if(AC_WTP[WTPIndex]->dhcp_snooping == 1){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_DHCP_SNOOPING;
			msg.mqinfo.u.WtpInfo.value2 = AC_WTP[WTPIndex]->dhcp_snooping;
		
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
			elem = NULL;
	}
	if(AC_WTP[WTPIndex]->sta_ip_report == 1){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_INFO_REPORT;
			msg.mqinfo.u.WtpInfo.value2 = AC_WTP[WTPIndex]->sta_ip_report;
		
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
			elem = NULL;
	}

	if(AC_WTP[WTPIndex]->wifi_extension_reportswitch == 1){
		if(AC_WTP[WTPIndex] != NULL){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_INFO_GET;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}
	}
	
	if(AC_WTP[WTPIndex]->ap_sta_wapi_report_switch == 1){
		if(AC_WTP[WTPIndex] != NULL){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_WAPI_INFO_SET;
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}
    }
	
	if(AC_WTP[WTPIndex]->ap_sta_report_switch == 1){
		if((AC_WTP[WTPIndex] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_INFO_SET;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}
	}
	//fengwenchao add 20110126 for XJDEV-32  from 2.0
	for(eth_index=0;eth_index < AP_ETH_IF_NUM;eth_index++){
		if(AC_WTP[WTPIndex]->apifinfo.eth[eth_index].eth_mtu != 1500){
			wid_set_ap_eth_if_mtu(WTPIndex,eth_index);
		}
	}
	//fengwenchao add end
	if(AC_WTP[WTPIndex]->apifinfo.report_switch == 1){
		if((AC_WTP[WTPIndex] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_IF_INFO_SET;

			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}
	}
	if(gCWNeighborDeadInterval != CW_NEIGHBORDEAD_INTERVAL_DEFAULT){
		wid_set_neighbordead_intervalt(WTPIndex,gCWNeighborDeadInterval);
	}
	if((AC_WTP[WTPIndex]->apstatisticsinterval != 1800)||(apstatistics != 0)/*switch is enable,so send interval*/){
		msgq msg;
		struct msgqlist *elem;
		if(AC_WTP[WTPIndex] != NULL){
				memset((char*)&msg, 0, sizeof(msg));
				msg.mqid = WTPIndex%THREAD_NUM+1;
				msg.mqinfo.WTPID = WTPIndex;
				msg.mqinfo.type = CONTROL_TYPE;
				msg.mqinfo.subtype = WTP_S_TYPE;
				msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STATISTICS_REPORT_INTERVAL;
				msg.mqinfo.u.WtpInfo.value2 = AC_WTP[WTPIndex]->apstatisticsinterval;

				elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
				if(elem == NULL){
					wid_syslog_info("%s malloc %s",__func__,strerror(errno));
					perror("malloc");
					return ;
				}
				memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
				elem->next = NULL;
				memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
				WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}

	}
	 /* zhangshu add for terminal disturb information, 2010-10-08 */
    if(AC_WTP[WTPIndex]->ter_dis_info.reportswitch == 1){
		if(AC_WTP[WTPIndex] != NULL){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_TERMINAL_DISTRUB_INFO;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}
	}
	if(AC_WTP[WTPIndex]->sta_deauth_message_reportswitch == 1){
		if((AC_WTP[WTPIndex] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_DEAUTH_SWITCH;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}
	}
	if(AC_WTP[WTPIndex]->sta_flow_information_reportswitch == 1){
		if((AC_WTP[WTPIndex] != NULL)){
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPIndex%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPIndex;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_STA_FLOW_INFORMATION_SWITCH;
			
			elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
			if(elem == NULL){
				wid_syslog_info("%s malloc %s",__func__,strerror(errno));
				perror("malloc");
				return ;
			}
			memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
			elem->next = NULL;
			memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
			WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		}
	}
	if(AC_WTP[WTPIndex]->option60_param != NULL){	
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = WTPIndex%THREAD_NUM+1;
		msg.mqinfo.WTPID = WTPIndex;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_OPTION60_PARAM;
		memcpy(msg.mqinfo.u.WtpInfo.value, AC_WTP[WTPIndex]->option60_param, strlen(AC_WTP[WTPIndex]->option60_param));
		elem = (struct msgqlist*)malloc(sizeof(struct msgqlist));
		if(elem == NULL){
			wid_syslog_info("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			return ;
		}
		memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
		elem->next = NULL;
		memcpy((char*)&(elem->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));
		WID_INSERT_CONTROL_LIST(WTPIndex, elem);
		
	}
}
struct msgqlist * WID_GET_CONTROL_LIST_ELEM(unsigned int WTPID){
	struct msgqlist *tmp = NULL;
	CWThreadMutexLock(&(gWTPs[WTPID].mutex_controlList)); 
	
	if((AC_WTP[WTPID] != NULL)&&(AC_WTP[WTPID]->ControlList != NULL)){
		tmp = AC_WTP[WTPID]->ControlList;
		AC_WTP[WTPID]->ControlList = AC_WTP[WTPID]->ControlList->next;
		tmp->next = NULL;
		AC_WTP[WTPID]->elem_num --;
		CWThreadMutexUnlock(&(gWTPs[WTPID].mutex_controlList));
		
		return tmp;
	}
	CWThreadMutexUnlock(&(gWTPs[WTPID].mutex_controlList));

	return NULL;
}

void WID_CLEAN_CONTROL_LIST(unsigned int WTPID)
{
	struct msgqlist *tmp = NULL;
	struct msgqlist *tmp_t = NULL;	
	CWThreadMutexLock(&(gWTPs[WTPID].mutex_controlList));
	
	if(AC_WTP[WTPID]->ControlWait != NULL){
		free(AC_WTP[WTPID]->ControlWait);
		AC_WTP[WTPID]->ControlWait = NULL;
	}
	if(AC_WTP[WTPID]->ControlList == NULL){
		AC_WTP[WTPID]->elem_num = 0;
		goto out;
	}
	tmp = AC_WTP[WTPID]->ControlList;
	while(tmp != NULL){
		tmp_t = tmp;		
		tmp = tmp->next;
		tmp_t->next = NULL;
		free(tmp_t);
		tmp_t = NULL;		
	}
	AC_WTP[WTPID]->ControlList = NULL;	
	AC_WTP[WTPID]->elem_num = 0;
out:
	CWThreadMutexUnlock(&(gWTPs[WTPID].mutex_controlList));
	
	return;
}


