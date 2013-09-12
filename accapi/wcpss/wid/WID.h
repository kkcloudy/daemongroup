/*
*this file define structs descripting framework and information of 
*WLAN/WTP/STA which creared,added,set by AC
*/



#ifndef _WID_DEFINE_H
#define _WID_DEFINE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include "wcpss/waw.h"
#include "ACMsgq.h"
#include <linux/tipc.h>
#ifndef _WID_TYPE_DEF
#define _WID_TYPE_DEF
typedef unsigned char 		u_int8_t;
typedef unsigned short 		u_int16_t;
typedef unsigned int		u_int32_t;
#endif/*_WCPSS_TYPE_DEF*/
#define WID_ID_LEN	32
#define WID_MAXN_INDEX 64
#define WID_IP_LEN	4
#define WID_MAC_LEN	6
#define WID_DEFAULT_NUM	16
#define WPA_ELEMENT_ID 221
//#define ETH_IF_NAME_LEN 16/*Added by weiay 20080710*/
#define ROGUE_AP_REPORT_INT 3600				/*xdw modify, change 120 to 3600, 20101217*/
#define WIFI_IOC_IF_CREATE  _IOWR(243, 1, struct interface_basic_INFO)
#define WIFI_IOC_IF_DELETE  _IOWR(243, 2, struct interface_basic_INFO) 
#define WIFI_IOC_IF_UPDATE  _IOWR(243, 6, struct interface_INFO)
#define WIFI_IOC_WSM_SWITCH  _IOWR(243, 7, unsigned int)
#define WIFI_IOC_ASD_SWITCH  _IOWR(243, 8, unsigned int)/*ht add for wifi and asd communicate switch,110308*/
#define WIFI_IOC_HANSISTATE_UPDATE 	_IOWR(243, 9, unsigned int) /*wuwl add control data packet*/
#define WIFI_IOC_ASD_THRED_ID _IOWR(243,10,unsigned int)/*wuwl add asd pid notice to wifi for netlink*/
#define WIFI_IOC_ADD_STA		_IOWR(243, 11, struct asd_to_wifi_sta) 
#define WIFI_IOC_DEL_STA		_IOWR(243, 12, struct asd_to_wifi_sta)
#define WIFI_IOC_BATCH_IF_CREATE  _IOWR(243, 13, struct interface_batch_INFO)
#define WIFI_IOC_BATCH_IF_DELETE  _IOWR(243, 14, struct interface_batch_INFO)

#define TEST_SWITCH_WAY  1 /*zhanglei change*/
#define WID_SYSTEM_CMD_LENTH 256
#define WTP_WEP_NUM 4
#define DEFAULT_SN_LENTH 20
#define D_LEN	128
#define SECTOR_NUM 4
#define TX_CHANIMASK_NUM 3

#define INSTANCE_CREATED		(1)		/* instance have created.		*/
#define INSTANCE_NO_CREATED	(0)		/* instance have not created.	*/

#define WIFI_IOC_MAGIC 244
#define WIFI_IOC_IP_ADD   _IOWR(WIFI_IOC_MAGIC, 4, ex_ip_info)
#define WIFI_IOC_IP_DEL   _IOWR(WIFI_IOC_MAGIC, 5, ex_ip_info)

#define		CW_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define		CW_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#define		CW_CREATE_STRING_ERR(str_name, str_length, on_err)	{str_name = (char*) (malloc(sizeof(char) * ((str_length)+1) ) ); if(!(str_name)) {on_err}}
#define		CW_CREATE_STRING_ERR_UNSIGNED(str_name, str_length, on_err)	{str_name = (unsigned char*) (malloc(sizeof(char) * ((str_length)+1) ) ); if(!(str_name)) {on_err}}

//#define		CW_ADD_OBJECT_ELE(obj_name1, obj_type1,obj_count1,,obj_ele1,obj_name2,obj_type2)	{if((obj_name2 == NULL)||((*obj_name1) == NULL)){printf("insert_elem_into_list_heads parameter error\n");	return -1;}	if((*obj_name1)->obj_count1 == 0){(*obj_name1)->obj_ele1 = obj_name2;(*obj_name1)->obj_count1++;return 0;}struct obj_type2 *pnode = (*obj_name1)->obj_ele1;(*obj_name1)->obj_ele1 = obj_name2;obj_name2->next = pnode;(*obj_name1)->obj_count1++; return 0;}
#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDRESS "%02X%02X-%02X%02X-%02X%02X"//qiuchen add it for Henan Mobile 2013.02.20

/* For new format of mobile syslog 2013-07-29 */
#define AUTELANID "@31656"
#define DS_STRUCTURE_ALL "[DS@31656 slot=\"%d\" inst=\"%d\"]"
#define DS_STRUCTURE "[DS@31656 slot=\"%d\"]"
#define LOG_MAC "mac=\"%02X:%02X:%02X:%02X:%02X:%02X\""
#define LOG_IP_V4 "ip=\"%lu.%lu.%lu.%lu\""
#define LOG_IP_STR "ip=\"%s\""
#define LOG_BSSID "bssid=\"%02X:%02X:%02X:%02X:%02X:%02X\""
#define LOG_IF "if=\"%s\""
#define LOG_NAME "name=\"%s\""
#define LOG_SSID "ssid=\"%s\""
#define LOG_SEC "security=\"%d\""
#define LOG_RADIO "radio=\"%d\""
#define LOG_RADIOS "radios=\"%d\""
#define LOG_CODE "reason code=\"%s%d\""
#define LOG_DESC "desc=\"%s\""
#define LOG_ROAM "ip=\"%lu.%lu.%lu.%lu\" bssid=\"%02X:%02X:%02X:%02X:%02X:%02X\" ap_mac=\"%02X:%02X:%02X:%02X:%02X:%02X\""
#define LOG_RDS "nas_id=\"%s\" nas_port_id=\"%s\""
#define LOG_ACT "tx=\"%lu\" rx=\"%lu\" tx_pkt=\"%lu\" rx_pkt=\"%lu\" online=\"%d\""
#define LOG_TYPE "type=\"%d\""
#define LOG_VLAN "vlan=\"%d\""
#define IPSTRINT(a) ((a & 0xff000000) >> 24),((a & 0xff0000) >> 16),((a & 0xff00) >> 8),(a & 0xff)
#endif

#ifndef OUIMAC2STR
#define OUIMAC2STR(a) (a)[0],(a)[1],(a)[2]
#define OUIMACSTR "%02x:%02x:%02x"
#endif 
typedef struct ex_ip_INFO
{
 unsigned char  if_name[16];
 unsigned int dip;
 unsigned int sip;
 unsigned char wtpmac[6];
}ex_ip_info;


/*

#define WLAN_NUM		15
#define WTP_NUM		15
#define L_RADIO_NUM		4
#define G_RADIO_NUM		60
#define BSS_NUM		240

typedef struct{
	u_int8_t WlanID;
	char WlanName[20];
	char ESSID[20];
}wASD_WLAN;

typedef struct{	
	u_int8_t	BSSIndex;
	u_int8_t	Radio_L_ID;
	u_int8_t	Radio_G_ID;
	u_int8_t	WlanID;
	char	BSSID[6];

}wASD_BSS;



typedef enum{
	WID_ADD = 0,
	WID_DEL = 1,
	WID_MODIFY = 2
}WIDOperate;

typedef enum{
	WLAN_TYPE = 0,
	WTP_TYPE = 1,
	RADIO_TYPE = 2,
	BSS_TYPE = 3
}WIDMsgType;

typedef struct {
	WIDMsgType MsgType;
	WIDOperate MsgOp;
	wASD_WLAN WLAN;
	wASD_BSS  BSS;
}WIDMsg;
*/
typedef enum {
	CW_FALSE_DCLI = 0,
	CW_TRUE_DCLI = 1
} CWBool_DCLI;
enum wid_debug{
	WID_DEFAULT = 0x1,
	WID_DBUS = 0x2,
	WID_WTPINFO = 0x4,
	WID_MB = 0x8,/*master and bak*/
	WID_ALL = 0xf
};
typedef enum {
	WID_SULKING = 0,
	WID_DISCOVERY = 1,
	WID_JOIN = 2,
	WID_CONFIGURE = 3,
	WID_DATA_CHECK = 4,
	WID_RUN = 5,
	WID_RESET = 6,
	WID_QUIT = 7
}WTPState;

typedef enum {
	WTP_INIT = 0,
	WTP_UNUSED = 1,
	WTP_NORMAL = 2,
	IF_NOINDEX = 3,
	IF_NOFLAGS = 4,
	IF_DOWN = 5,
	IF_NOADDR = 6,	
	WTP_TIMEOUT = 7
}WTPQUITREASON;

struct radio_info_type_dcli{
	char radio_type;
	char radio_id;
	char bss_count;
	char txpower;
	char reserved1;
	char reserved2;
	unsigned short reserved3;	
};
typedef enum{
	DOWN_LINK_IF_TYPE = 0x1,		
	DOWN_LINK_IP_TYPE = 0x2,
	LIC_TYPE = 0x4
}LISTEN_FLAG;
struct ifi {
  char    ifi_name[ETH_IF_NAME_LEN];	/* interface name, null-terminated */
  short   ifi_index;			/* interface index */
  unsigned int nas_id_len;
  char nas_id[NAS_IDENTIFIER_NAME];
  char isipv6addr;
  unsigned int addr;
  struct ifi  *ifi_next;	/* next of these structures */
  struct ifi  *ifi_list;  //fengwenchao add 20101223
  struct ifi  *ifi_last;   //fengwenchao add 20101223
  LISTEN_FLAG lic_flag;
};

struct wlanid{
	unsigned char wlanid;
	struct wlanid *next; 
	
};
typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char state;   
	unsigned int distance;
	
}Acktimeout;/*wcl add for RDIR-33*/
typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char WlanID;
	unsigned char Able;
	unsigned char subframe;  //zhangshu add, 2010-10-09
	unsigned int AmpduLimit;
	
}AmpduParameter;

/* zhangshu add for amsdu, 2010-10-09 */
typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char WlanID;
	unsigned char Able;
	unsigned char subframe;  //zhangshu add, 2010-10-09
	unsigned int AmsduLimit;
	
}AmsduParameter;

typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char WlanID;
	unsigned char Mixed_Greenfield;
	
}MixedGreenfieldParameter;
struct tag_wtpid{
	unsigned int wtpid;
	struct tag_wtpid *next; 
	
};
 struct tag_wtpid_list{

 	 int count;	 
	 struct tag_wtpid * wtpidlist;
} ;
typedef struct tag_wtpid_list update_wtp_list;


struct Radio_Wlan_Pair{
	unsigned char radioid;
	unsigned char wlanid;
};

struct interface_INFO{
	// wifi kernal module only support max count 15,this will change late when kernel support more weianying 2010/04/21
	char if_name[ETH_IF_NAME_LEN-1];
	unsigned char wlanID;
    int    BSSIndex;	
	unsigned int vrid;
	unsigned int isIPv6;
	unsigned int acip;
	unsigned int apip;
	unsigned char acipv6[IPv6_LEN];
	unsigned char apipv6[IPv6_LEN];
	unsigned int protect_type;
	unsigned short acport;
	unsigned short apport;
	unsigned char bssid[MAC_LEN];	
	unsigned char apmac[MAC_LEN];
	unsigned char acmac[MAC_LEN];
	unsigned char ifname[ETH_IF_NAME_LEN];	
	unsigned char WLANID;
	unsigned char wsmswitch;
	unsigned char if_policy;		/*0-local,1-tunnel,ht add 110314*/
	unsigned char Eap1XServerMac[MAC_LEN];	
	unsigned char Eap1XServerSwitch;
	unsigned char f802_3;//if 1---capwap+802.3,else 0---802.11
	unsigned short	vlanid;	
	unsigned char	vlanSwitch;
	unsigned char apname[DEFAULT_LEN];
	unsigned char essid[ESSID_LENGTH];
};
typedef struct interface_INFO IF_info;

