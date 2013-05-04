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
* CWWTP.h
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


#ifndef __CAPWAP_CWWTP_HEADER__
#define __CAPWAP_CWWTP_HEADER__

/*_______________________________________________________*/
/*  *******************___INCLUDE___*******************  */

#include "CWCommon.h"
#include "WTPProtocol.h"
#include "WTPBinding.h"
/*________________________________________________________________________*/
/*  *******************___DEFINE___*******************  */
#define LOG_FILE_NAME					"/jffs/wtp.log.txt"
#define MODEL_AQ1000		1000/*the model num of aq1000*/
#define MODEL_AQ1200		1200/*the model num of aq1200*/
#define MODEL_AQ3000		3000/*the model num of aq3000*/
#define MAX_RADIO_NUM		4
#define MAX_WLAN_NUM_PER_RADIO        4
#define MAX_WLAN_NUM_PER_WTP		(MAX_RADIO_NUM*4)
#define MAX_WTP_STA_NUMBER  20

#define ESSID_LENGTH	32

#define MAC_ADDR_LEN		6
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

enum { 
	WTP_WPA_KEY_MGMT_IEEE8021X = 1,   //wpa	+1x
	WTP_WPA_KEY_MGMT_PSK = 2,              	//wpa psk	
	WTP_WPA_KEY_MGMT_NONE = 4,            	 //open 
	WTP_WPA_KEY_MGMT_IEEE8021X_NO_WPA = 8,  // only 1x 	
	WTP_WPA_KEY_MGMT_WPA_NONE = 16, 	
	WTP_WPA_KEY_MGMT_FT_IEEE8021X = 32, 
	WTP_WPA_KEY_MGMT_FT_PSK = 64,
	WTP_WPA_KEY_MGMT_SHARED =128 ,  //shared
	WTP_WPA2_KEY_MGMT_IEEE8021X = 256,   //wpa2	+1x
	WTP_WPA2_KEY_MGMT_PSK = 512,              	//wpa2 psk	
	WTP_WPA2_KEY_MGMT_FT_IEEE8021X = 1024, 
	WTP_WPA2_KEY_MGMT_FT_PSK = 2048,
	WTP_WAPI_KEY_MGMT_PSK = 4096,    //pei add 090309
	WTP_WAPI_KEY_MGMT_CER = 8192    //pei add 090309
}wtp_wpa_key_mgmt;

enum {
 	WTP_WPA_CIPHER_NONE = 1,
	WTP_WPA_CIPHER_WEP40 = 2,
	WTP_WPA_CIPHER_WEP104 = 4,
	WTP_WPA_CIPHER_WEP128 = 8,
	WTP_WPA_CIPHER_TKIP = 16,
	WTP_WPA_CIPHER_CCMP = 32,
	WTP_WPA_CIPHER_AES_128_CMAC = 64,
	WTP_WAPI_CIPHER_SMS4 = 128    //pei add 090309
}wtp_cipher;


/* pei add for wapi , at 090505 */
/*设置WAI参数的IOCTL命令字*/
#define P80211_IOCTL_SETWAPI_INFO		(0x8BE0 + 23)//按照LSDK6.1修改

/*设置WAPI参数的IOCTL子命令字,表示设置WAPI状态*/
#define P80211_PACKET_WAPIFLAG			(u16)0x0001

/*设置WAPI参数的IOCTL子命令字,表示安装密钥*/
#define P80211_PACKET_SETKEY     			(u16)0x0003

/*向Driver发送消息的消息结构*/
struct ioctl_drv
{
	u16  io_packet;
	struct  _iodata
	{
		u16 wDataLen;
		u8 pbData[96];
	}iodata;
}__attribute__ ((packed));

