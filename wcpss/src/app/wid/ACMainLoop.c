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
* ACMainLoop.c
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
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "wcpss/waw.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/wid/WID.h" 
#include "CWAC.h"
#include "ACDbus.h"
#include "CWStevens.h"
#include <syslog.h>
#include "ACDbus_handler.h"
#include "ACImageData.h"
#include "ACBak.h"
#include "ACAccessCheck.h"
#include "ACLoadbanlance.h"
#include "AC.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif
#include "time.h"//qiuchen add it!
// index of the current thread in the global array
CWThreadSpecific gIndexSpecific;
extern unsigned char gWIDLOGHN;//qiuchen
int gCWWaitJoin = CW_WAIT_JOIN_DEFAULT;

CW_THREAD_RETURN_TYPE CWManageWTP(void *arg);
CW_THREAD_RETURN_TYPE CWManageTimers(void *arg);
void CWCriticalTimerExpiredHandler(int arg);
void CWSoftTimerExpiredHandler(int arg);

void CWACManageIncomingPacket(CWSocket sock, char *buf, int len, int incomingInterfaceIndex, int BindingSystemIndex, CWNetworkLev4Address *addrPtr,char *ifname);
void _CWCloseThread(int i);
void CWResetWTPProtocolManager(CWWTPProtocolManager *WTPProtocolManager);
__inline__ CWWTPManager *CWWTPByName(const char *addr);
__inline__ CWWTPManager *CWWTPByAddress(CWNetworkLev4Address *addressPtr, CWSocket sock, int *WTPID);
void CWDownWTP(unsigned int WTPIndex);
WIDAUTOAPINFO *parse_dynamic_wtp_login_situation(CWWTPVendorInfos * valPtr);
CWBool WidParseJoinRequestMessageForAutoApLogin(char *msg, int len, int *seqNumPtr, CWProtocolJoinRequestValues *valuesPtr);
void WidDestroyJoinRequestValuesForAutoApLogin(CWProtocolJoinRequestValues *valPtr);
int wid_parse_wtp_model_rmodel(WIDAUTOAPINFO *info);
int wid_auto_ap_mac_filter(unsigned char *mac);
int wid_auto_ap_binding(int wtpid,int ifindex);
void WidDestroyAutoApLoginInfo(WIDAUTOAPINFO *info);
int parse_dynamic_wtp_name(char *name);
int wid_dynamic_change_wtp_info(int wtpid,WIDAUTOAPINFO *info);
void wid_wtp_param_init(int WTPIndex, unsigned char flag);
CWBool AsdWsm_WTPOp(unsigned int WtpID,Operate op);
typedef struct {
	int index;
	CWSocket sock;
	int interfaceIndex;
} CWACThreadArg; // argument passed to the thread func

extern unsigned int wid_to_ap_wifi_locate_config(unsigned int radioid);

CWBool WID_WTP_INIT(void *arg){
		int i = ((CWACThreadArg*)arg)->index;
		CWSocket sock = ((CWACThreadArg*)arg)->sock;
		int interfaceIndex = ((CWACThreadArg*)arg)->interfaceIndex;
		unsigned int flag = 0;
		CW_FREE_OBJECT_WID(arg);
		if(!check_wtpid_func(i)){
			wid_syslog_err("%s\n",__func__);
			return CW_FALSE;
		}else{
		}
			
		if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
			wid_syslog_crit("can't lock threadmutex");
			exit(1);
			}
		if(CW_BAK_RUN != gWTPs[i].currentState){
			CWThreadMutexLock(&ACLicense);
			flag = g_wtp_count[gWTPs[i].oemoption]->flag;
			if(0 == flag)
			{
				if(g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count>=g_wtp_count[gWTPs[i].oemoption]->gmax_wtp_count)
				{
					wid_syslog_err("%s WTP[%d] quit Reason:license current %d >= max %d\n",__func__,i,g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count,g_wtp_count[gWTPs[i].oemoption]->gmax_wtp_count);
					CWThreadMutexUnlock(&ACLicense);
					CWThreadMutexUnlock(&gActiveWTPsMutex);
					return CW_FALSE;
				}
			}
			else
			{
				if(g_wtp_binding_count[flag]->gcurrent_wtp_count>=g_wtp_binding_count[flag]->gmax_wtp_count)
				{
					wid_syslog_err("%s WTP[%d] quit Reason:binding license current %d >= max %d\n",__func__,i,g_wtp_binding_count[flag]->gcurrent_wtp_count,g_wtp_binding_count[flag]->gmax_wtp_count);
					CWThreadMutexUnlock(&ACLicense);
					CWThreadMutexUnlock(&gActiveWTPsMutex);
					return CW_FALSE;
				}
			}			
			wid_syslog_debug_debug(WID_WTPINFO,"in %s license[%d] current %d max %d\n",__func__,gWTPs[i].oemoption,g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count,g_wtp_count[gWTPs[i].oemoption]->gmax_wtp_count);
			(g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count)++;
			/*xiaodawei modify, 20101111*/
			if(g_wtp_count[gWTPs[i].oemoption]->flag!=0){
				(g_wtp_binding_count[flag]->gcurrent_wtp_count)++;
			}
			CWThreadMutexUnlock(&ACLicense);
			gActiveWTPs++;
			wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,gActiveWTPs = %d\n",__func__,__LINE__,gActiveWTPs);
		}
		/*fengwenchao modify begin 20110525*/
		if((interfaceIndex < gMaxInterfacesCount)&&(interfaceIndex >= 0))
		{
			gInterfaces[interfaceIndex].WTPCount++;
			CWUseSockNtop(((struct sockaddr*) &(gInterfaces[interfaceIndex].addr)),
					wid_syslog_debug_debug(WID_DEFAULT,"One more WTP on %s (%d)", str, interfaceIndex);
					);
		}
		else
		{
			wid_syslog_crit("interfaceIndex  =  %d  , is not legal\n",interfaceIndex);
		}
		/*fengwenchao modify end*/
	
		CWThreadMutexUnlock(&gActiveWTPsMutex);
	
		CWACInitBinding(i);
		
		gWTPs[i].interfaceIndex = interfaceIndex;
		gWTPs[i].socket = sock;
	#ifdef CW_NO_DTLS
		AC_WTP[i]->CTR_ID = sock;
	#endif
		gWTPs[i].fragmentsList = NULL;
		gWTPs[i].currentState = CW_ENTER_JOIN; // we're in the join state for this session
		AC_WTP[i]->WTPStat = 2;
		gWTPs[i].isNotFree = CW_TRUE;
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
		wid_syslog_debug_debug(WID_DEFAULT,"New Session");
			
		if(!CWErr(CWTimerRequest(gCWWaitJoin, &(gWTPs[i].thread), &(gWTPs[i].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,i))) { // start WaitJoin timer
			_CWCloseThread(i);
		}
	
#ifndef CW_NO_DTLS
		wid_syslog_debug_debug(WID_DEFAULT,"Init DTLS Session");
	
		if(!CWErr(CWSecurityInitSessionServer(&gWTPs[i], sock, gACSecurityContext, &((gWTPs[i]).session), &(gWTPs[i].pathMTU) )  ) ) { // error joining
			CWTimerCancel(&(gWTPs[i].currentTimer),1);
			_CWCloseThread(i);
		}
#endif
		(gWTPs[i]).subState = CW_WAITING_REQUEST;
		
		if(gCWForceMTU > 0) gWTPs[i].pathMTU = gCWForceMTU;
		wid_syslog_debug_debug(WID_DEFAULT,"Path MTU for this Session: %d",  gWTPs[i].pathMTU);
	
	return CW_TRUE;

}


void STA_OP(TableMsg *msg){
	int WTPIndex = msg->u.STA.WTPID;
	int BSSIndex = msg->u.STA.BSSIndex;
    int auth_flag = 0;/* yjl 2014-2-28 */
	unsigned char WLANID = 0;
	msgq msginfo;
	if((AC_WTP[WTPIndex] != NULL)&&(AC_WTP[WTPIndex]->WTPStat == 5)){
		if (AC_BSS[BSSIndex] == NULL)			/* Huangleilei move for AXSSZFI-1718 */
		{
			wid_syslog_err("%s %d AC_BSS[%d] is NULL", __func__, __LINE__, BSSIndex);
			return ;
		}
		/*while((AC_WTP[WTPIndex]->CMD->staCMD > 0)&&(i < 10)){
			printf("please waiting seconds\n");			
			i++;
			sleep(1);
		}
		if(i == 10)
			return;*/
		if(msg->Op == CANCEL_TRAFFIC_LIMIT){
			unsigned char L_Radio_ID = 0;
			unsigned char mac[MAC_LEN] ;
			WLANID = msg->u.STA.wlanId;
			L_Radio_ID = BSSIndex % L_RADIO_NUM;
			memset(mac,0,MAC_LEN);
			if( msg != NULL)
				memcpy(mac, msg->u.STA.STAMAC, MAC_LEN);	
			wid_cancel_bss_traffic_limit_sta_value(WTPIndex,L_Radio_ID,WLANID,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],0,2,0);
			return ;
		}

		/*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
		else if(STA_PORTAL_AUTH == msg->Op)
		{
			char cmd[WID_SYSTEM_CMD_LENTH] = {0};
			unsigned char eag_ip[4] = {0};
			
			memcpy(eag_ip, &(msg->u.STA.portal_info.portal_ip), sizeof(eag_ip));
			auth_flag = 1;
			sprintf(cmd, AP_EXT_CMD_NOTIFY_STA_PORTAL_AUTH,	((msg->u.STA.BSSIndex)/L_BSS_NUM)%L_RADIO_NUM,
							msg->u.STA.wlanId, msg->u.STA.STAMAC[0], msg->u.STA.STAMAC[1], msg->u.STA.STAMAC[2],
							msg->u.STA.STAMAC[3], msg->u.STA.STAMAC[4], msg->u.STA.STAMAC[5], auth_flag, eag_ip[0], eag_ip[1], eag_ip[2],eag_ip[3]);
			wid_syslog_debug_debug(WID_DEFAULT, "wtp %d extension command: %s\n", msg->u.STA.WTPID, cmd);
			wid_radio_set_extension_command(msg->u.STA.WTPID, cmd);
			return ;
		}
		else if(msg->Op == STA_PORTAL_DEAUTH)
		{
			char cmd[WID_SYSTEM_CMD_LENTH] = {0};
			char eag_ip[4] = {0};
			auth_flag = 0;
			
			sprintf(cmd, AP_EXT_CMD_NOTIFY_STA_PORTAL_AUTH,	((msg->u.STA.BSSIndex)/L_BSS_NUM)%L_RADIO_NUM,
							msg->u.STA.wlanId, msg->u.STA.STAMAC[0], msg->u.STA.STAMAC[1], msg->u.STA.STAMAC[2],
							msg->u.STA.STAMAC[3], msg->u.STA.STAMAC[4], msg->u.STA.STAMAC[5], auth_flag, eag_ip[0], eag_ip[1], eag_ip[2],eag_ip[3]);
			wid_syslog_debug_debug(WID_DEFAULT, "wtp %d extension command: %s\n", msg->u.STA.WTPID, cmd);
			wid_radio_set_extension_command(msg->u.STA.WTPID, cmd);
			return ;
		}
		/*end**************************************************/
		
		memset((char*)&msginfo, 0, sizeof(msginfo));
		msginfo.mqid = WTPIndex%THREAD_NUM+1;		
		msginfo.mqinfo.WTPID = WTPIndex;
		msginfo.mqinfo.type = CONTROL_TYPE;
		msginfo.mqinfo.subtype = STA_S_TYPE;
		if(msg->Op == WID_ADD) {
			msginfo.mqinfo.u.StaInfo.Sta_Op = Sta_ADD;
			if(msg->u.STA.traffic_limit != 0)
				msginfo.mqinfo.u.StaInfo.traffic_limit = msg->u.STA.traffic_limit;
			if(msg->u.STA.send_traffic_limit != 0)
				msginfo.mqinfo.u.StaInfo.send_traffic_limit = msg->u.STA.send_traffic_limit;
			if(msg->u.STA.vlan_id > 0)
				msginfo.mqinfo.u.StaInfo.vlan_id = msg->u.STA.vlan_id;
			msginfo.mqinfo.u.StaInfo.sta_num = msg->u.STA.sta_num;
		}else if(msg->Op == WID_DEL)
			msginfo.mqinfo.u.StaInfo.Sta_Op = Sta_DEL;
		else if(msg->Op == RADIUS_STA_UPDATE)
		{		
			unsigned char mac[MAC_LEN] ;
			if( msg != NULL){
				memcpy(mac, msg->u.STA.STAMAC, MAC_LEN);	
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			if (AC_BSS[BSSIndex] == NULL 
				|| AC_WLAN[AC_BSS[BSSIndex]->WlanID] == NULL
				|| (AC_WLAN[AC_BSS[BSSIndex]->WlanID] != NULL && (AC_WLAN[AC_BSS[BSSIndex]->WlanID]->want_to_delete == 1)))		/* Huangleilei move for AXSSZFI-1718 */
			{
				wid_syslog_err("%s %d AC_BSS[%d] or AC_WLAN is NULL", __func__, __LINE__, BSSIndex);
				return ;
			}
			wid_radio_set_wlan_traffic_limit_sta_value(WTPIndex,(msg->u.STA.BSSIndex/L_BSS_NUM)%L_RADIO_NUM,AC_BSS[BSSIndex]->WlanID,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], msg->u.STA.send_traffic_limit,1);
			if (AC_BSS[BSSIndex] == NULL 
				|| AC_WLAN[AC_BSS[BSSIndex]->WlanID] == NULL
				|| (AC_WLAN[AC_BSS[BSSIndex]->WlanID] != NULL && (AC_WLAN[AC_BSS[BSSIndex]->WlanID]->want_to_delete == 1)))		/* Huangleilei move for AXSSZFI-1718 */
			{
				wid_syslog_err("%s %d AC_BSS[%d] or AC_WLAN is NULL", __func__, __LINE__, BSSIndex);
				return ;
			}
			wid_radio_set_wlan_traffic_limit_sta_value(WTPIndex,(msg->u.STA.BSSIndex/L_BSS_NUM)%L_RADIO_NUM,AC_BSS[BSSIndex]->WlanID,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], msg->u.STA.traffic_limit,0);
		}
		msginfo.mqinfo.u.StaInfo.Radio_L_ID = (msg->u.STA.BSSIndex/L_BSS_NUM)%L_RADIO_NUM;
		if(msg != NULL){
			memcpy(msginfo.mqinfo.u.StaInfo.STAMAC, msg->u.STA.STAMAC, MAC_LEN);	
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		if (AC_BSS[BSSIndex] == NULL 
			|| AC_WLAN[AC_BSS[BSSIndex]->WlanID] == NULL
			|| (AC_WLAN[AC_BSS[BSSIndex]->WlanID] != NULL && (AC_WLAN[AC_BSS[BSSIndex]->WlanID]->want_to_delete == 1))) 	/* Huangleilei move for AXSSZFI-1718 */
		{
			wid_syslog_err("%s %d AC_BSS[%d] or AC_WLAN is NULL", __func__, __LINE__, BSSIndex);
			return ;
		}
		msginfo.mqinfo.u.StaInfo.WLANID = AC_BSS[BSSIndex]->WlanID;
		if (msgsnd(ASD_WIDMSGQ, (msgq *)&msginfo, sizeof(msginfo.mqinfo), 0) == -1){
			wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
		if (AC_BSS[BSSIndex] == NULL 
			&& AC_WLAN[AC_BSS[BSSIndex]->WlanID] == NULL
			&& (AC_WLAN[AC_BSS[BSSIndex]->WlanID] != NULL && (AC_WLAN[AC_BSS[BSSIndex]->WlanID]->want_to_delete == 1))) 	/* Huangleilei move for AXSSZFI-1718 */
		{
			wid_syslog_err("%s %d AC_BSS[%d] or AC_WLAN is NULL", __func__, __LINE__, BSSIndex);
			return ;
		}

		WLANID = AC_BSS[BSSIndex]->WlanID;
		
		//AC_WTP[WTPIndex]->CMD->CMD++;
				
		printf("STA op1\n"); 		
		if((AC_WLAN[WLANID] != NULL)&&(AC_WLAN[WLANID]->Roaming_Policy == 1) && (AC_WLAN[WLANID]->want_to_delete != 1)){
			CWThreadMutexLock(&(gSTARoamingMutex));
				memset(STA_ROAM.STAMAC,0,MAC_LEN);
				STA_ROAM.STAOP = msg->Op;
				if(msg != NULL){
					memcpy(STA_ROAM.STAMAC, msg->u.STA.STAMAC, MAC_LEN);
					}
				else
					{
					wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
					}
				if (AC_BSS[BSSIndex] == NULL 
					|| AC_WLAN[AC_BSS[BSSIndex]->WlanID] == NULL
					|| (AC_WLAN[AC_BSS[BSSIndex]->WlanID] != NULL && (AC_WLAN[AC_BSS[BSSIndex]->WlanID]->want_to_delete == 1))) 	/* Huangleilei move for AXSSZFI-1718 */
				{
					wid_syslog_err("%s %d AC_BSS[%d] or AC_WLAN is NULL", __func__, __LINE__, BSSIndex);
					//CWSignalThreadCondition(&gSTARoamingWait);
					CWThreadMutexUnlock(&(gSTARoamingMutex));
					return ;
				}
				STA_ROAM.WLANDomain = AC_BSS[BSSIndex]->WlanID;
				CWSignalThreadCondition(&gSTARoamingWait);
			CWThreadMutexUnlock(&(gSTARoamingMutex));
		}
	}
}