struct interface_basic_INFO{
	char if_name[ETH_IF_NAME_LEN-1];
	unsigned char wlanID;
    int    BSSIndex;	
	unsigned int vrid;
};
typedef struct interface_basic_INFO if_basic_info;

#define PATCH_OP_RADIO_MAX 80
struct interface_batch_INFO{
	int count;
	if_basic_info ifinfo[PATCH_OP_RADIO_MAX];
};
typedef struct interface_batch_INFO if_batch_info;

typedef struct HANSI_INFO
{
	unsigned int instId;	
	unsigned char hpre_state;
	unsigned char hstate;
	unsigned char	vlanSwitch;	
	unsigned int asd_pid;	
	unsigned char dhcpoption82;
}HANSI_info;

/*added by weiay for rogue ap detection 2008/11/11*/
struct Neighbor_AP_ELE{
	unsigned char BSSID[MAC_LEN];
	unsigned short Rate; /*10 - 540 1080*/
	
	unsigned char Channel; /* 1 - 11	*/
	unsigned char RSSI;
	unsigned char NOISE;
	unsigned char BEACON_INT;
	
	unsigned char status; /*0 none 1 rogue ap 2 normal ap*/
	unsigned char opstatus; 
	unsigned short capabilityinfo;

	unsigned int wtpid;
	time_t fst_dtc_tm;
	time_t lst_dtc_tm;
	unsigned char encrp_type; /*/0 x 1 none 2 web 3shared*/
	unsigned char polcy; /* 0 no policy 1 have policy*/
	
	/*added more status*/

	/*fengwenchao add 20110401 for dot11RogueTable*/
	/*---------------------------------------------------------------------------*/
	unsigned int RogueAPAttackedStatus;  //是否对非法AP进行攻击，在黑名单中存在--1，不存在--0。
	unsigned int RogueAPToIgnore;        //是否忽略非法AP，在白名单中存在--1，不存在--0.
	/*---------------------------------------------------------------------------*/
	unsigned char ESSID[ESSID_DEFAULT_LEN+1];
	/*char *ESSID;*/
	char *IEs_INFO;
	struct Neighbor_AP_ELE *next;
	struct Neighbor_AP_ELE *neighborapInfos_list;  //fengwenchao add 20101219
	struct Neighbor_AP_ELE *neighborapInfos_last;  //fengwenchao add 20101219
}Neighbor_AP_ELE;



/*add for cpu average by nl 20100713*/
struct ap_cpu_info{
  unsigned int value;
  struct ap_cpu_info *next;
};

struct ap_snr_info{
  unsigned char value;
  struct ap_snr_info *next;
};
//qiuchen copy from v1.3
/*fengwenchao add 20120314 for onlinebug-162*/
struct wifi_snr_info{
 unsigned int snr_times; 	  
 unsigned char snr_max_value;
 unsigned char snr_min_value;
 unsigned char snr_average;
 double snr_math_average;
 struct ap_snr_info * ap_snr_info_head;
 unsigned int ap_snr_info_length;
};
/*fengwenchao add end*/
//qiuchen copy end


struct ap_cpu_mem_statistics{
  unsigned int cpu_value[10];
  unsigned int mem_value[10];
  unsigned int cpu_average;
  unsigned int cpu_times;
  unsigned int cpu_peak_value;
  unsigned int mem_average;
  unsigned int mem_times;
  unsigned int mem_peak_value;
  struct ap_cpu_info * ap_cpu_info_head;
  unsigned int ap_cpu_info_length;
  struct ap_cpu_info * ap_mem_info_head;
  unsigned int ap_mem_info_length;
  struct wifi_snr_info wifi_snr[L_RADIO_NUM];/*fengwenchao add 20120314 for onlinebug-162*///qiuchen copy from v1.3
  
  /*nl add for snr 2010-09-08*/
  unsigned int snr_times;		
  unsigned char snr_max_value;
  unsigned char snr_min_value;
  unsigned char snr_average;
  double snr_math_average;
  struct ap_snr_info * ap_snr_info_head;
  unsigned int ap_snr_info_length;
} ;

typedef struct ap_cpu_mem_statistics ap_cm_statistics;
typedef struct{
	unsigned char type;
	unsigned char ifindex;
	unsigned char state;/*0-not exist/1-up/2-down/3-error*/
	unsigned int eth_rate;/*10M,100M*/
	unsigned int eth_mtu;   /*fengwenchao add 20110126 for XJDEV-32 from 2.0*/
	time_t state_time;
}if_state_time;
struct ap_if_state_time{
  unsigned char report_switch;
  unsigned short report_interval;		//xiaodawei modify, 20101229
  unsigned char eth_num;
  unsigned char wifi_num;
  if_state_time eth[AP_ETH_IF_NUM];
  if_state_time wifi[AP_WIFI_IF_NUM];
} ;
typedef struct ap_if_state_time wid_ap_if_state_time;

struct tag_wids_set{
  unsigned char flooding;
  unsigned char sproof;
  unsigned char weakiv;
  unsigned char reserved; /*reserved*/
} ;
typedef struct tag_wids_set wid_wids_set;

struct tag_wids_statistics{
  unsigned int floodingcount;
  unsigned int sproofcount;
  unsigned int weakivcount;
} ;

typedef struct tag_wids_statistics wid_wids_statistics;

struct AP_VERSION{
	char *apmodel;
	char *versionname;
	char *versionpath;
	unsigned char radionum;
	unsigned char bssnum;
	struct AP_VERSION *next;
} ;

//typedef struct AP_VERSION AP_VERSION_ELE;
/*
typedef struct{
	int list_len;
	AP_VERSION_ELE *VERSION;
}AP_VERSION_INFO;
*/
/*
typedef struct{
	int list_len;
	AP_VERSION_ELE **VERSION_LIST;
}AP_VERSION_INFO;
*/
struct tag_wids_device_ele{
  unsigned char bssid[MAC_LEN]; /*attack device mac*/
  unsigned char vapbssid[MAC_LEN]; /* attack des*/
  unsigned char attacktype;
  unsigned char frametype;

  unsigned int attackcount;

  time_t fst_attack;
  time_t lst_attack;

  unsigned char channel;
  unsigned char rssi;
  unsigned char RogStaAttackStatus; //是否对可疑端站采取措施fengwenchao add 20110415 
  struct tag_wids_device_ele *next;
} ;
 struct tag_wids_device_info{

 	 int count;	 
	 struct tag_wids_device_ele * wids_device_info;
} ;


typedef struct tag_wids_device_info wid_wids_device;


struct sample_rate_info {
 unsigned char  time;
 unsigned int	past_uplink_throughput;
 unsigned int	current_uplink_throughput;
 unsigned int	past_downlink_throughput;
 unsigned int	current_downlink_throughput;
 unsigned int	uplink_rate;
 unsigned int	downlink_rate;
};
typedef struct sample_rate_info wid_sample_rate_info;

typedef struct{
	unsigned char monitorMode;
	unsigned int num;
	unsigned char scanningMode;
	unsigned int channel[SCANNING_CHANNEL_NUM];
} SCNANNING_MODE;

typedef struct{
	int neighborapInfosCount;
	int DeviceInterference;
	int wtp_online_num;  //fengwenchao add 20110402
	struct Neighbor_AP_ELE *neighborapInfos;
} Neighbor_AP_INFOS;

typedef struct{
	Neighbor_AP_INFOS 	 *rouge_ap_list;	
	wid_wids_device		 *wids_device_list;
}DCLI_AC_API_GROUP_TWO;
//fengwenchao  add 20101221
struct allwtp_neighborap{
	unsigned int wtpid;
	unsigned char WTPmac[6];
	unsigned char radio_num;
	struct allwtp_neighborap_radioinfo* radioinfo_head;
	struct allwtp_neighborap* next;
	struct allwtp_neighborap* allwtp_neighborap_list;
	struct allwtp_neighborap* allwtp_neighborap_last;
};
struct allwtp_neighborap_radioinfo{
	int wtpWirelessIfIndex;           // radio num 的一个计数
	int rouge_ap_count;              //临近AP的数量
	int failreason ;                 
	struct Neighbor_AP_ELE *neighborapInfos_head;
	struct allwtp_neighborap_radioinfo* next;
	struct allwtp_neighborap_radioinfo* radioinfo_list;
	struct allwtp_neighborap_radioinfo* radioinfo_last;	
};
//fengwenchao add  end
typedef struct
{	
	unsigned char opstate; /*0disable 1 enable*/
	unsigned char flag;
	unsigned short reportinterval;
	unsigned char countermeasures_mode;//0 ap 1 adhoc 2 all
	unsigned char countermeasures_switch;//0 close default 1 open
	unsigned short reserved; //reseved
	
} APScanningSetting;


#define 	UNKNOWN_MAC_TRAP_SWITCH_DISABLE		0
#define 	UNKNOWN_MAC_TRAP_SWITCH_ENABLE		1
#define 	UNKNOWN_MAC_TRAP_INTERVAL_DEFAULT	1800

#define 	AP_CONFIG_FILE_ERR_TRAP_SWITCH_DISABLE	0
#define 	AP_CONFIG_FILE_ERR_TRAP_SWITCH_ENABLE	1
#define 	AP_CONFIG_FILE_ERR_TRAP_SWITCH_INTERVAL_DEFAULT	1800

#define		STA_FLOW_OVERFLOW_TRAP_RX_SWITCH_DISABLE	0
#define		STA_FLOW_OVERFLOW_TRAP_RX_SWITCH_ENABLE	1
#define		STA_FLOW_OVERFLOW_RX_THRESHOLD			(2*1024*1024)

#define		STA_FLOW_OVERFLOW_TRAP_TX_SWITCH_DISABLE	0
#define		STA_FLOW_OVERFLOW_TRAP_TX_SWITCH_ENABLE	1
#define		STA_FLOW_OVERFLOW_TX_THRESHOLD			(1*1024*1024)


#define		ONLINE_STA_FULL_TRAP_SWITCH_DISABLE		0
#define		ONLINE_STA_FULL_TRAP_SWITCH_ENABLE		1
#define		ONLINE_STA_THREASHOLD					128

typedef struct {
	unsigned char unknown_mac_trap_switch;			//0 disable ; 1 enable
	unsigned int unknown_mac_trap_interval;			//ap report to ac interval
	unsigned char ap_config_file_err_trap_switch;	//0 disable; 1 enable
	unsigned int ap_config_file_err_trap_interval;	//ap report to ac interval
	unsigned char sta_flow_overflow_trap_switch;	//0 disable; 1 enable
	unsigned long sta_flow_overflow_rx_threshold;	//0 not limit; other value means limit value
	unsigned long sta_flow_overflow_tx_threshold;	//0 not limit; other value means limit value
	unsigned char online_sta_full_trap_switch;		//0 disable; 1 enable
	unsigned short online_sta_threshold;			//max allow count
}ap_trap_type_s;


struct  wlan_stats_info_profile{
	unsigned char type;  /*0-ath, 1-eth, 2-wifi*/  
	unsigned char ifname[16];
	unsigned char radioId;
	unsigned char wlanId;
	unsigned char mac[6];
	unsigned int rx_packets;
	unsigned int tx_packets;
	unsigned int rx_errors;   
 	unsigned int tx_errors; 
	unsigned int rx_drop;   
 	unsigned int tx_drop; 
	unsigned long long rx_bytes;
	unsigned long long tx_bytes; 
	unsigned int rx_rate;
	unsigned int tx_rate;
	unsigned int ast_rx_crcerr;   /*pei add 0220*/
	unsigned int ast_rx_badcrypt;   /*pei add 0220*/
	unsigned int ast_rx_badmic;   /*pei add 0220*/
	unsigned int ast_rx_phyerr;   /*pei add 0220*/