#define KEY_LEN 			16  	
#define MULTI_KEY_LEN  	KEY_LEN

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
typedef struct {
	unsigned char fc[2];
	unsigned char duration[2];
	unsigned char mac1[6];
	unsigned char mac2[6];
	unsigned char mac3[6];
	unsigned char seqctl[2];
}IEEE80211_Header;
typedef struct {
	unsigned char dmac[6];
	unsigned char smac[6];
}ETH_Header;
typedef struct {
	char *address;
	CWBool received;
	int seqNum;
} CWACDescriptor;

typedef struct wlan_info{
	unsigned char radio_id;
	unsigned char wlan_id;
	unsigned short vlan_id;  //pei add 0320
	unsigned char wlan_updown_time;  //pei add 0225
	unsigned char wlan_bssid[MAC_ADDR_LEN];
	struct wlan_info *next;
	unsigned short int capabilities;/*the capabilities information*/
	char essid[32];
	unsigned char wlan_mac_mode;
	unsigned char wlan_tunnel_mode;
	unsigned short key_length;
	char key[32];/*legth 32*/	
	int wlan_sendsock;
	int wlan_eapsock;
}CWWTPWlan;

/****radio struct****/
typedef struct radio_info{
	unsigned char radio_id;			/*radio id*/
	struct radio_info *next;
	char radio_name[IFNAMSIZ];		/*the name of radio EX:wifi0*/
	CWWTPWlan radio_first_wlan; 	/*the first wlan of the radio*/
	int wlancount;					/*wlan count */
	int wlan_id_table[MAX_WLAN_NUM_PER_RADIO];/*the redio's remote wlan ID array */
	CWBool status;            //radio status 1:enable  0:disable
}CWWTPRadio;
/****sta struct*****/
#if 1
typedef struct sta_info{
	unsigned char radio_id;
	unsigned char wlan_id;
	struct sta_info *next;
	unsigned char sta_mac[6];
	unsigned char rate;
	unsigned char authentication_type;
	unsigned short int aid; /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
	unsigned short capability;
	unsigned short listen_interval; /* or beacon_int for APs */
}CWWTPSta;
#endif
#if 0
/*hostapd code*/
struct sta_info {
	unsigned char radio_id;
	unsigned char wlan_id;
	struct sta_info *next; /* next entry in sta list */
	struct sta_info *hnext; /* next entry in hash table list */
	unsigned char addr[6];
	unsigned short int aid; /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
	unsigned int flags;
	unsigned short capability;
	unsigned short listen_interval; /* or beacon_int for APs */
	//char supported_rates[WLAN_SUPP_RATES_MAX];
	char tx_supp_rates;

	enum {
		STA_NULLFUNC = 0, STA_DISASSOC, STA_DEAUTH, STA_REMOVE
	} timeout_next;

	/* IEEE 802.1X related data */
	//struct eapol_state_machine *eapol_sm;

	/* IEEE 802.11f (IAPP) related data */
	s//truct ieee80211_mgmt *last_assoc_req;

	unsigned int acct_session_id_hi;
	unsigned int acct_session_id_lo;
	time_t acct_session_start;
	int acct_session_started;
	int acct_terminate_cause; /* Acct-Terminate-Cause */
	int acct_interim_interval; /* Acct-Interim-Interval */

	unsigned long last_rx_bytes;
	unsigned long last_tx_bytes;
	unsigned char acct_input_gigawords; /* Acct-Input-Gigawords */
	unsigned char acct_output_gigawords; /* Acct-Output-Gigawords */

	unsigned char *challenge; /* IEEE 802.11 Shared Key Authentication Challenge */