void KEY_OP(TableMsg *msg){
	int WTPIndex = msg->u.KEY.WTPID;	
//	int i = 0;
	msgq msginfo;
	if(AC_WTP[WTPIndex] != NULL){		
		/*while((AC_WTP[WTPIndex]->CMD->keyCMD > 0)&&(i < 10)){
			printf("please waiting seconds\n");
			i++;
			sleep(1);
		}
		if(i == 10)
			return;*/
			
		memset((char*)&msginfo, 0, sizeof(msginfo));
		msginfo.mqid = WTPIndex%THREAD_NUM+1;	
		msginfo.mqinfo.WTPID = WTPIndex;
		msginfo.mqinfo.type = CONTROL_TYPE;
		msginfo.mqinfo.subtype = STA_S_TYPE;
		msginfo.mqinfo.u.StaInfo.Sta_Op = Sta_SETKEY;
		msginfo.mqinfo.u.StaInfo.BSSIndex = msg->u.KEY.BSSIndex;
		msginfo.mqinfo.u.StaInfo.cipher = msg->u.KEY.cipher;
		msginfo.mqinfo.u.StaInfo.keylen = msg->u.KEY.key_len;
		msginfo.mqinfo.u.StaInfo.keyidx = msg->u.KEY.key_idx;
		wid_syslog_debug_debug(WID_DEFAULT,"key len %d\n",msg->u.KEY.key_len);
		if(msg != NULL){
			memcpy((msginfo.mqinfo.u.StaInfo.STAMAC),(msg->u.KEY.StaAddr),6);
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		if(msg != NULL){
			memcpy((msginfo.mqinfo.u.StaInfo.key),(msg->u.KEY.key),msg->u.KEY.key_len);
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		
		if (msgsnd(ASD_WIDMSGQ, (msgq *)&msginfo, sizeof(msginfo.mqinfo), 0) == -1){
			wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}
	}
}

void WLAN_OP(TableMsg *msg){
	int WLANID = msg->u.WLAN.WlanID;
	unsigned char oldsecid = msg->u.WLAN.WlanKey.OldSecurityIndex;
	unsigned char oldsecid_flag =  msg->u.WLAN.WlanKey.OldSecurityIndex_flag;
	int i = 0;
	int j = 0;
//	int k = 0;
	char *key;
	
	switch(msg->Op){
		case WID_MODIFY:
			if(AC_WLAN[WLANID] != NULL){
				if(AC_WLAN[WLANID]->EncryptionType != msg->u.WLAN.EncryptionType)

								
				if(gtrapflag>=4){
						wid_dbus_trap_wlan_encryption_type_change(WLANID);
				}

				/*fengwenchao add 20110309 通过oldsecid,oldsecid_flag判断之前绑定安全策略是否为((open||shared)&&(wep)),1-->YES清除标志位*/
				for(i=0; i<WTP_NUM; i++)
				{
					if((AC_WTP[i]!=NULL)/*&&(AC_WTP[i]->isused == 1)*/)
					{
						for(j=0; j<AC_WTP[i]->RadioCount; j++)
						{
							if(AC_WLAN[WLANID]->S_WTP_BSS_List[i][j] != 0)
							{
								int bssindex = AC_WLAN[WLANID]->S_WTP_BSS_List[i][j];
								//int wtpid = bssindex/(L_RADIO_NUM*L_BSS_NUM);
				
								if((oldsecid <= 4)&&(oldsecid >= 1)&&(oldsecid_flag == 1))
								{
									AC_RADIO[AC_BSS[bssindex]->Radio_G_ID]->wep_flag[oldsecid-1] = 0;
									//AC_WTP[wtpid]->wep_flag[oldsecid-1] = 0;
								}								
							}						
						}							
					}			
				}
				//fengwenchao add end	
				AC_WLAN[WLANID]->ap_max_inactivity = msg->u.WLAN.ap_max_inactivity;											
				AC_WLAN[WLANID]->SecurityType = msg->u.WLAN.SecurityType;
				AC_WLAN[WLANID]->SecurityID = msg->u.WLAN.SecurityID;
				AC_WLAN[WLANID]->EncryptionType = msg->u.WLAN.EncryptionType;
				AC_WLAN[WLANID]->PreAuth = msg->u.WLAN.PreAuth;
				AC_WLAN[WLANID]->KeyLen = 0;
				AC_WLAN[WLANID]->IpLen = 0;
				AC_WLAN[WLANID]->AECerLen = 0;
				AC_WLAN[WLANID]->ASCerLen = 0;
				AC_WLAN[WLANID]->asic_hex = 0;
				memset(AC_WLAN[WLANID]->WlanKey, 0, DEFAULT_LEN);
				memset(AC_WLAN[WLANID]->AsIp, 0, DEFAULT_LEN);
				memset(AC_WLAN[WLANID]->AECerPath, 0, DEFAULT_LEN);
				memset(AC_WLAN[WLANID]->ASCerPath, 0, DEFAULT_LEN);
				if(((AC_WLAN[WLANID]->SecurityType == OPEN)&&(AC_WLAN[WLANID]->EncryptionType == WEP))
					||(AC_WLAN[WLANID]->SecurityType == SHARED)
					||(AC_WLAN[WLANID]->SecurityType == WAPI_PSK))
				{
					AC_WLAN[WLANID]->KeyLen = msg->u.WLAN.WlanKey.key_len;
					if(msg != NULL){
						memcpy(AC_WLAN[WLANID]->WlanKey, msg->u.WLAN.WlanKey.key, msg->u.WLAN.WlanKey.key_len);
						}
					else
						{
						wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
						}
					key = (char *)WID_MALLOC(msg->u.WLAN.WlanKey.key_len+1);
					memset(key,0,(msg->u.WLAN.WlanKey.key_len+1));
					strncpy(key, msg->u.WLAN.WlanKey.key, msg->u.WLAN.WlanKey.key_len);
					AC_WLAN[WLANID]->SecurityIndex = msg->u.WLAN.WlanKey.SecurityIndex;

					if(msg->u.WLAN.ascii_hex == 1)
					{
						AC_WLAN[WLANID]->asic_hex = 0;
					}
					else if(msg->u.WLAN.ascii_hex == 2)
					{
						AC_WLAN[WLANID]->asic_hex = 1;
					}
				if(gtrapflag>=4){
						wid_dbus_trap_wlan_preshared_key_change(WLANID,key);
				}
					
					
					CW_FREE_OBJECT_WID(key);
					wid_syslog_debug_debug(WID_DEFAULT,"keylen %d %d\n",msg->u.WLAN.WlanKey.key_len,AC_WLAN[WLANID]->KeyLen);
					wid_syslog_debug_debug(WID_DEFAULT,"key %s %s\n",msg->u.WLAN.WlanKey.key,AC_WLAN[WLANID]->WlanKey);
					wid_syslog_debug_debug(WID_DEFAULT,"keytype %d %d\n",msg->u.WLAN.ascii_hex,AC_WLAN[WLANID]->asic_hex);
				}
				if((AC_WLAN[WLANID]->SecurityType == WAPI_AUTH))					
				{
					AC_WLAN[WLANID]->AECerLen = msg->u.WLAN.ae_cert_path_len;
					if(msg != NULL){
						memcpy(AC_WLAN[WLANID]->AECerPath, msg->u.WLAN.ae_cert_path, msg->u.WLAN.ae_cert_path_len);
						}
					else
						{
						wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
						}

					AC_WLAN[WLANID]->IpLen = msg->u.WLAN.as_ip_len;
					if(msg != NULL){
						memcpy(AC_WLAN[WLANID]->AsIp, msg->u.WLAN.as_ip, msg->u.WLAN.as_ip_len);
						}
					else
						{
						wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
						}

					AC_WLAN[WLANID]->ASCerLen = msg->u.WLAN.cert_path_len;
					if(msg != NULL){
						memcpy(AC_WLAN[WLANID]->ASCerPath, msg->u.WLAN.cert_path, msg->u.WLAN.cert_path_len);
						}
					else
						{
						wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
						}

					wid_syslog_debug_debug(WID_DEFAULT,"AECerLen %d %d\n",msg->u.WLAN.ae_cert_path_len,AC_WLAN[WLANID]->AECerLen);					
					wid_syslog_debug_debug(WID_DEFAULT,"IpLen %d %d\n",msg->u.WLAN.as_ip_len,AC_WLAN[WLANID]->IpLen);
					wid_syslog_debug_debug(WID_DEFAULT,"ASCerLen %d %d\n",msg->u.WLAN.cert_path_len,AC_WLAN[WLANID]->ASCerLen);
					wid_syslog_debug_debug(WID_DEFAULT,"AECerPath %s %s\n",msg->u.WLAN.ae_cert_path,AC_WLAN[WLANID]->AECerPath);
					wid_syslog_debug_debug(WID_DEFAULT,"ASCerPath %s %s\n",msg->u.WLAN.cert_path,AC_WLAN[WLANID]->ASCerPath);
					wid_syslog_debug_debug(WID_DEFAULT,"AsIp %s %s\n",msg->u.WLAN.as_ip,AC_WLAN[WLANID]->AsIp);
				}
			}
			break;
		case WID_DEL:			
			for(i = 1; i < WLAN_NUM; i++){
				if((AC_WLAN[i]!=NULL)&&(AC_WLAN[i]->Status == 1)&&(AC_WLAN[i]->SecurityID == msg->u.WLAN.SecurityID)){
					AC_WLAN[i]->SecurityID =0;
					AC_WLAN[i]->SecurityType = 0;
					AC_WLAN[i]->KeyLen = 0;
					memset(AC_WLAN[i]->WlanKey, 0, DEFAULT_LEN);
					AC_WLAN[i]->IpLen = 0;
					AC_WLAN[i]->AECerLen = 0;
					AC_WLAN[i]->ASCerLen = 0;
					AC_WLAN[i]->asic_hex = 0;
					memset(AC_WLAN[i]->AsIp, 0, DEFAULT_LEN);
					memset(AC_WLAN[i]->AECerPath, 0, DEFAULT_LEN);
					memset(AC_WLAN[i]->ASCerPath, 0, DEFAULT_LEN);
					continue;
				}else
					continue;
			}
			break;
		case WID_ADD:	
			break;
		case STA_INFO:	
			break;
		default:
			break;		
	}
}

/*
void trap_op(TableMsg *msg)
{	
	if(gtrapflag == 1)
	{
		switch(msg->Op)
		{
			case WTP_DENEY_STA :	
				signal_wtp_deny_sta(msg->u.WTP.WtpID);
				break;
			case WTP_DE_DENEY_STA :	
				signal_de_wtp_deny_sta(msg->u.WTP.WtpID);
				break;
			case STA_COME :	
				signal_sta_come(msg->u.STA.STAMAC, msg->u.STA.WTPID, msg->u.STA.BSSIndex, msg->u.STA.StaState);
				break;
			case STA_LEAVE :	
				signal_sta_leave(msg->u.STA.STAMAC, msg->u.STA.WTPID, msg->u.STA.BSSIndex, msg->u.STA.StaState);
				break;
			case VERIFY_INFO :
				signal_sta_verify(msg->u.STA.STAMAC,msg->u.STA.WTPID);
				break;
			case VERIFY_FAIL_INFO :
				signal_sta_verify_failed(msg->u.STA.STAMAC,msg->u.STA.WTPID);
				break;
			case ASSOC_FAIL_INFO :
				signal_sta_assoc_failed(msg->u.STA.STAMAC,msg->u.STA.WTPID,msg->u.STA.nRate);
				break;
			case JIANQUAN_FAIL_INFO :
				signal_sta_jianquan_failed(msg->u.STA.STAMAC,msg->u.STA.WTPID,msg->u.STA.nRate);
				break;
			default:
				break;		
		}
	}
}
*/
void BSS_pkt_op(TableMsg *msg)
{
	int i=0;

	unsigned char *buff = (unsigned char*)msg + sizeof(MsgType);
	BSS_pkt_header *bss_header = (BSS_pkt_header *)buff;
	BSSStatistics *BSS_pkt = (BSSStatistics *)(buff + sizeof(BSS_pkt_header));
	unsigned int bssindex = 0;
	unsigned int l_radio_id = 0;
	unsigned int l_bss_id = 0;
	unsigned int wtpid = bss_header->WTPID;
	unsigned int count = bss_header->bss_cnt;
	wid_syslog_debug_debug(WID_DEFAULT,"wtpid:%d count: %d\n",wtpid,count);
	if(AC_WTP[wtpid] != NULL)
	{
		if(count != 0)
		{
			for(i=0;i<count;i++)
			{
				bssindex = BSS_pkt->BSSIndex;
				l_radio_id = (bssindex-(wtpid*L_RADIO_NUM*L_BSS_NUM))/L_BSS_NUM;
				l_bss_id = bssindex%L_BSS_NUM;
				//printf("BSS_pkt->BSSIndex %d\n",BSS_pkt->BSSIndex);
				//printf("bssindex %d l_radio_id %d l_bss_id %d\n",bssindex,l_radio_id,l_bss_id);
				if(AC_WTP[wtpid]->WTP_Radio[l_radio_id] != NULL)
				{
					if(AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id] != NULL)
					{
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.BSSIndex = BSS_pkt->BSSIndex;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.Radio_G_ID = BSS_pkt->Radio_G_ID;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_unicast = BSS_pkt->rx_unicast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_unicast = BSS_pkt->tx_unicast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_broadcast = BSS_pkt->rx_broadcast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_broadcast = BSS_pkt->tx_broadcast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_pkt_unicast = BSS_pkt->rx_pkt_unicast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_pkt_unicast = BSS_pkt->tx_pkt_unicast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_pkt_broadcast = BSS_pkt->rx_pkt_broadcast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_pkt_broadcast = BSS_pkt->tx_pkt_broadcast;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.retry = BSS_pkt->retry;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.retry_pkt = BSS_pkt->retry_pkt;
						AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.err = BSS_pkt->err;
					/*
						printf("BSSIndex: %d\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.BSSIndex);
						printf("g_radio_id: %d\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.Radio_G_ID);
						printf("rx_unicast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_unicast);
						printf("tx_unicast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_unicast);
						printf("rx_broadcast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_broadcast);
						printf("tx_broadcast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_broadcast);
						printf("rx_pkt_unicast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_pkt_unicast);
						printf("tx_pkt_unicast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_pkt_unicast);
						printf("rx_pkt_broadcast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.rx_pkt_broadcast);
						printf("tx_pkt_broadcast: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.tx_pkt_broadcast);
						printf("retry: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.retry);
						printf("retry_pkt: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.retry_pkt);
						printf("err: %lld\n",AC_WTP[wtpid]->WTP_Radio[l_radio_id]->BSS[l_bss_id]->BSS_pkt_info.err);
					*/
					}
					else
					{
						wid_syslog_debug_debug(WID_DEFAULT,"bssindex %d is not exist\n",bssindex);
					}
				}
				else
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wtpid %d radioid %d is not exist\n",wtpid,l_radio_id);
				}
				BSS_pkt += 1;
			}	
		}
		else
		{
			wid_syslog_debug_debug(WID_DEFAULT,"wtp %d has not bss at all\n",wtpid);
		}
	}
	else
	{
		wid_syslog_debug_debug(WID_DEFAULT,"WID WSM WTP table not synchronous ,wtp %d is not exist\n",wtpid);
	}
}

void neighbor_ap_sta_info(TableMsg *msg)
{
	int WTPIndex = msg->u.WTP.WtpID;
	int N_wtpid = msg->u.WTP.N_WTP.N_WtpID;
	int N_sta_num  = msg->u.WTP.N_WTP.cur_sta_num;
	unsigned char mac[6] = {0};
	if(msg != NULL){
		memcpy(mac,msg->u.WTP.N_WTP.WTPMAC,MAC_LEN);
		}
	else
		{
		wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
		}
	int currentchannel = 0;
	if(!check_wtpid_func(WTPIndex)){
		return;
	}else{
		if(gtrapflag>=24){
			if(AC_WTP[WTPIndex])
			{
				currentchannel = AC_WTP[WTPIndex]->WTP_Radio[0]->Radio_Chan;
				wid_syslog_debug_debug(WID_DEFAULT,"rogue_terminal,%s\n",__func__);
				wid_syslog_debug_debug(WID_DEFAULT,"rogue_terminal,current wtpid:%d,neighbor ap id:%d,neighbor ap sta num:%d\n",WTPIndex,N_wtpid,N_sta_num);
				wid_syslog_debug_debug(WID_DEFAULT,"rogue_terminal_threshold=%d\n",AC_WTP[WTPIndex]->wtp_rogue_terminal_threshold);
			}
			if((gtrap_channel_terminal_interference_switch == 1)&&(AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->wtp_rogue_terminal_threshold <= N_sta_num))
			{
				if(AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag == 0)   //fengwenchao add 20110221
					{
				wid_syslog_debug_debug(WID_DEFAULT,"rogue_terminal,send terminal trap,%s\n",__func__);
				wid_dbus_trap_wtp_channel_terminal_interference(WTPIndex,0,currentchannel,mac);
				AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag = 1;
					}
			}else if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->wtp_rogue_terminal_threshold > N_sta_num)&&(AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag == 1)){
				wid_syslog_debug_debug(WID_DEFAULT,"rogue_terminal,send terminal trap clear,%s\n",__func__);
				wid_dbus_trap_wtp_channel_terminal_interference_clear(WTPIndex,0,currentchannel,NULL);	
				AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag = 0;      //fengwenchao add 20110221
			}	
		}
	}
}
CW_THREAD_RETURN_TYPE CWWawInforUpdate(void * arg){
	wid_pid_write_v2("CWWawInforUpdate",0,vrrid);
	while(1){
		int len;
		unsigned char buf[8192];
		
		CWGetMsgQueue(&ASD_WIDMSGQ);
		len = recvfrom(wAWSocket, buf, sizeof(buf), 0, NULL, 0);
		if (len < 0) {
			wid_syslog_info("%s recv %s",__func__,strerror(errno));
			perror("recv");
			return 0;
		}		
		TableMsg *msg = (TableMsg*)buf;
		wid_syslog_debug_debug(WID_DEFAULT,"Type:%d\n",msg->Type);
			switch(msg->Type){
			case WLAN_TYPE :{
					
					WLAN_OP(msg);
					break;
				}
	/*				case BSS_TYPE :{
					
					BSS_OP(msg, interfaces);
					break;
				}*/
				case STA_TYPE :
					//printf("STA OP\n");
					if(is_secondary != 1){
						STA_OP(msg);
					}
					break;
				case SKEY_TYPE :
					//printf("KEY OP\n");
					if(is_secondary != 1){
						KEY_OP(msg);
					}
					break;
				case TRAP_TYPE :
					//printf("TRAP_TYPE\n");
					/*trap_op(msg);*/
					break;
				case BSS_PKT_TYPE:
					//printf("BSS_INFO\n");
					//BSS_pkt_op(msg);
					break;
				case NEIGHBOR_AP_STA_CHECK:
					if(is_secondary != 1){
						neighbor_ap_sta_info(msg);
					}
					break;
				default:
					//printf("default\n");
					break;
			}
	}

}

int WTPAddrCheck(CWNetworkLev4Address *addrPtr){
	int i;
	CWThreadMutexLock(&gWTPsMutex);
		for(i = 0; i < WTP_NUM; i++) {
			if(gWTPs[i].isNotFree && &(gWTPs[i].address) != NULL &&
						!sock_cmp_addr((struct sockaddr*)addrPtr, (struct sockaddr*)&(gWTPs[i].address), sizeof(CWNetworkLev4Address))) { // we treat a WTP that sends packet to a different AC's interface as a new WTP
				CWThreadMutexUnlock(&gWTPsMutex);
				return i;
			}
		}
	CWThreadMutexUnlock(&gWTPsMutex);
	return -1;
}
void Delete_Interface(char *ifname, int ifindex){
	struct CWMultiHomedInterface *p,*p1;
	p1 = gACSocket.interfaces;
	if(gACSocket.interfaces == NULL)
		return;
	p = p1->if_next;
	while(p != NULL){
		if((strncmp(p->ifname,ifname,IFI_NAME)==0)&&(p->systemIndex == ifindex)){
			int gifindex;
			p1->if_next = p->if_next;
			p->if_next = NULL;
			if(p->kind == CW_PRIMARY){
				gifindex = p->gIf_Index;
				if((gifindex < gMaxInterfacesCount)&&(gifindex >= 0)){   //fengwenchao add (gifindex >= 0) 20110525
					gInterfaces[gifindex].enable = 0;
					gInterfaces[gifindex].WTPCount = 0;
				}
			}
			close(p->sock);
			WID_FREE(p);
			p = NULL;
			gACSocket.count--;
			p = p1->if_next;
			continue;
		}
		p1 = p;
		p = p1->if_next;
	}
	Check_gACSokcet_Poll(&gACSocket);

}

void Modify_WLAN_WTP_SETTING(int index){	
	int i = 0;
	char ifname[IFI_NAME];
	memset(ifname, 0, IFI_NAME);
	if(AC_WTP[index])
	{
		
		memcpy(ifname, AC_WTP[index]->BindingIFName, strlen(AC_WTP[index]->BindingIFName));
			
	}
	int ret = 0;
	struct ifi_info *ifi = (struct ifi_info*)WID_MALLOC(sizeof(struct ifi_info));
	if (NULL == ifi)
	{
		return ;
	}
	memset(ifi->ifi_name,0,sizeof(ifi->ifi_name));
	strncpy(ifi->ifi_name,ifname,sizeof(ifi->ifi_name));
	//struct CWMultiHomedInterface *p = NULL,*pbr = NULL; 
	ret = Get_Interface_Info(ifname,ifi);
	int ifindex = 0;
	if(AC_WTP[index])
	{
		ifindex = AC_WTP[index]->BindingSystemIndex;
	}
	if(ret != 0){
		Delete_Interface(ifname, ifindex);
		if(ifi->ifi_addr != NULL){
			WID_FREE(ifi->ifi_addr);
			ifi->ifi_addr = NULL;
		}
		if(ifi->ifi_brdaddr != NULL){
			WID_FREE(ifi->ifi_brdaddr);
			ifi->ifi_brdaddr = NULL;
		}
		WID_FREE(ifi);
		ifi = NULL;
		return; 
	}else{
		Check_Current_Interface_and_delete(ifname,ifi);		
		if(ifi->check_brdaddr < 2){
			ret = Bind_BroadAddr_For_WID(ifi,CW_CONTROL_PORT);		
			//ret = Bind_BroadAddr_For_WID(ifi,CW_CONTROL_PORT_AU);
		}
		for(i=0;i<ifi->addr_num;i++)
		{		
			if(ifi->addr[i] == 0)
				continue;
			//memcpy(&((struct sockaddr_in *) ifi->ifi_addr)->sin_addr,&(ifi->addr[i]),sizeof(struct in_addr));
			((struct sockaddr_in *) ifi->ifi_addr)->sin_addr.s_addr = ifi->addr[i];
			if(Lic_ip.lic_active_ac_ip == ifi->addr[i]){
				ret = Bind_Interface_For_WID(ifi,WID_LIC_AC_PORT,LIC_TYPE);	 
				wid_syslog_debug_debug(WID_DBUS,"%s,%d,WID_LIC_AC_PORT.\n",__func__,__LINE__);
			}else{
				ret = Bind_Interface_For_WID(ifi,CW_CONTROL_PORT,DOWN_LINK_IF_TYPE);
				if(ret == 1){
					ret = 0;
					continue;
				}				
				//ret = Bind_Interface_For_WID(ifi,CW_CONTROL_PORT_AU);
				unsigned int ip;
				ip = ((struct sockaddr_in *)(ifi->ifi_addr))->sin_addr.s_addr;
				WIDWsm_VRRPIFOp((unsigned char*)(ifi->ifi_name),ip,VRRP_REG_IF);
				Check_WLAN_WTP_IF_Index(ifi,ifname);	
			}
		}
	}	
	if(ifi->ifi_addr != NULL){
		WID_FREE(ifi->ifi_addr);
		ifi->ifi_addr = NULL;
	}
	if(ifi->ifi_brdaddr != NULL){
		WID_FREE(ifi->ifi_brdaddr);
		ifi->ifi_brdaddr = NULL;
	}
	WID_FREE(ifi);
	ifi = NULL;
	return;

}

int DELETE_LISTENNING_INTERFACE(char *ifname){	
	//int i = 0;
	//int ret = 0;
	int m=0,n=0;
	int ifindex = 0;
	int find = 0;
	struct CWMultiHomedInterface *p = NULL,*p_next = NULL; 
	p = gListenningIF.interfaces;
	if(p == NULL){  
		wid_syslog_debug_debug(WID_DEFAULT,"%s,line=%d,p=%p",__func__,__LINE__,p);
		return -1;
	}
	p_next = p->if_next;
	m = strlen(ifname);
	n= strlen(p->ifname);
	wid_syslog_debug_debug(WID_DEFAULT,"%s,name %s flag %d\n",__func__,p->ifname, p->lic_flag);
	if((m==n)&&(p->lic_flag == DOWN_LINK_IF_TYPE)&&(strcmp(ifname,p->ifname) == 0)){
		ifindex = p->systemIndex;
		gListenningIF.interfaces = p_next;
		p->if_next = NULL;
		WID_FREE(p);
		p = NULL;
		find = 1;
		if(gListenningIF.count > 0)
			gListenningIF.count--;
		Set_Interface_binding_Info(ifname,0);
		wid_syslog_debug_debug(WID_DEFAULT,"%s,11gListenningIF.count--;count=%d\n",__func__,gListenningIF.count);
	}else{
		while(p_next != NULL){
			n= strlen(p_next->ifname);
			if((m==n)&&(p_next->lic_flag == DOWN_LINK_IF_TYPE)&&(strcmp(ifname,p_next->ifname)==0)){
				//p_next = p_next->if_next;
				p->if_next = p_next->if_next;
				p_next->if_next = NULL;
				ifindex = p_next->systemIndex;
				WID_FREE(p_next);
				p_next = NULL;
				find = 1;
				if(gListenningIF.count > 0)
					gListenningIF.count--;
				Set_Interface_binding_Info(ifname,0);
				wid_syslog_debug_debug(WID_DEFAULT,"%s,22gListenningIF.count--;count=%d\n",__func__,gListenningIF.count);
				break;
			}
			p = p_next;
			p_next= p_next->if_next;
		}
	}
	if(find == 0){
		wid_syslog_debug_debug(WID_DEFAULT,"%s,not find if.line=%d,111111.\n",__func__,__LINE__);
		return INTERFACE_NOT_EXIST;
	}
	#if 0
	ret = Get_Interface_Info(ifname,ifi);
	if(ret != 0){
		Delete_Interface(ifname, ifindex);
		wid_syslog_debug_debug(WID_DEFAULT,"%s,not find if.line=%d,111111.\n",__func__,__LINE__);
	}else{
	}	
	#endif
	Delete_listenning_IF(ifname);
	
	return 0;

}