	unsigned int rx_frame;
	unsigned int tx_frame;	
	unsigned int rx_error_frame;
	unsigned int tx_error_frame;	
	unsigned int rx_drop_frame;
	unsigned int tx_drop_frame;	
	
	unsigned int rx_band;
	unsigned int tx_band;	
	unsigned long long rx_unicast;  // zhangshu modify rx_unicast 32bit to 64bit  2010-09-13
	unsigned long long tx_unicast;  // zhangshu modify tx_unicast 32bit to 64bit  2010-09-13
	unsigned int rx_multicast;
	unsigned int tx_multicast;
	unsigned int rx_broadcast;
	unsigned int tx_broadcast;
	unsigned int rx_pkt_unicast;
	unsigned int tx_pkt_unicast;
	unsigned int rx_pkt_multicast;
	unsigned int tx_pkt_multicast;
	unsigned int rx_pkt_broadcast;
	unsigned int tx_pkt_broadcast;
	unsigned int rx_pkt_retry;
	unsigned int tx_pkt_retry;
	unsigned int rx_pkt_data;
	unsigned int tx_pkt_data;
	unsigned int rx_retry;
	unsigned int tx_retry;

	unsigned int rx_pkt_mgmt;   // packets received of management          zhangshu modify 2010-09-13
	unsigned int tx_pkt_mgmt;	  // packets transtmitted of management    zhangshu modify 2010-09-13
	unsigned long long rx_mgmt;
	unsigned long long tx_mgmt;	

	unsigned long long rx_sum_bytes;  // zhangshu add,20100913  total number sent by interface
    unsigned long long tx_sum_bytes;  // zhangshu add,20100913  total number received by interface

	unsigned int rx_pkt_control;//zhangshu add for 1.3v,20100913
	unsigned int tx_pkt_control;//zhangshu add for 1.3v,20100913	
	unsigned int rx_errors_frames;   //zhangshu add for error frames, 2010-09-26
	unsigned int is_refuse_lowrssi; //fengwenchao add for chinamobile-177,20111122
	struct  wlan_stats_info_profile *next;
};

typedef struct wlan_stats_info_profile wlan_stats_info;

/* Huang Leilei copy from 1.3.18, 20130610 */
/* web manager stats from ap stats report */
typedef struct web_mng_ath_stats_s{
	unsigned int rx_pkt_unicast;
	unsigned int tx_pkt_unicast;
	unsigned int rx_pkt_multicast;
	unsigned int tx_pkt_multicast;
	unsigned int sub_rx_packets;
	unsigned int sub_tx_packets;
	unsigned int sub_tx_errors;
	unsigned int sub_rx_errors;
	unsigned int sub_tx_drops;
	unsigned int sub_rx_drops;
	unsigned long long sub_rx_bytes;
	unsigned long long sub_tx_bytes;
	unsigned int sub_ast_rx_crcerr;
	unsigned int sub_ast_rx_badcrypt;
	unsigned int sub_ast_rx_badmic;
	unsigned int sub_ast_rx_phyerr;
	unsigned int sub_rx_pkt_mgmt;
	unsigned int sub_tx_pkt_mgmt;
	unsigned long long sub_rx_mgmt;
	unsigned long long sub_tx_mgmt;
	unsigned long long sub_total_rx_bytes;
	unsigned long long sub_total_tx_bytes;
	unsigned long long sub_total_rx_pkt;
	unsigned long long sub_total_tx_pkt;
	unsigned int sub_tx_pkt_control;
	unsigned int sub_rx_pkt_control;
	unsigned int tx_pkt_signal;
	unsigned int rx_pkt_signal;
	unsigned int dwlink_retry_pkts;
	unsigned int stats_retry_frames;
	unsigned int rx_data_pkts;
	unsigned int tx_data_pkts;		
}web_mng_ath_stats_t;

typedef struct web_mng_eth_stats_s{
	unsigned int rx_pkt_broadcast;
	unsigned int rx_pkt_unicast;
	unsigned int tx_pkt_broadcast;
	unsigned int tx_pkt_unicast;
	unsigned int rx_pkt_multicast;
	unsigned int tx_pkt_multicast;
	unsigned int rx_packets;
	unsigned int tx_packets;
	unsigned int rx_errors;
	unsigned int tx_errors;
	unsigned long long rx_bytes;
	unsigned long long tx_bytes;
	unsigned int rx_drop;
	unsigned int tx_drop;
	unsigned long long rx_sum_bytes;
	unsigned long long tx_sum_bytes;
}web_mng_eth_stats_t;

typedef struct web_mng_wifi_stats_s{
	unsigned int wtp_rx_packets;
	unsigned int wtp_tx_packets;	
	unsigned long long wtp_rx_bytes;
	unsigned long long wtp_tx_bytes;
	unsigned int wtp_tx_errors;
	unsigned int wtp_ast_rx_crcerr;
	unsigned int wtp_ast_rx_badcrypt;
	unsigned int wtp_ast_rx_badmic;
	unsigned int wtp_ast_rx_phyerr;	
}web_mng_wifi_stats_t;


typedef struct web_manager_stats_s{
	web_mng_ath_stats_t ath_stats[L_RADIO_NUM];
	web_mng_eth_stats_t eth_stats[AP_ETH_IF_NUM];
	web_mng_wifi_stats_t wifi_stats;
	unsigned int sub_rx_packets_ath;
	unsigned int sub_tx_packets_ath;
	unsigned long long sub_rx_bytes_ath;
	unsigned long long sub_tx_bytes_ath;
	unsigned long long sub_total_rx_bytes_ath;
	unsigned long long sub_total_tx_bytes_ath;
	unsigned int sub_total_rx_pkt_ath;
	unsigned int sub_total_tx_pkt_ath;
	unsigned int rx_packets_eth;
	unsigned int tx_packets_eth;
	unsigned long long rx_bytes_eth;
	unsigned long long tx_bytes_eth;
	unsigned long long rx_sum_bytes_eth;
	unsigned long long tx_sum_bytes_eth;
}web_manager_stats_t; 
/* Huangleilei add end */

/*added end*/
typedef struct{
	unsigned int sta_count;
	unsigned char radioId;
	unsigned char wlanId;
	unsigned char mac[6];
	unsigned char mode;  /*11b-0x01,11a-0x02,11g-0x04,11n-0x08,*/
	unsigned char channel;
	unsigned char rssi;
	//unsigned short nRate;
	unsigned short tx_Rate;
	unsigned char isPowerSave;
	unsigned char isQos;
	unsigned int rx_bytes;
	unsigned int tx_bytes;

	/* zhangshu add for 1.3v 2010-09-14 */
	unsigned long long rx_data_bytes;        //add 64bit for rx_bytes
    unsigned long long tx_data_bytes;        //add 64bit for tx_bytes
    unsigned int rx_data_frames;             //add data frames from user to ap
    unsigned int tx_data_frames;             //add data frames from ap to user
    unsigned int rx_frames;                  //add total frames from user to ap
    unsigned int tx_frames;                  //add total frames from ap to user
    unsigned int rx_frag_packets;            //add frag packets from user to ap
    unsigned int tx_frag_packets;            //add frag packets from ap to user
    unsigned short rx_Rate;                  //receive rate

	//weichao add
	unsigned char sta_reason;
	unsigned short sub_reason;

	unsigned int MAXofRateset;	/* 终端与AP刚关联时根据双方能力而协商的无线速率集中的最高速率 */
	struct WID_WTP_STA_INFO wtp_sta_statistics_info;
} WIDStationInfo;

typedef struct {
	unsigned char longitude[LONGITUDE_LATITUDE_MAX_LEN];
	unsigned char latitude[LONGITUDE_LATITUDE_MAX_LEN];
	unsigned char power_mode;
	unsigned char manufacture_date[MANUFACTURE_DATA_MAX_LEN];
	unsigned char forward_mode;
	unsigned char radio_work_role[L_RADIO_NUM];
	unsigned char radio_count;
}CWWtpExtendinfo;


typedef struct {
	u_int32_t CMD;
	u_int32_t wlanCMD;
	u_int8_t  radiowlanid[L_RADIO_NUM][WLAN_NUM];
	u_int32_t setCMD;
	u_int32_t radioid[L_RADIO_NUM];
	int staCMD;
	char StaInf[8];
	int keyCMD;
	wAW_StaKey key;
}WID_CMD;

typedef struct {
	unsigned int STAOP;
	unsigned char STAMAC[WID_MAC_LEN];
	unsigned char WLANDomain;
}WID_STA;

typedef struct {
	char*	VlanName;
	u_int32_t	VID;
}WID_VLAN;

struct WID_TUNNEL_WLAN_VLAN {
  char		ifname[ETH_IF_NAME_LEN];	
  struct WID_TUNNEL_WLAN_VLAN  *ifnext;	
};

/*for mib*/
typedef struct {
	unsigned char	wlanid;
	unsigned char	l2_isolation_switch;
}WID_wtp_wlan_l2_isolation;
typedef struct {
	unsigned char	ifindex;
	unsigned char	snr_max;
	unsigned char	snr_min;
	unsigned char	snr_average;
	double 	snr_math_average;
	unsigned char	snr[10];
}WID_wtp_wifi_snr_stats;

typedef struct {
	WID_wtp_wlan_l2_isolation	wlan_l2isolation[L_BSS_NUM];
	unsigned char	dos_def_switch;
	unsigned char	igmp_snoop_switch;
}WID_mib_info;
typedef struct {
	unsigned int state;/*enable or not*/
	unsigned int tx_power;
}WID_oem_sector;
typedef struct {
	unsigned int state;/*enable or not*/
}WID_oem_tx_chainmask;

typedef struct {
	unsigned short supper_g_type;/*enable or not*/
	unsigned int supper_g_state;
}WID_oem_netgear_g;

typedef enum{
	DISABLE = 0,
	WDS_ANY = 1,
	WDS_SOME = 2
}WDS_STAT;

struct wds_bssid{
	unsigned char BSSID[MAC_LEN];
	struct wds_bssid *next;
};
struct wds_rbmac{
	unsigned char mac[MAC_LEN];
	unsigned char key[32];
	struct wds_rbmac *next;
};

/*fengwenchao copy from ht2.0 for requirements-407*/
struct tag_mcsid{
	unsigned int mcsid;
	struct tag_mcsid *next; 
	
};
 struct tag_mcsid_list{

 	 int count;	 
	 struct tag_mcsid * mcsidlist;
} ;

typedef struct tag_mcsid_list update_mcs_list;
/*fengwenchao add end*/
struct wlan_service_control{
	int TimerID;
	int TimerState;
	int wday[7];
	int times;
	int is_once;
};
typedef struct wlan_service_control WID_WSC;


