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
* ACProtocol.h
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
#include "wcpss/waw.h"
	 
#include "wcpss/wid/WID.h"
#include "CWAC.h"


#ifndef __CAPWAP_ACProtocol_HEADER__
#define __CAPWAP_ACProtocol_HEADER__

#define LTE_UPLINK_DATE_LEN 24
#define LTE_UPLINK_MAC_LEN 6
#define LTE_ID_LEN 10
#define LTE_UPLINK_MODE_LEN 8

//#define UNUSED_RADIO_ID 	1000
 #define WTP_TYPE_DEFAULT_LEN 32
 typedef struct{
 	char *locationData;
	char *name;
	unsigned int sessionID;
	CWWTPDescriptor descriptor;
	struct sockaddr_in ipv4Address;
	
	CWWTPRadiosInfo radiosInfo;

	char *ACName;
	CWACNamesWithIndex ACNameIndex;
	CWRadiosAdminInfo radioAdminInfo;
	int StatisticsTimer;
	CWWTPVendorInfos WTPBoardData;
	//CWRadiosInformation WTPRadioInfo;	
	WTPRebootStatisticsInfo *WTPRebootStatistics;

	void* bindingValuesPtr;
 } CWWTPProtocolManager;

typedef struct {
	char *location;
	char *name;
	CWWTPVendorInfos WTPBoardData;
	unsigned int sessionID;
	CWWTPDescriptor WTPDescriptor;
	struct sockaddr_in addr;
	CWframeTunnelMode frameTunnelMode;
	CWMACType MACType;
} CWProtocolJoinRequestValues;


typedef struct {
	char *ACName;
	CWACNamesWithIndex ACinWTP;
	int radioAdminInfoCount;
	CWRadioAdminInfoValues *radioAdminInfo;
	int StatisticsTimer;
	WTPRebootStatisticsInfo *WTPRebootStatistics;
} CWProtocolConfigureRequestValues;

typedef struct {
	CWRadiosOperationalInfo radioOperationalInfo;
	CWProtocolResultCode resultCode;
} CWProtocolChangeStateEventRequestValues;

typedef struct{
	unsigned int radioID;
	unsigned int TxQueueLevel;
	unsigned int wirelessLinkFramesPerSec;
	unsigned int ElectrifyRegisterCircle;
	unsigned int ColdStart;
	unsigned int ipmask;
	unsigned int ipgateway;
	unsigned int ipdnsfirst;
	unsigned int ipdnssecend;
	unsigned char cpuType[WTP_TYPE_DEFAULT_LEN];
	unsigned char flashType[WTP_TYPE_DEFAULT_LEN];
	unsigned char memType[WTP_TYPE_DEFAULT_LEN];
	unsigned int flashSize;
	unsigned int memSize;
	unsigned char eth_count;
	unsigned int eth_rate;
} WTPOperationalStatisticsValues;

typedef struct{
	unsigned int radioID;
	WTPRadioStatisticsInfo WTPRadioStatistics;
} WTPRadioStatisticsValues;

typedef struct {
	int ipv4Address;
	unsigned int length;
	unsigned char *MACoffendingDevice_forIpv4;
	int status;
} WTPDuplicateIPv4;

typedef struct {
	struct in6_addr ipv6Address;
	unsigned int length;
	unsigned char *MACoffendingDevice_forIpv6;
	int status;
} WTPDuplicateIPv6;

typedef struct{
	unsigned char radio_id;
	unsigned char mac_length;
	char *mac_addr;
}CWStationInfoValues;//added by weiay 20080702

typedef struct{
	unsigned char type;
	unsigned char op;
	unsigned char radioId;
	unsigned char wlanId;
	unsigned int  vlanId;
	unsigned char mac[6];
	unsigned char length;/*mark ipv4 or ipv6*/
	unsigned int ipv4Address;
	struct in6_addr ipv6Address;
}CWStationReportInfo;//added by wuwl 20100126