int DELETE_LISTENNING_IPADDR(unsigned int ipaddr,LISTEN_FLAG flag){	
	int /*m=0,*/n=0;
	int ifindex = 0;
	int find = 0;
	unsigned tmp_ip = 0;
	struct CWMultiHomedInterface *p = NULL,*p_next = NULL; 
	p = gListenningIF.interfaces;
	if(p == NULL){  
		wid_syslog_debug_debug(WID_DEFAULT,"%s,line=%d,p=%p",__func__,__LINE__,p);
		return -1;
	}
	p_next = p->if_next;
	tmp_ip = ((struct sockaddr_in *)&(p->addr))->sin_addr.s_addr;
	if((ipaddr == tmp_ip)&&(p->lic_flag == flag))
	{
		ifindex = p->systemIndex;
		gListenningIF.interfaces = p_next;
		p->if_next = NULL;
		WID_FREE(p);
		p = NULL;
		find = 1;
		if(gListenningIF.count > 0)
			gListenningIF.count--;
		wid_syslog_debug_debug(WID_DEFAULT,"%s,11gListenningIF.count--;count=%d\n",__func__,gListenningIF.count);
	}else{
		while(p_next != NULL){
			n= strlen(p_next->ifname);
			tmp_ip = ((struct sockaddr_in *)&(p_next->addr))->sin_addr.s_addr;
			if((ipaddr == tmp_ip)&&(p_next->lic_flag == flag))
			{
				//p_next = p_next->if_next;
				p->if_next = p_next->if_next;
				p_next->if_next = NULL;
				ifindex = p_next->systemIndex;
				WID_FREE(p_next);
				p_next = NULL;
				find = 1;
				if(gListenningIF.count > 0)
					gListenningIF.count--;
				wid_syslog_debug_debug(WID_DEFAULT,"%s,22gListenningIF.count--;count=%d\n",__func__,gListenningIF.count);
				break;
			}
			p = p_next;
			p_next= p_next->if_next;
		}
	}
	if(find == 0){
		wid_syslog_debug_debug(WID_DEFAULT,"%s,not find if.line=%d,111111.\n",__func__,__LINE__);
		return INTERFACE_NOT_EXIST;
	}
	Delete_listenning_IP(ipaddr,flag);
	
	return 0;

}

void CWACEnterMainLoop() {
	struct sigaction act;
	wid_syslog_info("AC enters in the MAIN_LOOP");
	
	// set signals
	// all the thread we spawn will inherit these settings
	
	act.sa_flags = 0;
	act.sa_handler = CWCriticalTimerExpiredHandler; // called when a timer requested by the thread has expired
	sigaction(CW_CRITICAL_TIMER_EXPIRED_SIGNAL, &act, NULL);
	
	act.sa_flags = 0;
	act.sa_handler = CWSoftTimerExpiredHandler; // called when a timer requested by the thread has expired
	sigaction(CW_SOFT_TIMER_EXPIRED_SIGNAL, &act, NULL);

	act.sa_flags = 0;
	act.sa_handler = CWCriticalTimerExpiredHandler; 
	sigaction(CW_CRITICAL_TIMER_EXPIRED_UPDATE, &act, NULL);
	
	CWThreadSetSignals(SIG_BLOCK, 3, CW_CRITICAL_TIMER_EXPIRED_SIGNAL, // this will be unblocked by the threads that needs timers
 					 CW_SOFT_TIMER_EXPIRED_SIGNAL,
 					 CW_CRITICAL_TIMER_EXPIRED_UPDATE); // this will be unblocked by the threads that needs timers

	
	if(!(CWThreadCreateSpecific(&gIndexSpecific, NULL))) {
		wid_syslog_crit("Critical Error With Thread Data");
		exit(1);
	}
	
	
//	p(WidSemid);
//	wait_v(WidSemid);
	//CWThreadMutexLock(&(gACInterfaceMutex));
	//CWWaitThreadCondition(&gACInterfaceWait, &gACInterfaceMutex);
	//CWThreadMutexUnlock(&gACInterfaceMutex);
/*	CWThread thread_interface;
	if(!CWErr(CWCreateThread(&thread_interface, CWInterface1, NULL))) {
		wid_syslog_crit("Error starting Interface Thread");
		exit(1);
	}
*/
	CWThread thread_wd;
	if(!CWErr(CWCreateThread(&thread_wd, CWThreadWD, NULL,0))) {
		wid_syslog_crit("Error starting Interface Thread");
		exit(1);
	}
	CWThread thread_sta_roaming;
	if(!CWErr(CWCreateThread(&thread_sta_roaming, CWSTARoamingOP, NULL,0))) {
		wid_syslog_crit("Error starting Interface Thread");
		exit(1);
	}
	CWThread thread_channel;
	
	CWThread thread_channel2;
	CWThread thread_txpower;
	if(!CWErr(CWCreateThread(&thread_channel, CWDynamicChannelSelection, NULL,0))) {
		wid_syslog_crit("Error starting Interface Thread");
		exit(1);
	}
	if(!CWErr(CWCreateThread(&thread_channel2, CWDynamicChannelSelection2, NULL,0))) {
		wid_syslog_crit("Error starting Interface Thread");
		exit(1);
	}
	
	if(!CWErr(CWCreateThread(&thread_txpower, CWTransmitPowerControl, NULL,0)))
	{
		wid_syslog_crit("Error starting Interface Thread");
		exit(1);
	}	
	
	CW_REPEAT_FOREVER {
		if((CWNetworkUnsafeMultiHomed(&gACSocket, CWACManageIncomingPacket, CW_FALSE)) == CW_FALSE) 	// CWACManageIncomingPacket will be called
		{	
			if(vrrid != 0){
				wid_syslog_debug_debug(WID_DEFAULT,"%s,CW_FALSE,vrrid=%d,in while(1),gACSocket->count=%d\n",__func__,vrrid,gACSocket.count);
				//sleep(2);
			}
		}
	}
}
// This callback function is called when there is something to read in a CWMultiHomedSocket (see ACMultiHomed.c)
// ***
// sock is the socket that can receive the packet and it can be used to reply
// ***
// buf (array of len chars) contains the packet which is ready on the socket's queue (obtained with MSG_PEEK)
// ***
// incomingInterfaceIndex is the index (different from the system index, see ACMultiHomed.c) of the interface the packet
// was sent to, in the array returned by CWNetworkGetInterfaceAddresses. If the packet was sent to a broadcast/multicast address,
// incomingInterfaceIndex is -1
void CWACManageIncomingPacket(CWSocket sock, char *buf, int readBytes, int incomingInterfaceIndex, int BindingSystemIndex, CWNetworkLev4Address *addrPtr, char *ifname) {
	CWWTPManager *wtpPtr = NULL;
//	char* pData;
	msgqData qData;	
	int ret = 0;
	int WTPID = 0;
	int j;
	int k;
	int m;
	unsigned int flag = 0;/*xiaodawei add, 20101109*/
	// check if sender address is known
	wtpPtr = CWWTPByAddress(addrPtr, sock, &WTPID);
	if((wtpPtr != NULL)&&(!check_wtpid_func(WTPID))){
		wid_syslog_err("%s\n",__func__);
		return;
	}else{
	}
	/*fengwenchao add 20111219*/
	if((AC_WTP[WTPID] != NULL)&&(AC_WTP[WTPID]->BindingSystemIndex != BindingSystemIndex))
	{
		AC_WTP[WTPID]->BindingSystemIndex= BindingSystemIndex;
		memset(AC_WTP[WTPID]->BindingIFName, 0, ETH_IF_NAME_LEN);
		if(ifname != NULL){
			memcpy(AC_WTP[WTPID]->BindingIFName,ifname, strlen(ifname));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		
		//weichao add
		wtp_get_ifindex_check_nas_id(WTPID);
	}
	/*fengwenchao add end*/
	if((wtpPtr != NULL)&&(wtpPtr->BAK_FLAG == 2)){
		wid_syslog_debug_debug(WID_DEFAULT,"wtp %d change ifindex%d\n",WTPID,incomingInterfaceIndex);
		wtpPtr->interfaceIndex = incomingInterfaceIndex;	
		for(j = 0; (AC_WTP[WTPID])&&(j < AC_WTP[WTPID]->RadioCount); j++){
			gWTPs[WTPID].interfaceIndex = incomingInterfaceIndex;
			if((AC_WTP[WTPID])&&(AC_WTP[WTPID]->WTP_Radio[j])){
				for(k = 0;k < L_BSS_NUM;k++ ){
					if((AC_WTP[WTPID])&&(AC_WTP[WTPID]->WTP_Radio[j])&&(AC_WTP[WTPID]->WTP_Radio[j]->BSS[k] != NULL)){
						wid_update_wtp_bss_infov2(WTPID,AC_WTP[WTPID]->WTP_Radio[j]->BSS[k]->BSSIndex);
					}
				}
			}
		}

		
		/*fengwenchao modify begin 20110525*/
		if((incomingInterfaceIndex <gMaxInterfacesCount)&&(incomingInterfaceIndex >= 0))
		{
			gInterfaces[incomingInterfaceIndex].WTPCount++;
		}
		else
		{
			wid_syslog_err("incomingInterfaceIndex =  %d ,  is not legal \n",incomingInterfaceIndex);
		}
		/*fengwenchao modify end*/
		wtpPtr->BAK_FLAG = 0;
		
		CWThreadMutexLock(&MasterBak);
		struct bak_sock *tmp = bak_list;
		if((is_secondary == 0)&&(bak_list!=NULL)){
				bak_add_del_wtp(tmp->sock,B_ADD,WTPID);
				for(j = 0; (AC_WTP[WTPID])&&(j < AC_WTP[WTPID]->RadioCount); j++){
					if((AC_WTP[WTPID])&&(AC_WTP[WTPID]->WTP_Radio[j])&&(AC_WTP[WTPID]->WTP_Radio[j]->AdStat == 1)){
						for(k = 0;k < L_BSS_NUM;k++ ){
							if((AC_WTP[WTPID])&&(AC_WTP[WTPID]->WTP_Radio[j])&&(AC_WTP[WTPID]->WTP_Radio[j]->BSS[k] != NULL)&&(AC_WTP[WTPID]->WTP_Radio[j]->BSS[k]->State == 1)){
								bak_add_del_bss(tmp->sock,B_ADD,AC_WTP[WTPID]->WTP_Radio[j]->BSS[k]->BSSIndex);
							}
						}
					}
				}
			
		}
		CWThreadMutexUnlock(&MasterBak);
	}
	if(wtpPtr != NULL) { // known WTP
		if((AC_WTP[WTPID]!= NULL)&&(AC_WTP[WTPID]->WTPStat == 9)){
			CWThreadMutexLock(&(gWTPs[WTPID].WTPThreadControllistMutex));
			WID_CLEAN_CONTROL_LIST(WTPID);
			CWThreadMutexUnlock(&(gWTPs[WTPID].WTPThreadControllistMutex));
			//AC_WTP[WTPID]->channelsendtimes = 0;
			for(m=0;((AC_WTP[WTPID])&&(m<AC_WTP[WTPID]->RadioCount)&&(m<L_RADIO_NUM));m++){
				AC_WTP[WTPID]->WTP_Radio[m]->channelsendtimes = 0;
			}
			gWTPs[WTPID].currentState = CW_ENTER_RUN;
			AC_WTP[WTPID]->WTPStat = 5;
			//fengwenchao add begin
			if(apstatistics == 1){                //主备切换，第一次由备变主的AC，show ap statistics相关信息没有与原主同步
				wid_set_ap_statistics_v1(WTPID,0);
				wid_set_ap_statistics_v1(WTPID,1);
				/*send to ap interval when switch is enable*/
				wid_set_ap_statistics_interval(WTPID,AC_WTP[WTPID]->apstatisticsinterval);
			}
			AsdWsm_WTPOp(WTPID,WID_MODIFY);    //主备切换更新ASD侧相关信息
			//fengwenchao add end
		}
		// Clone data packet		
		//CW_CREATE_OBJECT_SIZE_ERR(pData, readBytes, { wid_syslog_crit("Out Of Memory"); return; });
		qData.mqid = WTPID%THREAD_NUM+1;
		qData.mqinfo.WTPID = WTPID;
		qData.mqinfo.type = DATA_TYPE;
		qData.mqinfo.u.DataInfo.len = readBytes;
		memset(qData.mqinfo.u.DataInfo.Data, 0, 4096);
		if(buf != NULL){
			memcpy(qData.mqinfo.u.DataInfo.Data, buf, readBytes);
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		if (msgsnd(WidMsgQid, (msgq *)&qData, sizeof(qData.mqinfo), 0) == -1){
			wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
			perror("msgsnd");
		}

		//
		//CWLockSafeList(wtpPtr->packetReceiveList);
		//CWAddElementToSafeListTail(wtpPtr->packetReceiveList, pData, readBytes);
		//CWUnlockSafeList(wtpPtr->packetReceiveList);		
	} else { // unknown WTP
		int seqNum, tmp;
		CWDiscoveryRequestValues values;
		
		if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
			wid_syslog_crit("can't lock treadmutex");
			exit(1);
			}
			tmp = gActiveWTPs;
		CWThreadMutexUnlock(&gActiveWTPsMutex);
		
		if(gActiveWTPs >= WTP_NUM) {
			wid_syslog_crit("Too many WTPs");
			return;
		}
	
		/*
		 MAURO: passa il pacchetto privato del prefisso di lunghzza 
		 del preambolo. Ritorna in seqNum il numero di sequenza 
		 presente nella richiesta.
		if(CWErr(CWParseDiscoveryRequestMessage(&buf[4], readBytes - 4, &seqNum, &values))) { // is a CAPWAP Discovery Request message without preamble
		*/
		if(CWErr(CWParseDiscoveryRequestMessage(buf, readBytes, &seqNum, &values))) {
			wid_syslog_debug_debug(WID_DEFAULT,"######### Discovery State #########\n");
			CWProtocolMessage *msgPtr;
			unsigned int WTPID = 0;
			CWUseSockNtop(addrPtr, wid_syslog_debug_debug(WID_DEFAULT,"CAPWAP Discovery Request from %s", str););
			if(CWDisCheckWTPBoardData( BindingSystemIndex,addrPtr, &(values.WTPBoardData), &WTPID)){
				if(!check_wtpid_func(WTPID)){
					wid_syslog_err("%s\n",__func__);
					return;
				}else{
				}
				if(AC_WTP[WTPID] != NULL){
					wid_syslog_debug_debug(WID_DBUS,"known ap oui_mac:"OUIMACSTR"\n",OUIMAC2STR(AC_WTP[WTPID]->WTPMAC));
                    CWThreadMutexLock(&(gOuiMacXmlMutex));	
					if(oui_mac_filters(AC_WTP[WTPID]->WTPMAC)){
		                 wid_syslog_debug_debug(WID_DBUS,"#####%s :it's known wtp but also can't access because oui_mac_filters successful!############\n",__func__);
                         CWCmpWTPAttach(addrPtr);//free AC_ATTACH
                         CWThreadMutexUnlock(&(gOuiMacXmlMutex));	
						 return;
					}
					CWThreadMutexUnlock(&(gOuiMacXmlMutex));	
				}
				CWThreadMutexLock(&ACAccessWTP);
				ap_add(AC_WTP_ACC,(struct sockaddr_in *)addrPtr,&(values.WTPBoardData),&(values.WTPDescriptor),ifname);
				CWThreadMutexUnlock(&ACAccessWTP);
				int n = 0;
				for(n = 0; n < values.WTPBoardData.vendorInfosCount; n++)
				{
					if((values.WTPBoardData.vendorInfos)[n].type == CW_WTP_MANUFACTURE_OPTION)//CW_WTP_MANUFACTURE_OPTION = 7
					{
						gWTPs[WTPID].oemoption = (values.WTPBoardData.vendorInfos)[n].manuoption;
						if(gWTPs[WTPID].oemoption > (glicensecount-1))
						{
							wid_syslog_debug_debug(WID_WTPINFO,"wtp %d ##wtp oem option %d unknown",WTPID,gWTPs[WTPID].oemoption); 
							gWTPs[WTPID].oemoption = 0;							
						}
						wid_syslog_debug_debug(WID_DEFAULT,"#### save oemoption %d\n",gWTPs[WTPID].oemoption);
					}
				}
				/*xiaodawei modify, 20101108*/
				CWThreadMutexLock(&ACLicense);
				if(g_wtp_count[gWTPs[WTPID].oemoption]->flag==0){
					if(g_wtp_count[gWTPs[WTPID].oemoption]->gcurrent_wtp_count >= g_wtp_count[gWTPs[WTPID].oemoption]->gmax_wtp_count){
						wid_syslog_debug_debug(WID_DEFAULT,"###can not access ap type %d access count over type count\n",gWTPs[WTPID].oemoption);
						wid_syslog_debug_debug(WID_WTPINFO,"wtp %d ###can not access ap type %d access count over type count",WTPID,gWTPs[WTPID].oemoption);	
						CWDestroyDiscoveryRequestValues(&values);
						CWThreadMutexUnlock(&ACLicense);
						return;
					}
				}
				else{
					flag = g_wtp_count[gWTPs[WTPID].oemoption]->flag;
					if(g_wtp_binding_count[flag]->gcurrent_wtp_count >= g_wtp_binding_count[flag]->gmax_wtp_count){
						wid_syslog_debug_debug(WID_DEFAULT,"###can not access ap type %d access count over type count\n",gWTPs[WTPID].oemoption);
						wid_syslog_debug_debug(WID_WTPINFO,"wtp %d ###can not access ap type %d access count over type count",WTPID,gWTPs[WTPID].oemoption);	
						CWDestroyDiscoveryRequestValues(&values);
						CWThreadMutexUnlock(&ACLicense);
						return;;
					}
				}
				CWThreadMutexUnlock(&ACLicense);
				/*END*/
			// don't add this WTP to our list to minimize DoS attacks (will be added after join)
			// send response to WTP
				if(!CWErr(CWAssembleDiscoveryResponse(&msgPtr, seqNum, WTPID))) { // note: we can consider reassembling only changed part AND/OR do this in a new thread
					wid_syslog_warning("Critical Error Assembling Discovery Response");
					//exit(1);	// note: maybe an out-of-memory memory error can be resolved without exit()-ing
							// by killing some thread or doing other funky things
					CWDestroyDiscoveryRequestValues(&values); 
					return;
				}
			
				if(!CWErr(CWNetworkSendUnsafeUnconnected(sock, addrPtr, (*msgPtr).msg, (*msgPtr).offset))) {
					wid_syslog_warning("Critical Error Sending Discovery Response");
					//exit(1);
				}
				
				CW_FREE_PROTOCOL_MESSAGE(*msgPtr);
				CW_FREE_OBJECT_WID(msgPtr);
			}
			//////check if match dynamic ap login
			else
			{
			/////check the dynamic switch
				//if(g_AUTO_AP_LOGIN_SWITCH == 1)
				
				CWThreadMutexLock(&ACAccessWTP);
				ap_add(AC_WTP_ACC,(struct sockaddr_in *)addrPtr,&(values.WTPBoardData),&(values.WTPDescriptor),ifname);	
				CWThreadMutexUnlock(&ACAccessWTP);
				if(g_auto_ap_login.auto_ap_switch == 1)
				{
				wid_syslog_debug_debug(WID_DEFAULT,"wid begin auto ap filter\n");
					WIDAUTOAPINFO *auto_ap_info = NULL;
		
					//if(parse_dynamic_wtp_login_situation(&(values.WTPBoardData),dynamic_wtp_model,dynamic_wtp_sn,dynamic_wtp_mac) == CW_TRUE)
					auto_ap_info = parse_dynamic_wtp_login_situation(&(values.WTPBoardData));
					if((auto_ap_info != NULL)&&
						(auto_ap_info->mac!=NULL)){
					
                         	wid_syslog_debug_debug(WID_DBUS,"autologgin ap oui_mac:"OUIMACSTR"\n",OUIMAC2STR(auto_ap_info->mac));
                            CWThreadMutexLock(&(gOuiMacXmlMutex));	
							if(oui_mac_filters(auto_ap_info->mac)){
                                     wid_syslog_debug_debug(WID_DBUS,"#####%s :can't access because oui_mac_filters successful!############\n",__func__);
	                                 CWDestroyDiscoveryRequestValues(&values);
						             WidDestroyAutoApLoginInfo(auto_ap_info);
									 CWThreadMutexUnlock(&(gOuiMacXmlMutex));	
									 return;
							}
                            CWThreadMutexUnlock(&(gOuiMacXmlMutex));	
					}
					int n = 0;
					int oemoption = 0;
					for(n = 0; n < values.WTPBoardData.vendorInfosCount; n++)
					{
						if((values.WTPBoardData.vendorInfos)[n].type == CW_WTP_MANUFACTURE_OPTION)//CW_WTP_MANUFACTURE_OPTION = 7
						{
							oemoption = (values.WTPBoardData.vendorInfos)[n].manuoption;
							if(oemoption > (glicensecount-1))
							{
								wid_syslog_debug_debug(WID_WTPINFO,"auto wtp##wtp oem option %d unknown",oemoption); 
								oemoption = 0; 						
							}
							wid_syslog_debug_debug(WID_DEFAULT,"#### save oemoption %d\n",oemoption);
						}
					}
					/*xiaodawei modify, 20101109*/
					CWThreadMutexLock(&ACLicense);
					if(g_wtp_count[oemoption]->flag==0){
						if(g_wtp_count[oemoption]->gcurrent_wtp_count >= g_wtp_count[oemoption]->gmax_wtp_count){
							wid_syslog_debug_debug(WID_DEFAULT,"###can not access ap type %d access count over type count\n",oemoption);
							wid_syslog_debug_debug(WID_WTPINFO,"auto wtp ###can not access ap type %d access count over type count",oemoption);	
							CWDestroyDiscoveryRequestValues(&values);
							WidDestroyAutoApLoginInfo(auto_ap_info);
							CWThreadMutexUnlock(&ACLicense);
							return;
						}
					}
					else{
						flag = g_wtp_count[oemoption]->flag;
						if(g_wtp_binding_count[flag]->gcurrent_wtp_count >=g_wtp_binding_count[flag]->gmax_wtp_count){
							wid_syslog_debug_debug(WID_DEFAULT,"###can not access ap type %d access count over type count\n",oemoption);
							wid_syslog_debug_debug(WID_WTPINFO,"auto wtp ###can not access ap type %d access count over type count",oemoption);	
							CWDestroyDiscoveryRequestValues(&values);
							WidDestroyAutoApLoginInfo(auto_ap_info);
							CWThreadMutexUnlock(&ACLicense);
							return;;
						}
					}
					CWThreadMutexUnlock(&ACLicense);
					/*END*/
					if(auto_ap_info != NULL)
					{	
						//printf("model:%s sn:%s mac:%02X %02X %02X %02X %02X %02X\n",auto_ap_info->model,auto_ap_info->sn,auto_ap_info->mac[0],auto_ap_info->mac[1],auto_ap_info->mac[2],auto_ap_info.mac[3]->auto_ap_info->mac[4],auto_ap_info->mac[5]);
						if(!CWErr(CWAssembleDiscoveryResponse(&msgPtr, seqNum, 0)))
						{ 
							wid_syslog_warning("Critical Error Assembling Discovery Response");							
							WidDestroyAutoApLoginInfo(auto_ap_info);
							CWDestroyDiscoveryRequestValues(&values); 
							return;
							//exit(1);	
						}
					
						if(!CWErr(CWNetworkSendUnsafeUnconnected(sock, addrPtr, (*msgPtr).msg, (*msgPtr).offset))) 
						{
							wid_syslog_warning("Critical Error Sending Discovery Response");
							//exit(1);
						}
					
						CW_FREE_PROTOCOL_MESSAGE(*msgPtr);
						CW_FREE_OBJECT_WID(msgPtr);
					}
					WidDestroyAutoApLoginInfo(auto_ap_info);
				}
				else
				{
				//not process yet
				}
			}
			//////
			
			CWDestroyDiscoveryRequestValues(&values); // destroy useless values
		}
		else { // this isn't a Discovery Request
			int i = 0;
			CWACThreadArg *argPtr;
			char __str[128];
			char *ip;
			CWUseSockNtop(addrPtr,wid_syslog_debug_debug(WID_DEFAULT,"Possible Client Hello from %s", str););
			ip = sock_ntop_r(((struct sockaddr*)(addrPtr)), __str);


			if(((i = CWCmpWTPAttach(addrPtr))>0)&&(AC_WTP[i] != NULL)&&(AC_WTP[i]->isused == 1)/*&&(CWWTPMatchBindingInterface(i,BindingSystemIndex))*/){//by weiay

				
			if(!CWErr(CWThreadMutexLock(&gWTPsMutex))) {
				wid_syslog_crit("can't lock threadmutex");
				exit(1);
				}
			// look for the first free slot
//			for(i = 0; i < CW_MAX_WTP && gWTPs[i].isNotFree; i++);
//			for(i = 0; i < CW_MAX_WTP && (AC_WTP[i] == NULL); i++);
			
			CW_COPY_NET_ADDR_PTR(&(gWTPs[i].address), addrPtr);
	
			wid_syslog_debug_debug(WID_DEFAULT,"%s send the message",ip);
			if(AC_WTP[i])
			{
			memset(AC_WTP[i]->WTPIP,0,DEFAULT_LEN);
			if(strlen(ip) > DEFAULT_LEN){
				memcpy(AC_WTP[i]->WTPIP, ip, strlen(ip));
				
			}else{
					memcpy(AC_WTP[i]->WTPIP, ip, strlen(ip));
					
				}
			}

			/*fengwenchao add 20111123*/
			AC_WTP[i]->BindingSystemIndex= BindingSystemIndex;
			memset(AC_WTP[i]->BindingIFName, 0, ETH_IF_NAME_LEN);
			memcpy(AC_WTP[i]->BindingIFName,ifname, strlen(ifname));
			//weichao add
			wtp_get_ifindex_check_nas_id(i);
			//wid_syslog_info(" AC_WTP[%d]->BindingSystemIndex =  %d\n",i,AC_WTP[i]->BindingSystemIndex);
			//wid_syslog_info(" AC_WTP[%d]->BindingIFName =  %s\n",i,AC_WTP[i]->BindingIFName);
			/*fengwenchao add end*/
	//		AC_WTP[i]->WTPStat = 1;
			//gWTPs[i].isNotFree = CW_TRUE;
			gWTPs[i].isRequestClose = CW_FALSE;
			CWThreadMutexUnlock(&gWTPsMutex);
			#ifndef CW_NO_DTLS
			// Capwap receive packets list
			if (!CWErr(CWCreateSafeList(&gWTPs[i].packetReceiveList)))
			{
			    if(!CWErr(CWThreadMutexLock(&gWTPsMutex))) {
					wid_syslog_crit("can't lock threadmutex");
					exit(1);
				}
				gWTPs[i].isNotFree = CW_FALSE;
			    CWThreadMutexUnlock(&gWTPsMutex);
			    return;
			}
			
			CWSetMutexSafeList(gWTPs[i].packetReceiveList, &gWTPs[i].interfaceMutex);
			CWSetConditionSafeList(gWTPs[i].packetReceiveList, &gWTPs[i].interfaceWait);
			#endif		
			CW_CREATE_OBJECT_ERR_WID(argPtr, CWACThreadArg, { wid_syslog_crit("Out Of Memory"); return; });

			argPtr->index = i;
			argPtr->sock = sock;
			argPtr->interfaceIndex = incomingInterfaceIndex;
			if(WID_WTP_INIT(argPtr)==CW_FALSE)
				return;
			
			if (argPtr->interfaceIndex < 0) argPtr->interfaceIndex = 0; // if the packet was addressed to a broadcast address,
			// just choose an interface we like (note: we can consider
			// a bit load balancing instead of hard-coding 0-indexed interface).
			// Btw, Join Request should not really be accepted if addressed to a
			// broadcast address, so we could simply discard the packet
			// and go on.
			// If you leave this code, the WTP Count will increase for the interface
			// we hard-code here, even if it is not necessary the interface we use to
			// send packets to that WTP.
			// If we really want to accept Join Request from broadcast address,
			// we can consider asking to the kernel which interface will be used
			// to send the packet to a specific address (if it remains the same)
			// and than increment WTPCount for that interface instead of 0-indexed one.
			
			// create the thread that will manage this WTP
			/*if(!CWErr(CWCreateThread(&(gWTPs[i].thread), CWManageWTP, argPtr,1))) {
				CW_FREE_OBJECT(argPtr);
				if(!CWErr(CWThreadMutexLock(&gWTPsMutex))) {
					wid_syslog_crit("can't lock threadmutex");
					exit(1);
					}
					CWDestroySafeList(&gWTPs[i].packetReceiveList);
					gWTPs[i].isNotFree = CW_FALSE;
				CWThreadMutexUnlock(&gWTPsMutex);
				return;
			}*/
	
			// Clone data packet		
/*			CW_CREATE_OBJECT_SIZE_ERR(pData, readBytes, {wid_syslog_crit("Out Of Memory"); return; });
			memcpy(pData, buf, readBytes);

			//
			CWLockSafeList(gWTPs[i].packetReceiveList);
			CWAddElementToSafeListTail(gWTPs[i].packetReceiveList, pData, readBytes);
			CWUnlockSafeList(gWTPs[i].packetReceiveList);
*/
				qData.mqid = i%THREAD_NUM+1;
				qData.mqinfo.WTPID = i;
				qData.mqinfo.type = DATA_TYPE;
				qData.mqinfo.u.DataInfo.len = readBytes;
				memset(qData.mqinfo.u.DataInfo.Data, 0, 4096);
				if( buf != NULL){
					memcpy(qData.mqinfo.u.DataInfo.Data, buf, readBytes);
					}
				else
					{
					wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
					}
				if (msgsnd(WidMsgQid, (msgq *)&qData, sizeof(qData.mqinfo), 0) == -1){
					wid_syslog_info("%s msgsend %s",__func__,strerror(errno));
					perror("msgsnd");
				}

			}
			else{
				
				wid_syslog_debug_debug(WID_DEFAULT,"check again\n");
				int index;
				index = WTPAddrCheck(addrPtr);
			/*	if((index > 0)||((i>0)&&(AC_WTP[i]->isused)&&(BindingSystemIndex == -1))){
					//printf("index %d\n",index);
					//printf("i %d\n",i);
					wid_syslog_debug_debug("WTP %d with index %d",i,index);
					if(index == -1)
						index = i;
					if(AC_WTP[index] != NULL){						
						Modify_WLAN_WTP_SETTING(index);
						gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
						//printf("gInterfacesCount %d\n",gInterfacesCount);
						wid_syslog_debug_debug("gInterfacesCount %d\n",gInterfacesCount);
					}
				}
				//else if(g_AUTO_AP_LOGIN_SWITCH == 1)//index = -1 
				else*/ if(g_auto_ap_login.auto_ap_switch == 1)//index = -1 
				{	
					//parse join msg
					int id = 0;
					CWProtocolJoinRequestValues joinRequest;
					memset(&joinRequest,0,sizeof(CWProtocolJoinRequestValues));
					if(!(WidParseJoinRequestMessageForAutoApLogin(buf, readBytes, &seqNum, &joinRequest))) 
					{
						wid_syslog_debug_debug(WID_DEFAULT,"join request error\n");
						return;
					}
					
					//check dynamic wtp index
					wid_syslog_debug_debug(WID_DEFAULT,"wid create auto ap infomation\n");
					int wtp_index = 0;
					
					id = parse_dynamic_wtp_name(joinRequest.name);
					if(id == -1)
					{
						wid_syslog_debug_debug(WID_DEFAULT,"wtp have no wtp name index,drop it\n");
						WidDestroyJoinRequestValuesForAutoApLogin(&joinRequest);
						return;
					}
					else if(id != 0)
					{
						wtp_index = id;
					}
					else
					{
						wid_syslog_debug_debug(WID_DEFAULT,"wtp have no wtp name index,random set wtp index\n");
						if(!CWErr(CWThreadMutexLock(&gWTPsMutex))) 
						{
							wid_syslog_crit("can't lock threadmutex");
							exit(1);
						}
						for(i = 1; i < WTP_NUM; i++) 
						{	
							if((!(gWTPs[i].isNotFree))&&(AC_WTP[i] == NULL))
							{
								wtp_index = i;
								break;
							}
							
						}
						CWThreadMutexUnlock(&gWTPsMutex);
					
					}
					if(wtp_index == 0)
					{
						wid_syslog_debug_debug(WID_DEFAULT,"wid wtp num is already %d ,no space to let new wtp login\n",WTP_NUM-1);
						WidDestroyJoinRequestValuesForAutoApLogin(&joinRequest);
						return;
					}
					wid_syslog_debug_debug(WID_DEFAULT,"wid create auto ap will use index %d\n",wtp_index);
					//////////
					 WIDAUTOAPINFO *auto_ap_info2 = NULL;
					auto_ap_info2 = parse_dynamic_wtp_login_situation(&(joinRequest.WTPBoardData));
					if(auto_ap_info2 != NULL)
					{
						if(id != 0)
						{
							wid_syslog_debug_debug(WID_DEFAULT,"wtp have index,change wtp info,and wait wtp login next time\n");
							wid_dynamic_change_wtp_info(id,auto_ap_info2);
							WidDestroyAutoApLoginInfo(auto_ap_info2);
							WidDestroyJoinRequestValuesForAutoApLogin(&joinRequest);
							return;
						}
						char *name = "autoap";
						
						//printf("model:%s sn:%s mac:%02X %02X %02X %02X %02X %02X\n",auto_ap_info2->model,auto_ap_info2->sn,auto_ap_info2->mac[0],auto_ap_info2->mac[1],auto_ap_info2->mac[2],auto_ap_info2.mac[3],auto_ap_info2->mac[4],auto_ap_info2->mac[5]);
						/* 
						for(i=1; i<WTP_NUM; i++)
						{
							if((AC_WTP[i] != NULL)&&(memcmp(AC_WTP[i]->WTPSN, auto_ap_info2->sn, strlen(AC_WTP[i]->WTPSN)) == 0))
							{
								ret = -1;
								break;
							}

						}
						*/
						if (ret != -1)
						{
							int result = 0;
							result = wid_parse_wtp_model_rmodel(auto_ap_info2);
							if(result == -1)
							{
								wid_syslog_debug_debug(WID_DEFAULT,"wtp model rmodel wrong result %d\n",result);
								WidDestroyAutoApLoginInfo(auto_ap_info2);	
								WidDestroyJoinRequestValuesForAutoApLogin(&joinRequest);
								return;
							}
							char *sn = (char *)(auto_ap_info2->sn);
							char *model = (char *)(auto_ap_info2->realmodel);
							unsigned char *mac = (unsigned char *)WID_MALLOC(7);
							//int apcodeflag = auto_ap_info2->apcodeflag;
							char *code = (char*)(auto_ap_info2->model);
							memset(mac,0,7);
							mac[0] = auto_ap_info2->mac[0];
							mac[1] = auto_ap_info2->mac[1];
							mac[2] = auto_ap_info2->mac[2];
							mac[3] = auto_ap_info2->mac[3];
							mac[4] = auto_ap_info2->mac[4];
							mac[5] = auto_ap_info2->mac[5];
							/*check whether mac exist*/
							for(i=1; i<WTP_NUM; i++)
							{
								CWThreadMutexLock(&(gWTPs[i].WTPThreadMutex));
								if((AC_WTP[i] != NULL)\
									&&(((AC_WTP[i]->WTPMAC != NULL)&&(memcmp(AC_WTP[i]->WTPMAC, mac, MAC_LEN) == 0)&&(memcmp(AC_WTP[i]->WTPMAC, gdefaultMac, MAC_LEN) != 0))\
										/*||((AC_WTP[i]->WTPSN != NULL)&&(memcmp(AC_WTP[i]->WTPSN, sn, strlen(sn)) == 0)&&(memcmp(AC_WTP[i]->WTPSN, gdefaultsn, strlen(AC_WTP[i]->WTPSN)) != 0))*/\
										)\
									/*&&(AC_WTP[i]->WTPStat == 7)*/
								)
								{
									//if(AC_WTP[i]->WTPStat == 7)
									CWAddAC_ATTACH_For_Auto(addrPtr, i);
									wid_syslog_err("wtp mac has already be used\n");
									CW_FREE_OBJECT_WID(mac);
									WidDestroyAutoApLoginInfo(auto_ap_info2);	
									WidDestroyJoinRequestValuesForAutoApLogin(&joinRequest);
									CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));
									return;
								}
								CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));
							}
							//ret = WID_CREATE_NEW_WTP(name,wtp_index,sn,model,1);
							if(!check_wtpid_func(wtp_index)){
								wid_syslog_err("%s\n",__func__);
								CW_FREE_OBJECT_WID(mac);
								WidDestroyAutoApLoginInfo(auto_ap_info2);	
								WidDestroyJoinRequestValuesForAutoApLogin(&joinRequest);
								return ;
							}else{
							}
							CWThreadMutexLock(&(gWTPs[WTPID].WTPThreadMutex));//fengwenchao add 20121123 for AXSSZFI-1050
							ret = WID_CREATE_NEW_WTP(name,wtp_index,mac,model,0,0,code);
							CWThreadMutexUnlock(&(gWTPs[WTPID].WTPThreadMutex));
							
							wid_syslog_debug_debug(WID_DEFAULT,"create auto wtp result %d\n",ret);
							if(ret == 0)
							{
								AC_WTP[wtp_index]->wtp_login_mode = 1;
								
								memset(AC_WTP[wtp_index]->WTPSN, 0, NAS_IDENTIFIER_NAME);
								if(wid_illegal_character_check((char*)sn,strlen(sn),0) == 1){
									if(AC_WTP[wtp_index]->WTPSN && sn){
										memcpy(AC_WTP[wtp_index]->WTPSN, sn, strlen(sn));
										}
									else
										{
										wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
										}
								}else{
									wid_syslog_info("%s wtp %d sn %s something wrong\n",__func__,wtp_index,sn);
								}
								
								if(wid_auto_ap_binding(wtp_index,BindingSystemIndex) == 0)
								{
									//send to asd
									
									CWAddAC_ATTACH_For_Auto(addrPtr, wtp_index);
									AsdWsm_WTPOp(wtp_index,WID_ADD);
									//printf("auto wtp %d create successfully\n",wtp_index);
									wid_syslog_debug_debug(WID_DEFAULT,"auto wtp %d create successfully\n",wtp_index);
									int n = 0;
									for(n = 0; n < joinRequest.WTPBoardData.vendorInfosCount; n++)
									{
										if((joinRequest.WTPBoardData.vendorInfos)[n].type == CW_WTP_MANUFACTURE_OPTION)//CW_WTP_MANUFACTURE_OPTION = 7
										{
											gWTPs[wtp_index].oemoption = (joinRequest.WTPBoardData.vendorInfos)[n].manuoption;
											if(gWTPs[wtp_index].oemoption > (glicensecount-1))
											{
												gWTPs[wtp_index].oemoption = 0;
											}
											wid_syslog_debug_debug(WID_DEFAULT,"#### save oemoption %d\n",gWTPs[wtp_index].oemoption);
										}
									}
								}
								else
								{
									CWThreadMutexLock(&(gWTPs[WTPID].WTPThreadMutex));
									if(AC_WTP[wtp_index] != NULL)
									{
							
										WID_DELETE_WTP(wtp_index);
									}
									CWThreadMutexUnlock(&(gWTPs[WTPID].WTPThreadMutex));
								}
							}
							CW_FREE_OBJECT_WID(mac);
						}
					}
					//////
					/*
					if(AC_WTP[wtp_index] != NULL)
					{
						if(!CWErr(CWThreadMutexLock(&gWTPsMutex))) 
						{
							wid_syslog_crit("can't lock threadmutex");
							exit(1);
						}
						CW_COPY_NET_ADDR_PTR(&(gWTPs[wtp_index].address), addrPtr);
						memcpy(AC_WTP[wtp_index]->WTPIP, ip, strlen(ip));
					
						gWTPs[wtp_index].isNotFree = CW_TRUE;
						gWTPs[wtp_index].isRequestClose = CW_FALSE;
						CWThreadMutexUnlock(&gWTPsMutex);
					}*/
				WidDestroyAutoApLoginInfo(auto_ap_info2);	
				WidDestroyJoinRequestValuesForAutoApLogin(&joinRequest);
				}
				
				return;	
			}
			
		}
	}
}