typedef struct {
	u_int32_t	BSSIndex;/*Index of BSSID in AC*/
	u_int8_t	Radio_L_ID;
	u_int32_t	Radio_G_ID;
	u_int8_t	WlanID;
	u_int8_t	*BSSID;
	u_int8_t	VidIf_Bitmap;
	WID_VLAN	WVLAN;
	u_int8_t	Wlan_Index;
	u_int8_t	State;
	u_int8_t	keyindex;
	char	BSS_IF_NAME[ETH_IF_NAME_LEN];
	u_int8_t	BSS_IF_POLICY;
	u_int8_t	BSS_TUNNEL_POLICY;
	u_int8_t	ath_l2_isolation;
	u_int8_t	cwmmode;/*two states:0---20mode;1---20/40 mode*/
	char	nas_id[NAS_IDENTIFIER_NAME];
	u_int32_t	nas_id_len;
	unsigned int bss_accessed_sta_num;
	unsigned int bss_max_allowed_sta_num;
	unsigned int vlanid;	/*bss vlanid,first*/
	unsigned int wlan_vlanid;	/*wlan vlanid,second*/
	u_int8_t	band_width;
	u_int8_t	traffic_limit_able;/*disable-0 able-1*/
	u_int32_t	traffic_limit;/*bss traffic limit*/
	u_int32_t	average_rate;/*sta average*/
	u_int32_t	send_traffic_limit;/*bss send traffic limit*/
	u_int32_t	send_average_rate;/*sta send average*/
	u_int8_t	ip_mac_binding;/*disable-0 enable-1*/
	u_int32_t 	upcount;
	u_int32_t 	downcount;
	BSSStatistics	BSS_pkt_info;
	struct acl_config *acl_conf;		/*ht add 08.12.15*/
	WDS_STAT	WDSStat;
	unsigned char wds_mesh;
	struct wds_bssid *wds_bss_list;
	unsigned int wblwm;/*wds_bss_list wds 0,mesh 1*/
	unsigned int vMAC_STATE;
	unsigned int sta_static_arp_policy;
	char arp_ifname[ETH_IF_NAME_LEN];
	char	nas_port_id[32];				//mahz add 2011.5.25
	unsigned char limit_sta_rssi; /*fengwenchao add 20120222 for RDIR-25*/

	unsigned int noResToStaProReqSW;
	unsigned int unicast_sw;
	unsigned int muti_bro_cast_sw;
	unsigned int muti_rate;
	unsigned int wifi_sw;
	unsigned int  hotspot_id;
	unsigned char multi_user_optimize_switch;
	char master_to_disable;//qiuchen add it for AXSSZFI-1191
	unsigned char wsm_sta_info_reportswitch;
	unsigned short wsm_sta_info_reportinterval;
	unsigned char enable_wlan_flag;		/* wlan apply flag. Huangleilei add it for AXSSZFI-1622 */
}WID_BSS;


struct radio{
	u_int32_t	WTPID;
	u_int8_t	Radio_L_ID;/*Radio Local ID*/
	u_int32_t	Radio_G_ID;/*Radio Global ID*/
	u_int32_t	Radio_Type;/*a/b/g/n*/
	u_int32_t	supportRadio_Type;/*a/b/g/n*/
	u_int32_t	Radio_Type_Bank;
	/*u_int16_t   Radio_Rate;//11m/bps or 54m/bps*/
	u_int32_t	Support_Rate_Count;
	struct Support_Rate_List *Radio_Rate;/*sz*/
	int **RadioRate;
	u_int8_t	Radio_Chan;/*Channel*/
	u_int16_t	Radio_TXP;/*TX power*/
	u_int16_t	Radio_TXPOF;/*TX  power offset*/
	u_int16_t   FragThreshold;/*Max fragmation size*/
	u_int16_t	BeaconPeriod;/*Beacon interval*/
	u_int8_t	IsShortPreamble;/*short preamble is 1*/
	u_int8_t	DTIMPeriod;
	u_int8_t	ShortRetry;
	u_int8_t	LongRetry;
	u_int16_t	rtsthreshold;
	u_int8_t	AdStat;/*Admin State 2(disable)/1(enable)*/
	u_int8_t	OpStat;/*Operate State 2(disable)/1(enable)*/
	u_int32_t	CMD;/*command tag*/
	u_int32_t   bss_num;
	WID_BSS * BSS[L_BSS_NUM];
	int			QOSID;
	int			QOSstate;
	int 		Radio_country_code; /*wcl add for OSDEVTDPB-31*/
	u_int8_t bandwidth;
	u_int8_t ishighpower;
	u_int8_t	diversity;/*disable-0/able-1*/
	u_int8_t	txantenna;/*auto-0/main-1/vice-2*/
	u_int16_t	channelchangetime;
	char *excommand;
	u_int8_t    radio_countermeasures_flag; /*AP功率压制标志位,fengwenchao add 20110325*/
	u_int8_t 	isBinddingWlan;
	u_int8_t	BindingWlanCount;
	u_int8_t	auto_channel;
	u_int8_t	auto_channel_cont;
	u_int8_t    channelsendtimes;
	u_int8_t	txpowerautostate;
	u_int8_t	wifi_state;
	u_int32_t 	upcount;
	u_int32_t 	downcount;
	u_int32_t 	rx_data_deadtime;
	/*11 n Parameters start*/
	u_int16_t guardinterval;
	u_int16_t mcs;
	u_int16_t cwmode;
	Acktimeout ack;/*wcl add for RDIR-33*/
	AmpduParameter	Ampdu;
	AmsduParameter	Amsdu;  //zhangshu add for a-msdu, 2010-10-09
	MixedGreenfieldParameter	MixedGreenfield;
	unsigned char  chainmask_num;  // 1-3  to indicate the number of radio chainmask
	unsigned char  tx_chainmask_state_value;/*tx chainmask state.such as 0x0011*/
	unsigned char  rx_chainmask_state_value;/*rx chainmask state.such as 0x0011, zhangshu add 2010-10-09 */
	WID_oem_tx_chainmask	*tx_chainmask[TX_CHANIMASK_NUM];
	char channel_offset;
	
	/*11 n Parameters end*/
/*A8 start*/
	WID_oem_sector	*sector[SECTOR_NUM];
	unsigned short	sector_state_value;/*sector state.such as 0x0011.here only sector0 and sector1 is enable*/
	WID_oem_netgear_g supper_g;
	unsigned int REFlag;/*RadioExternFlag*/
	int distance; /*zhanglei add for A8*/
	unsigned int cipherType;/*0 disable  1 wep 2 aes*/
	char wepkey[32];
	int rbmacNum;
	struct wds_rbmac *rbmac_list;
	unsigned char inter_vap_able;
	unsigned char intra_vap_able;
	unsigned int keep_alive_period;
	unsigned int keep_alive_idle_time;
	unsigned char congestion_avoidance;
/*A8 end*/	
	struct wlanid	*Wlan_Id; /*binding wlan id*/
	unsigned char	*WlanId; /*binding wlan id*/
	unsigned short     txpowerstep;//zhaoruijia,20100917,add radio txpower step
	char br_ifname[WLAN_NUM][IF_NAME_MAX];
	u_int32_t	wep_flag[WTP_WEP_NUM];/*which bss binding a wlan use wep*/
	u_int8_t	max_bss_count;
	WID_WSC	StartService;
	WID_WSC	StopService;
	struct radio *next;
	unsigned char radio_disable_flag;  /*fengwenchao add 20110920 for radio disable config save flag*/
	char mcs_list[L_BSS_NUM];/*fengwenchao add 20120314 for requirements-407*/
	int mcs_count;/*fengwenchao add 20120314 for requirements-407*/
	u_int8_t	radio_work_role;
	unsigned char radio_channel_use_rate;
	unsigned int radio_channel_change_counter;
	unsigned int radio_channel_width;
	int radio_noise;
};
typedef struct radio WID_WTP_RADIO;
struct wtp_extend {
	
	unsigned char type;
	unsigned char op;
	unsigned char radioId;
	unsigned char wlanid;/*binding wlan id*/
	unsigned char state;/*switch state*/
	u_int8_t	Radio_L_ID;/*Radio Local ID*/
	//unsigned int reseved;
};
typedef struct wtp_extend WID_WTP_EXTEND;

/* struct of wid trap */
/* zhangshu append 20100824 */
struct wid_trap{
	unsigned int ignore_percent;
	u_int8_t ignore_switch;
	unsigned int sta_addr_redirect;
	unsigned char rogue_terminal_trap_flag;
};

/* zhangshu add for Terminal Disturb Info Report, 2010-10-08 */
typedef struct terminalDisturbInfo{
    unsigned char reportswitch;   // 0-off, 1-on
	unsigned short reportpkt;
	unsigned short sta_trap_count;
} terminal_disturb_info;


/*fengwenchao add 20111117 for GM-3*/
struct heart_time_value_head
{
	unsigned int heart_time_value;
	struct heart_time_value_head *next;
};

struct heart_time
{	
	unsigned char heart_statistics_switch;
	unsigned int heart_statistics_collect;
	unsigned int heart_time_delay;
	unsigned int heart_transfer_pkt;
	unsigned int heart_lose_pkt;
	unsigned int heart_time_value_length;
	unsigned int heart_time_avarge;
	struct heart_time_value_head *heart_time_value_head;
};
/*fengwenchao add end*/