	int pairwise; /* Pairwise cipher suite, WPA_CIPHER_* */
	unsigned char *wpa_ie;
	size_t wpa_ie_len;
	struct wpa_state_machine *wpa_sm;
	enum {
		WPA_VERSION_NO_WPA = 0 /* WPA not used */,
		WPA_VERSION_WPA = 1 /* WPA / IEEE 802.11i/D3.0 */,
		WPA_VERSION_WPA2 = 2 /* WPA2 / IEEE 802.11i */
	} wpa;
	int wpa_key_mgmt; /* the selected WPA_KEY_MGMT_* */
	struct rsn_pmksa_cache *pmksa;
	struct rsn_preauth_interface *preauth_iface;
	unsigned char req_replay_counter[8 /* WPA_REPLAY_COUNTER_LEN */];
	int req_replay_counter_used;
	unsigned int dot11RSNAStatsTKIPLocalMICFailures;
	unsigned int dot11RSNAStatsTKIPRemoteMICFailures;
//#ifdef JUMPSTART	
//	struct jsw_session *js_session;
//#endif /* JUMPSTART */

}
#endif

enum VendorSpecPayloadType {
	APScanning=0,
	APThroughputInfo,
	APMaxThroughput,
	ExtendCmd,
	InterfaceUpdown,
	APMonitor,
	StaInfo,
	InterfaceState,
	AttackDetectInfo,
};

enum EventRespType {
	NEIGHBOR_AP_INFO=1,
	MONITOR,
	EXTRA_INFO,
	STA_INFO,
	IF_STATE,
	ATTACK_DETECT_INFO,
};

/*_____________________________________________________________*/
/*  *******************___WTP VARIABLES___*******************  */
extern char* gInterfaceName;

extern char **gCWACAddresses;
extern int gCWACCount;

extern char *gWTPLocation;
extern char *gWTPName;
extern int gIPv4StatusDuplicate;
extern int gIPv6StatusDuplicate;
extern char *gWTPForceACAddress;
extern char *gWTPACDomainName;
extern char *gWTPSoftWareVersion;
extern char gWTPHardWareVersion[20];  //pei add for test hardversion 0214
extern char versionbuf[100];


extern CWAuthSecurity gWTPForceSecurity;

extern CWSocket gWTPSocket;
extern CWSocket gWTPDataSocket;

extern int gWTPPathMTU;

extern CWACDescriptor *gCWACList;
extern CWACInfoValues *gACInfoPtr;
extern int leddev;
extern int blind_status;
extern int gEchoInterval;
extern int gWTPStatisticsTimer;
extern WTPRebootStatisticsInfo gWTPRebootStatistics;
extern CWWTPRadiosInfo gRadiosInfo;
#ifndef CW_NO_DTLS//csc
extern CWSecurityContext gWTPSecurityContext;
extern CWSecuritySession gWTPSession;
#endif

extern CWPendingRequestMessage gPendingRequestMsgs[MAX_PENDING_REQUEST_MSGS];

