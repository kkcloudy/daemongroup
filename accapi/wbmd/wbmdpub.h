#ifndef _WBMD_PUB_H
#define	_WBMD_PUB_H
#include <sys/types.h>
#include <netinet/in.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/types.h>

#define MAC_LEN	6
#define DEFAULT_LEN	256
#define NAME_LEN	20
#define IF_NAME_MAX 16
#define THREAD_NUM	16
#define PATH_LEN 64
#define IP_LEN	16
#define WBRIDGE_NUM	1025
#define WBRIDGE_HASH	256
#define WB_HASH(ip) (ip%WBRIDGE_HASH)
#define IF_NUM	16
struct WbmdRmPropertyInfo{	
	int rmPropertiesIfIndex;
	char rmType[NAME_LEN];
	int rmFrequency;
	int rmBitRate;
	int rmSid;
	int rmCurPowerLevel;
	int rmModulation;
	int rmAntenna;
	int rmDistance;
	int rmBurst;
	int rmLongRange;
	int rmPowerCtl;
	int rmTXRT;
	int rmTXVRT;
	int rmPTP;
	int rmWOCD;
	int rmBCsid;
	int rmDistanceAuto;
	int rmNoiseFloor;
	int rmBandwidth;
	int rmChainMode;
};

struct WbmdMintNodeInfo{
	unsigned char netAddress[MAC_LEN];
	int nodeType;
	int nodeMode;
	int linksCount;
	int nodesCount;
	int nodeInterfaceId;
	int protocolEnabled;
	char nodeName[NAME_LEN] ;
	int autoBitrateEnable;
	int autoBitrateAddition;
	int autoBitrateMinLevel;
	int extraCost;
	int fixedCost;
	int nodeID;
	int ampLow;
	int ampHigh;
	int authMode;
	int authRelay;
	int crypt;
	int compress;
	int overTheAirUpgradeEnable;
	int overTheAirUpgradeSpeed;
	int roaming;
	int polling;
	int mintBroadcastRate;
	int noiseFloor;
	char secretKey[DEFAULT_LEN];
};

struct WbmdBasicIfStaticsInfo{
	int ifIndex;
	char ifDescr[IF_NAME_MAX];
	int ifType;
	int ifMtu;
	int ifSpeed;
	unsigned char ifPhysAddress[MAC_LEN];
	int ifAdminStatus;
	int ifOperStatus;
	int ifLastChange;
	int ifInOctets;
	int ifInUcastPkts;
	int ifInNUcastPkts;
	int ifInDiscards;
	int ifInErrors;
	int ifInUnknownProtos;
	int ifOutOctets;
	int ifOutUcastPkts;
	int ifOutNUcastPkts;
	int ifOutDiscards;
	int ifOutErrors;
	int ifOutQLen;
	int ifSpecific;
};

struct WbmdSysInfo{
	int sysCpu;
	int sysTemperature;
	int sysMemTotal;
	int sysMemFree;
};

struct wbridge_info{
	int WBID;
	unsigned int IP;
	struct sockaddr_in wbaddr;
	int CheckTimerID;
	int GetIfInfoTimerID;
	int GetIfInfoTimes;
	int GetMintInfoTimerID;
	int GetMintInfoTimes;
	int GetRfInfoTimerID;
	int GetRfInfoTimes;
	int WBState;
	int PackNo;
	netsnmp_session session;
	int Check_Count;	
    char *argv[255];
    int  argn;
	time_t access_time;
	time_t leave_time;
	time_t last_access_time;
	int if_num;	
	struct WbmdBasicIfStaticsInfo WBIF[IF_NUM];
	struct WbmdSysInfo	WBSYS;
	struct WbmdMintNodeInfo WBMintNode;
	struct WbmdRmPropertyInfo WBRmProperty;
	struct wbridge_info *hnext;
}; 

struct wbridge_ip_hash{
	struct wbridge_info * wBridge_hash[WBRIDGE_HASH];
	int wb_num;
};

typedef struct{
	unsigned int wb_num;
	struct wbridge_info **wbridge;
}DCLI_WBRIDGE_API_GROUP;

typedef enum{
	WBMD_ADD = 1,
	WBMD_DEL = 2,
	WBMD_SET = 3,
	WBMD_GET = 4,
}WbmdOP;

typedef enum{
	WBMD_WB = 1,
	WBMD_RF = 2,
	WBMD_IF = 3,
	WBMD_MINT = 4,	
}WbmdType;

struct WbmdMsg{
	WbmdOP op;
	WbmdType type;
	int WBID;
	union{

	}u;
};

struct WbmdMsgQ{
	long mqid;
	struct WbmdMsg mqinfo;
};

typedef enum{
	WBMD_CHECK = 1,
	WBMD_DATA = 2
}WbmdCheckType;

struct WbmdCheckMsg{
	WbmdCheckType type;
	int WBID;
	int Datalen;
	char Data[DEFAULT_LEN];
};

struct WbmdCheckMsgQ{
	long mqid;
	struct WbmdCheckMsg mqinfo;
};



#endif
