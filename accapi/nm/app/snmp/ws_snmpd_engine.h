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
* ws_snmpd_engine.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/accapi/nm/app/snmp/ws_snmpd_engine.h,v $
*$Author: shaojunwu $
*$Date: 2010/05/31 11:51:50 $
*$Revision: 1.1 $
*$State: Exp $
*
*$Log: ws_snmpd_engine.h,v $
*Revision 1.1  2010/05/31 11:51:50  shaojunwu
*modify for some ws has  html output for can't contain in libnm
*/

#ifndef _WS_SNMPD_ENGINE_H
#define _WS_SNMPD_ENGINE_H


#define TRAP_RTN_OK					0
#define TRAP_RTN_ERR			    (TRAP_RTN_OK-1)
#define TRAP_RTN_NULL_POINT			(TRAP_RTN_OK-2)
#define TRAP_RTN_MALLOC_ERR			(TRAP_RTN_OK-3)


#define TRAP_RTN_DBUS_ERR			(TRAP_RTN_OK-100)
#define TRAP_DBUD_MESSAGE_ERR		(TRAP_RTN_DBUS_ERR-1)



#define	XML_FILE_PATH	"/opt/services/option/snmpd_option"
#define CONF_FILE_PATH		"/opt/services/conf/snmpd_conf.conf"
#define STATUS_FILE_PATH		"/opt/services/status/snmpd_status.status"
#define SUB_STATUS_FILE_PATH		"/opt/services/status/subagent_status.status"
#define TRAP_STATUS_FILE_PATH		"/opt/services/status/trap-helper_status.status"

#define STATISTICS_INTERVAL     "statistics-interval"
#define SAMPLING_INTERVAL       "sampling-interval"
#define HEARTBEAT_INTERVAL      "heartbeat-interval"
#define HEARTBEAT_MODE          "trap heartbeat mode"         
#define RESEND_INTERVAL         "resend_interval"
#define RESEND_TIMES            "resend_times"
#define CPU_THRESHOLD           "cpu-threshold"
#define MEMORY_THRESHOLD        "memory-threshold"
#define TEMPERATURE_THRESHOLD   "temperature-threshold"


#define SNMP_ENGINE_VERSION			0x00000001
#define SNMP_ENGINE_VERSION_MASK	0xffffff00
#define SNMP_ENGINE_MAX_INFO_LEN	64
#define SNMP_ENGINE_INFO_STR		"AuteCS snmp agent web config engine data"


#define SNMP_AGENG_PORT			161
#define SNMP_TRAP_DEFAULT_PORT	162


#define SCRIPT_PATH				"sudo /opt/services/init/snmpd_init"
#define SCRIPT_PARAM_START		"start"
#define SCRIPT_PARAM_RESTART	"restart"
#define SCRIPT_PARAM_STOP		"stop"


// ap inner
#define wtpDownTrap    						"wtpDownTrap"						//0.1
#define wtpSysStartTrap 					"wtpSysStartTrap"					//0.2
#define wtpChannelModifiedTrap  			"wtpChannelModifiedTrap"			//0.3
#define wtpIPChangeAlarmTrap				"wtpIPChangeAlarmTrap"				//0.4
#define wtpFlashWriteFailTrap				"wtpFlashWriteFailTrap"				//0.5
#define wtpColdStartTrap					"wtpColdStartTrap"					//0.6
#define wtpAuthModeChangeTrap				"wtpAuthModeChangeTrap"				//0.7
#define wtpPreSharedKeyChangeTrap			"wtpPreSharedKeyChangeTrap"			//0.8
#define wtpElectrifyRegisterCircleTrap		"wtpElectrifyRegisterCircleTrap"	//0.9
#define wtpAPUpdateTrap						"wtpAPUpdateTrap"					//0.10

