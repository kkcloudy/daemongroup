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
* CWAC.h
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


#ifndef __CAPWAP_CWAC_HEADER__
#define __CAPWAP_CWAC_HEADER__

/*_______________________________________________________*/
/*  *******************___INCLUDE___*******************  */
#include "CWCommon.h"
#include "ACMultiHomedSocket.h"
#include "ACProtocol.h"
#include "ACInterface.h"
#include "ACBinding.h"
#include "wcpss/waw.h"

/*______________________________________________________*/
/*  *******************___DEFINE___*******************  */
#define CW_MAX_WTP					1024
#define CW_CRITICAL_TIMER_EXPIRED_UPDATE		500
#define CW_CRITICAL_TIMER_EXPIRED_SIGNAL		SIGUSR2
#define CW_SOFT_TIMER_EXPIRED_SIGNAL			SIGUSR1

#define CW_WLAN_START_SERVICE_TIMER_EXPIRED		501
#define CW_WLAN_STOP_SERVICE_TIMER_EXPIRED		502

#define LOG_FILE_NAME					"ac.log.txt"

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
typedef struct {			
	CWNetworkLev4Address address;
	CWThread thread;
#ifndef CW_NO_DTLS
	CWSecuritySession session;
#endif
	CWBool isNotFree;
	CWBool isRequestClose;
	CWStateTransition currentState;
	int interfaceIndex;
	CWSocket socket;
	//char buf[CW_BUFFER_SIZE];
	enum  {
		CW_DTLS_HANDSHAKE_IN_PROGRESS,
		CW_WAITING_REQUEST,
		CW_COMPLETED,
	} subState;		
	CWSafeList packetReceiveList;
	
	CWTimerID currentTimer; 	// depends on the current state: WaitJoin, NeighborDead
	CWTimerID updateTimer; 	
	CWTimerID heartbeatTimer; 
	CWList fragmentsList;
	int pathMTU;
	
	/**** ACInterface ****/
	int interfaceResult;
	CWBool interfaceCommandProgress;
	int interfaceCommand;
	CWThreadMutex interfaceSingleton;
	CWThreadMutex interfaceMutex;
	CWThreadMutex WTPThreadMutex;
	CWThreadMutex WTPThreadControllistMutex;
	CWThreadMutex mutex_controlList;
	CWThreadMutex RRMThreadMutex;
	CWThreadMutex WIDSThreadMutex;
	CWThreadCondition interfaceWait;
	CWThreadCondition interfaceComplete;
	WTPQosValues* qosValues;
	/**** ACInterface ****/

	CWWTPProtocolManager WTPProtocolManager;

	CWProtocolMessage *messages;	// Retransmission
 	int messagesCount;
 	int retransmissionCount;
 	CWTimerID currentPacketTimer;
 	CWBool isRetransmitting;

	int responseType;		// expected response
	int responseSeqNum;
	unsigned int BAK_FLAG;
	unsigned int oemoption;//0 autelan made 1 oem product
	//int unrecognizedMsgType;	//Unrecognized message type value
} CWWTPManager;		// Struct that describes a WTP from the AC's point of view

typedef struct {			
	CWNetworkLev4Address address;
	unsigned int WTPID;	
}CWWTPAttach;

typedef struct {
	int index;
	CWSocket sock;
	int interfaceIndex;
} CWACThreadArg_clone; // argument passed to the thread func

#define SYSFS_CLASS_NET "/sys/class/net/"
#define SYSFS_PATH_MAX  256