extern CWSafeList gPacketReceiveList;
extern CWSafeList gPacketDataReceiveList;/*receive data packet*/
extern CWSafeList gFrameList;
extern CWThreadCondition gInterfaceWait;
extern CWThreadMutex gInterfaceMutex;
extern char WTPModelNum[10];  //pei 0923  /* ap-code, pei 090519 */
extern char WTPRealModelNum[20]; /* ap real model num, pei 090519 */
extern int WTPSerialNum;
extern unsigned char WTPBoardID[20];
extern CWStateTransition gtemp_state;   //pei add 0618
extern int debug_print;        //pei add 0827-----------
extern CWImageIdentifierValues *gImageIdentifier_ACSupported;        //pei add 0618
extern int gtemp_seqNum;        //0619
extern CWBool isFirstWlan;        	//pei add 0624
#if 1
typedef struct {
	unsigned char radioId;
	unsigned char channel;
	int txpower;
	unsigned short rate;
	CWBool rateAutoEnable;
	unsigned int radiotype;
	unsigned short fragThreshold;
	unsigned short rtsThreshold;
	unsigned char shortRetry;
	unsigned char longRetry;
	unsigned short beaconInterval;
	unsigned char preamble;
	unsigned char dtim;
	unsigned char gIsRadioEnable;
} radioInfoValues;
extern radioInfoValues gRadioInfoValue[2];
//#else
extern unsigned char gchannel;        //pei add 0624
extern int gtxpower;        //pei add 0624
extern unsigned short grate; 			        //pei add 0715
extern CWBool gRateAutoEnable;  //pei add 1128
extern unsigned int gradiotype;         		//pei add 0715
extern unsigned short gFragThreshold;  //pei add 0722
extern unsigned short gRtsThreshold;    //pei add 0729
extern unsigned char gShortRetry;     //pei add 0729
extern unsigned char gLongRetry;     //pei add 0729
extern unsigned short gBeaconInterval;   //pei add 0722
extern unsigned char gPreamble;                //pei add 0722
extern unsigned char gDtim;	                 //pei add 0722
#endif
extern unsigned char gIsRadioEnable;	        //pei add 0724
extern unsigned char gApScanningEnable;   //pei add 1118
extern unsigned short gApScanningInterval;   //pei add 1127
extern unsigned char gApScanningThreadEnable;   //pei add 1125
extern unsigned char gAPThroughputInfoCollectEnable; //pei add 0204
extern Radio_QosValues gRadioQosValues; //pei add 0207
extern unsigned char gMaxThroughput;  //pei add 0209
extern WlanMaxThroughputValues gWlanMaxThroughputValue[16];
extern unsigned char gQosEnable;  //pei add 0210
extern unsigned char gWTPStaticIPEnable;  //pei add 0214
extern unsigned short gPowerOnUntilRuntime;  //pei add 0214
extern unsigned char g_DelStaMac[6];
extern int WTPBoardRevesion;
extern unsigned char WTPBaseMAC[6];
extern int gNetLink_signal;
extern CWBool CWWTPNeighborDeadTimerExpiredFlag;
extern CWBool WTPWLanReceiveThreadRun ;
extern CWBool WTPDataChannelThreadRun ;

extern CWWTPRadio wtp_radio[MAX_RADIO_NUM];
//extern CWWTPWlan  wtp_wlan[MAX_WLAN_NUM_PER_WTP];
extern CWWTPRadio *wtp_radio_list;
extern CWWTPWlan *wtp_wlan_list;
extern CWWTPSta *wtp_sta_list;



extern int wtp_wlan_count;/*the wlan number of the wtp*/
extern int wtp_radio_count;/*the radio number of the wtp*/
extern int wtp_sta_count;/*the sta count of the wtp*/
extern int receiveframethread;

/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */

//in WTP.c
void CWWTPGetWlanName(char *wlanname,unsigned char radioId, unsigned char wlanId);
void CWWTPGetRadioName(char *radioame,int radio_id);

CWBool CWWTPLoadConfiguration();
CWBool CWWTPLoadBoardConfiguration();

CWBool CWWTPInitConfiguration();
void CWWTPResetRadioStatistics(WTPRadioStatisticsInfo *radioStatistics);
CWBool CWReceiveMessage(CWProtocolMessage *msgPtr);
CWBool CWWTPSendAcknowledgedPacket(int seqNum, CWList msgElemlist, CWBool (assembleFunc) (CWProtocolMessage **, int *, int, int, CWList), CWBool (parseFunc) (char*, int, int, void*), CWBool (saveFunc) (void*), void *valuesPtr);
void CWWTPDestroy();
CWBool STATableAdd();
int macAddrCmp (unsigned char* addr1, unsigned char* addr2);/*compare two MAC*/
CWBool CWWTPUpdate(unsigned char *imgname,unsigned char *ip);
CWBool CWAssembleResetResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWProtocolResultCode resultCode) ;

CWBool apservicerestart(void);
unsigned int ip_int2str(unsigned int ipAddress,unsigned char *buff);  //pei add 0214

CWBool WlanTableAdd(CWWTPWlan *addWlanValues, unsigned short vlanId);     //pei add 0624
CWBool WLanTableDelete(DeleteWlanValues *deleteWlanValues);
CWBool STATableAdd(AddSTAValues *addSTAValues);     //pei add 0708
CWBool STATableDelete(DeleteSTAValues *deleteSTAValues);     //pei add 0708
CWBool CWWTPLoadVersionConfiguration();