#define wtpCoverholeTrap					"wtpCoverholeTrap"					//0.11
#define wtpWirePortExceptionTrap			"wtpWirePortExceptionTrap"			//0.12  not support now
#define CPUusageTooHighTrap					"CPUusageTooHighTrap"				//0.13
#define CPUusageTooHighRecovTrap			"CPUusageTooHighRecovTrap"			//0.14
#define MemUsageTooHighTrap					"MemUsageTooHighTrap"				//0.15
#define MemUsageTooHighRecovTrap			"MemUsageTooHighRecovTrap"			//0.16
#define TemperTooHighTrap					"TemperTooHighTrap"					//0.17
#define TemperTooHighRecoverTrap			"TemperTooHighRecoverTrap"			//0.18
#define APMtWorkModeChgTrap					"APMtWorkModeChgTrap"				//0.19
#define APswitchBetweenACTrap				"APswitchBetweenACTrap"				//0.20 not support now

#define SSIDkeyConflictTrap					"SSIDkeyConflictTrap"				//0.21
#define wtpOnlineTrap						"wtpOnlineTrap"						//0.22
#define wtpOfflineTrap						"wtpOfflineTrap"					//0.23
#define wtpCoverHoleClearTrap				"wtpCoverHoleClearTrap"				//0.24
#define wtpSoftWareUpdateSucceed			"wtpSoftWareUpdateSucceed"		//0.25
#define wtpSoftWareUpdateFailed				"wtpSoftWareUpdateFailed"				//0.29
#define wtpConfigurationErrorTrap			"wtpConfigurationErrorTrap"				//0.30
/*wangchao add*/
#define ReservedReason						"ReservedReason"				//0.31
#define ACCTimeout							"ACCTimeout"					//0.32
#define ExtendReason						"ExtendReason"					//0.32
#define LteFiUplinkSwitch					"LteFiUplinkSwitch"				//0.34

//ap app
#define wtpChannelObstructionTrap			"wtpChannelObstructionTrap"				//1.1
#define	wtpAPInterferenceDetectedTrap		"wtpAPInterferenceDetectedTrap" 		//1.2
#define wtpStaInterferenceDetectedTrap		"wtpStaInterferenceDetectedTrap"		//1.3
#define wtpDeviceInterferenceDetectedTrap	"wtpDeviceInterferenceDetectedTrap"		//1.4
#define wtpSubscriberDatabaseFullTrap		"wtpSubscriberDatabaseFullTrap"			//1.5
#define wtpDFSFreeCountBelowThresholdTrap	"wtpDFSFreeCountBelowThresholdTrap"		//1.6
#define wtpFileTransferTrap					"wtpFileTransferTrap"					//1.7
#define wtpStationOffLineTrap				"wtpStationOffLineTrap"					//1.8
#define wtpSolveLinkVarifiedTrap			"wtpSolveLinkVarifiedTrap"				//1.9
#define wtpLinkVarifyFailedTrap				"wtpLinkVarifyFailedTrap"				//1.10

#define wtpStationOnLineTrap				"wtpStationOnLineTrap"					//1.11				
#define APInterfClearTrap					"APInterfClearTrap"						//1.12
#define APStaInterfClearTrap				"APStaInterfClearTrap"					//1.13
#define APOtherDeviceInterfDetectedTrap		"APOtherDeviceInterfDetectedTrap"		//1.14 not support now
#define APOtherDevInterfClearTrap			"APOtherDevInterfClearTrap"				//1.15
#define APModuleTroubleTrap					"APModuleTroubleTrap"					//1.16
#define APModuleTroubleClearTrap			"APModuleTroubleClearTrap"				//1.17
#define APRadioDownTrap						"APRadioDownTrap"						//1.18
#define APRadioDownRecovTrap				"APRadioDownRecovTrap"					//1.19
#define APTLAuthenticationFailedTrap		"APTLAuthenticationFailedTrap"			//1.20