struct fdb_entry
{
        u_int8_t mac_addr[6];
        u_int16_t port_no;
        unsigned char is_local;
        struct timeval ageing_timer_value;
};
struct __fdb_entry
{
        u_int8_t mac_addr[6];
        u_int8_t port_no;
        u_int8_t is_local;
        u_int32_t ageing_timer_value;
        u_int32_t unused;
};
typedef enum {
		DEFAULT_VALUE = 0,
		ACCEPT_UNLESS_DENIED = 1,
		DENY_UNLESS_ACCEPTED = 2
} acl_policy;
struct acl_config {
	acl_policy  macaddr_acl;
	acl_policy  wlan_last_macaddr_acl;
	unsigned int num_deny_mac;
	unsigned int num_wids_mac;
	struct maclist *deny_mac;
	unsigned int num_accept_mac;
	struct maclist *accept_mac;
};
/*________________________________________________________________*/
/*  *******************___EXTERN VARIABLES___*******************  */
extern CWWTPManager *gWTPs;
extern CWThreadMutex gWTPsMutex;
#ifndef CW_NO_DTLS
extern CWSecurityContext gACSecurityContext;
#endif
extern int gACHWVersion;
extern int gACSWVersion;
extern char gdefaultsn[DEFAULT_SN_LENTH];
extern char gdefaultMac[MAC_LEN];
extern int gActiveStations;
extern int gActiveWTPs;
extern int gActiveWTPs_Autelan;
extern int gActiveWTPs_OEM;
extern int gStaticWTPs;
extern int scanningWTPs;
extern int scanningWTPs1;

extern CWThreadMutex gActiveWTPsMutex;
extern CWThreadMutex gAllThreadMutex;//zhanglei add for wlan op
extern CWThreadMutex gACInterfaceMutex;//zhanglei add for wlan op
extern CWThreadCondition gACInterfaceWait;//zhanglei add for wlan op
extern CWThreadCondition gInterfaceComplete;//zhanglei add for wlan op
extern CWThreadMutex gSTARoamingMutex;//zhanglei add for sta roaming op
extern CWThreadCondition gSTARoamingWait;//zhanglei add for sta roaming op
extern CWThreadMutex gACChannelMutex;//zhanglei for dynamic channel selection
extern CWThreadCondition gACChannelWait;//zhanglei for dynamic channel selection
extern CWThreadCondition gChannelComplete;//zhanglei for dynamic channel selection

extern CWThreadMutex gACChannelMutex2;//zhanglei for dynamic channel selection
extern CWThreadCondition gACChannelWait2;//zhanglei for dynamic channel selection
extern CWThreadCondition gChannelComplete2;//zhanglei for dynamic channel selection

extern CWThreadMutex gOuiMacXmlMutex;//zhaoruijia add for black&whiteouilist.xml

extern CWThreadMutex gACTxpowerMutex;
extern CWThreadCondition gACTxpowerWait;
extern CWThreadCondition gTxpowerComplete; 
 
extern int lic_bak_fd;
extern int lic_active_fd;
extern struct sockaddr Lic_Active_addr;

extern int gLimit;
extern int gMaxWTPs;
extern CWAuthSecurity gACDescriptorSecurity;
extern int gRMACField;
extern int gWirelessField;
extern int gDTLSPolicy;
extern CWThreadSpecific gIndexSpecific;
extern char *gACName;
extern char *gACHWVersion_char;
extern char *gACSWVersion_char;
extern int gDiscoveryTimer;
extern int gEchoRequestTimer;
extern int gCheckRequestTimer;
extern int gIdleTimeout;
extern CWProtocolNetworkInterface *gInterfaces;
extern int gInterfacesCount;
extern int gInterfacesCountIpv4;
extern int gInterfacesCountIpv6;

extern int gMaxInterfacesCount;
extern char **gMulticastGroups;
extern int gMulticastGroupsCount;
extern int wAWSocket;
extern int sockPerThread[SOCK_NUM]; /*wuwl add */
extern int wASDSocket;
extern int wWSMSocket;
extern int WidMsgQid;
extern int WidAllQid;
extern int ASD_WIDMSGQ;
extern int channel_state;
extern int txpower_state;
extern unsigned char channelRange[4];

extern int tx_wtpid;
extern unsigned char control_scope;
extern unsigned char coverage_threshold;
extern unsigned char tx_power_threshold;
extern unsigned char neighborrogueapcount;
extern unsigned char power_constant;
extern unsigned char tx_power_max;
extern int g_AUTO_AP_LOGIN_SWITCH;
extern char* g_AUTO_AP_LOGIN_BINDING_L3_INTERFACE ;
extern int g_AUTO_AP_LOGIN_BINDING_WLANID ;
extern int g_AUTO_AP_LOGIN_SAVE_CONFIG_SWITCH ;
extern int g_interface_state ;
extern wid_auto_ap_info	g_auto_ap_login;
extern int g_AC_ALL_EXTENTION_INFORMATION_SWITCH;
extern ac_balance_flag	ac_flow_num_balance_flag;