struct wtp{
	u_int32_t	WTPID;
	char* 	WTPSN;/*WTP Serial No*/
	char*	WTPNAME;
	char*	WTPModel;/*model of wtp which determin the number of Radios and rates supported in WTP*/
	char*	APCode;/*   inner develop code*/
	u_int8_t	RadioCount;
	u_int8_t	ViaL3IF;/*interface that Data of WTP come from*/
	char*	WTPIP;
	u_int8_t*	WTPMAC;
	u_int8_t	WTPStat;/*wtp state (online(1) /offline(0))*/
	u_int8_t	BindL3IF;/*force interface that Data of WTP come from*/
	u_int32_t	WFR_Index;/*First Radio Global Index*/
	u_int8_t	CTR_ID;/*CAPWAP Control Tunnel ID*/
	u_int8_t	DAT_ID;/*CAPWAP Data Tunnel ID*/
	u_int8_t	Reboot_s;/*?????*/
	u_int8_t 	isused;
	u_int8_t 	quitreason;/*quit reason*/
	u_int8_t  unused_flag;
	/*u_int8_t 	isBinddingWlan;*/
	/*u_int8_t	BindingWlanCount;*/
	int		tunnel_mode;
	int 	wtp_login_mode; /*static 0;dynamic 1*/
	unsigned char apply_wlan_num;
	unsigned char apply_wlanid[WLAN_NUM];/*binding wlan id store apply wlan id*/
	struct wlanid	*Wlan_Id; /*binding wlan id*/
	u_int32_t BindingSock;/*special sock binding to this WTP added by weiay 20080530*/
	u_int32_t BindingSystemIndex;
	char BindingIFName[ETH_IF_NAME_LEN];/*20080710*/
	char nas_id[NAS_IDENTIFIER_NAME];/*zhanglei add*/
	unsigned char radio_num;
	WID_WTP_RADIO * WTP_Radio[L_RADIO_NUM];
	WID_CMD		*CMD;
	Neighbor_AP_INFOS *NeighborAPInfos;/*added by weiay 2008/11/11*/
	Neighbor_AP_INFOS *NeighborAPInfos2;
	Neighbor_AP_INFOS *rouge_ap_infos;
	unsigned int  wtp_allowed_max_sta_num;/*xm add 08/12/04*/
	unsigned int  wtp_triger_num;/*xm add 08/12/04*/
	unsigned int rx_echocount;
	wlan_stats_info apstatsinfo[TOTAL_AP_IF_NUM]; /*ath 4 wifi 2 eth 2 total num 8*/
	pthread_mutex_t mutex_web_report;				/* Huangleilei copy from 1.3.18, 20130610 */
	web_manager_stats_t web_manager_stats;		/* Huangleilei copy from 1.3.18, 20130610 */
	web_manager_stats_t pre_web_manager_stats;	/* Huangleilei copy from 1.3.18, 20130610 */
	unsigned long long rx_bytes;/*total rx byte for this ap*/
	unsigned long long tx_bytes;/*total tx byte for this ap*/
	unsigned long long rx_bytes_before;/*tmp total rx byte for this ap*/
	unsigned long long tx_bytes_before;/*tmp total tx byte for this ap*/
	unsigned int  wtp_flow_triger;/*xm add 09/02/05*/
	unsigned int ap_ipadd;
	unsigned int ap_mask_new;
	unsigned int ap_gateway;
	unsigned int ap_dnsfirst;
	unsigned int ap_dnssecend;
	unsigned int img_now_state;
	u_int8_t resetflag;
	u_int8_t ap_mask;
	u_int8_t	ap_sta_report_switch;
	u_int16_t	ap_sta_report_interval;
	unsigned char moment_infor_report_switch;
	char *sysver;
	char *ver;
	char *ApReportVer;  /*fengwenchao add 20110216 for ap updata successful or fail*/
	char *codever;/*used to recognize the version of oem production,so we can config the txantenna*/
	char *location;
	char *netid;
	unsigned char wifi_extension_reportswitch;
	unsigned short wifi_extension_reportinterval;
	unsigned char sta_deauth_message_reportswitch;
	unsigned char sta_flow_information_reportswitch;
	int collect_time;/*for cpu average computing by nl*/
	wid_wifi_info	wifi_extension_info;
	wid_sample_rate_info	wid_sample_throughput;
	WID_mib_info	mib_info;
	ap_cm_statistics	apcminfo;
	wid_ap_if_state_time	apifinfo;
	wid_wids_statistics wids_statist;
	wid_wids_device *wids_device_list;
	time_t	*add_time;
	time_t	*quit_time;
	time_t	imagedata_time;
	time_t	config_update_time;
	time_t	manual_update_time;
	u_int8_t	updateStat; //0waitupdate//1update 2updatesuccess
	u_int8_t updatefailcount;//0noaccess//1overmaxcount 2pkterror
	u_int8_t	updatefailstate; //0waitupdate//1update 2updatesuccess
	u_int32_t	ElectrifyRegisterCircle;
	/*u_int32_t	wep_flag[WTP_WEP_NUM];*//*which bss binding a wlan use wep*/
	WID_wtp_wifi_snr_stats wtp_wifi_snr_stats;
	struct msgqlist *ControlList;
	struct msgqlist *ControlWait;
	unsigned int rate;
	unsigned int old_bytes;
	char*	updateversion;
	char*	updatepath;
	char *apply_interface_name;
	char* option60_param;//to ap
	char sendsysstart;
	char isipv6addr;
	unsigned int wtp_trap_switch;
	unsigned int wtp_seqnum_switch; /*wcl add*/
	unsigned int wtp_trap_lev;
	unsigned int wtp_cpu_use_threshold;
	unsigned int wtp_mem_use_threshold;
	unsigned int wtp_rogue_ap_threshold;
	unsigned int wtp_rogue_terminal_threshold;
	u_int8_t	ap_sta_wapi_report_switch;
	unsigned int ap_sta_wapi_report_interval;
	unsigned int trap_collect_time;
	unsigned int cputrap_resend_times;
	unsigned int memtrap_resend_times;
	unsigned int rogueaptrap_resend_times;
	unsigned int rogueteminaltap_resend_times;
	unsigned int cpu_resend_times;
	unsigned int cpu_clear_resend_times;
	unsigned int memtrap_clear_resend_times;
	unsigned int ap_temp_resend_times;
	unsigned int ap_temp_clear_resend_times;
	int EchoTimer;
	u_int8_t	tpc_policy;
	struct wtp *next;
	//WID_WTP_EXTEND	WtpExtend;
	int dhcp_snooping;
	int sta_ip_report;
	unsigned char cpuType[WTP_TYPE_DEFAULT_LEN];
	unsigned char flashType[WTP_TYPE_DEFAULT_LEN];
	unsigned char memType[WTP_TYPE_DEFAULT_LEN];
	unsigned int flashSize;
	unsigned int memSize;
	int neighbordeatimes;
	char login_interfaceIP[128];	//xiaodawei transplant from 2.0 for telecom test, 20100301
	unsigned int apstatisticsinterval;
	unsigned int APGroupID;
  /*zhaoruijia,tranlate  neighbor_channel_interference to 1.3,start*/
	unsigned char neighborchannel_trap_flag;
  	unsigned char channel_device_interference_flag; /*fengwenchao add 20110221 for wid_dbus_trap_wtp_channel_device_interference */
	unsigned char ap_rogue_threshold_flag;          /*fengwenchao add 20110221 for wid_dbus_trap_ap_rogue_threshold */
	unsigned char ac_discovery_danger_ap_flag;      /*fengwenchao add 20110221 for wid_dbus_trap_wtp_ac_discovery_danger_ap*/ 
	unsigned char find_wids_attack_flag;            /*fengwenchao add 20110221 for wid_dbus_trap_wtp_find_wids_attack */
	unsigned char channel_count_minor_flag;         /*fengwenchao add 20110221 for wid_dbus_trap_wtp_channel_count_minor*/
	unsigned char samechannel_trap_flag;
	int neighborchannelrssithold;
	int samechannelrssithold;
  /*zhaoruijia,tranlate  neighbor_channel_interference to 1.3,end*/
    unsigned int ntp_interval;     /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,start*/
	unsigned char ntp_state;
    unsigned char ntp_trap_flag;  /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,end*/
	SCNANNING_MODE WIDS;
	struct wid_trap wid_trap;   /* zhangshu append 20100824 */
	int apcodeflag;
	unsigned int elem_num;
	/* zhangshu add for Terminal Disturb Info Report, 2010-10-08 */
	terminal_disturb_info ter_dis_info;
	struct heart_time heart_time;   /*fengwenchao add 20111117 for GM-3*/
	unsigned int radio_5g_sw;
	char master_to_disable;//qiuchen add it

	unsigned char longitude[LONGITUDE_LATITUDE_MAX_LEN];
	unsigned char latitude[LONGITUDE_LATITUDE_MAX_LEN];
	unsigned char power_mode;
	unsigned char manufacture_date[MANUFACTURE_DATA_MAX_LEN];
	unsigned char forward_mode;
	unsigned char radio_work_role;
		
	unsigned char unauthorized_mac_reportswitch;
	unsigned short unauthorized_mac_reportinterval;

	unsigned char wtp_configure_error_reportswitch;
	unsigned short wtp_configure_error_reportinterval;

	unsigned char sta_flow_overflow_rx_reportswitch;
	unsigned short sta_flow_overflow_rx_reportinterval;
	unsigned int sta_flow_overflow_rx_threshold;	//kB
	
	unsigned char sta_flow_overflow_tx_reportswitch;
	unsigned short sta_flow_overflow_tx_reportinterval;
	unsigned int sta_flow_overflow_tx_threshold;	//kB

	unsigned char sta_online_full_reportswitch;
	unsigned short sta_online_full_reportinterval;
};
typedef struct wtp WID_WTP;


/*xm add for balence sta*/
struct bss_s{
	unsigned int bss_index;
	unsigned int arrival_num;

	/*unsigned int sta_num;
	unsigned int rd_flow;   xm add 09/02/06*/
	
	struct bss_s *next;
};

typedef struct bss_s bss_arrival_num;


struct log_node_s
{	
	unsigned char mac[6]; 
	
	bss_arrival_num *from_bss_list;
	unsigned int list_len;
	
	struct log_node_s *next;
	
};
typedef struct log_node_s log_node;

struct PMK_BSSInfo{
	unsigned int BSSIndex;
	struct PMK_BSSInfo *next;
};

struct PMK_STAINFO{
	struct PMK_STAINFO *next;
	struct PMK_STAINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int PreBssIndex;
	unsigned char *BSSIndex;
	struct PMK_BSSInfo *bss;
	unsigned int BssNum;
	unsigned int idhi;
	unsigned int idlo;	
};

struct ROAMING_STAINFO{
	struct ROAMING_STAINFO *next;
	struct ROAMING_STAINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int BssIndex;
	unsigned int PreBssIndex;
	unsigned char BSSID[MAC_LEN];
	unsigned char PreBSSID[MAC_LEN];
	unsigned int need_notice_wifi;
};

struct PreAuth_BSSINFO{
	struct PreAuth_BSSINFO *next;
	struct PreAuth_BSSINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int BSSIndex;
};
#if 0
struct wlan_service_control{
	int TimerID;
	int TimerState;
	int wday[7];
	int times;
	int is_once;
};
typedef struct wlan_service_control WID_WSC;
#endif
struct a_sta_info{
	struct a_sta_info *next;
	struct a_sta_info *hnext;
	unsigned char addr[MAC_LEN];
};
struct wlan{
	char*	WlanName;
	u_int8_t	WlanID;
	char* 	ESSID;
	unsigned char* 	ESSID_CN_STR;
	u_int8_t	chinaEssid;
	u_int8_t	AAW;/*aute add wtp 0(disable)/1(default enable)*/
	u_int32_t	S_WTP_BSS_List[WTP_MAX_MAX_NUM][L_RADIO_NUM];/*Static WTP BSS List*/
	u_int32_t	D_WTPADD_List[WID_DEFAULT_NUM];/*Dynamic WTP ID List*/
	u_int8_t	L3_IF_Index[WID_MAXN_INDEX];
	u_int8_t	Status;/*WLAN Status 1(default disable)/0(enable)*/
	u_int8_t	VidIf_Bitmap;
	u_int16_t   flow_check;
	u_int32_t   no_flow_time;
	u_int32_t   limit_flow;
	WID_VLAN	WVLAN;
	u_int8_t	Wlan_Index;
	unsigned char ifcount;
	struct ifi * Wlan_Ifi;
	WID_WTP * WLAN_WTP;
	u_int32_t	CMD;
	unsigned char	SecurityID;
	u_int8_t	HideESSid; /* 1  hide  0 not hode*/
	u_int8_t 	wlan_if_policy;
	char WlanL3IFName[ETH_IF_NAME_LEN];
	char	WlanKey[DEFAULT_LEN];
	unsigned int KeyLen;
	unsigned int SecurityType;
	unsigned int EncryptionType;
	unsigned char SecurityIndex;
	unsigned char OldSecurityIndex ;       /*fengwenchao add 20110310 记录绑新的安全策略之前的安全策略*/
	unsigned char NowSecurityIndex_flag;   /*fengwenchao add 20110310 记录当前的安全策略是否为(open||shared)&&(wep),1-->YES,2-->NO*/
	unsigned char OldSecurityIndex_flag;   /*fengwenchao add 20110310 记录先前的安全策略是否为(open||shared)&&(wep),1-->YES,2-->NO*/
	unsigned char asic_hex;/* 0 asic; 1 hex*/
	char	AsIp[DEFAULT_LEN];
	unsigned int IpLen;
	char	ASCerPath[DEFAULT_LEN];
	unsigned int ASCerLen;
	char	AECerPath[DEFAULT_LEN];
	unsigned int AECerLen;
	unsigned int vlanid;
	unsigned char wlan_1p_priority;/*0-7;0 means no priority*/
	struct WID_TUNNEL_WLAN_VLAN *tunnel_wlan_vlan;
	struct wlan *next;

	char	nas_port_id[NAS_PORT_ID_LEN];				//mahz add 2011.5.25
	unsigned int wlan_accessed_sta_num;
 	unsigned int wlan_max_allowed_sta_num;      /*xm add  08/12/01*/
	unsigned int sta_list_len;
	log_node  *sta_from_which_bss_list;
	unsigned int balance_para;
	unsigned int flow_balance_para;
	unsigned char extern_balance;				/*xm0814*/
	unsigned char flow_compute;					/*xm0714*/
	unsigned char balance_switch;            /*remember init them*/
	unsigned char balance_method;            /*remember init them*/
	unsigned char Roaming_Policy;			/*Roaming (1 enable /0 disable)*/
	unsigned char isolation_policy;				/* (1 enable /0 disable)*/
	unsigned char multicast_isolation_policy;	/* (1 enable /0 disable)*/
	unsigned char sameportswitch;		/* (1 enable /0 disable)*/
	unsigned char bridge_ucast_solicit_stat;	/* (1 enable / 0 disable) */		/* Huang Leilei add 2012-11-13 */
	unsigned char bridge_mcast_solicit_stat;	/* (1 enable / 0 disable) */		/* Huang Leilei add 2012-11-13 */
	struct acl_config *acl_conf;				/*ht add 08.12.15*/
	