typedef struct{
	int errorReportCount;
	CWDecryptErrorReportValues *errorReport;
	WTPDuplicateIPv4 *duplicateIPv4;
	WTPDuplicateIPv6 *duplicateIPv6;
	int WTPOperationalStatisticsCount;
	WTPOperationalStatisticsValues *WTPOperationalStatistics;
	int WTPRadioStatisticsCount;
	WTPRadioStatisticsValues *WTPRadioStatistics;
	WTPRebootStatisticsInfo *WTPRebootStatistics;
	CWStationInfoValues *CWStationInfo;//20080702
	Neighbor_AP_INFOS *neighbor_ap_infos;
	wid_sample_rate_info wid_sample_throughput;
	wid_wifi_info ap_wifi_info;
	WIDStationInfo *ap_sta_info;
	wid_ap_if_state_time ap_if_info;
	wid_wids_device *wids_device_infos;
	CWStationReportInfo *ApReportStaInfo;
	WIDStaWapiInfoList *wid_sta_wapi_infos;
	CWWtpExtendinfo *wtp_extend_info;
} CWProtocolWTPEventRequestValues;


/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */
CWBool CWParseChangeStateEventRequestMessage (char *msg, int len, int *seqNumPtr, CWProtocolChangeStateEventRequestValues *valuesPtr);
CWBool CWAssembleChangeStateEventResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum);


CWBool CWAssembleMsgElemACDescriptor(CWProtocolMessage *msgPtr);			// 1
CWBool CWAssembleMsgElemACIPv4List(CWProtocolMessage *msgPtr);				// 2
CWBool CWAssembleMsgElemACIPv6List(CWProtocolMessage *msgPtr);				// 3
CWBool CWAssembleMsgElemACName(CWProtocolMessage *msgPtr);				// 4
CWBool CWAssembleMsgElemCWControlIPv4Addresses(CWProtocolMessage *msgPtr,unsigned int WTPID);		//10
CWBool CWAssembleMsgElemCWControlIPv6Addresses(CWProtocolMessage *msgPtr);		//11
CWBool CWAssembleMsgElemCWTimer(CWProtocolMessage *msgPtr, int WTPID);				//12
CWBool CWAssembleMsgElemDecryptErrorReportPeriod(CWProtocolMessage *msgPtr, int WTPID);		//16
CWBool CWAssembleMsgElemIdleTimeout(CWProtocolMessage *msgPtr);				//23
CWBool CWAssembleMsgElemWTPFallback(CWProtocolMessage *msgPtr);				//37
CWBool CWAssembleMsgElemAPScanningSet(CWProtocolMessage *msgPtr); //added by weiay
CWBool CWAssembleMsgElemAPThroughoutSet(CWProtocolMessage *msgPtr,int wtpid);

CWBool CWAssembleMsgElemAPExtensinCommandSet(CWProtocolMessage *msgPtr, int wtpid,char *command);
CWBool CWAssembleMsgElemAPOption60ParameterSet(CWProtocolMessage *msgPtr, int wtpid,char *command);

CWBool CWAssembleMsgElemAPElectronicMenu(CWProtocolMessage *msgPtr,unsigned char wlanid,unsigned char radioid,unsigned char level,unsigned char state);
//lilong add 2014.12.01
CWBool CWAssembleMsgElemAPLongitudeLatitude(CWProtocolMessage *msgPtr, unsigned char *longitude, unsigned char*latitude);
CWBool CWAssembleMsgElemAPConfigureErrorSet(CWProtocolMessage *msgPtr,int wtpid);
CWBool CWAssembleMsgElemAPUnauthorizedMacSet(CWProtocolMessage *msgPtr,int wtpid);


CWBool  CWAssembleStaticAPIP(CWProtocolMessage *msgPtr,int wtpid);
CWBool CWAssembleMsgElemAPInterfaceInfo(CWProtocolMessage *msgPtr);

//---------------------------------------------------------/