extern CWMultiHomedSocket gACSocket;
CWMultiHomedSocket gListenningIF;		//special socket to handle multiple network interfaces
extern int gCOUNTRYCODE;
/****************globle variable*****************/
extern int gINFOREPORTSWITCH;  
extern unsigned short gWIFIEXTENSIONREPORTINTERVAL; 
extern unsigned short gINFOREPORTINTERVAL;  
extern int gSTAINFOREPORT;
extern int gSTAREPORTSWITCH;
extern unsigned short gSTAREPORTINTERVAL;
extern int cpu_mem_collect_time; 
extern int gAPIFINFOETH_MTU[AP_ETH_IF_NUM];
extern int gAPIFINFOETH_RATE[AP_ETH_IF_NUM];
extern int gBANDWIDTH;
extern int gAP_STA_WAPI_REPORT_SWITCH;
extern unsigned int gAP_STA_WAPI_REPORT_INTERVAL;
extern int gTER_DIS_INFOREPORTPKT;
extern int gTER_DIS_INFOSTA_TRAP_COUNT;
extern int gTER_DIS_INFOREPORTSWITCH;
extern int gNEIGHBORCHANNELRSSITHOLD;
extern int gSAMECHANNELRSSITHOLD;
extern int gWTP_ROGUE_AP_THRESHOLD;
extern int gWTP_ROGUE_TERMINAL_THRESHOLD;
extern int gWTP_CPU_USE_THRESHOLD;
extern int gWTP_MEM_USE_THRESHOLD;
extern int gDHCP_SNOOPING;
extern int gWTP_FLOW_TRIGER;
extern int gWTP_MAX_STA;
extern int gNTP_STATE;
extern int gNTP_INTERVAL;

/****************globle variable*****************/

extern WTPQosValues* gDefaultQosValues;
extern CWWTPAttach **AC_ATTACH;
extern WID_STA STA_ROAM;
extern unsigned int receiver_signal_level;
extern wid_sample_info WID_SAMPLE_INFORMATION;
extern CWThreadMutex	ACIPLISTMutex;

extern unsigned int g_ap_cpu_threshold;
extern unsigned int g_ap_memuse_threshold;
extern unsigned int g_ap_temp_threshold;
extern struct ifi *WID_IF;
extern wid_ac_ip_group *AC_IP_GROUP[ACIPLIST_NUM];
extern struct ifi *WID_IF_V6;

extern char is_secondary;
extern char secondary_AC_cmd[80];
extern unsigned int img_now;
extern int gCWImageDataPendingTimer;
extern unsigned int G_LocalHost_num;
extern unsigned char WID_WATCH_DOG_OPEN;

extern char MSGQ_PATH[PATH_LEN];
extern unsigned int vrrid;
extern unsigned int ApAccessNat;
extern unsigned char countermeasurecount;
extern int gloadbanlance;
extern int havecreatethread;
extern unsigned char wtp_link_detect;
extern unsigned int wsmswitch;
extern unsigned char vlanSwitch;
extern unsigned char DhcpOption82Switch;
extern int wid_bak_sock;
extern CWTimerID	bak_check_timer;
extern int BakCheckInterval;
extern int LicBakReqInterval;
extern CWTimerID	Lic_bak_req_timer;
extern struct lic_ip_info Lic_ip;
extern unsigned int g_ap_auto_update_service_tftp;
extern unsigned int g_service_ftp_state;
extern unsigned int g_wid_wsm_error_handle_state;
extern int rrm_rid;
extern unsigned int slotid;
extern unsigned int local;
extern int multicast_listen_state;
extern int gWLAN_MAX_ALLOWED_STA_NUM;
extern int gWLAN_MAX_ALLOWED_STA_NUM_FOR_BSS;/*fengwenchao add 20120323*/
extern unsigned char gWLAN_ATH_L2_ISOLATION;/*fengwenchao add 20120323*/
extern sta_static_arp gWLAN_STA_STATIC_ARP_POLICY;/*fengwenchao add 20120323*/
extern unsigned char gWLAN_LIMIT_STA_RSSI;/*fengwenchao add 20120323*/
ap_uni_muti_bro_cast gWLAN_UNI_MUTI_BRO_CAST;/*fengwenchao add 20120323*/
/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */

//in AC.c
void CWACInit(void);
void CWWIDInit(void);//zhanglei add
void CWACEnterMainLoop(void);
CWBool CWACSendAcknowledgedPacket(int WTPIndex, int msgType, int seqNum);
CWBool CWACResendAcknowledgedPacket(int WTPIndex);
void CWACStopRetransmission(int WTPIndex);
void CWACDestroy(void);
//CWBool CWWIDtoASDInitSocket(int *sock);
//CWBool CWWIDtoWSMInitSocket(int *sock);
CWBool CWwAWInitSocket(int *sock);
CWBool CWInitMsgQueue(int *msgqid);
CWBool CWGetMsgQueue(int *msgqid);
//in ACTest.h
CWBool ACQosTest(int WTPIndex);

//in ACRunState.h
CWBool CWACParseGenericRunMessage(int WTPIndex, CWProtocolMessage *msg, CWControlHeaderValues* controlVal);
CWBool CWBindingAssembleWlanConfigurationRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist * elem);
CWBool CWBindingAssembleStaConfigurationRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex);
CWBool CWSaveChangeStateEventRequestMessage (CWProtocolChangeStateEventRequestValues *valuesPtr, CWWTPProtocolManager *WTPProtocolManager);
CWBool CWAssembleConfigurationUpdateRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum);
CWBool CWAssembleConfigurationUpdateRequest_Radio(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist * elem);
CWBool CWAssembleConfigurationUpdateRequest_WTP(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem);
CWBool CWAssembleStaConfigurationRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem);
CWBool CWAssembleStaConfigurationRequest_key(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem);

//in ACDiscoveryState.c
CWBool CWAssembleDiscoveryResponse(CWProtocolMessage **messagesPtr, int seqNum, unsigned int WTPID);
CWBool CWParseDiscoveryRequestMessage(char *msg, int len, int *seqNumPtr, CWDiscoveryRequestValues *valuesPtr);

CWBool CWAssembleMsgElemWTPVersion(CWProtocolMessage *msgPtr,char *version);

//in ACRetransmission.c
CWBool CWACSendFragments(int WTPIndex);

//in ACRunStateCheck.c
CWBool CWACCheckForConfigurationUpdateRequest(int WTPIndex);

//in ACProtocol_User.c
CWBool CWACGetVendorInfos(CWACVendorInfos *valPtr);
int CWACGetRMACField();
int CWACGetWirelessField();
int CWACGetDTLSPolicy();
void CWACDestroyVendorInfos(CWACVendorInfos *valPtr);

//in ACMainLoop.c
void CWACManageIncomingPacket(CWSocket sock, char *buf, int len, int incomingInterfaceIndex,int BindingSystemIndex, CWNetworkLev4Address *addrPtr, char *ifname);
void *CWManageWTP(void *arg);
void CWCloseThread();

// in CWSecurity.c
#ifndef CW_NO_DTLS
CWBool CWSecurityInitSessionServer(CWWTPManager* pWtp, CWSocket sock, CWSecurityContext ctx, CWSecuritySession *sessionPtr, int *PMTUPtr);
#endif
CWBool ACEnterJoin(int WTPIndex, CWProtocolMessage *msgPtr);
CWBool ACEnterConfigure(int WTPIndex, CWProtocolMessage *msgPtr);
CWBool ACEnterImageData(int WTPIndex, CWProtocolMessage *msgPtr);
CWBool ACEnterDataCheck(int WTPIndex, CWProtocolMessage *msgPtr);
CWBool ACEnterRun(int WTPIndex, CWProtocolMessage *msgPtr, CWBool dataFlag);
CWBool ACEnterReset(int WTPIndex, CWProtocolMessage *msgPtr);