#define APStaAuthErrorTrap					"APStaAuthErrorTrap"					//1.21
#define APStAssociationFailTrap				"APStAssociationFailTrap"				//1.22
#define APUserWithInvalidCerInbreakNetTrap	"APUserWithInvalidCerInbreakNetTrap"	//1.23
#define APStationRepititiveAttackTrap		"APStationRepititiveAttackTrap"			//1.24
#define APTamperAttackTrap					"APTamperAttackTrap"					//1.25
#define APLowSafeLevelAttackTrap			"APLowSafeLevelAttackTrap"				//1.26
#define APAddressRedirectionAttackTrap		"APAddressRedirectionAttackTrap"		//1.27
#define APMeshAuthFailureTrap				"APMeshAuthFailureTrap"					//1.28
#define APChildExcludedParentTrap			"APChildExcludedParentTrap"				//1.29
#define APParentChangeTrap					"APParentChangeTrap"					//1.30
	
#define APChildMovedTrap					"APChildMovedTrap"						//1.31
#define APExcessiveParentChangeTrap			"APExcessiveParentChangeTrap"			//1.32
#define APMeshOnsetSNRTrap					"APMeshOnsetSNRTrap"					//1.33
#define APMeshAbateSNRTrap					"APMeshAbateSNRTrap"					//1.34
#define APConsoleLoginTrap					"APConsoleLoginTrap"					//1.35
#define APQueueOverflowTrap					"APQueueOverflowTrap"					//1.36
#define APAddUserFailClearTrap				"APAddUserFailClearTrap"				//1.37
#define APChannelTooLowClearTrap			"APChannelTooLowClearTrap"				//1.38
#define APFindUnsafeESSID					"APFindUnsafeESSID"						//1.39
#define APFindSYNAttack						"APFindSYNAttack"						//1.40
#define ApNeighborChannelInterfTrap			"ApNeighborChannelInterfTrap"			//1.41
#define ApNeighborChannelInterfTrapClear	"ApNeighborChannelInterfTrapClear"		//1.42
#define wtpStationOffLineAbnormalTrap	    "wtpStationOffLineAbnormalTrap"			//1.44
#define wtpUserLogoffAbnormalTrap	    	"wtpUserLogoffAbnormalTrap"				//1.45
#define wtpUserTrafficOverloadTrap	    	"wtpUserTrafficOverloadTrap"				//1.51
#define wtpUnauthorizedStaMacTrap	    	"wtpUnauthorizedStaMacTrap"				//1.52



//ac inner
#define acSystemRebootTrap					"acSystemRebootTrap"					//0.1
#define acAPACTimeSynchroFailureTrap		"acAPACTimeSynchroFailureTrap"			//0.2
#define acChangedIPTrap						"acChangedIPTrap"						//0.3
#define acTurntoBackupDeviceTrap			"acTurntoBackupDeviceTrap"				//0.4
#define acTurntoMasterDeviceTrap			"acTurntoMasterDeviceTrap"				//0.4
#define acConfigurationErrorTrap			"acConfigurationErrorTrap"				//0.5
#define acSysColdStartTrap					"acSysColdStartTrap"					//0.6
#define acHeartbeatTrap						"acHeartbeatTrap"						//0.7

#define acAPACTimeSynchroFailureTrapClear   "acAPACTimeSynchroFailureTrapClear"		//0.22

#define acBoardExtractTrap					"acBoardExtractTrap"					//0.24
#define acBoardInsertTrap					"acBoardInsertTrap"						//0.25
#define acPortDownTrap						"acPortDownTrap"						//0.26
#define acPortUpTrap						"acPortUpTrap"							//0.27



//ac app 
#define acDiscoveryDangerAPTrap				"acDiscoveryDangerAPTrap"				//1.1
#define acRadiusAuthenticationServerNotReachTrap		"acRadiusAuthenticationServerNotReachTrap"	//1.2
#define acRadiusAccountServerNotLinkTrap	"acRadiusAccountServerNotLinkTrap"		//1.3
#define acPortalServerNotReachTrap			"acPortalServerNotReachTrap"			//1.4
#define acAPLostNetTrap						"acAPLostNetTrap"						//1.5
#define acCPUUtilizationOverThresholdTrap	"acCPUUtilizationOverThresholdTrap"		//1.6
#define acMemUtilizationOverThresholdTrap	"acMemUtilizationOverThresholdTrap"		//1.7
#define acBandwithOverThresholdTrap			"acBandwithOverThresholdTrap"			//1.8
#define acLostPackRateOverThresholdTrap		"acLostPackRateOverThresholdTrap"		//1.9
#define acMaxUsrNumOverThresholdTrap		"acMaxUsrNumOverThresholdTrap"			//1.10