//CWBool CWParseACName(CWProtocolMessage *msgPtr, int len, char **valPtr);
CWBool CWParseACNameWithIndex(CWProtocolMessage *msgPtr, int len, CWACNameWithIndexValues *valPtr);			// 5
CWBool CWParseDiscoveryType(CWProtocolMessage *msgPtr, int len, CWDiscoveryRequestValues *valPtr);			//20
CWBool CWParseMsgElemDuplicateIPv4Address(CWProtocolMessage *msgPtr, int len, WTPDuplicateIPv4 *valPtr); 		//21
CWBool CWParseLocationData(CWProtocolMessage *msgPtr, int len, char **valPtr);						//27
CWBool CWParseWTPRadioAdminState (CWProtocolMessage *msgPtr, int len, CWRadioAdminInfoValues *valPtr);		//29 
CWBool CWParseSessionID(CWProtocolMessage *msgPtr, int len, CWProtocolJoinRequestValues *valPtr);		//32
CWBool CWParseWTPStatisticsTimer(CWProtocolMessage *msgPtr, int len, int *valPtr);					//33 
CWBool CWParseWTPBoardData(CWProtocolMessage *msgPtr, int len, CWWTPVendorInfos *valPtr);				//35 
CWBool CWCheckWTPBoardData(int WTPIndex, CWWTPVendorInfos *valPtr);
CWBool CWCheckRunWTPBoardData(int WTPIndex, CWWTPVendorInfos *valPtr);
CWBool CWDisCheckWTPBoardData(int bindingSystemIndex,CWNetworkLev4Address *addrPtr, CWWTPVendorInfos *valPtr, unsigned int *WTPID);
CWBool CWAddAC_ATTACH_For_Auto(CWNetworkLev4Address *addrPtr, unsigned int WTPID);
CWBool CWWTPMatchBindingInterface(int wtpid,int bindingSystemIndex);
int CWCmpWTPAttach(CWNetworkLev4Address *addrPtr);
CWBool CWParseWTPDescriptor(CWProtocolMessage *msgPtr, int len, CWWTPDescriptor *valPtr);				//37 
CWBool CWParseWTPFrameTunnelMode(CWProtocolMessage *msgPtr, int len, CWframeTunnelMode *valPtr);			//38
CWBool CWParseWTPIPv4Address(CWProtocolMessage *msgPtr, int len, CWProtocolJoinRequestValues *valPtr);			//39 
CWBool CWParseWTPMACType(CWProtocolMessage *msgPtr, int len, CWMACType *valPtr);				//40
CWBool CWParseWTPName(CWProtocolMessage *msgPtr, int len, char **valPtr);						//41
CWBool CWParseWTPOperationalStatistics(CWProtocolMessage *msgPtr, int len, WTPOperationalStatisticsValues *valPtr);	//42
CWBool CWParseWTPRadioStatistics(CWProtocolMessage *msgPtr, int len, WTPRadioStatisticsValues *valPtr); 		//43
CWBool CWParseWTPRebootStatistics(CWProtocolMessage *msgPtr, int len, WTPRebootStatisticsInfo *valPtr);			//44 
CWBool CWParseMsgElemDecryptErrorReport(CWProtocolMessage *msgPtr, int len, CWDecryptErrorReportValues *valPtr);
CWBool CWParseMsgElemDuplicateIPv6Address(CWProtocolMessage *msgPtr, int len, WTPDuplicateIPv6 *valPtr);
//added by weiay 20080702
CWBool CWParseMsgElemCWStationInfoValue(CWProtocolMessage *msgPtr, int len, CWStationInfoValues *valPtr);
CWBool CWParseMsgElemAPNeighborAPInfos(CWProtocolMessage *msgPtr, int len, Neighbor_AP_INFOS *valPtr);
CWBool CWParseMsgElemAPWidsInfos(CWProtocolMessage *msgPtr, int len, wid_wids_device *valPtr,int wtpindex);
CWBool CWParseMsgElemAPExtensionInfo(CWProtocolMessage *msgPtr, int len, wid_wifi_info *valPtr, int wtpindex);
CWBool CWParseMsgElemAPStaInfoReport(CWProtocolMessage * msgPtr,int len,WIDStationInfo * valPtr,int wtpindex);
CWBool CWParseMsgElemAPIfInfoReport(CWProtocolMessage * msgPtr,int len,wid_ap_if_state_time *valPtr,int wtpindex);
//CWBool CWParseWTPRadioInfo(CWProtocolMessage *msgPtr, int len, CWRadiosInformation *valPtr, int radioIndex);	