// simple job: see if we have a thread that is serving address *addressPtr
__inline__ CWWTPManager *CWWTPByAddress(CWNetworkLev4Address *addressPtr, CWSocket sock, int *WTPID) {
	int i;
	if(addressPtr == NULL) return NULL;
	
	CWThreadMutexLock(&gWTPsMutex);
		for(i = 0; i < WTP_NUM; i++) {
			if(gWTPs[i].isNotFree && &(gWTPs[i].address) != NULL &&
						!sock_cmp_addr((struct sockaddr*)addressPtr, (struct sockaddr*)&(gWTPs[i].address), sizeof(CWNetworkLev4Address)) //&&
						/*gWTPs[i].socket == sock*/) { // we treat a WTP that sends packet to a different AC's interface as a new WTP
					if(gWTPs[i].BAK_FLAG == 1){
						wid_syslog_debug_debug(WID_DEFAULT,"change wtp %d sock %d\n",i,sock);
						gWTPs[i].socket = sock;
						gWTPs[i].BAK_FLAG = 2;
					}else if(gWTPs[i].socket != sock){
						continue;
					}
					*WTPID = i;
				CWThreadMutexUnlock(&gWTPsMutex);
				return &(gWTPs[i]);
			}
		}
	CWThreadMutexUnlock(&gWTPsMutex);
	
	return NULL;
}

CWBool CWGetMsgQueue(int *msgqid){
	key_t key;

	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {		
		wid_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		exit(1);
	}
	if ((*msgqid = msgget(key, 0644)) == -1) {
		wid_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		exit(1);
	}
	return CW_TRUE;
}