//in WTPRunState.c
CWBool CWAssembleWTPEventRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWList msgElemList);
CWBool CWAssembleWTPEventDeleteStationRequest (CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, DeleteSTAValues *valuesPtr); 	//pei add 0703
CWBool CWParseWTPEventRequestDeleteStationMessage(char *msgPtr, int len, DeleteSTAValues *valuesPtr);   //pei add 0703

CW_THREAD_RETURN_TYPE CWWTPReceiveDtlsPacket(void *arg);
CWBool CWWTPCheckForBindingFrame();

//in WTPProtocol_User.c
CWBool CWWTPGetACNameWithIndex (CWACNamesWithIndex *ACsInfo);
int CWWTPGetStatisticsTimer ();
void CWWTPGetIPv6Address(struct sockaddr_in6* myAddr);
CWBool CWGetWTPRadiosAdminState(CWRadiosAdminInfo *valPtr);
CWBool CWGetDecryptErrorReport(int radioID, CWDecryptErrorReportInfo *valPtr);

//in WTPRetransmission.c
int CWSendPendingRequestMessage(CWPendingRequestMessage *pendingRequestMsgs, CWProtocolMessage *messages, int fragmentsNum);
int CWFindPendingRequestMsgsBox(CWPendingRequestMessage *pendingRequestMsgs, const int length, const int msgType, const int seqNum);
void CWResetPendingMsgBox(CWPendingRequestMessage *pendingRequestMsgs);
CWBool CWUpdatePendingMsgBox(CWPendingRequestMessage *pendingRequestMsgs, unsigned char msgType, int seqNum,  int timer_sec, CWTimerArg timer_arg, void (*timer_hdl)(CWTimerArg), int retransmission, CWProtocolMessage *msgElems,int fragmentsNum);

