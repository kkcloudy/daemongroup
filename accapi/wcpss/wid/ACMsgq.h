#ifndef ACMSGQ_H
#define ACMSGQ_H

typedef enum{
	CONTROL_TYPE = 1,
	DATA_TYPE = 2 
}MQType;

typedef enum{
	WLAN_S_TYPE = 1,
	WTP_S_TYPE = 2,
	Radio_S_TYPE = 3,
	STA_S_TYPE = 4,
	WDS_S_TYPE = 5
}MQSubType;

typedef struct{
	unsigned int len;
	char Data[4096];
}MQ_DATA;

typedef enum{
	WLAN_ADD = 0,
	WLAN_DEL = 1,
	WLAN_HIDE_ESSID = 2,
	WLAN_WDS_ENABLE = 3,
	WLAN_WDS_DISABLE = 4,
	WLAN_CHANGE_TUNNEL
}WlanOP;

typedef struct{
	WlanOP Wlan_Op;
	unsigned char WLANID;
	unsigned char Radio_L_ID;
	char WlanEssid[ESSID_LENGTH];
	unsigned int bssindex;
	u_int8_t	HideESSid; /* 1  hide  0 not hode*/
	char	WlanKey[DEFAULT_LEN];
	unsigned int KeyLen;
	unsigned int SecurityType;
	//unsigned int EncryptionType;
	unsigned char SecurityIndex;
	unsigned char asic_hex;/* 0 asic; 1 hex*/
	unsigned char Roaming_Policy;			/*Roaming (1 enable /0 disable)*/
	unsigned short flow_check;
	unsigned int no_flow_time;
	unsigned int limit_flow;
	unsigned char multi_user_optimize_switch;
}MQ_WLAN;

typedef enum{
	WTP_REBOOT = 0,
	WTP_EXTEND_CMD = 1,
	WTP_SCANNING_OP = 2,
	WTP_STATISTICS_REPORT = 3,
	WTP_SET_IP = 4,
	WTP_TIMESTAMP = 5,
	WTP_EXTEND_INFO_GET = 6,
	WTP_SAMPLE_INFO_SET = 7,
	WTP_STA_INFO_SET = 8,
	WTP_IF_INFO_SET = 9,
	WTP_RESEND = 10,
	WTP_WIDS_SET = 11,
	WTP_NEIGHBORDEAD_INTERVAL = 12,
	WTP_SET_IP_DNS = 13,
	WTP_DHCP_SNOOPING = 14,
	WTP_STA_INFO_REPORT = 15,
	WTP_STA_WAPI_INFO_SET = 16,
	WTP_STATISTICS_REPORT_INTERVAL = 17,
	WTP_NTP_SET = 18,
	WTP_TERMINAL_DISTRUB_INFO = 19,
	WTP_IF_ETH_MTU = 20,          /* fengwenchao add 20110126 for XJDEV-32  from 2.0 */
	WTP_FLOW_CHECK = 21,	
	WTP_SET_NAME_PASSWD = 22,
	WTP_MULTI_USER_OPTIMIZE =23,
	WTP_STA_DEAUTH_SWITCH = 24,
	WTP_STA_FLOW_INFORMATION_SWITCH = 25,
	WTP_OPTION60_PARAM =26,
	WTP_LONGITUDE_LATITUDE_SET = 27,
	WTP_UNAUTHORIZED_MAC_REPORT = 28,
	WTP_CONFIGURE_ERROR_REPROT = 29,
	WTP_STA_FLOW_OVERFLOW_RX_REPORT = 30,
	WTP_STA_FLOW_OVERFLOW_TX_REPROT = 31,
	WTP_STA_ONLINE_FULL_REPROT = 32,
	WTP_WIFI_LOCATE_PUBLIC_CONFIG = 33,
    WTP_LAN_VLAN1 = 34,
    WTP_LINK_QUARLITY=35,
    WTP_ELECTRONIC_MENU=36  //lilong add 2014.12.01
}WtpOP;

typedef struct{
	char mac[MAC_LEN];
}MACTYPE;