#define acDisconnectNumOverThresholdTrap	"acDisconnectNumOverThresholdTrap"		//1.11
#define acDropRateOverThresholdTrap			"acDropRateOverThresholdTrap"			//1.12
#define acOfflineSucRateBelowThresholdTrap	"acOfflineSucRateBelowThresholdTrap"	//1.13
#define acAuthSucRateBelowThresholdTrap		"acAuthSucRateBelowThresholdTrap"		//1.14
#define acAveIPPoolOverThresholdTrap		"acAveIPPoolOverThresholdTrap"			//1.15
#define acMaxIPPoolOverThresholdTrap		"acMaxIPPoolOverThresholdTrap"			//1.16
#define acDHCPSucRateOverThresholdTrap		"acDHCPSucRateOverThresholdTrap"		//1.17
#define acPowerOffTrap						"acPowerOffTrap"						//1.18
#define acPowerOffRecovTrap					"acPowerOffRecovTrap"					//1.19
#define acCPUusageTooHighRecovTrap			"acCPUusageTooHighRecovTrap"			//1.20

#define acMemUsageTooHighRecovTrap			"acMemUsageTooHighRecovTrap"			//1.21
#define acTemperTooHighTrap					"acTemperTooHighTrap"					//1.22
#define acTemperTooHighRecovTrap			"acTemperTooHighRecovTrap"				//1.23
#define acDHCPAddressExhaustTrap			"acDHCPAddressExhaustTrap"				//1.24
#define acDHCPAddressExhaustRecovTrap		"acDHCPAddressExhaustRecovTrap"			//1.25
#define acRadiusAuthServerAvailableTrap		"acRadiusAuthServerAvailableTrap"		//1.26
#define acRadiusAccServerAvailableTrap		"acRadiusAccServerAvailableTrap"		//1.27
#define acPortalServerAvailableTrap			"acPortalServerAvailableTrap"			//1.28
#define acFindAttackTrap					"acFindAttackTrap"						//1.29
#define acMaxUsrNumOverThresholdTrapClear	"acMaxUsrNumOverThresholdTrapClear"		//1.30

#define acHeartTimePackageTrap				"acHeartTimePackageTrap"				//1.50

 

/****struct parameter define****/

#define MAX_SNMP_NAME_LEN 	31
#define SPE_NAME_TMP 	200	

#if 1
#define MAX_IP_ADDR_LEN		32
#define MAX_IPv6_ADDR_LEN		128
#else
#define MAX_IP_ADDR_LEN		128
#endif

#define MIN_SNMP_PASSWORD_LEN	12
#define MAX_SNMP_PASSWORD_LEN	25


#define MAX_COMMUNITY_NUM		10
#define MAX_SNMPV3_USER_NUM		10
#define MAX_TRAPRECEIVER_NUM	10

#define MAX_OID_LEN				256
#define MAX_SYSTEM_NAME_LEN		128
#define MAX_SYSTEM_DESCRIPTION	128
#define MAX_FILE_CONTENT		50000  //文件内字符串数目大小
#define MAX_MIB_SYSNAME			128
#define HOST_NAME_LENTH			128

#define MAX_VIEW_INEX_NUM		50  //每个视图包含的最大规则数(include\exclude) 为50
#define MAX_VIEW_NUM  			30  //最大视图个数
#define MAX_VIEW_NAME_LEN 		21

#define MAX_oid 				128

#define MAX_GROUP_NAME_LEN   	21
#define MAX_SNMP_GROUP_NUM      20