int CreateWlan = 1;
// session's thread function: each thread will manage a single session with one WTP
CW_THREAD_RETURN_TYPE CWManageWTP(void *arg) {	
	int QID = ((CWACThreadArg*)arg)->index;
	wid_pid_write_v2("CWManageWTP",QID,vrrid);
	int WTPMsgqID;
	CW_FREE_OBJECT_WID(arg);
	CWGetMsgQueue(&WTPMsgqID);
	int total;
	int WTPFirst;
	int WTPEnd = WTP_NUM/THREAD_NUM + 1;
	//printf("%d\n",QID);
	CW_REPEAT_FOREVER
	{
//		int readBytes;
		int  wait = 1;
		int i = 0;
//		int WTPID;
		int GoBack = 1;
		CWProtocolMessage msg;
		CWBool dataFlag = CW_FALSE;
		msgq WTPMsgq;
		msgqData WTPMsgqData;
		msg.msg = NULL;
		msg.offset = 0;
		struct msgqlist *elem;
		// Wait WTP action		
		memset((char*)&WTPMsgq, 0, sizeof(WTPMsgq));
		memset((char*)&WTPMsgqData,0,sizeof(WTPMsgqData));
		for(WTPFirst = 0; WTPFirst <= WTPEnd; WTPFirst++){
			i = WTPFirst*THREAD_NUM + QID - 1;
			if(i >= WTP_NUM)
				break;
			if((AC_WTP[i]!=NULL)&&((AC_WTP[i]->WTPStat != 5)||(AC_WTP[i]->ControlList == NULL)||(AC_WTP[i]->ControlWait != NULL)))
			{	
				if((AC_WTP[i]!=NULL)&&(AC_WTP[i]->ControlWait != NULL))				
					wid_syslog_debug_debug(WID_DEFAULT,"wtpid %d ControlWait != NULL\n",i);
				continue;
			}else if(AC_WTP[i]!=NULL){
			
				//printf("WTPID %d\n",i);
				wait = 0;
				GoBack = 0;
				break;
			}
		}
		
		//printf("2\n");
		while (wait)
		{
			// TODO: Check system
			memset((char*)&WTPMsgq, 0, sizeof(WTPMsgq));
			memset((char*)&WTPMsgqData,0,sizeof(WTPMsgqData));
			if (msgrcv(WTPMsgqID, (msgq*)&WTPMsgqData, sizeof(WTPMsgqData.mqinfo), QID, 0) == -1) {
				perror("msgrcv");
				//continue;
				break;
				//exit(1);
			}
			if(&WTPMsgq && (&WTPMsgqData)){
				memcpy(&WTPMsgq,&WTPMsgqData,sizeof(WTPMsgq));
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,wtpid:%d,WTPMsgq.mqinfo.type=%d.",__func__,__LINE__,WTPMsgq.mqinfo.WTPID,WTPMsgq.mqinfo.type);
			wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,sizeof(WTPMsgq):%d,sizeof(WTPMsgqData)=%d.",__func__,__LINE__,sizeof(WTPMsgq),sizeof(WTPMsgqData));
			wid_syslog_debug_debug(WID_DEFAULT,"%s,%d,rcv msg WTPMsgq.mqinfo.type=%d.",__func__,__LINE__,WTPMsgq.mqinfo.type);
			//printf("recv a msg\n");
			if(WTPMsgq.mqinfo.type == DATA_TYPE){
				//printf("data msg\n");
				i = WTPMsgq.mqinfo.WTPID;
				if(!check_wtpid_func(i)){
					wid_syslog_err("%s\n",__func__);
					return CW_FALSE;
				}else{
				}
				if(AC_WTP[i] == NULL){
					continue;
				}
				GoBack = 0;
				break;
			}
			else if(WTPMsgq.mqinfo.type == CONTROL_TYPE){
				
				wid_syslog_debug_debug(WID_DEFAULT,"control msg %d\n",++total);
				i = WTPMsgq.mqinfo.WTPID; 
				if(!check_wtpid_func(i)){
					wid_syslog_err("%s\n",__func__);
					return CW_FALSE;
				}else{
				}
				if((WTPMsgq.mqinfo.subtype == WTP_S_TYPE)&&(WTPMsgq.mqinfo.u.WtpInfo.Wtp_Op == WTP_REBOOT)){					
					if ((gWTPs[i].currentState != CW_QUIT)&&(gWTPs[i].isRequestClose))
					{
						//printf("2\n");
						
						/*if(is_secondary == 1){
							if(AC_WTP[i]->isused == 1){							
								gWTPs[WTPID].isRequestClose = CW_FALSE; 
								CWTimerCancel(&(gWTPs[i].currentTimer),0); // this will do nothing if the timer isn't active
								CWACStopRetransmission(i);
							}else{
								_CWCloseThread(i);
							}
							continue;
						}*/
						wid_syslog_debug_debug(WID_DEFAULT,"Request close thread");
						_CWCloseThread(i);
					}
					GoBack = 1;
					break;
				}else if((WTPMsgq.mqinfo.subtype == WTP_S_TYPE)&&(WTPMsgq.mqinfo.u.WtpInfo.Wtp_Op == WTP_RESEND)){
					if(!CWErr(CWACResendAcknowledgedPacket(i))) {
						_CWCloseThread(i);
					}
					GoBack = 1;
					break;
				}else{
					elem = (struct msgqlist*)WID_MALLOC(sizeof(struct msgqlist));
					if(elem == NULL){
						perror("malloc");
						return 0;
					}
					memset((char*)&(elem->mqinfo), 0, sizeof(msgqdetail));
					elem->next = NULL;
					memcpy((char*)&(elem->mqinfo),(char*)&(WTPMsgq.mqinfo),sizeof(WTPMsgq.mqinfo));
					WID_INSERT_CONTROL_LIST(i, elem);
					GoBack = 1;
					break;
				}
			}
			else{
				wid_syslog_debug_debug(WID_DEFAULT,"something wronge\n");
				continue;
			}
			//
			//CWWaitThreadCondition(&gWTPs[i].interfaceWait, &gWTPs[i].interfaceMutex);
		}

		if(GoBack){
			continue;
		}		
		if(is_secondary == 1){							
			continue;
		}
	/*	if (gWTPs[i].isRequestClose)
		{
			wid_syslog_notice("Request close thread");
			_CWCloseThread(i);
		}*/

		//
		CWThreadSetSignals(SIG_BLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL, CW_CRITICAL_TIMER_EXPIRED_SIGNAL);

		// 
		//if (CWGetCountElementFromSafeList(gWTPs[i].packetReceiveList) > 0)
		if(WTPMsgq.mqinfo.type == DATA_TYPE)
		{
			CWBool bCrypt = CW_FALSE;
//			char* pBuffer;

			//CWThreadMutexLock(&gWTPs[i].interfaceMutex);

			//pBuffer = (char*)CWGetHeadElementFromSafeList(gWTPs[i].packetReceiveList, NULL);
#ifdef CW_NO_DTLS
			bCrypt = CW_FALSE;
#else
			if ((pBuffer[0] & 0x0f) == CW_PACKET_CRYPT)
				bCrypt = CW_TRUE;
#endif
			//CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);

			if (bCrypt)
			{
/*				if(!CWErr(CWSecurityReceive(gWTPs[i].session, gWTPs[i].buf, CW_BUFFER_SIZE - 1, &readBytes))) 
				{
					wid_syslog_debug_debug("Error during security receive");		// error
					CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
					continue;
				}*/
			}
			else
			{
			//	CWThreadMutexLock(&gWTPs[i].interfaceMutex);
			//	pBuffer = (char*)CWRemoveHeadElementFromSafeList(gWTPs[i].packetReceiveList, &readBytes);
			//	CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);

			//	memcpy(gWTPs[i].buf, pBuffer, readBytes);
				//memcpy(gWTPs[i].buf, WTPMsgq.mqinfo.u.DataInfo.Data, WTPMsgq.mqinfo.u.DataInfo.len);
				//readBytes = WTPMsgq.mqinfo.u.DataInfo.len;
				//memset((char*)&WTPMsgq, 0, sizeof(WTPMsgq));
			//	CW_FREE_OBJECT(pBuffer);
			}

			if(!CWProtocolParseFragment(WTPMsgqData.mqinfo.u.DataInfo.Data, WTPMsgqData.mqinfo.u.DataInfo.len, &(gWTPs[i].fragmentsList), &msg, &dataFlag)) 
			{
				if(CWErrorGetLastErrorCode() == CW_ERROR_NEED_RESOURCE) 
				{
					wid_syslog_debug_debug(WID_DEFAULT,"Need At Least One More Fragment");
				} 
				else 
				{
					CWErrorHandleLast();
				}

				CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
				memset((char*)&WTPMsgqData, 0, sizeof(WTPMsgqData));
				continue;
			}
			memset((char*)&WTPMsgqData, 0, sizeof(WTPMsgqData));

			switch(gWTPs[i].currentState) 
			{
				case CW_ENTER_JOIN: 	// we're inside the join state
				{					
					syslog_wtp_log(i, 1, "NONE", 1);
					if(gWIDLOGHN & 0x01)
						syslog_wtp_log_hn(i,1,0);
					if(gWIDLOGHN & 0x02)
						wid_syslog_auteview(LOG_INFO,AP_UP,AC_WTP[i],0);
					wid_syslog_debug_debug(WID_DEFAULT,"wtp %d enter join state",i);
					if(!ACEnterJoin(i, &msg)) 
					{
						if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT) 
						{
							// Log and ignore other messages
							CWErrorHandleLast();
							wid_syslog_warning("Received something different from a Join Request");
						} 
						else 
						{
							// critical error, close session
							wid_syslog_info("--> Critical Error... closing thread");
							CWErrorHandleLast();
							CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
							_CWCloseThread(i);
						}
					}
	
					break;
				}

				case CW_ENTER_IMAGE_DATA: 	// we're inside the join state
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wtp %d enter imagedata state",i);
					if(!ACEnterImageData(i, &msg)) 
					{
						if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT) 
						{
							// Log and ignore other messages
							CWErrorHandleLast();
							wid_syslog_warning("Received something different from a Join Request");
						} 
						else 
						{
							// critical error, close session
							wid_syslog_info("--> Critical Error... closing thread");
							CWErrorHandleLast();
							CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
							_CWCloseThread(i);
						}
					}
	
					break;
				}

		
				case CW_ENTER_RESET:	// we're inside the reset state
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wtp %d enter reset state",i);
					if(!ACEnterReset(i, &msg)) 
					{
						if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT) 
						{
							// Log and ignore other messages
							CWErrorHandleLast();
							wid_syslog_warning("Received something different from a Join Request");
						} 
						else 
						{
							// critical error, close session
							wid_syslog_info("--> Critical Error... closing thread");
							CWErrorHandleLast();
							CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
							_CWCloseThread(i);
						}
					}
		
					break;
				}
				
				case CW_ENTER_CONFIGURE:
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wtp %d enter configure state",i);
					if(!ACEnterConfigure(i, &msg)) 
					{
						if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT) 
						{
							// Log and ignore other messages
							CWErrorHandleLast();
							wid_syslog_warning("Received something different from a Configure Request");
						} 
						else 
						{
							// critical error, close session
							wid_syslog_info("--> Critical Error... closing thread");
							CWErrorHandleLast();
							CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
							_CWCloseThread(i);
						}
					}
					break;
				}

				case CW_ENTER_DATA_CHECK:
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wtp %d enter datacheck state",i);
					if(!ACEnterDataCheck(i, &msg)) 
					{
						if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT) 
						{
							// Log and ignore other messages
							CWErrorHandleLast();
							wid_syslog_warning("Received something different from a Change State Event Request");
						} 
						else 
						{
							// critical error, close session
							CWErrorHandleLast();
							CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
							_CWCloseThread(i);
						}
					}
					//printf("CW_ENTER_DATA_CHECK\n");
					break;
				}	

				case CW_ENTER_RUN:
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wtp %d enter run state",i);
					if(!ACEnterRun(i, &msg, dataFlag)) 
					{
						if(CWErrorGetLastErrorCode() == CW_ERROR_INVALID_FORMAT) 
						{
							// Log and ignore other messages
							CWErrorHandleLast();
							wid_syslog_warning("--> WTP %d Received something different from a valid Run Message\n",i);
						} 
						else 
						{
							// critical error, close session
							wid_syslog_info("--> Critical Error... closing thread");
							CWErrorHandleLast();
							CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
							_CWCloseThread(i);
						}
					}
	
					break;
				}

				default:
				{
					wid_syslog_debug_debug(WID_DEFAULT,"Not Handled Packet");
					break;
				}
			}

			CW_FREE_PROTOCOL_MESSAGE(msg);
		}
		else
		{	
			CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
			elem = NULL;
			elem = WID_GET_CONTROL_LIST_ELEM(i);
			CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
			if(elem == NULL)
				continue;
			if((elem->mqinfo.subtype == WTP_S_TYPE)&&(elem->mqinfo.u.WtpInfo.Wtp_Op == WTP_REBOOT)){					
				if ((gWTPs[i].currentState != CW_QUIT)&&(gWTPs[i].isRequestClose))
				{
					wid_syslog_info("reboot wtp %d for table update\n",i);
					_CWCloseThread(i);
				}
				WID_FREE(elem);
				continue;
			}

	 if((AC_WTP[i])&&(AC_WTP[i]->ControlList == NULL))
 	 {
	 	 if((AC_WTP[i])&&(AC_WTP[i]->sendsysstart == 0))
	 	 {
		    if(elem->mqinfo.subtype == WTP_S_TYPE)
			{

				if((elem->mqinfo.u.WtpInfo.Wtp_Op == WTP_EXTEND_CMD)&&(memcmp(elem->mqinfo.u.WtpInfo.value,"sysstart",8) == 0))
				{
				}
				else
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wlan cmd expand :sysstart (before)");
					char wlan_cmd_expand[DEFAULT_LEN] = "sysstart";
					msgq msg1;

					struct msgqlist *elem1;
					memset((char*)&msg1, 0, sizeof(msg1));

					msg1.mqid = i%THREAD_NUM+1;

					msg1.mqinfo.WTPID = i;
					msg1.mqinfo.type = CONTROL_TYPE;

					msg1.mqinfo.subtype = WTP_S_TYPE;

					msg1.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;

					memcpy(msg1.mqinfo.u.WtpInfo.value, wlan_cmd_expand, strlen(wlan_cmd_expand));
					
					elem1 = (struct msgqlist*)WID_MALLOC(sizeof(struct msgqlist));

					if(elem1 == NULL){

						perror("malloc");

						return 0;

					}

					memset((char*)&(elem1->mqinfo), 0, sizeof(msgqdetail));

					elem1->next = NULL;

					memcpy((char*)&(elem1->mqinfo),(char*)&(msg1.mqinfo),sizeof(msg1.mqinfo));
					WID_INSERT_CONTROL_LIST(i, elem1);
					//wid_radio_set_extension_command(i,wlan_cmd_expand);
					wid_syslog_debug_debug(WID_DEFAULT,"wlan cmd expand :sysstart (after)");
					AC_WTP[i]->sendsysstart = 1;
				}
		    }
			else
			{
				wid_syslog_debug_debug(WID_DEFAULT,"wlan cmd expand :sysstart (before)");
				char wlan_cmd_expand[DEFAULT_LEN] = "sysstart";
				//wid_radio_set_extension_command(i,wlan_cmd_expand);
				msgq msg1;

				struct msgqlist *elem1;
				memset((char*)&msg1, 0, sizeof(msg1));

				msg1.mqid = i%THREAD_NUM+1;

				msg1.mqinfo.WTPID = i;
				msg1.mqinfo.type = CONTROL_TYPE;

				msg1.mqinfo.subtype = WTP_S_TYPE;

				msg1.mqinfo.u.WtpInfo.Wtp_Op = WTP_EXTEND_CMD;

				memcpy(msg1.mqinfo.u.WtpInfo.value, wlan_cmd_expand, strlen(wlan_cmd_expand));
				elem1 = (struct msgqlist*)WID_MALLOC(sizeof(struct msgqlist));

				if(elem1 == NULL){

					perror("malloc");

					return 0;

				}

				memset((char*)&(elem1->mqinfo), 0, sizeof(msgqdetail));

				elem1->next = NULL;
				memcpy((char*)&(elem1->mqinfo),(char*)&(msg1.mqinfo),sizeof(msg1.mqinfo));

				WID_INSERT_CONTROL_LIST(i, elem1);
				wid_syslog_debug_debug(WID_DEFAULT,"wlan cmd expand :sysstart (after)");
				AC_WTP[i]->sendsysstart = 1;
			}
	 	 }
	 	 }
		 if((elem->mqinfo.subtype == WLAN_S_TYPE)){
			
			wid_syslog_debug_debug(WID_DEFAULT,"***** WTP %d IEEE80211_WLAN_CONFIG_RESPONSE*****\n",i);		
			CWThreadMutexLock(&gWTPs[i].interfaceMutex);
			int seqNum = CWGetSeqNum();
			
			CWBool bResult = CW_FALSE;
	
			if (CWBindingAssembleWlanConfigurationRequest(&(gWTPs[i].messages), &(gWTPs[i].messagesCount), gWTPs[i].pathMTU, seqNum, i, elem))
			{
				if(CWACSendAcknowledgedPacket(i, CW_MSG_TYPE_VALUE_IEEE80211_WLAN_CONFIG_RESPONSE, seqNum)) 
					bResult = CW_TRUE;
				else
					CWACStopRetransmission(i);
			}
			gWTPs[i].interfaceCommand = NO_CMD;
			if (bResult){
				CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
				if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
					wid_syslog_debug_debug(WID_DEFAULT,"WLAN op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
					WID_FREE(AC_WTP[i]->ControlWait);
					AC_WTP[i]->ControlWait = NULL;
				}
					AC_WTP[i]->ControlWait = elem;
				CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
			}
			else
			{					
				CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
				//WID_INSERT_CONTROL_LIST(i, elem);
				if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
					//printf("WLAN op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
					wid_syslog_debug_debug(WID_WTPINFO,"WLAN op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);

					WID_FREE(AC_WTP[i]->ControlWait);
					AC_WTP[i]->ControlWait = NULL;
				}
					AC_WTP[i]->ControlWait = NULL;
					WID_FREE(elem);
					elem = NULL;
				wid_syslog_err("Error sending command.(WLAN_S_TYPE)");
				CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
			}
	
	
			CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);
		}
		else if((elem->mqinfo.subtype == Radio_S_TYPE) )
		{	
			wid_syslog_debug_debug(WID_DEFAULT,"***** WTP %d CONFIGURE_UPDATE_RESPONSE_Radio*****\n",i);
			CWThreadMutexLock(&gWTPs[i].interfaceMutex);
			int seqNum = CWGetSeqNum();
			//int j=0;
					CWBool bResult = CW_FALSE;
	
					if (CWAssembleConfigurationUpdateRequest_Radio(&(gWTPs[i].messages), &(gWTPs[i].messagesCount), gWTPs[i].pathMTU, seqNum, i, elem))
					{
						if(CWACSendAcknowledgedPacket(i, CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_RESPONSE, seqNum)) 
							bResult = CW_TRUE;
						else
							CWACStopRetransmission(i);
					}
					
					if (bResult){
						CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
						if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
							wid_syslog_debug_debug(WID_DEFAULT,"radio op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
							WID_FREE(AC_WTP[i]->ControlWait);
							AC_WTP[i]->ControlWait = NULL;
						}
							AC_WTP[i]->ControlWait = elem;
						CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
					}
					else
					{					
						CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
						//WID_INSERT_CONTROL_LIST(i, elem);
						if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
							WID_FREE(AC_WTP[i]->ControlWait);
							AC_WTP[i]->ControlWait = NULL;
						}
						AC_WTP[i]->ControlWait = NULL;
						if(elem){
							WID_FREE(elem);
							elem = NULL;
						}
						CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
						wid_syslog_err("Error sending command.(Radio_S_TYPE)");
					}

			CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);
		}
		else if((elem->mqinfo.subtype == STA_S_TYPE)){
			if(elem->mqinfo.u.StaInfo.Sta_Op != Sta_SETKEY){
				wid_syslog_debug_debug(WID_DEFAULT,"***** WTP %d STATION_CONFIGURATION_RESPONSE*****\n",i);
				CWThreadMutexLock(&gWTPs[i].interfaceMutex);
				int seqNum = CWGetSeqNum();
				
				CWBool bResult = CW_FALSE;
		
				if (CWAssembleStaConfigurationRequest(&(gWTPs[i].messages), &(gWTPs[i].messagesCount), gWTPs[i].pathMTU, seqNum, i, elem))
				{
					if(CWACSendAcknowledgedPacket(i, CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE, seqNum)) 
						bResult = CW_TRUE;
					else
						CWACStopRetransmission(i);
				}
		
				if (bResult){
					//printf("add sta op successful\n");
					CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
					wid_syslog_debug_debug(WID_DEFAULT," sta op successful\n");
					if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
						wid_syslog_debug_debug(WID_DEFAULT,"sta op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
						WID_FREE(AC_WTP[i]->ControlWait);
						AC_WTP[i]->ControlWait = NULL;
					}
						AC_WTP[i]->ControlWait = elem;
					CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
				}
				else
				{					
					CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
					//WID_INSERT_CONTROL_LIST(i, elem);
					if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
						WID_FREE(AC_WTP[i]->ControlWait);
						AC_WTP[i]->ControlWait = NULL;
					}
					AC_WTP[i]->ControlWait = NULL;
					if(elem){
						WID_FREE(elem);
						elem = NULL;
					}
					CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
					wid_syslog_err("Error sending command.(STA_S_TYPE:Sta_SETKEY)");
				}
		
		
				CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);
			}else {
				wid_syslog_debug_debug(WID_DEFAULT,"***** WTP %d STATION_KEY_CONFIGURATION_RESPONSE*****\n",i);
				CWThreadMutexLock(&gWTPs[i].interfaceMutex);
				int seqNum = CWGetSeqNum();
				
				CWBool bResult = CW_FALSE;
		
				if (CWAssembleStaConfigurationRequest_key(&(gWTPs[i].messages), &(gWTPs[i].messagesCount), gWTPs[i].pathMTU, seqNum, i, elem))
				{
					if(CWACSendAcknowledgedPacket(i, CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE, seqNum)) 
						bResult = CW_TRUE;
					else
						CWACStopRetransmission(i);
				}
				
		
				if (bResult){
					//printf("sta key op successful\n");
					wid_syslog_debug_debug(WID_DEFAULT,"sta key op successful");
					CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
					if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
	//					printf("sta op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
						wid_syslog_debug_debug(WID_DEFAULT,"sta op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
						WID_FREE(AC_WTP[i]->ControlWait);
						AC_WTP[i]->ControlWait = NULL;
					}
						AC_WTP[i]->ControlWait = elem;
					CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
				}
				else
				{					
					CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
					//WID_INSERT_CONTROL_LIST(i, elem);
					if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
						WID_FREE(AC_WTP[i]->ControlWait);
						AC_WTP[i]->ControlWait = NULL ;
					}
					AC_WTP[i]->ControlWait = NULL;
					if(elem){
						WID_FREE(elem);
						elem = NULL;
					}
					CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
					wid_syslog_err("Error sending command.(STA_S_TYPE)");
				}
		
		
				CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);
			}
		}
		else if((elem->mqinfo.subtype == WTP_S_TYPE) )
		{	
			wid_syslog_debug_debug(WID_DEFAULT,"***** WTP %d CONFIGURE_UPDATE_RESPONSE_WTP*****\n",i);
			CWThreadMutexLock(&gWTPs[i].interfaceMutex);
			int seqNum = CWGetSeqNum();
//			int j=0;
			//for(j=0;	j<AC_WTP[i]->RadioCount; j++)
			//	if (AC_WTP[i]->WTP_Radio[j]->CMD != 0x0)
			//	{
					CWBool bResult = CW_FALSE;
	
					if (CWAssembleConfigurationUpdateRequest_WTP(&(gWTPs[i].messages), &(gWTPs[i].messagesCount), gWTPs[i].pathMTU, seqNum, i, elem))
					{
						if(CWACSendAcknowledgedPacket(i, CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_RESPONSE, seqNum)) 
							bResult = CW_TRUE;
						else
							CWACStopRetransmission(i);
					}
					
					//AC_WTP[i]->CMD->setCMD = 0;
					//AC_WTP[i]->WTP_Radio[j]->CMD = 0x0;				
					//gWTPs[i].interfaceCommand = NO_CMD;

					if (bResult){
						CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
						if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
	//						printf("wtp op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
							wid_syslog_debug_debug(WID_DEFAULT,"wtp op something wrong (subtype)%d\n",AC_WTP[i]->ControlWait->mqinfo.subtype);
							WID_FREE(AC_WTP[i]->ControlWait);
							AC_WTP[i]->ControlWait = NULL;
						}
							AC_WTP[i]->ControlWait = elem;
						CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
					}
					else
					{					
						CWThreadMutexLock(&(gWTPs[i].WTPThreadControllistMutex));
						//WID_INSERT_CONTROL_LIST(i, elem);
						if((AC_WTP[i])&&(AC_WTP[i]->ControlWait != NULL)){
							WID_FREE(AC_WTP[i]->ControlWait);
							AC_WTP[i]->ControlWait = NULL;
						}
						AC_WTP[i]->ControlWait = NULL;
						if(elem){
							WID_FREE(elem);
							elem = NULL;
						}
						CWThreadMutexUnlock(&(gWTPs[i].WTPThreadControllistMutex));
						wid_syslog_err("Error sending command.(WTP_S_TYPE)");
					}
			//}

			CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);
		}

		else if((elem->mqinfo.subtype == WDS_S_TYPE) )
		{
			CWThreadMutexLock(&gWTPs[i].interfaceMutex);
			wid_set_wds_state(i,elem->mqinfo.u.WlanInfo.Radio_L_ID,elem->mqinfo.u.WlanInfo.WLANID,elem->mqinfo.u.WlanInfo.Wlan_Op);
			CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);
			WID_FREE(elem);
			elem = NULL;
		}
		
	}
		CWThreadSetSignals(SIG_UNBLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL, CW_CRITICAL_TIMER_EXPIRED_SIGNAL);
	}

}