//in WTPDriverInteraction.c
int set_cwmin(int sock, struct iwreq wrq, int acclass, int sta, int value);
int get_cwmin(int sock, struct iwreq* wrq, int acclass, int sta);
int set_cwmax(int sock, struct iwreq wrq, int acclass, int sta, int value);
int get_cwmax(int sock, struct iwreq* wrq, int acclass, int sta);
int set_aifs(int sock, struct iwreq wrq, int acclass, int sta, int value);
int get_aifs(int sock, struct iwreq* wrq, int acclass, int sta);
int Check_Interface_State(char *ifname);
/**/
/*************************wlanconfig*******************************/
/*------------------------addwlan--------------------------------*/
int wapid_ioctl(char *ifname, unsigned short cmd, char *buf, int buf_len); //pei add 090505
int create_wlan(int sock, AddWlanValues *addWlanValues, WPA_IE *wpa_ie, WlanVlanValues *wlanVlanValues, WapiValues *wapiValues);   //pei add 0624
int delete_wlan(int sock, DeleteWlanValues *value);
int add_sta(int sock,AddSTAValues *addStaValues);
int del_sta(int sock, DeleteSTAValues *deleteSTAValues, unsigned char *wlan_id);
int madwifi_set_key(int sock,STASessionKeyValues *staSessionKey,int alg,unsigned char radioId,unsigned char wlan_id);
int madwifi_set_ieee8021x(int sock, WPA_IE *wpa_ie, unsigned char radioId,unsigned char wlan_id, int wpa_enable);
int madwifi_set_privacy(int sock, unsigned char radioId,unsigned char wlan_id, int enabled);
int set80211priv(int sock,char *ifname, int op, void *data, int len);
int madwifi_configure_wpa(int sock, WPA_IE *wpa_ie, char *ifname, int wpa_enable);
int set_channel(char *wlanname ,int channel);
int set_txpower_cmd(char *wlanname,int txpower);
int set_rate_cmd(char *wlanname, unsigned char radioId, int rate);   //pei add 0716
int set_radiotype_cmd(unsigned char radioId, unsigned char wlanId, int radiotype);  //pei add 0716
int set_fragthreshold_cmd(char *wlanname, unsigned short fragThreshold);         //pei add 0722
int set_rtsthreshold_cmd(char *wlanname, unsigned short rtsThreshold);         //pei add 0729
int set_radioconfig_cmd(char *wlanname, unsigned char preamble, unsigned char dtim, unsigned short beaconInterval);         //pei add 0722
CW_THREAD_RETURN_TYPE CWWTPCheckNetLink(void *arg);      //pei add 0703
CW_THREAD_RETURN_TYPE CWWTPSetApScanning(void *arg);  //pei test for rogue AP 1120
CW_THREAD_RETURN_TYPE CWWTPSetApMonitor(void *arg);      //pei add 0226
CW_THREAD_RETURN_TYPE CWWTPSetExtraInfo(void *arg);  //pei add 0226
CW_THREAD_RETURN_TYPE CWWTPGetStaInfo(void *arg);
CW_THREAD_RETURN_TYPE CWWTPGetIfState(void *arg);
CWBool CWSetApScanning(ApScanningSetValues *apScanningSetValues);  //pei test for rogue AP 1120
CWBool CWSetApMonitor(ApMonitorSetValues *apMonitorSetValues);      //pei add 0226
CWBool CWSetApExtraInfo(ExtraInfoValues *extraInfoValues);
CWBool CWGetStaInfo(GetStaInfoValues *getStaInfoValues);
CWBool CWGetIfState(GetIfStateValues *getIfStateValues);
CWBool CWGetAttackDetectInfo(GetAttackDetectInfoValues *getAttackDetectInfoValues);
CWBool CWSetApThroughputInfoCollect(ApThroughputInfoValues *apThroughputInfoCollectValues);  //pei add 0204
CWBool CWGetAPThroughputInfo(int wlanCount, int ethCount, int wifiCount, wlan_stats_info *wlanStatsInfo);  //pei add 0205
CWBool CWAssembleMsgElemVendorSpecificPayload(CWProtocolMessage *msgPtr, int interface_count, wlan_stats_info *valuesPtr);     //pei add 0205

//in WTPDiscoveryState.c
CWStateTransition CWWTPEnterDiscovery();
void CWWTPPickACInterface();

CWStateTransition CWWTPEnterSulking();
CWStateTransition CWWTPEnterJoin();
CWStateTransition CWWTPEnterImageData();       //pei add 0716
CWStateTransition CWWTPEnterConfigure();
CWStateTransition CWWTPEnterDataCheck();
CWStateTransition CWWTPEnterRun();
CWStateTransition CWWTPEnterReset();


CWBool CWStartHeartbeatTimer();
CWBool CWStopHeartbeatTimer();
CWBool CWStartNeighborDeadTimer();
CWBool CWStopNeighborDeadTimer();
CWBool CWResetTimers();

void CWWTPHeartBeatTimerExpiredHandler(void *arg); 
void CWWTPRetransmitTimerExpiredHandler(CWTimerArg arg);
/*in WTPBinding.c*/


CWBool CWNetworkInitSocketClientUnconnect(CWSocket *sockPtr, struct sockaddr_in *addrPtr, int port);   /* pei test 1222 */

/**************************** iwconfig ****************************/
/*--------------------------- Frequency ---------------------------*/
int set_freq(int sock, struct iwreq wrq, int value);
/*--------------------------- Transmit Power ---------------------------*/
/*remeember that fox =1 you can change the tx power you have to change it to 0 after change*/
int set_txpower(int sock, struct iwreq wrq, int value);
/*watchdog*/
CWBool CWWTPStartWatchdog();
CWBool CWStartWatchDogTimer();
void CWWTPWatchDogTimerExpiredHandler(void *arg);

/******************************  WTPFrameReceive.c  **************************/
int getMacAddr(int sock, char* interface, unsigned char* macAddr);
#endif