	unsigned int wlan_send_traffic_limit;/*nl add 20100318*/
	unsigned int wlan_traffic_limit;	/*nl add 20100318*/

	unsigned int wlan_station_average_send_traffic_limit;/*nl add 20100401*/
	unsigned int wlan_station_average_traffic_limit;	/*nl add 20100401*/
	
	struct PMK_STAINFO *sta_list;		
	struct PMK_STAINFO *sta_hash[256];	
	unsigned int num_sta;	
	struct PreAuth_BSSINFO *bss_list;		
	struct PreAuth_BSSINFO *bss_hash[256];	
	unsigned int num_bss;
	unsigned int PreAuth;	
	struct ROAMING_STAINFO *r_sta_list;		
	struct ROAMING_STAINFO *r_sta_hash[256];	
	unsigned int r_num_sta;	
	struct a_sta_info *a_sta_list;
	struct a_sta_info *a_sta_hash[256];
	unsigned int a_num_sta;				//sta accessed num
	WDS_STAT	WDSStat;
	unsigned int wds_mesh;
	unsigned char AC_Roaming_Policy;
	unsigned char group_id;
	unsigned int sta_ip_mac_bind;
	wid_wifi_info	wifi_extension_info;
	WID_WSC	StartService;
	WID_WSC	StopService;
	unsigned char bkid[16];

	/* zhangshu add for set eap mac ,2010-10-22 */
	unsigned char eap_mac_switch;
	unsigned char *eap_mac;	
	unsigned char eap_mac2[MAC_LEN];
	/*zhanglei add for uplink detect*/	
	unsigned char uplinkState;
	unsigned char uplinkWlanId;
	char * uplink_addr;
	unsigned int wlan_noResToStaProReqSW;
	unsigned int wlan_unicast_sw;
	unsigned int wlan_muti_bro_cast_sw;
	unsigned int wlan_muti_rate;
	unsigned int wlan_wifi_sw;
	unsigned int bss_allow_max_sta_num; //fengwenchao add 20120323
	unsigned char wlan_ath_l2_isolation; //fengwenchao add 20120323
	unsigned int wlan_sta_static_arp_policy; //fengwenchao add 20120323
	char wlan_arp_ifname[ETH_IF_NAME_LEN];//fengwenchao add 20120323
	unsigned char wlan_limit_sta_rssi;//fengwenchao add 20120323
	unsigned int ap_max_inactivity; 	   //weichao add
	unsigned int  hotspot_id;	
	u_int8_t	WLAN_TUNNEL_POLICY;
	unsigned char multi_user_optimize_switch ;
	unsigned char want_to_delete;		/* now, AC will begin to delete wlan,but it will wait until all the state of wlan of wtp used was disable. Huangleilei add it for AXSSZFI-1622 */
	struct radius_client_data *radius;
	struct asd_radius_servers *radius_server;
	unsigned int sta_roaming_times;
	unsigned int sta_roaming_suc_times;
};
typedef struct wlan WID_WLAN;


struct AUTOAPINFO
{
	unsigned char*		mac;
	unsigned char*		model;
	unsigned char*		realmodel;
	unsigned char*		sn;
	unsigned int		oemoption;
	unsigned int		apcodeflag;
};
typedef struct AUTOAPINFO WIDAUTOAPINFO;




typedef enum {
	WID_WPA_CIPHER_NONE = 1,
	WID_WPA_CIPHER_WEP40 = 2,
	WID_WPA_CIPHER_WEP104 = 4,
	WID_WPA_CIPHER_WEP128 = 8,
	WID_WPA_CIPHER_TKIP = 16,
	WID_WPA_CIPHER_CCMP = 32,
	WID_WPA_CIPHER_AES_128_CMAC = 64,
	WID_WAPI_CIPHER_SMS4 = 128
}wid_cipher;

typedef enum {
	WID_WPA_KEY_MGMT_IEEE8021X = 1,
	WID_WPA_KEY_MGMT_PSK = 2,
	WID_WPA_KEY_MGMT_NONE = 4,
	WID_WPA_KEY_MGMT_IEEE8021X_NO_WPA = 8,
	WID_WPA_KEY_MGMT_WPA_NONE = 16,
	WID_WPA_KEY_MGMT_FT_IEEE8021X = 32,
	WID_WPA_KEY_MGMT_FT_PSK = 64,
	WID_WPA_KEY_MGMT_SHARED = 128,	
	WID_WPA2_KEY_MGMT_IEEE8021X = 256,
	WID_WPA2_KEY_MGMT_PSK = 512,
	WID_WPA2_KEY_MGMT_FT_IEEE8021X = 1024,
	WID_WPA2_KEY_MGMT_FT_PSK = 2048,
	WID_WAPI_KEY_MGMT_PSK = 4096, 
	WID_WAPI_KEY_MGMT_CER = 8192
}wid_wpa_key_mgmt;

typedef enum{
	WID_WPA_PROTO_WPA = 1,
	WID_WPA_PROTO_RSN = 2
}wid_wpa_proto;

typedef enum{
	NO_INTERFACE = 0,
	WLAN_INTERFACE = 1,
	BSS_INTERFACE = 2
}wid_if_policy;

#if 0
typedef struct {	
	unsigned char elem_id;
	unsigned char len;
	unsigned char oui[3];
	unsigned char oui_type;
	unsigned char version[2]; /* little endian */
	int proto;
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int capabilities;
	unsigned int num_pmkid;
	const unsigned char *pmkid;
	int mgmt_group_cipher;
}WPA_IE;
#endif

typedef struct {
	u_int8_t	WlanID;
	u_int8_t	WTPID;
	u_int8_t	Radio_G_ID;
	u_int8_t	BSSIndex;
	u_int8_t	*mac;
}ASD_STA;


typedef struct{
	struct sockaddr_un addr;
	int 	addrlen;
}unixAddr;
typedef struct{
	struct sockaddr_tipc addr;
	int addrlen;
}unixTipc;

struct white_mac{
	unsigned char elem_mac[6];
	struct white_mac *next;
};

typedef struct{
	int imaccount;
	struct white_mac *list_mac;
}white_mac_list;


/*xm add////////////////*/
struct oui_node{
	unsigned char oui[3];
	struct oui_node *next;
};


typedef struct{
	unsigned int list_len;
	struct oui_node *oui_list;
}OUI_LIST_S;


struct essid_node{
	char *essid;
	unsigned int len;
	struct essid_node *next;
};


typedef struct{
	unsigned int list_len;
	struct essid_node *essid_list;
}ESSID_LIST_S;


struct attack_mac_node{
	unsigned char mac[6];
	struct attack_mac_node *next;
};


typedef struct{
	unsigned int  list_len;
	struct attack_mac_node*attack_mac_list;
}ATTACK_MAC_LIST_S;

typedef struct{	
	ATTACK_MAC_LIST_S	*dcli_attack_mac_list;
	ESSID_LIST_S		*dcli_essid_list;
	OUI_LIST_S		 *dcli_oui_list;
	
}DCLI_AC_API_GROUP_ONE;

typedef struct{
	unsigned int WTPID;
	unsigned char flags;
	unsigned char channel;
	unsigned short txpower;	
	unsigned char H_channel_list[4];	
	unsigned char N_channel_list[4];
	unsigned int WTPID_List[4][2];
}WTP_RRM_INFO;

typedef struct{
	unsigned int wtpid;
	unsigned char neighbor_rssi[4];
	unsigned char txpower;
	unsigned char pre_txpower;
	unsigned char wtp_cnt;
}transmit_power_control;

extern WID_WLAN		*AC_WLAN[WLAN_NUM];
extern WID_WTP		**AC_WTP;
extern WID_WTP_RADIO	**AC_RADIO;
extern WID_BSS		**AC_BSS;
extern WID_WLAN		*ASD_WLAN[WLAN_NUM];
extern WID_WTP		**ASD_WTP;
extern WID_WTP_RADIO	**ASD_RADIO;
extern WID_BSS		**ASD_BSS;
extern WID_ACCESS	*AC_WTP_ACC;
extern Neighbor_AP_INFOS *gRogueAPInfos;/*added by weiay 2008/11/11*/
extern APScanningSetting gapscanset;
extern int wids_judge_policy;
extern white_mac_list *pwhite_mac_list;
extern white_mac_list *pblack_mac_list;
extern struct sta_static_info *STA_STATIC_TABLE[STA_HASH_SIZE];
extern unsigned int	sta_static_num;

extern int TableSock;
extern int TableSend;
extern int DataSend;
extern int DataSock;		//mahz add 2011.10.17
extern int TipcSend;
#ifdef ASD_MULTI_THREAD_MODE
extern int 		LoopSend;
extern unixAddr ASD_LOOP;
#endif

extern unixAddr toWSM;
extern unixAddr toWID;
extern unixAddr toEAG;
extern unixAddr toDHCP;			//WEICHAO  add 2011.11.11
extern unixTipc toFASTFWD;
extern unsigned char wirelessdata_switch;
extern unsigned char wireddata_switch;
extern unsigned char apstatistics;
extern unsigned int apstatisticsinterval;
extern wid_wids_set gwids;
extern unsigned char gdhcp_flooding_status;
extern unsigned char gwidsinterval;
extern unsigned char gprobethreshold;
extern unsigned char gotherthreshold;
extern unsigned int glasttimeinblack;
extern update_wtp_list *updatewtplist;
extern update_wtp_list *updatefailwtplist;
extern unsigned int checkwtpcount;
extern int gtrapflag;
extern int gtrap_ap_run_quit_trap_switch;
extern int gtrap_ap_cpu_trap_switch;
extern int gtrap_ap_mem_trap_switch;
extern int gtrap_rrm_change_trap_switch;
extern int gtrap_flash_write_fail_trap_switch;

extern int gtrap_channel_device_ap_switch;/*zhaoruijia,translate  neighbor_channel_interference to 1.3,start*/

extern int gtrap_channel_device_interference_switch;
extern int gtrap_channel_terminal_interference_switch;
extern int gtrap_rogue_ap_threshold_switch;
extern int gtrap_wireless_interface_down_switch;
extern int gtrap_channel_count_minor_switch;
extern int gtrap_channel_change_switch;

extern int gtrap_ap_run_quit_switch;
extern unsigned char updatemaxfailcount;
extern unsigned char aphotreboot;
extern unsigned char gwidspolicy;
extern wid_wids_device *wids_ignore_list;
extern unsigned char gessidfilterflag;
extern unsigned char gmacfilterflag;
extern unsigned char sta_deauth_message_reportswitch;
extern unsigned char sta_flow_information_reportswitch;
extern unsigned char g_radio_5g_sw;
extern OUI_LIST_S 			g_oui_list;
extern ESSID_LIST_S 		g_essid_list;
extern ATTACK_MAC_LIST_S 	g_attack_mac_list;  /*xm add */
/*fengwenchao add 20120117 for onlinebug-96*/
extern int gwtpstate_mb;
extern int gwtpstate_mb_timer;
/*fengwenchao add end*/