#define MAX_INTERFACE_NAME_LEN      33
#define MAX_SNMP_PFM_INTERFACE_NUM  10    

#define MAX_TRAP_PARAMETER_LEN  50

/******END*******/


struct oid_list {
    char oid[MAX_oid];

    struct oid_list *next;
};

typedef struct view_oid_node {
	int oid_num;
	struct oid_list *oidHead;
}view_list;

typedef struct STSNMPView_s {
	char name[MAX_VIEW_NAME_LEN];
	view_list	view_included;
	view_list	view_excluded;

	struct STSNMPView_s *next;
}STSNMPView;

/*
*   community
*/
#define ACCESS_MODE		unsigned int
#define	ACCESS_MODE_RO	0
#define ACCESS_MODE_RW	1

#define CONF_STATUS		unsigned int
#define RULE_DISABLE	0
#define RULE_ENABLE		1


#define MAX_IP_FORMAT 	16


enum {
	WTPRESTART = 1,						/*1*/
	WTPCHANNELMODIFIED,					/*2*/
	WTPIPCHANGE,						/*3*/
	WTPFLASHWRITEFAILORWTPUPDATE,		/*4*/
	WTPAUTHMODECHANGE,					/*5*/
	WTPPRESHAREDKEYCHANGE,				/*6*/
	WTPELECTRIFYREGISTECIRECLE,			/*7*/
	WTPLINKVARIFYFAILED,				/*8*/
	WTPCOVERHOLE,						/*9*/
	WTPCPUUSAGETOOHIGH,					/*10*/
	WTPMEMUSAGETOOHIGH,					/*11*/
	WTPTEMPERTOOHIGH,					/*12*/
	WTPMTWORKMODECHANGE,				/*13*/
	WTPSSIDkEYCONFLICT,					/*14*/
	WTPONLINEOFFLINE,					/*15*/
	WTPCHANNELOBSTRUCT,					/*16*/
	WTPINTERFERENCEDETECTED,			/*17*/
	WTPSTAINTERFERENCEDETECTED,			/*18*/
	WTPOTHERDEVICEINTERFERENCE,			/*19*/
	WTPSUBSCRIBERDATABASEFULL,			/*20*/
	WTPDFSFREECOUNTBELOWTHRESHOLD,		/*21*/
	WTPSTATIONOFFLINE,					/*22*/
	WTPMODULETROUBLE,					/*23*/
	WTPRADIODOWN,						/*24*/
	WTPSTAAUTHERROR,					/*25*/
	WTPSTAASSOCIATEFAIL,				/*26*/
	WTPSTAWITHINVALIDCERINBREAKNET,		/*27*/
	WTPSTAREPITITIVEATTACK,				/*28*/
	WTPTAMPERATTACK,					/*29*/
	WTPLOWSAFELEVELATTACK,				/*30*/
	WTPADDRESSREDIRECTATTACK,			/*31*/
	WTPADDUSERFAIL,						/*32*/
	WTPCHANNELTOOLOWCLEAR,				/*33*/
	WTPFINDUNSAFEESSID,					/*34*/
	WTPFINDSYNATTACK,					/*35*/
	WTPNEIGHBORCHANNELINTER,			/*36*/
	WTPSTATIONOFFLINEABNORMAL = 44, 	/*44*/
	WTPUSERLOGOOFFABNORMAL = 45,		/*45*/
	WTPCONFIGERROR=51,				/*51*/
	WTPUSERTAFFICOVERLOAD,				/*52*/
	WTPUNAUTHORIZEDSTAMACTRAP,			/*53*/
	MAXAPTRAPNUM,						/*54*/
	LTEFIQUITREASON	,				/*55 wangchao*/
	LTEUPLINKSWITCH						/*56*/	
};