//---------------------------------------------------------/
CWBool CWACGetACIPv4List(int **listPtr, int *countPtr);
CWBool CWACGetACIPv6List(struct in6_addr **listPtr, int *countPtr);
char *CWACGetName(void);
int CWACGetHWVersion(void);
int CWACGetSWVersion(void);
int CWACGetStations(void);
int CWACGetLimit(void);
int CWACGetActiveWTPs(void);
int CWACGetMaxWTPs(void);
int CWACGetSecurity(void);
int CWACGetInterfacesCount(void);
int CWACGetInterfacesIpv4Count(void);
int CWACGetInterfacesIpv6Count(void);

int CWACGetInterfaceIPv4AddressAtIndex(int i);
char *CWACGetInterfaceIPv6AddressAtIndex(int i);
int CWACGetInterfaceWTPCountAtIndex(int i);
CWBool CWACGetDiscoveryTimer(int *timer);
CWBool CWACGetEchoRequestTimer(int *timer);
CWBool CWACGetIdleTimeout(int *timer);
CWBool CWGetWTPRadiosOperationalState(int radioID, CWRadiosOperationalInfo *valPtr);

//---------------------------------------------------------/
CWBool CWACSupportIPv6();
void CWDestroyDiscoveryRequestValues(CWDiscoveryRequestValues *valPtr);
CWBool CWParseAPStatisInfo(CWProtocolMessage *msgPtr, int len, int WTPIndex);
CWBool CWAssembleMsgElemAPStatisticsSet(CWProtocolMessage *msgPtr,int apstatics);  //fengwenchao modify 20110422
CWBool  CWAssemblewtpextensioninfomation(CWProtocolMessage *msgPtr,int wtpid);
CWBool  capwap_comm_switch_interval_assemble(CWProtocolMessage *msgPtr,int wtpid, unsigned char type);
CWBool  CWAssembleTimestamp(CWProtocolMessage *msgPtr,int wtpid);
CWBool CWParseWTPextensioninfomation(CWProtocolMessage * msgPtr,int len,int wtpindex);
CWBool CWParseMsgElemAPInterfaceInfo(CWProtocolMessage * msgPtr,int len,wid_sample_rate_info * valPtr);
CWBool CWAssemblewtpstainfomationreport(CWProtocolMessage * msgPtr,int wtpid);
CWBool CWAssemblewtpifinforeport(CWProtocolMessage * msgPtr,int wtpid);
CWBool  CWAssembleWidsSet(CWProtocolMessage *msgPtr,int wtpid);
CWBool  CWAssembleStaticAPIPDNS(CWProtocolMessage *msgPtr,int wtpid);
CWBool CWParseMsgElemAPStaWapiInfos(CWProtocolMessage *msgPtr, int len, WIDStaWapiInfoList *valPtr,int wtpindex);
CWBool CWParseAPStatisInfo_v2(CWProtocolMessage *msgPtr, int len,char *valPtr, int WTPIndex);
CWBool CWParseAP_Ntp_resultcode(CWProtocolMessage *msgPtr, int len,char *valPtr, int WTPIndex);
CWBool CWParseAttack_addr_Redirect(CWProtocolMessage *msgPtr, int len, int WTPIndex);
CWBool CWParseAP_challenge_replay(CWProtocolMessage *msgPtr, int len, int WTPIndex);
CWBool CWParseWtp_Sta_Terminal_Disturb_Report(CWProtocolMessage *msgPtr, int len,char *valPtr, int WTPIndex);
CWBool CWParseMsgElemCWWtpStaIpMacReportInfo(CWProtocolMessage *msgPtr, int len, CWStationReportInfo *valPtr);
CWBool CWParselinkquality(CWProtocolMessage *msgPtr, int len, int WTPIndex);
CWBool CWAssembleMsgElemAPStatistics_Interval_Set(CWProtocolMessage *msgPtr,unsigned wtpid);
CWBool CWAssembleMsgElemAP_NTP_Set(CWProtocolMessage *msgPtr,unsigned wtpid);
CWBool	CWAssembleWtpStaWapiInfoReport(CWProtocolMessage *msgPtr,int wtpid);
CWBool	CWAssembleSnoopingAble(CWProtocolMessage *msgPtr,int wtpid);
CWBool  CWAssembleIpMacReport(CWProtocolMessage *msgPtr,int wtpid);
CWBool	CWAssembleTerminalDisturbInfoReport(CWProtocolMessage *msgPtr,int wtpid);
CWBool  CWAssembleWtpEthMtu(CWProtocolMessage *msgPtr,int wtpid,unsigned char eth_index);
void CWProtocolRetrieve64(CWProtocolMessage *msgPtr,unsigned long long *val) ;
extern int wid_dbus_trap_wid_lte_fi_uplink_switchb(unsigned int wtpindex);
extern int wid_dbus_trap_wtp_ap_ACTimeSynchroFailure(int wtpindex,unsigned char flag);
extern int wid_dbus_trap_wtp_channel_terminal_interference(int wtpindex,unsigned char radio_id, char chchannel,unsigned char mac[6]);
extern int wid_dbus_trap_wtp_channel_terminal_interference_clear(int wtpindex,unsigned char radio_id, char chchannel,unsigned char mac[6]);
CWBool CWPareseWtp_Sta_Flow_Check_Report(CWProtocolMessage *msgPtr, int len, WIDStationInfo *valPtr,int wtpindex);
CWBool CWAssembleMsgElemAPFlowCheck(CWProtocolMessage *msgPtr,unsigned char radioid,unsigned char wlanid,unsigned short flow_check,unsigned int no_flow_time,unsigned int  limit_flow);
CWBool CWAssembleMsgElemAPnoRespToStaProReq(CWProtocolMessage *msgPtr,MQ_Radio radioinfo);
CWBool CWAssembleMsgElemAPUniMutiBroCastIsolationSWandRateSet(CWProtocolMessage *msgPtr,MQ_Radio radioinfo);
CWBool CWAssembleMsgElemAPUniMutiBroCastRateSet(CWProtocolMessage *msgPtr,MQ_Radio radioinfo);
CWBool CWAssembleMsgElemAPPasswd(CWProtocolMessage *msgPtr,char *username,char*password);
CWBool CWAssembleMsgElemAPMultiUserOptimize(CWProtocolMessage *msgPtr,unsigned char wlanid,unsigned char radioid,unsigned char value);
CWBool CWPareseWtp_Sta_leave_Report(CWProtocolMessage *msgPtr, int len, WIDStationInfo *valPtr,int wtpindex);
CWBool CWParseWTPEtendinfo(CWProtocolMessage *msgPtr, int len, CWWtpExtendinfo *valPtr, int wtpindex);
CWBool CWParseWTPTrapInfo(CWProtocolMessage *msgPtr, int len, int wtpindex);
CWBool CWParseLTEFITrapInfo(CWProtocolMessage *msgPtr, int len, int wtpindex);