/*country code area*/
enum country_code_result{
	COUNTRY_CHINA_CN,/* 0*/
	COUNTRY_EUROPE_EU,/* 1*/
	COUNTRY_USA_US,/* 2*/
	COUNTRY_JAPAN_JP,/* 3*/
	COUNTRY_FRANCE_FR,/* 4*/
	COUNTRY_SPAIN_ES,/* 5*/
	COUNTRY_CODE_SUCCESS,
	COUNTRY_CODE_NO_CHANGE,
	COUNTRY_CODE_ERROR_SMALL_LETTERS,
	COUNTRY_CODE_ERROR
};
enum channel_cwmode{
	CHANNEL_CWMODE_SUCCESS,/* 0*/
	CHANNEL_CWMODE_HT40,/* 1*/
	CHANNEL_CWMODE_HT20,
	CHANNEL_CWMODE_ERROR,
	CHANNEL_CROSS_THE_BORDER      /*fengwenchao add 20110323  信道越界*/
};
enum dcli_sn{
	FIRST = 1,
	SECOND,
	THIRD,
	FOURTH,
	FIFTH,
	SIXTH,
	SEVENTH,
	EIGHTH,
	NINTH,
	TENTH,
	ELEVENTH,
	TWELFTH,
	THIRTEENTH,
	FOURTEENTH,
	FIFTEENTH,
	SIXTEENTH
};

/*qos area*/
#define WID_QOS_ARITHMETIC_NAME_LEN 20


struct qos {
 u_int8_t	QueueDepth;
 u_int16_t	CWMin;
 u_int16_t	CWMax;
 u_int8_t	AIFS;
 u_int16_t	TXOPlimit;
 u_int8_t	Dot1PTag;
 u_int8_t	DSCPTag;
 u_int8_t	ACK;
 u_int8_t	mapstate;
 u_int8_t	wmm_map_dot1p;
 u_int8_t	dot1p_map_wmm_num;
 u_int8_t	dot1p_map_wmm[8];
 u_int8_t	qos_average_rate;
 u_int8_t	qos_max_degree;
 u_int8_t	qos_policy_pri;
 u_int8_t	qos_res_shove_pri;
 u_int8_t	qos_res_grab_pri;
 u_int8_t	qos_max_parallel;
 u_int8_t	qos_bandwidth;
 u_int8_t	qos_bandwidth_scale;
 u_int8_t	qos_use_wred;
 u_int8_t	qos_use_traffic_shaping;
 u_int8_t	qos_use_flow_eq_queue;
 u_int8_t	qos_flow_average_rate;
 u_int8_t	qos_flow_max_degree;
 u_int8_t	qos_flow_max_queuedepth;
};
typedef struct qos qos_profile;

struct wid_qos {
 unsigned int	QosID;
 char			*name;
 qos_profile * radio_qos[4];
 qos_profile * client_qos[4];
 unsigned char	qos_total_bandwidth;
 unsigned char	qos_res_scale;
 unsigned char	qos_share_bandwidth;
 unsigned char	qos_res_share_scale;
 char			qos_manage_arithmetic[WID_QOS_ARITHMETIC_NAME_LEN];
 char			qos_res_grab_arithmetic[WID_QOS_ARITHMETIC_NAME_LEN];
 char			qos_res_shove_arithmetic[WID_QOS_ARITHMETIC_NAME_LEN];
 unsigned char	qos_use_res_grab;
 unsigned char	qos_use_res_shove;
 struct wid_qos *next;
};
typedef struct wid_qos AC_QOS;
typedef struct {
	unsigned int  qos_num;
	
	AC_QOS **qos;
}DCLI_WQOS;
#define WID_QOS_CWMIN_DEFAULT	15
#define WID_QOS_CWMAX_DEFAULT	15
#define WID_QOS_AIFS_DEFAULT	15

extern AC_QOS *WID_QOS[QOS_NUM];

enum wid_qos_type {
	WID_BESTEFFORT=0,
	WID_BACKGTOUND,
	WID_VIDEO,
	WID_VOICE
};
typedef enum{
	unkonwn_type=0,
	averagerate_type,
	max_burstiness_type,
	manage_priority_type,
	shove_priority_type,
	grab_priority_type,
	max_parallel_type,
	bandwidth_type,
	bandwidth_percentage_type,
	flowqueuelenth_type,
	flowaveragerate_type,
	flowmaxburstiness_type
}flow_parameter_type;
typedef enum{
	qos_unkonwn_type=0,
	totalbandwidth_type,
	resourcescale_type,
	sharebandwidth_type,
	resourcesharescale_type
}qos_parameter_type;

struct model_info {
 char	*model;
 unsigned short	ap_eth_num;
 unsigned short	ap_wifi_num;
 unsigned short	ap_11a_antenna_gain;
 unsigned short	ap_11bg_antenna_gain;
 unsigned int	ap_if_mtu;
 unsigned int	ap_if_rate;
 char	*hw_version;
 char	*sw_name;
 char	*sw_version;
 char	*sw_supplier;
 char	*supplier;
};
typedef struct model_info model_infomation;
struct model_code_info {
 char	*code;
 unsigned char	cpu_type;
 unsigned char	mem_type;
 unsigned char	ap_eth_num;
 unsigned char	ap_wifi_num;
 unsigned char	ap_antenna_gain;
 unsigned char	support_mode[L_RADIO_NUM];
 unsigned int	ap_if_mtu;
 unsigned int	ap_if_rate;
 unsigned int	card_capacity;
 unsigned int	flash_capacity;
 char	*hw_version;
 char	*sw_name;
 char	*sw_version;
 char	*sw_supplier;
 char	*supplier;
};
typedef struct model_code_info wid_code_infomation;
typedef struct {
	unsigned char  state; 				/*0-disable, 1-num balance, 2-flow balance*/
	unsigned int   num_balance_para;	/*default : 1*/
	unsigned int   flow_balance_para;	/*default : 1 (Mbps)*/
}ac_balance_flag;
struct sample_info {
 unsigned char	monitor_time;
 unsigned char	sample_time;
 unsigned char	monitor_switch;
 unsigned char	sample_switch;
};
typedef struct sample_info wid_sample_info;
/*ethereal bridge area*/
struct WID_EBR_IF {
  char	*ifname;	
  struct WID_EBR_IF  *ifnext;	
};
typedef struct WID_EBR_IF EBR_IF_LIST;

struct wid_ebr {
 unsigned int	EBRID;
 char			*name;
 unsigned char	state;						/* (1 enable /0 disable)*/
 unsigned char	isolation_policy;			/* (1 enable /0 disable)*/
 unsigned char	multicast_isolation_policy;	/* (1 enable /0 disable)*/
 unsigned char	sameportswitch;		/* (1 enable /0 disable)*/
 unsigned char	bridge_ucast_solicit_stat;	/* (1 enable/ 0 disable) */		/* Huang Leilei add 2012-11-13 */
 unsigned char	bridge_mcast_solicit_stat;	/* (1 enable/ 0 disable) */		/* Huang Leilei add 2012-11-13 */
 unsigned char  multicast_fdb_learn;		/* (1 enable /0 disable)*/
 unsigned int	r_num; 
 unsigned int	eth_num;
 EBR_IF_LIST	*iflist;
 EBR_IF_LIST	*uplinklist;
 struct wid_ebr *next;
};
typedef struct wid_ebr ETHEREAL_BRIDGE;

extern ETHEREAL_BRIDGE *WID_EBR[EBR_NUM];
/*auto ap area*/
struct auto_ap_if {
  unsigned char	wlannum;
  unsigned char	wlanid[L_BSS_NUM];
  unsigned int ifindex;
  char		ifname[ETH_IF_NAME_LEN];  
  struct auto_ap_if  *ifnext;	
};
typedef struct auto_ap_if wid_auto_ap_if;

typedef struct {
unsigned char	auto_ap_switch;
unsigned char	save_switch;
unsigned char	ifnum;
unsigned int	list_len;
char *ifname;
wid_auto_ap_if	*auto_ap_if;
}wid_auto_ap_info;

struct wid_ac_ip{
	char *ip;
	unsigned int wtpcount;
	unsigned int threshold;
	unsigned char priority;
	struct wid_ac_ip *next;
};

/*fengwenchao add 20120323*/
typedef struct {
	unsigned int policy;
	char arp_ifname[ETH_IF_NAME_LEN];
}sta_static_arp;

typedef struct{
	unsigned int unicast_policy;
	unsigned int multicast_broadcast_policy;
	unsigned int wifi_policy;
	unsigned int rate;
}ap_uni_muti_bro_cast;
/*fengwenchao add end*/

typedef struct{
	unsigned char GroupID;
	unsigned char load_banlance; //added by weianying 2010/0323
	unsigned int diff_count;
	int isock;
	unsigned char *ifname;
	unsigned char *ipaddr;
	unsigned int ipnum;
	struct wid_ac_ip *ip_list;
}wid_ac_ip_group;
typedef struct{
	unsigned int count;
	wlan_stats_info *ap_statics_ele;
}Ap_statics_INFOS;
typedef struct{
	char * WTPIP;
	unsigned long wtpip;
	Ap_statics_INFOS *ap_statics_list;
	unsigned char txpowr;
	unsigned char rssi[4];

	unsigned int floodingcount;
	unsigned int sproofcount;
	unsigned int weakivcount;

	unsigned int ap_mask_new;
	unsigned int ap_gateway;
	unsigned int ap_dnsfirst;
	unsigned int ap_dnssecend;

}DCLI_AC_API_GROUP_THREE;
/*
struct bak_sock{
	int sock;
	unsigned int ip;
	struct bak_sock *next;
};*/
typedef struct{
	int list_len;
	struct bak_sock *b_sock_node;
}Bak_Sock_INFO;
struct CWConfigVersionInfo_dcli
{
	char *str_ap_model; //for oem change
	char *str_ap_version_name;
	char *str_ap_version_path;
	unsigned char radio_num;
	unsigned char bss_num;
	CWBool_DCLI ischanged;
	CWBool_DCLI ismodelchanged;
	struct radio_info_type_dcli radio_info[4];
	char *str_ap_code;// for model match
	char *str_oem_version;
	int	apcodeflag;
	struct CWConfigVersionInfo_dcli *next;
};

typedef struct CWConfigVersionInfo_dcli CWConfigVersionInfo_dcli;

typedef struct{
	int list_len;
	CWConfigVersionInfo_dcli *config_ver_node;
}Config_Ver_Info;

typedef struct{
	wid_code_infomation *code_info;
	model_infomation *model_info;
	Config_Ver_Info *config_ver_info;

	Bak_Sock_INFO *bak_sock;	
}DCLI_AC_API_GROUP_FOUR;