enum {
	ACRESTART = 1,						/*1*/
	ACAPTIMESYNCHROFAIL,				/*2*/
	ACCHANGEDIP,						/*3*/
	ACMASTERANDSTANDBYSWITCH,			/*4*/ 
	ACCONFIGERROR,						/*5*/
	ACDISCOVERDANGERAP,					/*6*/
	ACRADIUSAUTHSERVERNOTREACH,			/*7*/
	ACRADIUSACCOUNTSERVERNOTREACH,		/*8*/
	ACPORTALSERVERNOTREACH,				/*9*/
	ACAPLOSTNET,						/*10*/
	ACCPUUTILIZATIONOVERTHRESHOLD,		/*11*/
	ACMEMUTILIZATIONOVERTHRESHOLD,		/*12*/
	ACBANDWITHOVERTHRESHOLD,			/*13*/
	ACLOSTPACKRATEOVERTHRESHOLD,		/*14*/
	ACMAXUSRNUMOVERTHRESHOLD,			/*15*/
	ACAUTHSUCCRATEBELOWTHRESHOLD,		/*16*/
	ACAVEIPPOOLOVERTHRESHOLD,			/*17*/
	ACMAXIPPOOLOVERTHRESHOLD,			/*18*/
	ACPOWEROFF,							/*19*/
	ACDHCPADDRESSEXHAUST,				/*20*/
	ACTEMPERTOOHIGH,					/*21*/
	ACFINDATTACK,						/*22*/
	ACHEARTTIMEPACKAGE,					/*23*/
	ACBOARDPULLOUT,						/*24*/
	ACPORTDOWN,							/*25*/
	MAXACTRAPNUM
};


typedef struct STCommunity_s{
	char	community[MAX_SNMP_NAME_LEN];
	char	ip_addr[MAX_IP_ADDR_LEN];
	char	ip_mask[MAX_IP_ADDR_LEN];
	ACCESS_MODE	access_mode;
	CONF_STATUS	status;
	
	struct STCommunity_s *next;
}STCommunity;

typedef struct IPV6STCommunity_s{
	char	community[MAX_SNMP_NAME_LEN];
	char	ip_addr[MAX_IPv6_ADDR_LEN];
	unsigned int	prefix;
	ACCESS_MODE	access_mode;
	CONF_STATUS	status;
	
	struct IPV6STCommunity_s *next;
}IPV6STCommunity;


/*
*   v3 user
*/

#define	AUTH_PROTOCAL	unsigned int
#define AUTH_PRO_NONE	0
#define AUTH_PRO_MD5	1
#define AUTH_PRO_SHA	2

#define	PRIV_PROTOCAL	unsigned int
#define PRIV_PRO_NONE	0
#define PRIV_PRO_AES	1
#define PRIV_PRO_DES	2

typedef struct STSNMPV3User_s{
	char 		name[MAX_SNMP_NAME_LEN];
	ACCESS_MODE access_mode;
	
	/*authentication  info */
	struct {
		AUTH_PROTOCAL protocal;
		char	passwd[MAX_SNMP_PASSWORD_LEN];
	}	authentication;
	
	/*privacy  info */
	struct {
		PRIV_PROTOCAL protocal;
		char	passwd[MAX_SNMP_PASSWORD_LEN];
	}	privacy;
	
	char   group_name[MAX_GROUP_NAME_LEN];
	CONF_STATUS status;	

	struct STSNMPV3User_s *next;
}STSNMPV3User;


#define V1      1
#define V2      2
#define V3      3

typedef struct snmp_interface {
    char ifName[MAX_INTERFACE_NAME_LEN];

    struct snmp_interface *next;
}SNMPINTERFACE;
/*
*   system info
*/
typedef struct {
	char sys_name[MAX_SYSTEM_NAME_LEN];
	char sys_description[MAX_SYSTEM_DESCRIPTION];
	char sys_oid[MAX_OID_LEN];
	unsigned int agent_port;
	unsigned int agent_port_ipv6;
	unsigned int trap_port;
	
	CONF_STATUS	v1_status;
	CONF_STATUS	v2c_status;
	CONF_STATUS	v3_status;	
	int 		cache_time;
    int         collection_mode;
	unsigned int sysoid_boardtype;
}STSNMPSysInfo;