void _CWCloseThread(int i) {
	unsigned int flag = 0;				/*xiaodawei add for binding flag, 20101125*/
	if(!check_wtpid_func(i)){
		wid_syslog_err("%s\n",__func__);
		return;
	}else{
		if(AC_WTP[i] == NULL){
			wid_syslog_info("%s wtp %d not exist\n",__func__,i);
			return;			
		}
	}
	
	/*if((AC_WTP[i] != NULL)&&(AC_WTP[i]->isused == 1)&&(AC_WTP[i]->unused_flag == 1))
	{
		AC_WTP[i]->isused = 0;
		AC_WTP[i]->unused_flag = 0;
	}*/

	
	CWStateTransition state = CW_QUIT;
	state = gWTPs[i].currentState;
 //	CWThreadSetSignals(SIG_BLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL, CW_CRITICAL_TIMER_EXPIRED_SIGNAL);
	AsdWsm_DataChannelOp(i, WID_DEL);
//**** ACInterface ****
	gWTPs[i].qosValues=NULL;
	CWThreadMutexUnlock(&(gWTPs[i].interfaceMutex));
//**** ACInterface ****

	if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
		wid_syslog_crit("can't lock threadmutex");
		exit(1);
		}
	/*fengwenchao modify begin 20110525*/
	if((gWTPs[i].interfaceIndex <gMaxInterfacesCount)&&(gWTPs[i].interfaceIndex >= 0))
	{
		gInterfaces[gWTPs[i].interfaceIndex].WTPCount--;
	}
	else
	{
		wid_syslog_err("gWTPs[%d].interfaceIndex = %d ,  is not legal \n",i,gWTPs[i].interfaceIndex);
	}
	/*fengwenchao modify end*/
		gActiveWTPs--;
		CWThreadMutexLock(&ACLicense);
		if(g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count > 0){
			(g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count)--;
			if(g_wtp_count[gWTPs[i].oemoption]->flag!=0){
				flag = g_wtp_count[gWTPs[i].oemoption]->flag;
				if(flag < glicensecount){
					if(g_wtp_binding_count[flag] != NULL){
						if(g_wtp_binding_count[flag]->gcurrent_wtp_count > 0){
							(g_wtp_binding_count[flag]->gcurrent_wtp_count)--;
						}
						wid_syslog_debug_debug(WID_DEFAULT,"%s,flag=%u,g_wtp_binding_count->gcurrent_wtp_count=%u",__func__,flag,g_wtp_binding_count[flag]->gcurrent_wtp_count);
					}
				}
			}
		}
		CWThreadMutexUnlock(&ACLicense);
		/*
		(g_wtp_count[gWTPs[i].oemoption]->gcurrent_wtp_count)--;*/
		/*xiaodawei modify, 20101111*/
		/*if(g_wtp_count[gWTPs[i].oemoption]->flag!=0){
			(g_wtp_binding_count[(g_wtp_count[gWTPs[i].oemoption]->flag)]->gcurrent_wtp_count)--;
		}
		*/
		//gWTPs[i].oemoption = 0;
		if(gloadbanlance >= 1)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"ap quit send active wtp count is %d \n",gActiveWTPs);
			CWThreadMutexLock(&ACIPLISTMutex);
			SendActiveWTPCount(gActiveWTPs);
			CWThreadMutexUnlock(&ACIPLISTMutex);
		}
		/*fengwenchao modify 20110525*/
		if((gWTPs[i].interfaceIndex < gMaxInterfacesCount)&&(gWTPs[i].interfaceIndex >= 0))
		{
			CWUseSockNtop(((struct sockaddr*) &(gInterfaces[gWTPs[i].interfaceIndex].addr)),
				wid_syslog_debug_debug(WID_DEFAULT,"Remove WTP on Interface %s (%d)", str, gWTPs[i].interfaceIndex);
			);
		}
		else
		{
			wid_syslog_err("gWTPs[%d].interfaceIndex = %d , is not legal \n",i,gWTPs[i].interfaceIndex);
		}
		/*fengwenchao modify end*/
	CWThreadMutexUnlock(&gActiveWTPsMutex);
	
	wid_syslog_debug_debug(WID_DEFAULT,"Close Thread: %08x", (unsigned int)CWThreadSelf());
	
	if(gWTPs[i].subState != CW_DTLS_HANDSHAKE_IN_PROGRESS) {
		//CWSecurityDestroySession(gWTPs[i].session);
	}
	
	CWTimerCancel(&(gWTPs[i].currentTimer),0); // this will do nothing if the timer isn't active
	CWTimerCancel(&(gWTPs[i].updateTimer),0);

	if((find_in_wtp_list(i) == CW_TRUE))
	{
		//printf("_CWCloseThread ap mangement\n");
		if((AC_WTP[i])&&(AC_WTP[i]->isused)){
			if(!CWErr(CWTimerRequest(CW_REACCESS_INTERVAL_DEFAULT, &(gWTPs[i].thread), &(gWTPs[i].updateTimer), CW_CRITICAL_TIMER_EXPIRED_UPDATE,i)))
			{
				//printf("error _CWCloseThread ap mangement\n");
				//_CWCloseThread(WTPIndex);	
			}
		}else{
			delete_wtp_list(i);
		}
		// start Change State Pending timer

	}
	CWACStopRetransmission(i);

	if (gWTPs[i].interfaceCommandProgress == CW_TRUE)
	{
		CWThreadMutexLock(&gWTPs[i].interfaceMutex);
		gWTPs[i].currentState = CW_QUIT;
		gWTPs[i].interfaceResult = 1;
		gWTPs[i].interfaceCommandProgress = CW_FALSE;
//		CWSignalThreadCondition(&gWTPs[i].interfaceComplete);

		CWThreadMutexUnlock(&gWTPs[i].interfaceMutex);
	}
	
	//gWTPs[i].session = NULL;
	gWTPs[i].subState = CW_DTLS_HANDSHAKE_IN_PROGRESS;
	//CWDeleteList(&(gWTPs[i].fragmentsList), CWProtocolDestroyFragment);
	
	//CW_FREE_OBJECT(gWTPs[i].configureReqValuesPtr);
	
	//CWCleanSafeList(gWTPs[i].packetReceiveList, free);
	//CWDestroySafeList(gWTPs[i].packetReceiveList);
	CWThreadMutexLock(&(gWTPs[i].WTPThreadMutex));
	CWDownWTP(i);	
	wid_wtp_param_init(i,0);
	CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));
	CWThreadMutexLock(&gWTPsMutex);
	gWTPs[i].isNotFree = CW_FALSE;	
	//gWTPs[i].oemoption = 0;
	gWTPs[i].currentState = CW_QUIT;
	CWThreadMutexUnlock(&gWTPsMutex);	
//	CWDownWTP(i);
	/*fengwenchao modify begin 20110525*/
	if(WID_WATCH_DOG_OPEN == 1)
	{
		if((gWTPs[i].interfaceIndex < gMaxInterfacesCount)&&(gWTPs[i].interfaceIndex >= 0))	
		{
			if(gInterfaces[gWTPs[i].interfaceIndex].WTPCount == 0){
				gInterfaces[gWTPs[i].interfaceIndex].tcpdumpflag = 1;
				gInterfaces[gWTPs[i].interfaceIndex].datacount = 0;
				gInterfaces[gWTPs[i].interfaceIndex].times = 0;
				memset(gInterfaces[gWTPs[i].interfaceIndex].ip,0,128);
				sock_ntop_r1(((struct sockaddr*)&(gInterfaces[gWTPs[i].interfaceIndex].addr)), gInterfaces[gWTPs[i].interfaceIndex].ip);
				memset(gInterfaces[gWTPs[i].interfaceIndex].ifname,0,16);
				memcpy(gInterfaces[gWTPs[i].interfaceIndex].ifname,AC_WTP[i]->BindingIFName,strlen(AC_WTP[i]->BindingIFName));
				
			}
		}
		else
		{
			wid_syslog_err("gWTPs[%d].interfaceIndex =  %d  ,is not legal \n",i,gWTPs[i].interfaceIndex);
		}
	}
	/*fengwenchao modify end*/
	/*if(gInterfaces[gWTPs[i].interfaceIndex].WTPCount == 0){
		Modify_WLAN_WTP_SETTING(i);		
		gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
		//printf("gInterfacesCount %d\n",gInterfacesCount);
	}*/

	if((AC_WTP[i] != NULL)&&(AC_WTP[i]->isused == 1)&&(AC_WTP[i]->unused_flag == 1))
	{
			AC_WTP[i]->isused = 0;
			AC_WTP[i]->unused_flag = 0;
	}

	if(state != CW_ENTER_RUN)
	{
		if(g_auto_ap_login.save_switch != 1)
		{
			if((AC_WTP[i])&&(AC_WTP[i]->wtp_login_mode == 1))
			{
				AC_WTP[i]->isused = 0;
				//printf("ap is auto login,delete\n");
				CWThreadMutexLock(&(gWTPs[i].WTPThreadMutex));
				WID_DELETE_WTP(i);
				CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));
			}
		}
	}
//	CWExitThread();
}

void CWCloseThread() {
	int *iPtr;
	if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		wid_syslog_crit("Error Closing Thread");
		return;
	}
	
	_CWCloseThread(*iPtr);
}

void CWCriticalTimerExpiredHandler(int arg) {
	int *iPtr;
	msgq msg;
	CWThreadSetSignals(SIG_BLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL, CW_CRITICAL_TIMER_EXPIRED_SIGNAL);
 	
 	wid_syslog_debug_debug(WID_DEFAULT,"Critical Timer Expired for Thread: %08x", (unsigned int)CWThreadSelf());
	wid_syslog_debug_debug(WID_DEFAULT,"Abort Session");
	//CWCloseThread();

	if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) 
	{
		wid_syslog_debug_debug(WID_DEFAULT,"Error Handling Critical timer");
		CWThreadSetSignals(SIG_UNBLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL, CW_CRITICAL_TIMER_EXPIRED_SIGNAL);
		return;
	}

	// Request close thread
	gWTPs[*iPtr].isRequestClose = CW_TRUE;	
	syslog_wtp_log(*iPtr, 0, "Critical Timer Expired", 0);
	if(gWIDLOGHN & 0x01)
		syslog_wtp_log_hn(*iPtr,0,1);
	if(gWIDLOGHN & 0x02)
		wid_syslog_auteview(LOG_WARNING,AP_DOWN,AC_WTP[*iPtr],1);
	memset((char*)&msg, 0, sizeof(msg));
	msg.mqid = (*iPtr)%THREAD_NUM+1;
	msg.mqinfo.WTPID = *iPtr;
	msg.mqinfo.type = CONTROL_TYPE;
	msg.mqinfo.subtype = WTP_S_TYPE;
	msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_REBOOT;
	msg.mqinfo.u.WtpInfo.WTPID = *iPtr;
	if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
		perror("msgsnd");
	
//	CWDownWTP(*iPtr);
//	CWSignalThreadCondition(&gWTPs[*iPtr].interfaceWait);
}

void CWSoftTimerExpiredHandler(int arg) {
	int *iPtr;
	msgq msg;
	CWThreadSetSignals(SIG_BLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL,CW_CRITICAL_TIMER_EXPIRED_SIGNAL);

	wid_syslog_debug_debug(WID_DEFAULT,"Soft Timer Expired for Thread: %08x", (unsigned int)CWThreadSelf());
	
	if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		wid_syslog_debug_debug(WID_DEFAULT,"Error Handling Soft timer");
		CWThreadSetSignals(SIG_UNBLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL,CW_CRITICAL_TIMER_EXPIRED_SIGNAL);
		return;
	}
	
	if((!gWTPs[*iPtr].isRetransmitting) || (gWTPs[*iPtr].messages == NULL)) {
		wid_syslog_debug_debug(WID_DEFAULT,"Soft timer expired but we are not retransmitting");
		CWThreadSetSignals(SIG_UNBLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL,CW_CRITICAL_TIMER_EXPIRED_SIGNAL);
		CWThreadMutexLock(&(gWTPs[*iPtr].WTPThreadControllistMutex));
		if((AC_WTP[*iPtr] != NULL)&&(AC_WTP[*iPtr]->ControlWait != NULL)){
			WID_FREE(AC_WTP[*iPtr]->ControlWait);
			AC_WTP[*iPtr]->ControlWait = NULL;
		}
		CWThreadMutexUnlock(&(gWTPs[*iPtr].WTPThreadControllistMutex));
		return;
	}

	(gWTPs[*iPtr].retransmissionCount)++;
	
	wid_syslog_debug_debug(WID_DEFAULT,"Retransmission Count increases to %d", gWTPs[*iPtr].retransmissionCount);
	
	if(gWTPs[*iPtr].retransmissionCount >= gCWMaxRetransmit) 
	{
		wid_syslog_debug_debug(WID_DEFAULT,"Peer is Dead");
		//?? _CWCloseThread(*iPtr);
		// Request close thread
		gWTPs[*iPtr].isRequestClose = CW_TRUE;
//		CWDownWTP(*iPtr);
		syslog_wtp_log(*iPtr, 0, "Soft Timer Expired", 0);
		if(gWIDLOGHN & 0x01)
			syslog_wtp_log_hn(*iPtr,0,2162689);
		if(gWIDLOGHN & 0x02)
			wid_syslog_auteview(LOG_WARNING,AP_DOWN,AC_WTP[*iPtr],5);
		memset((char*)&msg, 0, sizeof(msg));
		msg.mqid = (*iPtr)%THREAD_NUM+1;
		msg.mqinfo.WTPID = *iPtr;
		msg.mqinfo.type = CONTROL_TYPE;
		msg.mqinfo.subtype = WTP_S_TYPE;
		msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_REBOOT;
		msg.mqinfo.u.WtpInfo.WTPID = *iPtr;
		if (msgsnd(ACDBUS_MSGQ, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1)
			perror("msgsnd");
//		CWSignalThreadCondition(&gWTPs[*iPtr].interfaceWait);
		return;
	}

	if(!CWErr(CWACResendAcknowledgedPacket(*iPtr))) {
		_CWCloseThread(*iPtr);
	}
	
//wid_syslog_debug_debug("~~~~~~fine ritrasmissione ~~~~~");
	CWThreadSetSignals(SIG_UNBLOCK, 2, CW_SOFT_TIMER_EXPIRED_SIGNAL,CW_CRITICAL_TIMER_EXPIRED_SIGNAL);
}

void CWResetWTPProtocolManager(CWWTPProtocolManager *WTPProtocolManager)
{
	int i;
	CW_FREE_OBJECT_WID(WTPProtocolManager->locationData);
	CW_FREE_OBJECT_WID(WTPProtocolManager->name);
	WTPProtocolManager->sessionID = 0;
	WTPProtocolManager->descriptor.maxRadios= 0;
	WTPProtocolManager->descriptor.radiosInUse= 0;
	WTPProtocolManager->descriptor.encCapabilities= 0;
	
	for(i = 0; i < (WTPProtocolManager->descriptor.vendorInfos).vendorInfosCount; i++) {
		if(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_HARDWARE_VERSION)
		{
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].sysver);
		}else if(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION)
		{
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].ver);
		}
		else
		{
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].valuePtr);
		}
	}
	WTPProtocolManager->descriptor.vendorInfos.vendorInfosCount= 0;
	CW_FREE_OBJECT_WID(WTPProtocolManager->descriptor.vendorInfos.vendorInfos);
	
	WTPProtocolManager->radiosInfo.radioCount= 0;
	CW_FREE_OBJECT_WID(WTPProtocolManager->radiosInfo.radiosInfo);
	CW_FREE_OBJECT_WID(WTPProtocolManager->ACName);
	(WTPProtocolManager->ACNameIndex).count = 0;
	CW_FREE_OBJECT_WID((WTPProtocolManager->ACNameIndex).ACNameIndex);
	(WTPProtocolManager->radioAdminInfo).radiosCount = 0;
	CW_FREE_OBJECT_WID((WTPProtocolManager->radioAdminInfo).radios);
	WTPProtocolManager->StatisticsTimer = 0;
	
	for(i = 0; i < WTPProtocolManager->WTPBoardData.vendorInfosCount; i++) {

		if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].model);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].SN);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].mac);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].Rmodel);
		}
		else
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].valuePtr);
		}
	}
	(WTPProtocolManager->WTPBoardData).vendorInfosCount = 0;
	CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData).vendorInfos);
	CW_FREE_OBJECT_WID(WTPProtocolManager->WTPRebootStatistics);

	//CWWTPResetRebootStatistics(&(WTPProtocolManager->WTPRebootStatistics));

	/*
		**AUTELEN.SOFTWARE.WID:**
		CWNetworkLev4Address address;
		int pathMTU;
		struct sockaddr_in ipv4Address;
		CWProtocolConfigureRequestValues *configureReqValuesPtr;
		CWTimerID currentPacketTimer;
	*/
}

void syslog_wid_wtp_log(char *format,...)
{
	char *ident = "WTP_LOG";
	char buf[2048]; 
	int level = LOG_INFO;
	memset(buf,0,2048);
	openlog(ident, 0, LOG_DAEMON);    
	va_list ptr;
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(level,buf);
	closelog();
}
//use the wtplog
void syslog_wtp_log_hn(int WTPIndex, int login,unsigned int reason_code){
	if(AC_WTP[WTPIndex] == NULL)
		return;
	if(login)
		wid_syslog_hn(LOG_WARNING,"AP","Tunnel Up:Serial Id:%s UpInfo:1 AP Name:%s IPv4:%s IPv6:-NA- FirstTrapTime:%d",AC_WTP[WTPIndex]->WTPSN,AC_WTP[WTPIndex]->WTPNAME,AC_WTP[WTPIndex]->WTPIP,time(NULL));
	else
		wid_syslog_hn(LOG_INFO,"AP","LOG:Connection with AP [%s] goes down by reason of No.%d",AC_WTP[WTPIndex]->WTPModel,reason_code);

}
void syslog_wtp_log(int WTPIndex, int login, char *note, unsigned char flag)//xiaodawei add flag, 0--ap quit, 1--ap run, 20110121
{

	if(AC_WTP[WTPIndex] == NULL)
		return;
	if(login){
		wid_syslog_notice_local7("WTP %d LOGIN\tWTP IP %s\tWTP MAC %02X:%02X:%02X:%02X:%02X:%02X\t[NOTE:%s]\n",WTPIndex,AC_WTP[WTPIndex]->WTPIP,AC_WTP[WTPIndex]->WTPMAC[0],AC_WTP[WTPIndex]->WTPMAC[1],AC_WTP[WTPIndex]->WTPMAC[2],AC_WTP[WTPIndex]->WTPMAC[3],AC_WTP[WTPIndex]->WTPMAC[4],AC_WTP[WTPIndex]->WTPMAC[5],note);
		//syslog_wid_wtp_log("[WTP_LOGIN::%s]\tWTP %d\tWTP SN %s\t[NOTE:%s]\n",nowReadable,WTPIndex,AC_WTP[WTPIndex]->WTPSN,note);
	}else{
		//syslog_wid_wtp_log("[WTP_LOGOUT::%s]\tWTP %d\tWTP SN %s\t[NOTE:%s]\n",nowReadable,WTPIndex,AC_WTP[WTPIndex]->WTPSN,note);
		wid_syslog_notice_local7("WTP %d LOGOUT\tWTP IP %s\tWTP MAC %02X:%02X:%02X:%02X:%02X:%02X\t[NOTE:%s]\n",WTPIndex,AC_WTP[WTPIndex]->WTPIP,AC_WTP[WTPIndex]->WTPMAC[0],AC_WTP[WTPIndex]->WTPMAC[1],AC_WTP[WTPIndex]->WTPMAC[2],AC_WTP[WTPIndex]->WTPMAC[3],AC_WTP[WTPIndex]->WTPMAC[4],AC_WTP[WTPIndex]->WTPMAC[5],note);
	}
	
}
void wid_wtp_param_init(int WTPIndex, unsigned char flag)//xiaodawei add flag, 0--ap quit, 1--ap run, 20110121
{
	time_t now =0;
	int i = 0;
	//char *nowReadable = NULL;
	now = time(NULL);
	//int len;
	//nowReadable = ctime(&now);
	//len = strlen(nowReadable);
	//nowReadable[len-1] = '\0';
	//printf("%s",nowReadable);
	if(AC_WTP[WTPIndex] == NULL)
		return;
	//process some switch
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->mib_info.dos_def_switch = 0;
			AC_WTP[WTPIndex]->mib_info.igmp_snoop_switch = 0;
		}
		for(i=0;i<L_BSS_NUM;i++)
		{
			if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->mib_info.wlan_l2isolation[i].wlanid != 0))
			{
				AC_WTP[WTPIndex]->mib_info.wlan_l2isolation[i].wlanid = 0;
				AC_WTP[WTPIndex]->mib_info.wlan_l2isolation[i].l2_isolation_switch = 0;
			}
		}

		//AC_WTP[WTPIndex]->wifi_extension_info.reportswitch = g_AC_ALL_EXTENTION_INFORMATION_SWITCH;
	}
	if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->add_time != NULL))
	{
		*(AC_WTP[WTPIndex]->add_time) = 0;
	}
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->wifi_extension_info.cpu_trap_flag = 0;
			AC_WTP[WTPIndex]->wifi_extension_info.mem_trap_flag = 0;
			AC_WTP[WTPIndex]->wifi_extension_info.temp_trap_flag = 0;
			//memset(AC_WTP[WTPIndex]->wifi_extension_info.wifi_trap_flag,0,AP_WIFI_IF_NUM);  fengwenchao  modify 20110302 此处初始化会导致wifi故障清除告警无法发送
		}
	}
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->apcminfo.cpu_average = 0;
			AC_WTP[WTPIndex]->apcminfo.cpu_peak_value = 0;
			AC_WTP[WTPIndex]->apcminfo.cpu_times = 0;
			AC_WTP[WTPIndex]->apcminfo.mem_average = 0;
			AC_WTP[WTPIndex]->apcminfo.mem_peak_value = 0;
			AC_WTP[WTPIndex]->apcminfo.mem_times = 0;
			memset(AC_WTP[WTPIndex]->apcminfo.cpu_value,0,10);
			memset(AC_WTP[WTPIndex]->apcminfo.mem_value,0,10);
		}
	}
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->wtp_wifi_snr_stats.snr_average= 0;
			AC_WTP[WTPIndex]->wtp_wifi_snr_stats.snr_max= 0;
			AC_WTP[WTPIndex]->wtp_wifi_snr_stats.snr_min= 100;
			memset(AC_WTP[WTPIndex]->wtp_wifi_snr_stats.snr,0,10);
		}
	}
	/*fengwenchao add 20111118 for GM-3*/
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->heart_time.heart_lose_pkt = 0;
			AC_WTP[WTPIndex]->heart_time.heart_time_avarge = 0;
			AC_WTP[WTPIndex]->heart_time.heart_time_delay = 0;
			AC_WTP[WTPIndex]->heart_time.heart_transfer_pkt = 0;
		}
	}
	/*fengwenchao add end*/
	if(1 == flag)
	{
		if(AC_WTP[WTPIndex])
		{
			//AC_WTP[WTPIndex]->apifinfo.report_switch = 0;
			AC_WTP[WTPIndex]->apifinfo.eth_num = 1;
			//AC_WTP[WTPIndex]->apifinfo.wifi_num = 1;//12.17
			time_t t;
			time(&t);
			//for(i=0;i<2;i++)
			//{
				AC_WTP[WTPIndex]->apifinfo.eth[0].type = 0;
				AC_WTP[WTPIndex]->apifinfo.eth[0].ifindex = 0;
				AC_WTP[WTPIndex]->apifinfo.eth[0].state = 1;
				AC_WTP[WTPIndex]->apifinfo.eth[0].state_time = t;
				AC_WTP[WTPIndex]->apifinfo.wifi[0].type = 0;
				AC_WTP[WTPIndex]->apifinfo.wifi[0].ifindex = 0;
				AC_WTP[WTPIndex]->apifinfo.wifi[0].state = 1;
				AC_WTP[WTPIndex]->apifinfo.wifi[0].state_time = t;
		//	}
		}
	}
	else
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->apifinfo.eth_num = 1;
			//AC_WTP[WTPIndex]->apifinfo.wifi_num = 1;//12.17
			time_t t;
			time(&t);
			AC_WTP[WTPIndex]->apifinfo.eth[0].type = 0;
			AC_WTP[WTPIndex]->apifinfo.eth[0].ifindex = 0;
			AC_WTP[WTPIndex]->apifinfo.eth[0].state = 2;
			AC_WTP[WTPIndex]->apifinfo.eth[0].state_time = t;
			AC_WTP[WTPIndex]->apifinfo.wifi[0].type = 0;
			AC_WTP[WTPIndex]->apifinfo.wifi[0].ifindex = 0;
			AC_WTP[WTPIndex]->apifinfo.wifi[0].state = 2;
			AC_WTP[WTPIndex]->apifinfo.wifi[0].state_time = t;
		}
	}
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->ElectrifyRegisterCircle = 0;
		}
	}
	{
		if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->wid_sample_throughput.current_downlink_throughput = 0;
			AC_WTP[WTPIndex]->wid_sample_throughput.current_uplink_throughput = 0;
			AC_WTP[WTPIndex]->wid_sample_throughput.past_downlink_throughput = 0;
			AC_WTP[WTPIndex]->wid_sample_throughput.past_uplink_throughput = 0;
			AC_WTP[WTPIndex]->wid_sample_throughput.uplink_rate = 0;
			AC_WTP[WTPIndex]->wid_sample_throughput.downlink_rate = 0;
		}
	}
}