typedef struct{
	int result;
	int hw_version;
	int sw_version;
	char *hw_version_char;
	char *sw_version_char;
	char *ac_name;
	int sta_count;
	int max_wtp;
	int gmaxwtps;
	int static_wtp;
	int force_mtu;
	int log_switch;
	int log__level;
	char loglevel[100];	
//	char clog_swith[2][4];/* init it : = {"OFF","ON"},when use it*/
	int log_size;
	unsigned char uclev3_protocol;
//	char caauth_security[2][20];/* init it : = {"CW_PRESHARED","CW_X509_CERTIFICATE"},when use it*/
	unsigned char auth_security;
//	char calev3_protocol[2][8];/* init it :  = {"CW_IPv6","CW_IPv4"},when use it*/
	int trapflag;
	unsigned char apstaticstate;
	unsigned char macfiltrflag;
	unsigned char essidfiltrflag;
	int g_ac_all_extention_information_switch;
	unsigned int apstatisticsinterval;
	/*xiaodawei add, 20110115*/
	unsigned char radioresmgmt;
	int tranpowerctrl;
	unsigned char tranpwrctrlscope;
	unsigned char autoaploginswitch;
	unsigned char autoaplogin_saveconfigswitch;
	unsigned char wirelessctrlmonitor;
	unsigned char wirelessctrlsample;
	unsigned char widwatchdog;
	unsigned char ac_balance_method;
	unsigned char ap_hotreboot;
	unsigned int ap_acc_through_nat;
	unsigned char wtp_wids_policy;
	unsigned char radio_src_mgmt_countermeasures;
	unsigned char radio_src_mgmt_countermeasures_mode;
	int wireless_interface_vmac;
	unsigned char wtp_link_detect;
	unsigned int wsm_switch;
	unsigned int service_tftp;
	unsigned int service_ftp;
	/*xiaodawei add for trap switch, 20110115*/
	int ap_run_quit;
	int ap_cpu_threshold;
	int ap_mem_threshold;
	int ap_update_fail;
	int rrm_change;
	int rogue_ap_threshold;
	int rogue_terminal_threshold;
	int rogue_device;
	int wireless_interface_down;
	int channel_count_minor;
	int channel_change;
	int rogue_ap;
	int country_code; /*wcl add*/
	/*end of trap switch*/
}wireless_config;
typedef struct{
	int state;
	unsigned char scope;
	unsigned char th1;
	unsigned char th2;
	unsigned char constant;
	unsigned char max;
}txpower_control;
typedef struct{
	int flag,vrid,master_uplinkip,master_downlinkip,bak_uplinkip,bak_downlinkip,vir_uplinkip,vir_downlinkip,global_ht_ip,global_ht_opposite_ip;
	char *vir_uplinkname;
	char *vir_downlinkname;
	char *global_ht_ifname;

}wid_vrrp_state;
typedef struct{
	int num;

	/*ap  threadhold*/
	unsigned int cpu ;
	unsigned int memoryuse ;
	unsigned int temperature ;

	unsigned char rrm_state;
	unsigned int d_channel_state;
	unsigned short report_interval;
	unsigned char flag;

	int neighbordead_interval;
	unsigned int timer;

	/*balance  configuration*/
	unsigned int state;
	unsigned int number;
	unsigned int flow;
	
	wireless_config *wireless_control;
	wid_auto_ap_info *auto_login;
	txpower_control *tx_control;
	wid_sample_info *sample_info;
	wid_vrrp_state * wid_vrrp;
	unsigned char countermeasures_mode;
	unsigned char countermeasures_switch;
}DCLI_AC_API_GROUP_FIVE;
typedef struct{
	int list_len;
	WID_WTP *WTP_LIST;
	
}WID_WTP_INFO;
typedef struct{

	unsigned int num ;
	unsigned int TotalNum;

	unsigned int join_num;
	unsigned int configure_num;
	unsigned int datacheck_num;
	unsigned int run_num;
	unsigned int quit_num;
	unsigned int imagedata_num;
	unsigned int bak_run_num;
	unsigned int wtp_model_type;      /*fengwenchao add 20110307 WTP MODEL 的种类*/
	unsigned int wtp_version_type;    /*fengwenchao add 20110314 WTP VERSION 的种类*/

	WID_WTP_INFO *WTP_INFO;

	struct WTP_MODEL_VERSION **WTP_M_V;    /*fengwenchao add 20110314*/
	WID_WTP **WTP;
	struct AP_VERSION **AP_VERSION;
}DCLI_WTP_API_GROUP_ONE;
typedef struct{

	wid_sample_rate_info *sample_info;
	WID_BSS **bssstatistics;
	WID_WTP **WTP;
}DCLI_WTP_API_GROUP_TWO;

struct dcli_wtp_api{

	int echotimer;
	int checktimer;
	unsigned int ElectrifyRegisterCircle;
	time_t addtime;
	time_t quittime;
	time_t starttime;
	time_t imagadata_time;
	time_t config_update_time;
	char *wtp_location;
	char *netid;
	unsigned int wlan_num;
	WID_WLAN **WLAN;
	
	wid_wids_set wids;	
	unsigned char dhcp_flooding_status;
	unsigned char interval;
	unsigned char probethreshold;	
	unsigned char otherthreshold;	
	unsigned int lasttime;

	unsigned int txpower;	
	char *model;
	
	char *versionname;
	char *versionpath;
	char *apcode;
	unsigned char Count_onetimeupdt;
	struct dcli_wtp_api *next;

	int old_ap_img_state;
	unsigned int checktimes;
};
typedef struct dcli_wtp_api	DCLI_WTP_API_GROUP_THREE;

typedef struct{
	unsigned int radio_num;
	unsigned char bss_num;
	int bss_num_int;
	unsigned int wlan_num;
	unsigned int qos_num;
	unsigned short interval;
	WID_WTP_RADIO **RADIO;
	WID_WTP **WTP;
	WID_BSS **BSS;
}DCLI_RADIO_API_GROUP_ONE;
typedef struct{
	unsigned char wlan_num;
	unsigned int enable_num;
	WID_WLAN **WLAN;
}DCLI_WLAN_API_GROUP;
typedef struct{
//	unsigned char ebr_num;
	unsigned int ebr_num;
	ETHEREAL_BRIDGE **EBR;
}DCLI_EBR_API_GROUP;
typedef struct{
	unsigned int ip_list_num;
	wid_ac_ip_group **AC_IP_LIST;

}DCLI_AC_IP_LIST_API_GROUP;
enum dcli_method {	
	WID_DBUS_QOS_METHOD_SHOW_QOS_LIST_CASE=1,
	WID_DBUS_QOS_METHOD_SHOW_QOS_CASE,
	WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO_CASE,
	WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO_CASE
};
typedef struct{
	unsigned int dcli_ap_run_quit_trap_switch;
	unsigned int dcli_ap_cpu_trap_switch;
	unsigned int dcli_ap_mem_trap_switch;
	unsigned int dcli_flash_write_fail_trap_switch;
	unsigned int dcli_rrm_change_trap_switch;
	unsigned int dcli_rogue_ap_threshold_switch;
	unsigned int dcli_channel_terminal_interference_switch;
	unsigned int dcli_channel_device_interference_switch;
	unsigned int dcli_wireless_interface_down_switch;
	unsigned int dcli_channel_count_minor_switch;
	unsigned int dcli_channel_change_switch;
	unsigned int dcli_rogue_ap_switch;
}WID_TRAP_SWITCH;
typedef struct{
	unsigned int cpu;
	unsigned int memoryuse;
	unsigned int rogue_ap_threshold;
	unsigned int rogue_termi_threshold;
	unsigned int wtpid;
	unsigned int trap_switch;
	unsigned int collecttime;
	int samechannelrssi_theshold;
    int neighborchannelrssi_theshold;
	
}WID_TRAP_THRESHOLD;

struct WTP_GROUP_MEMBER{	
	struct WTP_GROUP_MEMBER *next;
	struct WTP_GROUP_MEMBER *hnext;
	unsigned int WTPID;
};

typedef struct{
	unsigned int GID;
	unsigned char *GNAME;
	unsigned int WTP_COUNT;
	struct WTP_GROUP_MEMBER *WTP_M;
	struct WTP_GROUP_MEMBER *WTP_HASH[256];	
	WID_WTP	WTP_CONFIG;
}WID_WTP_GROUP;
extern WID_WTP_GROUP	*WTP_GROUP[WTP_GROUP_NUM];

struct FLASHDISCONN_STAINFO{
	struct FLASHDISCONN_STAINFO *next;
	struct FLASHDISCONN_STAINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int wtpid;
	unsigned int bssindex;
	unsigned char wlanid;
	unsigned int ipv4Address;
	unsigned long long  rxbytes;
	unsigned long long  txbytes;
	unsigned long long  rxpackets;
	unsigned long long  txpackets;
};

typedef struct{
	struct FLASHDISCONN_STAINFO *fd_sta_list;		
	struct FLASHDISCONN_STAINFO *fd_sta_hash[256];	
	unsigned int fd_num_sta;	
}asd_sta_flash_disconn;

//fengwenchao add 20101222 for  show wtp config of all wtp

struct WLAN_INFO{
	char *wlanname;
	unsigned char Wlanid;
	unsigned char bifnum;                               //wlan绑定的接口数量
	
	unsigned char wlanBindSecurity;           			//wlan绑定的安全策略的ID
	unsigned char wlanHideEssid;              			//是否隐藏ESSID 1--yes 0--no
	unsigned char wlanServiceEnable;          			//WLAN Status 1(default disable)/0(enable)
	unsigned int wlanMaxConnectUsr;          			//wlan最大接入用户数
	unsigned char wlanLoadBalanceStatus;      			//负载均衡状态
	unsigned char wlanLoadBalanceStatusBaseOnFlow;    	//wlan基于流量的负载均衡
	unsigned char wlanLoadBalanceStatusBaseOnUsr;       //wlan基于用户的负载均衡
	unsigned int wlanLoadBalanceTrafficDiffThreshhd;    //流量差阈值
	unsigned int wlanLoadBalanceUsersDiffThreshhd;      //用户差阈值
	unsigned int  wlanStaOnlineNum;                     //wlan在线用户数
	 float wlanUsrWirelessResoUseRate;           //用户侧（无线侧）可用资源利用率  wlanStaOnlineNum占wlanMaxConnectUsr的百分比
	unsigned int wlanBindSecType;                       //wlan绑定的安全策略的安全类型
	unsigned int wlanBindEncryType;                     //wlan绑定的安全策略的加密类型
	unsigned char wlanLoadBalanceFunction;               //balance switch 开关
	unsigned int indorpPkts;                        //SSID总入丢包数
	unsigned int intotlePkts;                       //SSID总入包数
	unsigned int outdorpPkts;                       //SSID总出丢包数
	unsigned int outtotlePkts;                      //SSID总出包数
	unsigned int SSIDDownBandWidthRate;                 //SSID下行带宽利用率
	unsigned int SSIDUpBandWidthRate;                   //SSID上行带宽利用率

	/*fengwenchao add 20110401 for dot11WlanDataPktsTable*/
	/*-----------------------------------------------*/
	unsigned int ethernetRecvCorrectFrames;        //网络侧（有线侧）MAC层接收到的正确数据帧数目
	unsigned long long ethernetRecvCorrectBytes;   //网络侧（有线侧）MAC层接收到的数据正确数据包字节数
	unsigned long long ethernetSendCorrectBytes;   //网络侧（有线侧）MAC层接发送出去的数据正确数据包字节数
	/*-----------------------------------------------*/
	
	struct WLAN_INFO *wlan_info_list;
	struct WLAN_INFO *wlan_info_last;
	struct WLAN_INFO *next;
	struct ifi *ifi_head;
};

struct WTP_CONFIG_INFORMATION{
	struct WTP_CONFIG_INFORMATION *wtp_config_list;
	struct WTP_CONFIG_INFORMATION *wtp_config_last;
	struct WTP_CONFIG_INFORMATION *next;
	struct WLAN_INFO *wlan_info_head;

	int WTPID;
	char *wtpBindPort;   //绑定的端口
	unsigned char apply_wlan_num;   //绑定的wlan数量
	unsigned char *wtpMacAddr;

	unsigned char wtpused;   //wtp的状态  1--used; 0--unused
	unsigned int wtpMaxStaNum;  //wtp允许的最大sta num
	unsigned int wtpLoadBalanceTrigerBaseUsr;  //基于用户的负载平衡 wtp number triger  <1-64>
	unsigned int wtpLoadBalanceTrigerBaseFlow;  //基于流量的负载平衡 wtp flow triger  <0-1024>
};

//fengwenchao add 20101222 for  show wtp config of all wtp end

//fengwenchao add 20110307 for REQUIREMENTS-144 
struct WTP_MODEL_VERSION{
	char* wtp_model;
	unsigned int wtp_model_num;
	char* wtp_version;
	unsigned int wtp_version_num;
	//int wtpid_group[2048];
	//int wtpid_flag;
};

//fengwenchao add end

struct Listenning_IF{
	char ifname[ETH_IF_NAME_LEN];
	unsigned int ipaddr;
	struct Listenning_IF *if_next;
	LISTEN_FLAG lic_flag;
};


typedef struct{
    int count;
	int a;
	struct Listenning_IF *interface;
}Listen_IF; 


#endif/*_WID_DEFINE_H*/