/*
*   group
*/
#define SEC_LEVEL   unsigned int
#define SEC_NOAUTH  0
#define SEC_AUTH    1
#define SEC_PRIV    2

typedef struct STSNMPGroup_s {
	char  group_name[MAX_GROUP_NAME_LEN];
	char  group_view[MAX_VIEW_NAME_LEN];
	ACCESS_MODE	access_mode;
	SEC_LEVEL   sec_level;

	struct STSNMPGroup_s *next;
}STSNMPGroup;


/*
*   product_oid
*/
typedef struct {
	unsigned int 	value;
	char * 			borad_type;
}BORAD_OID_TYPE;

typedef struct {
	unsigned int 	value;
	char * 			product_type;
	char * 			product_node;
}PRODUCT_OID_SERIAL;


/*
*   trap receiver
*/
typedef struct STSNMPTrapReceiver_s {
	unsigned int    local_id;
	unsigned int    instance_id;
	
	char 	        name[MAX_SNMP_NAME_LEN];
	char	        sour_ipAddr[MAX_IP_ADDR_LEN];
	unsigned int  	sour_port;
	char	        dest_ipAddr[MAX_IP_ADDR_LEN];
	unsigned int  	dest_port;
	
	unsigned int 	version;
	unsigned char 	trapcom[MAX_SNMP_NAME_LEN];
	CONF_STATUS     status;

	struct STSNMPTrapReceiver_s *next;
}STSNMPTrapReceiver;

typedef struct {
	char trapName[128];
	char trapDes[128];
	char trapEDes[128];
	int  trapSwitch;
}TRAP_DETAIL_CONFIG;

struct trap_group_switch {
    unsigned long long low_switch;
    unsigned long long high_switch;
};

typedef struct {
    char paraStr[128];
    unsigned int data;
}TRAPParameter;

typedef struct trapHeartbeatIP_s {
    unsigned int    local_id;
	unsigned int    instance_id;
	
    char	        ipAddr[MAX_IP_ADDR_LEN];
    
    struct trapHeartbeatIP_s *next;
}TRAPHeartbeatIP;


#ifndef OEM_COM
#define OEM_COM
#define ENTERPRISE_OID_LENGTH 20
#define PRODUCT_OID_LENGTH 20
#define TOTAL_OID_LENGTH 128
#define ENTERPRISE_NODE_PATH	"/devinfo/enterprise_snmp_oid"
#define ENTERPRISE_NODE_SH	" cat /devinfo/enterprise_snmp_oid"

#define PRODUCT_NODE_PATH		"/devinfo/snmp_sys_oid"
#define PRODUCT_NODE_SH		"cat /devinfo/snmp_sys_oid"

#define ENTERPRISE_OID 	"31656"
#define MAX_OID_LENTH 	128

#endif



//将192.169.1.1格式的地址转化成int型的地址
#define INET_ATON(ipaddr,addr_str)	\
		{\
			unsigned int a1,a2,a3,a4;\
			int ret;\
			ret = sscanf(addr_str,"%u.%u.%u.%u",&a1,&a2,&a3,&a4);\
			if( ret == 4 ){\
				ipaddr = a1*256*256*256+a2*256*256+a3*256+a4;\
			}else{\
				ipaddr=0;\
			}\
		}
//将int 32的值转化成ip地址字符串
#define INET_NTOA(ip_int,addr_str)\
		{\
			unsigned int a1,a2,a3,a4;\
			unsigned int ip_uint = (unsigned int)ip_int;\
			a1 = (ip_uint&0xff000000)>>24;\
			a2 = (ip_uint&0x00ff0000)>>16;\
			a3 = (ip_uint&0x0000ff00)>>8;\
			a4 = (ip_uint&0x000000ff);\
			sprintf( addr_str, "%d.%d.%d.%d", a1,a2,a3,a4 );\
		}

#endif