void CWDownWTP(unsigned int WTPIndex){
	int i =0;
	int j =0;
//	state = 0;
	//msgq msg;
	//struct msgqlist *elem;	
	//unsigned int bssindex;	
	//char buf[DEFAULT_LEN];
	//struct wds_bssid *wds = NULL;
	//msgq msg2;
	//struct msgqlist *elem2;
	char out[] = "timeout";
	char normal[] = "normal";
	//msgq msg4 ;
	//struct msgqlist *elem4 = NULL; 						
	//char *command = NULL;
//	char *ath_str = NULL;
	if(!check_wtpid_func(WTPIndex)){
		wid_syslog_err("%s\n",__func__);
		return;
	}else{
	}
	if(AC_WTP[WTPIndex] == NULL)
	{
		wid_syslog_err("%s\n",__func__);
		return;
	}
	if(AC_WTP[WTPIndex] != NULL)
	{
	if(AC_WTP[WTPIndex]->sendsysstart != 2)
		AC_WTP[WTPIndex]->sendsysstart = 0;
	
	AC_WTP[WTPIndex]->WTPStat = 7;	
	AC_WTP[WTPIndex]->CTR_ID = 0;
	AC_WTP[WTPIndex]->DAT_ID = 0;	
	AC_WTP[WTPIndex]->CMD->keyCMD = 0;
	AC_WTP[WTPIndex]->CMD->staCMD = 0;
	AC_WTP[WTPIndex]->CMD->CMD = 0;	
	AC_WTP[WTPIndex]->sysver = NULL;
	AC_WTP[WTPIndex]->ver = NULL;	
	AC_WTP[WTPIndex]->neighbordeatimes = 0;
	CW_FREE_OBJECT_WID(AC_WTP[WTPIndex]->codever);
	//memset(AC_WTP[WTPIndex]->WTPIP, 0, 128);
	//memset(AC_WTP[WTPIndex]->WTPMAC, 0, 6);
	//release memory
	/*initialization for apstatsinfo when wtp quit*/
	wid_apstatsinfo_init(WTPIndex);		//xiaodawei add, 20110107
	
	memset(AC_WTP[WTPIndex]->longitude, '\0', LONGITUDE_LATITUDE_MAX_LEN);
	memset(AC_WTP[WTPIndex]->latitude, '\0', LONGITUDE_LATITUDE_MAX_LEN);
	memset(AC_WTP[WTPIndex]->manufacture_date, '\0', MANUFACTURE_DATA_MAX_LEN);
	AC_WTP[WTPIndex]->power_mode = UNKNOWN_POWER_MODE;
	AC_WTP[WTPIndex]->forward_mode = UNKNOWN_FORWARD_MODE;
	AC_WTP[WTPIndex]->radio_work_role = RADIO_WORK_ROLE_UNKNOWN;
	
	}
	CWThreadMutexLock(&(gWTPs[WTPIndex].RRMThreadMutex));
	if((AC_WTP[WTPIndex])&&((AC_WTP[WTPIndex]->NeighborAPInfos) != NULL))
	{
		destroy_ap_info_list(&(AC_WTP[WTPIndex]->NeighborAPInfos));
	}
	
	if((AC_WTP[WTPIndex])&&((AC_WTP[WTPIndex]->NeighborAPInfos2) != NULL))
	{
		destroy_ap_info_list(&(AC_WTP[WTPIndex]->NeighborAPInfos2));
	}
	if((AC_WTP[WTPIndex])&&((AC_WTP[WTPIndex]->rouge_ap_infos) != NULL))
	{
		destroy_ap_info_list(&(AC_WTP[WTPIndex]->rouge_ap_infos));
	}
	CWThreadMutexUnlock(&(gWTPs[WTPIndex].RRMThreadMutex));

	//make sure quit state and reason
	if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->isused == 1))
	{
		WTPQUITREASON quitreason = WTP_INIT;
		Check_Interface_Config(AC_WTP[WTPIndex]->BindingIFName,&quitreason);
		if(quitreason == WTP_INIT)
		{
			AC_WTP[WTPIndex]->quitreason = WTP_TIMEOUT;
		}
		else
		{
			AC_WTP[WTPIndex]->quitreason = quitreason;
		}
	}
	if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->quitreason == WTP_TIMEOUT))
	{
		if(gtrapflag>=4){
			wid_dbus_trap_wtp_divorce_networwok(WTPIndex);
		}
	}
	//AC_WTP[WTPIndex]->channelsendtimes = 1;
	for(j=0;((AC_WTP[WTPIndex])&&(j<AC_WTP[WTPIndex]->RadioCount)&&(j<L_RADIO_NUM));j++)
	{

	    if(AC_WTP[WTPIndex]->WTP_Radio[j] != NULL)    //zhangshu add ,2010-10-11
	    {
    		if(AC_WTP[WTPIndex]->WTP_Radio[j]->auto_channel_cont == 0){
    			AC_WTP[WTPIndex]->WTP_Radio[j]->channelsendtimes = 1;	
    		}
    	}
	}
	//ap quit time
	if(AC_WTP[WTPIndex]->quit_time == NULL)
	{
		AC_WTP[WTPIndex]->quit_time = (time_t *)WID_MALLOC(sizeof(time_t));
		time(AC_WTP[WTPIndex]->quit_time);
	}
	else
	{
		time(AC_WTP[WTPIndex]->quit_time);
	}
	//unsigned char pcy = 0;
	unsigned int state = 0;
	time_t now;
	time(&now);
	syslog(LOG_INFO|LOG_LOCAL7, "WTP %d down,WTP MAC:"MACSTR",WTP IP:%s,Down Time:%s\n",
		WTPIndex,MAC2STR(AC_WTP[WTPIndex]->WTPMAC),AC_WTP[WTPIndex]->WTPIP,ctime(&now));

	//for let-fi
	if (gtrapflag >= 1 && AC_WTP[WTPIndex]->lte_fi_quit_reason) {
		wid_dbus_trap_let_fi_run_quit(WTPIndex, AC_WTP[WTPIndex]->lte_fi_quit_reason);
		wid_syslog_err("trap lte-fi quit reason to snmp\n");
		AC_WTP[WTPIndex]->lte_fi_quit_reason = 0;
	}

	if (gtrapflag >= 1 && AC_WTP[WTPIndex]->lte_switch_date) {
		wid_dbus_trap_wid_lte_fi_uplink_switch(WTPIndex);
		wid_syslog_err("trap lte-fi sys-switch to snmp\n");
		AC_WTP[WTPIndex]->lte_switch_date = NULL;
	}
	
	unsigned int rand_num = rand() * now % 100;
	
	if(gtrapflag>=1 && AC_WTP[WTPIndex]->wid_trap.ignore_percent <= rand_num)
	{
		if (gWTPs[WTPIndex].currentState == CW_ENTER_RUN)
		{
			wid_dbus_trap_ap_run_quit(WTPIndex,state);
		}
    }
	else{
	    AC_WTP[WTPIndex]->wid_trap.ignore_switch = 1;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"** WTP %d quit reason is %s **",WTPIndex,(AC_WTP[WTPIndex]->quitreason == 7)?out:normal);
	//end
	CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
	WID_CLEAN_CONTROL_LIST(WTPIndex);
	CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
	for(i=0; ((AC_WTP[WTPIndex])&&(i<AC_WTP[WTPIndex]->RadioCount)); i++){
		
		//AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_TXP = 0	;
		//AC_WTP[WTPIndex]->WTP_Radio[i]->Radio_Chan = 0;
		AC_WTP[WTPIndex]->WTP_Radio[i]->channelchangetime = 0;
		AC_WTP[WTPIndex]->WTP_Radio[i]->CMD = 0x0;
		AC_WTP[WTPIndex]->WTP_Radio[i]->AdStat = 2;
		AC_WTP[WTPIndex]->WTP_Radio[i]->OpStat = 2;
		AC_WTP[WTPIndex]->WTP_Radio[i]->wifi_state = 1;
		int m =0;
		m = i;
		int type = 2;//auto
		int flag = 0;//disable
		unsigned char wlanid =0;
		AC_WTP[WTPIndex]->CMD->wlanCMD = 0;

		if((AC_WTP[WTPIndex] != NULL)&&(AC_WTP[WTPIndex]->isused == 1)
			&&(AC_WTP[WTPIndex]->WTP_Radio[i] != NULL)
			&&(AC_WTP[WTPIndex]->WTP_Radio[i]->Wlan_Id != NULL))
		{
			struct wlanid *wlan_link = NULL;
			wlan_link = AC_WTP[WTPIndex]->WTP_Radio[i]->Wlan_Id;

			while(wlan_link)
			{
				wlanid = wlan_link->wlanid;
				if((wlanid > 0)&&(wlanid < WLAN_NUM)&&(AC_WLAN[wlanid] != NULL))
				{
					if(gtrapflag>=4)
					{
						wid_dbus_trap_ap_ath_error(WTPIndex,i,wlanid,type,flag);
					}

				}
				wlan_link = wlan_link->next;
			}
		}
		
	int j = 0;			
	for(j = 0;j < L_BSS_NUM; j++)
		if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->WTP_Radio[i])&&(AC_WTP[WTPIndex]->WTP_Radio[i]->BSS[j] != NULL))
		{	unsigned int BssIndex = AC_WTP[WTPIndex]->WTP_Radio[i]->BSS[j]->BSSIndex;
			AC_BSS[BssIndex]->master_to_disable = AC_WTP[WTPIndex]->master_to_disable;//qiuchen add it for AXSSZFI-1191
			AsdWsm_BSSOp(BssIndex, WID_DEL, 0);			
			wid_update_bss_to_wifi(BssIndex,WTPIndex,0);
			AC_WTP[WTPIndex]->WTP_Radio[i]->BSS[j]->State = 0;
			memset(AC_WTP[WTPIndex]->WTP_Radio[i]->BSS[j]->BSSID,0,6);
		}
		
	if(AC_WTP[WTPIndex])
		{
			AC_WTP[WTPIndex]->rx_bytes_before = AC_WTP[WTPIndex]->rx_bytes;
			AC_WTP[WTPIndex]->tx_bytes_before = AC_WTP[WTPIndex]->tx_bytes;
		}
	  /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,start*/
	  #if 0
		if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->ntp_state != 1)){
		wid_set_wtp_ntp(WTPIndex);
	  }
	#endif
     /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,end*/
     
	AC_WTP[WTPIndex]->master_to_disable = 0;//qiuchen add it for AXSSZFI-1191
	//WID_CONFIG_SAVE(WTPIndex);