CW_THREAD_RETURN_TYPE CWInterface(void* arg);
CW_THREAD_RETURN_TYPE CWInterface1(void* arg);
CW_THREAD_RETURN_TYPE CWDynamicChannelSelection(void * arg);
CW_THREAD_RETURN_TYPE CWDynamicChannelSelection2(void * arg);
CW_THREAD_RETURN_TYPE CWTransmitPowerControl(void * arg);
CW_THREAD_RETURN_TYPE CWSTARoamingOP(void* arg);
CW_THREAD_RETURN_TYPE CWThreadWD(void * arg);

//void CWTimerExpiredHandler(int arg);

CWBool AsdWsm_RadioOp(unsigned int WTPID, Operate op);
CWBool AsdWsm_WLANOp(unsigned char WlanID, Operate op, int both);
CWBool AsdWsm_BSSOp(unsigned int BSSIndex, Operate op, int both);
CWBool AsdWsm_DataChannelOp(unsigned int WTPID, Operate op);
CWBool wid_to_wsm_bss_pkt_info_get(unsigned int wtpindex);
CWBool AsdWsm_StationOp(unsigned int WTPID, CWStationInfoValues *valuesPtr,Operate op);//added by weiay 20080702
CWBool AsdWsm_ap_report_sta_info(unsigned int WTPID,CWStationReportInfo *valuesPtr, Operate op);//wuwl add 20100126
//CWBool WidAsd_StationInfoUpdate(unsigned int WTPID,WIDStationInfo * valuesPtr);
CWBool WidAsd_WTPTerminalStatisticsUpdate(unsigned int WTPID,unsigned int count,WIDStationInfo valuesPtr[64]);
CWBool WidAsd_StationInfoUpdate(unsigned int WTPID,unsigned int count,WIDStationInfo valuesPtr[64]);
CWBool AsdWsm_WTP_Channelchange_Op(unsigned int WtpID,unsigned int radioid,Operate op);
CWBool wid_asd_bss_traffic_limit(unsigned int bssindex);
CWBool wid_asd_bss_cancel_average_traffic_limit(unsigned int bssindex); //fengwenchao add for AXSSZFI-1374

CWBool wid_asd_send_wids_able(unsigned int able);
CWBool wid_asd_send_wids_info(struct tag_wids_device_ele * info,unsigned int WTPID);
CWBool WIDWsm_VRRPIFOp(unsigned char* name,unsigned int ip, unsigned int op);
CW_THREAD_RETURN_TYPE CWWawInforUpdate(void * arg);
int wid_sem_creat();
int p(int semid);
int v(int semid);
int wid_new_sem();
void wait_v(int semid);
void _CWCloseThread(int i);
void CWCaptrue(int n ,unsigned char *buffer);
void str2higher(char **str);
CWBool get_sock_descriper(int isystemindex, int* sockdes);
void ACInterfaceReInit();
void syslog_wtp_log(int WTPIndex, int login, char *note, unsigned char flag);
int RadioNumCheck(int WTPIndex);
CWBool check_ascii_32_to126(const char * str);
CWBool WID_WTP_INIT(void *arg);
void InitPath(unsigned int vrrid,char *buf);
void CWWIDDbusPathInit();
CWBool WidAsdStaWapiInfoUpdate(unsigned int WTPID,WIDStaWapiInfoList*valuesPtr);
int wid_illegal_character_check(char *str , int len, int modify);
void wid_pid_write_v2(char *name,int id,unsigned int vrrid);
CWBool find_in_wtp_list(int id);
CWBool delete_wtp_list(int id);
CWBool insert_uptfail_wtp_list(int id);
void update_complete_check();
int WID_ENABLE_WLAN(unsigned char WlanID);
int WID_DISABLE_WLAN(unsigned char WlanID);
int WID_RADIO_SET_STATUS(unsigned int RadioID, unsigned char status);
int bak_check_req(int sockfd);
CWBool WidAsd_StationInfoUpdate1(unsigned int WTPID,WIDStationInfo*valuesPtr);
CWBool AsdWsm_StationOpNew(unsigned int WTPID,char *mac, Operate op,unsigned short reason);
int update_license_req(int sockfd,struct sockaddr_in *addr);
CWBool WidAsd_StationLeaveReport(unsigned int WTPID,unsigned int count,WIDStationInfo*valuesPtr);
void syslog_wtp_log_hn(int WTPIndex, int login,unsigned int reason_code);//qiuchen
#endif