CWBool  CWAssembleWtpStaDeauthreport(CWProtocolMessage *msgPtr,int wtpid);
CWBool  CWAssembleWtpStaFlowInformationreport(CWProtocolMessage *msgPtr,int wtpid);
CWBool CWParaseWTPTerminalStatistics(
				CWProtocolMessage *msgPtr, 
				int len,
				struct WID_WTP_TERMINAL_STATISTICS *wtp_terminal_statistics, 
				int wtpindex);
CWBool CWAssembleMsgElemAPSetCPEChannelIntf(CWProtocolMessage *msgPtr, unsigned char op, unsigned short vlanId,unsigned char radioId,unsigned char wlanId);
CWBool CWAssembleMsgElemRadiosetMGMTratebasewlan(CWProtocolMessage *msgPtr, unsigned char radioId,unsigned char wlanId,unsigned int rate);
CWBool CWAssembleMsgElemWTPlanvlan(CWProtocolMessage *msgPtr, 
	unsigned char state, unsigned short vlanid); //lilong add 2014.09.15
CWBool CWAssembleWifiLocatePublicConfig
(
	CWProtocolMessage *msgPtr,
	unsigned int l_radioid,
	unsigned char state,
	unsigned char scan_type,
	unsigned char rssi,
	unsigned char result_filter,
	unsigned int version_num,
	unsigned short report_interval,
	unsigned short channel_scan_interval,
	unsigned short	channel_scan_time,
	unsigned int server_ip,
	unsigned short server_port,
	unsigned char *channel
);

#endif