/*	free(AC_WTP[WTPIndex]->WTP_Radio[0]->BSS[0]->BSSID);
	AC_WTP[WTPIndex]->WTP_Radio[0]->BSS[0]->BSSID = NULL;
	free(AC_WTP[WTPIndex]->WTP_Radio[0]->BSS[0]);
	AC_WTP[WTPIndex]->WTP_Radio[0]->BSS[0] = NULL;*/

	}
}
//dynamic area
CWBool WidParseJoinRequestMessageForAutoApLogin(char *msg, int len, int *seqNumPtr, CWProtocolJoinRequestValues *valuesPtr) {
	CWControlHeaderValues controlVal;
	CWProtocolTransportHeaderValues transportVal;
	
	int offsetTillMessages;
	CWProtocolMessage completeMsg;
	
	if(msg == NULL || seqNumPtr == NULL || valuesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_DEFAULT,"Parse Join Request");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
  	
	CWBool dataFlag = CW_FALSE;
	if(!(CWParseTransportHeader(&completeMsg, &transportVal, &dataFlag))) return CW_FALSE;	
	
	if(!(CWParseControlHeader(&completeMsg, &controlVal))) return CW_FALSE; // will be handled by the caller
	
	// different type
	if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_JOIN_REQUEST)
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Join Request as Expected");
	
	*seqNumPtr = controlVal.seqNum;
	controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS; // skip timestamp
	
	offsetTillMessages = completeMsg.offset;
	
	// parse message elements
	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) {
		unsigned short int elemType=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int elemLen=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(&completeMsg,&elemType,&elemLen);
		
		//wid_syslog_debug_debug("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);
		//printf("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);							
		switch(elemType) 
		{
			case CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE:
				if(!(CWParseWTPBoardData(&completeMsg, elemLen, &(valuesPtr->WTPBoardData)))) return CW_FALSE; // will be handled by the caller
				break; 
			
			case CW_MSG_ELEMENT_LOCATION_DATA_CW_TYPE:
				if(!(CWParseLocationData(&completeMsg, elemLen, &(valuesPtr->location)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_SESSION_ID_CW_TYPE:
				if(!(CWParseSessionID(&completeMsg, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE:
				if(!(CWParseWTPDescriptor(&completeMsg, elemLen, &(valuesPtr->WTPDescriptor)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_IPV4_ADDRESS_CW_TYPE:
				if(!(CWParseWTPIPv4Address(&completeMsg, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_NAME_CW_TYPE:
				if(!(CWParseWTPName(&completeMsg, elemLen, &(valuesPtr->name)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE:
				if(!(CWParseWTPFrameTunnelMode(&completeMsg, elemLen, &(valuesPtr->frameTunnelMode)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE:
				if(!(CWParseWTPMACType(&completeMsg, elemLen, &(valuesPtr->MACType)))) return CW_FALSE; // will be handled by the caller
				break;
			
			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element");
		}
	}
	if(completeMsg.offset != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");

	return CW_TRUE;
}
int parse_dynamic_wtp_name(char *name)
{
	unsigned int wtpid = 0;
	int ret = 0;
	ret = parse_int_ID(name, &wtpid);
	wid_syslog_debug_debug(WID_DEFAULT,"name %s wtpid %d\n",name,wtpid);
	if(ret == 0)
	{
		if(wtpid == 0)
		{
			return 0;//use any id
		}
		else
		{
			if(AC_WTP[wtpid] == NULL)
			{
				return 0;
			}
			else if(gWTPs[wtpid].currentState == 5)
			{
				return -1;//drop
			}
			else
			{
				return wtpid;//change info
			}
		}
	}
	else
	{
		return 0;
	}
}
WIDAUTOAPINFO *parse_dynamic_wtp_login_situation(CWWTPVendorInfos *valPtr)
{
	int i;
	//model wtp infomation
	//char *model = "2010";
	//char *sn = "01010020A30008600002";
	
	//check result flags
	int rmodel_state = 0;
	int model_state = 0;
	int sn_state = 0;
	int mac_state = 0;
	int len = 0;
	//WIDAUTOAPINFO *return_autoapinfo = NULL;
	
	WIDAUTOAPINFO *autoapinfo;
	autoapinfo = (WIDAUTOAPINFO *)WID_MALLOC(sizeof(WIDAUTOAPINFO));
	if(autoapinfo == NULL)
	{
		return NULL;
	}
	
	autoapinfo->mac = NULL;
	autoapinfo->model = NULL;
	autoapinfo->sn = NULL;
	autoapinfo->realmodel = NULL;
	autoapinfo->apcodeflag = 0;
	//save mac parameter
	unsigned char *WTP_MAC = NULL;
	
	
	wid_syslog_debug_debug(WID_DEFAULT," check WTP SN whether match dynamic wtp policy\n");
	
	for(i = 0; i < valPtr->vendorInfosCount; i++){
		if((valPtr->vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)
		{
			len = (valPtr->vendorInfos)[i].length;
			autoapinfo->model = (unsigned char*)WID_MALLOC(len+1);
			if(autoapinfo->model == NULL)
			{
				if(autoapinfo){
					WID_FREE(autoapinfo);
					autoapinfo = NULL;
				}
				perror("malloc");
				return NULL;
			}
			memset(autoapinfo->model,0,len+1);
			memcpy(autoapinfo->model,(valPtr->vendorInfos)[i].model,strlen((char *)(valPtr->vendorInfos)[i].model));
			model_state = 1; 
			//printf("wtp model(code) %s\n",autoapinfo->model);
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
		{
			len = (valPtr->vendorInfos)[i].length;
			char * sn = (char *)((valPtr->vendorInfos)[i].SN);
			str2higher(&sn);
			(valPtr->vendorInfos)[i].SN = (unsigned char *)sn;

			autoapinfo->sn = (unsigned char*)WID_MALLOC(len+1);
			if(autoapinfo->sn == NULL)
			{
				if(autoapinfo){
					WID_FREE(autoapinfo);
					autoapinfo = NULL;
				}
				perror("malloc");
				return NULL;
			}
			memset(autoapinfo->sn,0,len+1);
			memcpy(autoapinfo->sn, (valPtr->vendorInfos)[i].SN,strlen((char *)(valPtr->vendorInfos)[i].SN));	
			sn_state = 1;
			//printf("wtp sn %s\n",autoapinfo->sn);
		}
		if((valPtr->vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
		{
			len = (valPtr->vendorInfos)[i].length; 
			autoapinfo->realmodel = (unsigned char*)WID_MALLOC(len+1);
			if(autoapinfo->realmodel == NULL)
			{
				if(autoapinfo){
					WID_FREE(autoapinfo);
					autoapinfo = NULL;
				}
				perror("malloc");
				return NULL;
			}
			memset(autoapinfo->realmodel,0,len+1);
			memcpy(autoapinfo->realmodel,(valPtr->vendorInfos)[i].Rmodel,strlen((char *)(valPtr->vendorInfos)[i].Rmodel));
			rmodel_state = 1;
			//printf("wtp rmodel %s\n",autoapinfo->realmodel);
		}
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_ID)
		{
			continue;
		}
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_REVISION)
		{
			continue;
		}
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS)
		{
			len = (valPtr->vendorInfos)[i].length;
			if(len != 6)
			{
				//printf("wtp mac error\n");
			}
			/*fengwenchao add 20120113 for AXSSZFI-703*/
			unsigned char muti_brc_mac[MAC_LEN] = {0};
			if( (valPtr->vendorInfos)[i].mac != NULL){
				memcpy(muti_brc_mac,(valPtr->vendorInfos)[i].mac,MAC_LEN);
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			//wid_syslog_info("muti_brc_mac =   %02X:%02X:%02X:%02X:%02X:%02X\n",muti_brc_mac[0],muti_brc_mac[1],muti_brc_mac[2],muti_brc_mac[3],muti_brc_mac[4],muti_brc_mac[5]);
			if(muti_brc_mac[0] & 0x1)
			{
				wid_syslog_err("<error> access wtp mac is broadcast or multicast mac \n");
				return NULL;
			}
			/*fengwenchao add end*/

			WTP_MAC = (unsigned char*)WID_MALLOC(MAC_LEN+1);
			memset(WTP_MAC,0,MAC_LEN+1);
			if(WTP_MAC == NULL)
			{
				perror("malloc");
				if(autoapinfo){
					WID_FREE(autoapinfo);
					autoapinfo = NULL;
				} 			
				return NULL;
			}
			autoapinfo->mac = (unsigned char*)WID_MALLOC(MAC_LEN+1);
			if(autoapinfo->mac == NULL)
			{
				perror("malloc");
				WID_FREE(WTP_MAC);
				WTP_MAC = NULL;
				if(autoapinfo){
					WID_FREE(autoapinfo);
					autoapinfo = NULL;
				}
				return NULL;
			}
			memset(autoapinfo->mac,0,MAC_LEN+1);
			memcpy(WTP_MAC, (valPtr->vendorInfos)[i].mac,MAC_LEN);
			memcpy(autoapinfo->mac, (valPtr->vendorInfos)[i].mac,MAC_LEN);
				
			if(wid_auto_ap_mac_filter(WTP_MAC) == 0)
			{
				mac_state = 1;
				wid_syslog_debug_debug(WID_DEFAULT,"WTP MAC:%02X %02X %02X match dynamic wtp mac policy\n",WTP_MAC[0],WTP_MAC[1],WTP_MAC[2]);
				//printf("WTP MAC:%02X %02X %02X match dynamic wtp mac policy\n",WTP_MAC[0],WTP_MAC[1],WTP_MAC[2]);
				//printf("WTP MAC:%02X %02X %02X match dynamic wtp mac policy\n",autoapinfo->mac[0],autoapinfo->mac[1],autoapinfo->mac[2]);
		
			}
			else
			{
				mac_state = 1;//change to 1 because ap mac change
				//printf("not autelan ap\n");
				wid_syslog_debug_debug(WID_DEFAULT,"not autelan ap\n");
			}
			if(WTP_MAC){
				WID_FREE(WTP_MAC);
				WTP_MAC = NULL;
			}
		}
		else 
		{
		//not process yet
		}
	}
	//decide return value
	if((model_state == 1)&&(mac_state == 1)&&(sn_state == 1))
	{
		//printf("incoming wtp match dynamic wtp policy,AC will send a response\n");
		wid_syslog_debug_debug(WID_DEFAULT,"incoming wtp match dynamic wtp policy,AC will send a response\n");
		//return_autoapinfo = autoapinfo;
		//printf("WTP MAC:%02X %02X %02X match dynamic wtp mac policy\n",return_autoapinfo->mac[0],return_autoapinfo->mac[1],return_autoapinfo->mac[2]);
		
		//CW_FREE_OBJECT(autoapinfo);
		return autoapinfo;
	}
	else
	{
		//printf("incoming wtp not match dynamic wtp policy\n");
		wid_syslog_debug_debug(WID_DEFAULT,"incoming wtp not match dynamic wtp policy,no response\n");
		if(autoapinfo->mac != NULL){
			WID_FREE(autoapinfo->mac);
		}
		if(autoapinfo->model != NULL){
			WID_FREE(autoapinfo->model);
		}
		if(autoapinfo->sn != NULL){
			WID_FREE(autoapinfo->sn);
		}
		if(autoapinfo->realmodel != NULL){
			WID_FREE(autoapinfo->realmodel);
		}
		WID_FREE(autoapinfo);
		return NULL;
	}
}
/*
int dynamicWTPaddrbinding(CWNetworkLev4Address *addrPtr)
{
	int i = 0;
	CWThreadMutexLock(&gWTPsMutex);
		for(i = 1; i < CW_MAX_WTP; i++) 
		{	
		
			if((!(gWTPs[i].isNotFree))
				//&& (&(gWTPs[i].address) == NULL)
				&&(AC_WTP[i] == NULL))
			{ 
				//printf("sock_cpy_addr_port\n");
				if((sock_cpy_addr_port((struct sockaddr*)&(gWTPs[i].address),(struct sockaddr*)addrPtr)) == 0)
				{	
					CWThreadMutexUnlock(&gWTPsMutex);
					//printf("i %d\n",i);
					return i;
				}
			}
		}
	CWThreadMutexUnlock(&gWTPsMutex);
	return -1;
}*/
int wid_auto_ap_mac_filter(unsigned char *mac)
{
	//check auto ap whether aute ap
	if(((*(mac)) == 0)&&((*(mac+1)) == 31)&&((*(mac+2)) == 100))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
int wid_auto_ap_binding(int wtpid,int ifindex)
{
	int interface_state = 0;
	int state = 0;
	int used_state = 0;
	int ret = 0;
	int ret1 = 0;
	int i=0,j=0;
	unsigned int radioid = wtpid*L_RADIO_NUM;
	
	char *name;
	name = (char *)WID_MALLOC(ETH_IF_NAME_LEN+1);
	if(name == NULL)
	{
		perror("malloc");
		return -1;
	}
	memset(name,0,ETH_IF_NAME_LEN+1);

	//find ifindex in the auto ap login iflist
	wid_auto_ap_if *iflist;
	iflist = g_auto_ap_login.auto_ap_if;
	int wlannum = 0;
	unsigned char wlan[L_BSS_NUM];
	memset(wlan,0,L_BSS_NUM);

	while(iflist != NULL)
	{
		if(iflist->ifindex == ifindex)
		{
			memset(name,0,ETH_IF_NAME_LEN+1);
			memcpy(name,iflist->ifname,strlen(iflist->ifname));
			
			wlannum = iflist->wlannum;
			for(i=0;i<L_BSS_NUM;i++)
			{
				if(iflist->wlanid[i] != 0)
				{
					wlan[j] = iflist->wlanid[i];
					j++;
				}
			}
			interface_state = 1;
			break;
		}
		iflist = iflist->ifnext;
	}

	if(interface_state != 1)
	{
		//printf("wtp login interface is not the auto ap login interface\n");
		//printf("wtp login interface ifindex %d\n",ifindex);
		WID_FREE(name);
		name = NULL;
		return -1;
	}
	else//find ifindex in the list,then binding
	{
		ret = WID_BINDING_IF_APPLY_WTP(wtpid,name);
		if(ret == 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_binding interface success\n");
			state = 1;
			for(i=0;i<wlannum;i++)
			{
				if(wlan[i] == 0)
				{
					wid_syslog_debug_debug(WID_DEFAULT,"expect not happen\n");
					continue;
				}
				ret1 = WID_ADD_WLAN_APPLY_RADIO(radioid,wlan[i]);
				if(ret1 == 0)
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_binding wtp %d wlan %d success\n",wtpid,wlan[i]);
				}
				else
				{
					wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_binding wtp %d wlan %d fail\n",wtpid,wlan[i]);
				}
			}		
		}
		else
		{
			wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_binding interface fail\n");
			state = 0;
		}
	}


	if(state == 1)
	{
		used_state = WID_USED_WTP(wtpid);
		if(used_state == 0)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_binding wtp used success\n");
			WID_FREE(name);
			name = NULL;			
			return 0;
		}
		else
		{
			wid_syslog_debug_debug(WID_DEFAULT,"wid_auto_ap_binding wtp used fail\n");
			WID_FREE(name);
			name = NULL;				
			return -1;
		}
	}
	WID_FREE(name);
	name = NULL;		
	
	return -1;

}
int wid_dynamic_change_wtp_info(int wtpid,WIDAUTOAPINFO *info)
{
	int len = 0;
	if((AC_WTP[wtpid] == NULL)||(info == NULL))
	{
		return -1;
	}
	if(info->mac != NULL)
	{
		
		if(memcmp(AC_WTP[wtpid]->WTPMAC,info->mac,MAC_LEN) != 0)
		{
			memset(AC_WTP[wtpid]->WTPMAC,0,MAC_LEN);
			
		    memcpy(AC_WTP[wtpid]->WTPMAC,info->mac,MAC_LEN);		
				
		}
/*		len = strlen((char *)info->mac);
		if(strncmp((char *)AC_WTP[wtpid]->WTPMAC,(char *)info->mac,len) != 0)
		{
			CW_FREE_OBJECT(AC_WTP[wtpid]->WTPMAC);
			AC_WTP[wtpid]->WTPMAC = (unsigned char *)malloc(len+1);
			memset(AC_WTP[wtpid]->WTPMAC,0,len+1);
			AC_WTP[wtpid]->WTPMAC[0] = info->mac[0];
			AC_WTP[wtpid]->WTPMAC[0] = info->mac[1];
			AC_WTP[wtpid]->WTPMAC[0] = info->mac[2];
			AC_WTP[wtpid]->WTPMAC[0] = info->mac[3];
			AC_WTP[wtpid]->WTPMAC[0] = info->mac[4];
			AC_WTP[wtpid]->WTPMAC[0] = info->mac[5];
		}*/
	}
	if(info->sn != NULL)
	{
		len = strlen((char *)info->sn);
		if((AC_WTP[wtpid])&&(AC_WTP[wtpid]->WTPSN)&&(strncmp(AC_WTP[wtpid]->WTPSN,(char *)info->sn,len) != 0))
		{
			/*default code
			CW_FREE_OBJECT(AC_WTP[wtpid]->WTPSN);
			AC_WTP[wtpid]->WTPSN = (char *)malloc(len+1);
			memset(AC_WTP[wtpid]->WTPSN,0,len+1);
			memcpy(AC_WTP[wtpid]->WTPSN,info->sn,len);
			*/
			memset(AC_WTP[wtpid]->WTPSN,0,NAS_IDENTIFIER_NAME);
			if(wid_illegal_character_check((char*)info->sn,len,0) == 1){
				if(AC_WTP[wtpid]->WTPSN ){
					memcpy(AC_WTP[wtpid]->WTPSN,info->sn,strlen((char *)info->sn));
					}
				else
					{
					wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
					}
			}else{
				wid_syslog_info("%s wtp %d sn %s something wrong\n",__func__,wtpid,info->sn);
			}
		}
	}
	if(info->model != NULL)
	{
		len = strlen((char *)info->model);
		if((AC_WTP[wtpid])&&(AC_WTP[wtpid]->APCode)&&(strncmp(AC_WTP[wtpid]->APCode,(char *)info->model,len) != 0))
		{
			CW_FREE_OBJECT_WID(AC_WTP[wtpid]->APCode);
			AC_WTP[wtpid]->APCode = (char *)WID_MALLOC(len+1);
			memset(AC_WTP[wtpid]->APCode,0,len+1);
			if(AC_WTP[wtpid]->APCode ){
				memcpy(AC_WTP[wtpid]->APCode,info->model,strlen((char *)info->model));
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
		}
	}
	if(info->realmodel != NULL)
	{
		len = strlen((char *)info->realmodel);
		if((AC_WTP[wtpid])&&(AC_WTP[wtpid]->WTPModel)&&(strncmp(AC_WTP[wtpid]->WTPModel,(char *)info->realmodel,len) != 0))
		{
			CW_FREE_OBJECT_WID(AC_WTP[wtpid]->WTPModel);
			AC_WTP[wtpid]->WTPModel= (char *)WID_MALLOC(len+1);
			memset(AC_WTP[wtpid]->WTPModel,0,len+1);
			if(AC_WTP[wtpid]->WTPModel != NULL)
				memcpy(AC_WTP[wtpid]->WTPModel,info->realmodel,strlen((char *)info->realmodel));
			
		}
	}
	return 0;
}
/* book modify,2011-11-15 */
int wid_parse_wtp_model_rmodel(WIDAUTOAPINFO *info)
{
	int ret = 0;
	int len1 = 0;/*WTPModel lenth*/
	int len2 = 0;/*str model lenth*/
	//int i = 0;
	CWConfigVersionInfo *pnode;
	//CWConfigVersionInfo *confignode;
	if(info == NULL)
	{
		return -1;
	}
	else if(info->realmodel == NULL)
	{
	    wid_syslog_debug_debug(WID_WTPINFO, " model is null\n");
		return -1;
	}
	else 
	{
		//char *WTPModel = (char *)info->model;
		char *RealModel = (char *)info->realmodel;

		pnode = gConfigVersionInfo;

		if(RealModel  != NULL)
		{
			len1 = strlen(RealModel);
		}
		while(pnode != NULL)
		{
			if(pnode->str_ap_model != NULL)
			{
				len2 = strlen(pnode->str_ap_model);
			}
			if((pnode->str_ap_model != NULL)&&(RealModel != NULL)&&(len2 == len1)&&(strcmp(pnode->str_ap_model,RealModel) == 0)) 
			{
				ret = 1;
    			break;
			}
			pnode = pnode->next;
		}	
	}
	
	if(ret == 1)
	    return 0;
	
	return -1;
}
void WidDestroyJoinRequestValuesForAutoApLogin(CWProtocolJoinRequestValues *valPtr) {
	int i;
	
	if(valPtr == NULL) return;
	
	for(i = 0; i < valPtr->WTPBoardData.vendorInfosCount; i++) {
		
		if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((valPtr->WTPBoardData.vendorInfos)[i].model);
		}
		else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
		{
			CW_FREE_OBJECT_WID((valPtr->WTPBoardData.vendorInfos)[i].SN);
		}
		else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS)
		{
			CW_FREE_OBJECT_WID((valPtr->WTPBoardData.vendorInfos)[i].mac);
		}
		else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((valPtr->WTPBoardData.vendorInfos)[i].Rmodel);
		}
		else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_CODE_VERSION)
		{
			CW_FREE_OBJECT_WID((valPtr->WTPBoardData.vendorInfos)[i].codever);
		}
		else
		{
			CW_FREE_OBJECT_WID((valPtr->WTPBoardData.vendorInfos)[i].valuePtr);
		}
	}
	
	CW_FREE_OBJECT_WID(valPtr->WTPBoardData.vendorInfos);

	for(i = 0; i < (valPtr->WTPDescriptor.vendorInfos).vendorInfosCount; i++) {

		if(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_HARDWARE_VERSION)
		{
			CW_FREE_OBJECT_WID(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].sysver);
		}else if(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION)
		{
			CW_FREE_OBJECT_WID(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].ver);
		}
		else
		{
			CW_FREE_OBJECT_WID(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].valuePtr);
		}
	}
	CW_FREE_OBJECT_WID((valPtr->WTPDescriptor.vendorInfos).vendorInfos);
	
}
void WidDestroyAutoApLoginInfo(WIDAUTOAPINFO *info)
{
	if(info == NULL) return;

	CW_FREE_OBJECT_WID(info->mac);
	CW_FREE_OBJECT_WID(info->sn);
	CW_FREE_OBJECT_WID(info->model);
	CW_FREE_OBJECT_WID(info->realmodel);
	
	CW_FREE_OBJECT_WID(info);

	return ;
}

int wid_wtp_login_loadconfig(unsigned int wtpid)
{
	wlan_t wlanid = 0;
	unsigned char l_radioid = 0;
	struct wlanid *wlan_id = NULL;
	//unsigned int bssindex = 0;
	unsigned int radioid = 0;
	//unsigned int bind_wlan = 0;
	//int i=0;
	char apcmd[WID_SYSTEM_CMD_LENTH] = {0};
	char *showStr = NULL;	
	char *cursor = NULL;
	int totalLen = 0;
	cursor = (char*)apcmd;
	showStr = (char*)apcmd;
	//unsigned char ip[MAX_IP_STRLEN] = {0};  
	//unsigned char *ipstr = ip;
	
	WID_CHECK_WTP_STANDARD_RET(wtpid, -1);

	wid_syslog_info("%s: wtp %d enter run reload config...\n", __func__, wtpid);	

	for (l_radioid = 0; (l_radioid < AC_WTP[wtpid]->RadioCount); l_radioid++)
	{
		memset(apcmd, 0, WID_SYSTEM_CMD_LENTH);
		totalLen = 0;
		cursor = (char*)apcmd;
		showStr = (char*)apcmd;
		
		radioid = wtpid * L_RADIO_NUM + l_radioid;
		
		/* list radio config */
		if (NULL == AC_WTP[wtpid]->WTP_Radio[l_radioid])
		{
			continue;
		}
		
		/*radio-config-without-bind-wlan-begin*/
		
		wid_syslog_info("%s: local radio %d global radio %d wifi-locate is %d\n", __func__,
							l_radioid,
							AC_WTP[wtpid]->WTP_Radio[l_radioid]->Radio_G_ID,
							AC_WTP[wtpid]->WTP_Radio[l_radioid]->scanlocate_group.wifi_locate.enable);

		/* wifi-locate */
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid] != NULL
			&&AC_WTP[wtpid]->WTP_Radio[l_radioid]->scanlocate_group.wifi_locate.enable == 1)
		{
			wid_syslog_info("%s: begin wifi-locate-config\n", __func__);
			wid_to_ap_wifi_locate_config(radioid);
		}
		/*radio-config-without-bind-wlan-end*/

		
		if (0 == AC_WTP[wtpid]->WTP_Radio[l_radioid]->isBinddingWlan)
		{
			continue;
		}

		/* reload RADIO's wlan config */
		wlan_id = AC_WTP[wtpid]->WTP_Radio[l_radioid]->Wlan_Id;
		for (; NULL != wlan_id; wlan_id = wlan_id->next)
		{
			totalLen = 0;
			cursor = (char*)apcmd;
			showStr = (char*)apcmd;
			memset(apcmd, 0,WID_SYSTEM_CMD_LENTH);
			
			wlanid = wlan_id->wlanid;
			if ((wlanid >= WLAN_NUM)
				|| (NULL == AC_WLAN[wlanid])
				|| (0 != AC_WLAN[wlanid]->Status)) /* is wlan disable */
			{
				continue;
			}

			#if 0
			//huxuefeng
			/* wlan config */

			if(AC_WLAN[wlanid]->extra_portal.isenable !=0)
			{
                                totalLen += sprintf(cursor,ENABLE_EXTRA_PORTAL_IP";",
								l_radioid,wlanid);
				cursor = showStr + totalLen;
			}

			if( AC_WLAN[wlanid]->extra_portal.ip_num !=0)
			{
				for(i=0; i<DEFAULT_PORTAL_IP_NUM; i++)
				{
					if(AC_WLAN[wlanid]->extra_portal.portal_ip[i] != 0)
					{
						ip_long2str(AC_WLAN[wlanid]->extra_portal.portal_ip[i], &ipstr);
						totalLen += sprintf(cursor,ADD_EXTRA_PORTA_IP";",
									l_radioid,wlanid, ipstr);
						cursor = showStr + totalLen;
					}

				}

			}
			#endif

#if 0
//huxuefeng
			if(totalLen != 0)
			{
				wid_syslog_debug_debug(WID_DEFAULT,"%s-%d to wtp %d cmd is %s\n",
										__func__,  __LINE__, wtpid, apcmd);
				wid_radio_set_extension_command(wtpid, apcmd);
			}
#endif
			#if 0
			//huxuefeng
			/* bss config */
			bssindex = AC_WLAN[wlanid]->S_WTP_BSS_List[wtpid][l_radioid];
			if (0 == bssindex)
			{
				continue;
			}
			if (0 != AC_WLAN[wlanid]->balance_switch)
			{
				//wtp_balance_handle(wtpid, wlanid, AC_WLAN[wlanid]->balance_switch);
				wid_bss_balance_switch(wtpid, l_radioid, wlanid);
			}
			/* qos config */
			if (0 != memcmp(QOS_AP_11E_TO_1P_DEFAULT, AC_WLAN[wlanid]->qos_ap_11e_to_1p, MAX_QOS_11E_VAL+1))
			{
				set_ap_bss_qos_map_default(wtpid, l_radioid, QOS_MAP_11E_TO_1P, wlanid);
			}
			if (0 != memcmp(QOS_AP_11E_TO_DSCP_DEFAULT, AC_WLAN[wlanid]->qos_ap_11e_to_dscp, MAX_QOS_11E_VAL+1))
			{
				set_ap_bss_qos_map_default(wtpid, l_radioid, QOS_MAP_11E_TO_DSCP, wlanid);
			}
			if (0 != memcmp(QOS_AP_DSCP_TO_11E_DEFAULT, AC_WLAN[wlanid]->qos_ap_dscp_to_11e, MAX_QOS_DSCP_VAL+1))
			{
				set_ap_bss_qos_map_default(wtpid, l_radioid, QOS_MAP_DSCP_TO_11E, wlanid);
			}
			if (0 != memcmp(QOS_AP_1P_TO_11E_DEFAULT, AC_WLAN[wlanid]->qos_ap_1p_to_11e, MAX_QOS_1P_VAL+1))
			{
				set_ap_bss_qos_map_default(wtpid, l_radioid, QOS_MAP_1P_TO_11E, wlanid);
			}
			if (0 != AC_WLAN[wlanid]->qos_ap_11e_to_1p_switch)
			{
				set_ap_bss_qos_map_switch(wtpid, l_radioid, wlanid, QOS_MAP_11E_TO_1P, AC_WLAN[wlanid]->qos_ap_11e_to_1p_switch);
			}
			if (0 != AC_WLAN[wlanid]->qos_ap_11e_to_dscp_switch)
			{
				set_ap_bss_qos_map_switch(wtpid, l_radioid, wlanid, QOS_MAP_11E_TO_DSCP, AC_WLAN[wlanid]->qos_ap_11e_to_dscp_switch);
			}
			#endif

			#if 0
			//huxuefeng
			for(bind_wlan = 0; bind_wlan < 8; bind_wlan++)
    		{
    			if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->cpe_intf[bind_wlan].vlan_count != 0)
    			{
    				wid_radio_set_cpe_channel(wtpid,l_radioid,bind_wlan);
    			}
    		}
			#endif
		}

		#if 0
		//huxuefeng
		/* reload RADIO's config */
		if(AC_WTP[wtpid]->WTP_Radio[l_radioid]->spec_analysis.enalbe)
		{
		//	set_ap_spectrum_analysis(radioid);
		}
	
		if(((AC_WTP[wtpid]->WTP_Radio[l_radioid]->Radio_Type&IEEE80211_11N)||(AC_WTP[wtpid]->WTP_Radio[l_radioid]->Radio_Type&IEEE80211_11AC))
    	    &&(AC_WTP[wtpid]->WTP_Radio[l_radioid]->cwmode != 0))
    	{
    		wid_radio_set_cmmode(wtpid*L_RADIO_NUM+l_radioid);
		}
		if((AC_WTP[wtpid]->WTP_Radio[l_radioid]->Radio_Type&IEEE80211_11N)||(AC_WTP[wtpid]->WTP_Radio[l_radioid]->Radio_Type&IEEE80211_11AC))
    	{
            wid_radio_set_guard_interval(wtpid*L_RADIO_NUM+l_radioid);
            wid_radio_set_guard_interval_change_rate(wtpid*L_RADIO_NUM+l_radioid);										
			wid_radio_change_rate(wtpid*L_RADIO_NUM+l_radioid);
		}
		#endif
	}

	/* WTP CONFIG */
#if 0	
	if(1 == AC_WTP[wtpid]->mib_info.dos_def_switch)
    {
        wid_set_ap_dos_def_enable(wtpid);
    }
#endif
//	AC_WTP[wtpid]->load_config_flag = 1;	/* DONE */

	return 0;
}