typedef struct{
	WtpOP Wtp_Op;
	unsigned int WTPID;
	unsigned int i1;
	unsigned int i2;
	unsigned short s1;
	unsigned short s2;
	unsigned short s3;
	unsigned char c1;
	unsigned char c2;
	unsigned char value1;
	unsigned int value2;
	unsigned short value3;
	unsigned char value4;
	char value[512];
	char username[32];
	char passwd[32];
	MACTYPE macarry[128];
}MQ_WTP;

typedef enum{
	Radio_Channel = 0,
	Radio_TXP = 1,
	Radio_Mode = 2,
	Radio_Rates = 3,
	Radio_FragThreshold = 4, 
	Radio_BeaconPeriod = 5,
	Radio_Preamble = 6,
	Radio_DTIMPeriod = 7,
	Radio_ShortRetry = 8,
	Radio_LongRetry = 9,
	Radio_rtsthreshold = 10,
	Radio_Qos = 11,
	Radio_STATUS = 12,
	Radio_Throughput = 13,
	Radio_BSS_Throughput = 14,
	Radio_TXPOF = 15,
	Radio_11N_GI_MCS_CMMODE = 16,
	Radio_ampdu_op = 17,
	Radio_11N_MCS_LIST = 18,  /*fengwenchao add 20120314 for requirements-407*/
	RADIO_WSM_STA_INFO_REPORT = 19,
	Radio_puren_mixed_op,
	Radio_channel_ext_offset,
	RAdio_tx_chainmask,
	RAdio_rx_chainmask,  //zhangshu add 2010-10-09
	Radio_amsdu_op,       //zhangshu add 2010-10-09
	Radio_acktimeout_distance, /*wcl add for RDIR-33*/
	Radio_NO_RESP_STA_PRO_REQ,
	Radio_UNI_MUTIBRO_CAST_ISO_SW,
	Radio_UNI_MUTIBRO_CAST_TATE,
	Radio_Countrycode  ,/*wcl add for OSDEVTDPB-31*/
	Radio_set_cpe_channel,
	Radio_set_MGMT_rate
}RadioOP;

typedef struct{
	RadioOP Radio_Op;
	unsigned char Radio_L_ID;
	unsigned int Radio_G_ID;
	unsigned char BSS_L_ID;
	unsigned int id1;
	unsigned char wlanid;
	unsigned char id_char;
	unsigned short vlan_id;
	unsigned char op;
	unsigned int rate;
}MQ_Radio;

typedef enum{
	Sta_ADD = 0,
	Sta_DEL = 1,
	Sta_AUTH = 2,
	Sta_UNAUTH = 3,
	Sta_SETKEY = 4
}StaOP;

typedef struct{
	StaOP Sta_Op;
	unsigned int BSSIndex;
	unsigned char WLANID;
	unsigned char Radio_L_ID;
	char STAMAC[6];
	unsigned int cipher;
	unsigned int keylen;
	unsigned char keyidx;
	char key[128];
	unsigned int traffic_limit;		/*ht add 091014*/
	unsigned int send_traffic_limit;
	unsigned int vlan_id;
	unsigned int sta_num;
}MQ_STA;

typedef struct{
	int WTPID;
	MQType type;
	MQSubType subtype;
	union{
		MQ_DATA DataInfo;
		MQ_WLAN WlanInfo;
		MQ_WTP	WtpInfo;
		MQ_Radio	RadioInfo;
		MQ_STA	StaInfo;
	}u;
}msgqdetailData;

typedef struct{
	long mqid;
	msgqdetailData mqinfo;
}msgqData;

typedef struct{
	int WTPID;
	MQType type;
	MQSubType subtype;
	union{
		//MQ_DATA DataInfo;
		MQ_WLAN WlanInfo;
		MQ_WTP	WtpInfo;
		MQ_Radio	RadioInfo;
		MQ_STA	StaInfo;
	}u;
	//qiuchen add it for AXSSZFI-1191
	int master_to_disable;
	//end
}msgqdetail;

typedef struct{
	long mqid;
	msgqdetail mqinfo;
}msgq;

struct msgqlist{
	msgqdetail mqinfo;
	struct msgqlist *next;
};
void WID_CONFIG_SAVE(unsigned int WTPIndex);
#endif
