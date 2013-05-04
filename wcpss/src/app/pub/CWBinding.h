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
* CWBinding.h
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

#ifndef __CAPWAP_CWBinding_HEADER__
#define __CAPWAP_CWBinding_HEADER__

#define CW_BINDING_HLEN				4
#define CW_BINDING_WIRELESSID			1
#define CW_BINDING_DATALENGTH			6

#define NUM_QOS_PROFILES			4
#define UNUSED_QOS_VALUE			255

#define VOICE_QUEUE_INDEX			0
#define VIDEO_QUEUE_INDEX			1
#define BESTEFFORT_QUEUE_INDEX			2
#define BACKGROUND_QUEUE_INDEX			3

#define BINDING_MIN_ELEM_TYPE			1024
#define BINDING_MAX_ELEM_TYPE			2047

// Wireless ID viene preso dal campo WBID
//#define CW_TRANSPORT_HEADER_WIRELESS_ID_START	0
//#define CW_TRANSPORT_HEADER_WIRELESS_ID_LEN	8

//#define CW_TRANSPORT_HEADER_LENGTH_START	8
#define CW_TRANSPORT_HEADER_LENGTH_START	0
#define CW_TRANSPORT_HEADER_LENGTH_LEN		8

//#define CW_TRANSPORT_HEADER_RSSI_START		16
#define CW_TRANSPORT_HEADER_RSSI_START		8
#define CW_TRANSPORT_HEADER_RSSI_LEN		8

//#define CW_TRANSPORT_HEADER_SNR_START		24
#define CW_TRANSPORT_HEADER_SNR_START		16
#define CW_TRANSPORT_HEADER_SNR_LEN		8

// Poiche' nel draft 09 il campo del CAPWAP header Wireless Specific 
// Information e' stato privato del sottocampo Wireless ID con il
// conseguente shift a sx di 8 bit dei sottocampi successivi il sottocampo
// datarate del binding si trova a cavallo tra 2 word da 4 byte quindi
// vanno specificati due offset.
//#define CW_TRANSPORT_HEADER_DATARATE_START	0
//#define CW_TRANSPORT_HEADER_DATARATE_LEN	16

#define CW_TRANSPORT_HEADER_DATARATE_1_START	24
#define CW_TRANSPORT_HEADER_DATARATE_1_LEN	8

#define CW_TRANSPORT_HEADER_DATARATE_2_START	0
#define CW_TRANSPORT_HEADER_DATARATE_2_LEN	8

//#define CW_TRANSPORT_HEADER_PADDING_START	16
//#define CW_TRANSPORT_HEADER_PADDING_LEN		16
#define CW_TRANSPORT_HEADER_PADDING_START	8
#define CW_TRANSPORT_HEADER_PADDING_LEN		24


#define BINDING_MSG_ELEMENT_TYPE_ADD_WLAN   1024
#define BINDING_MSG_ELEMENT_TYPE_ASSIGNED_WTP_BSSID 1026
#define BINDING_MSG_ELEMENT_TYPE_DELETE_WLAN		1027
#define BINDING_MSG_ELEMENT_TYPE_SET_CHAN	1028
#define	BINDING_MSG_ELEMENT_TYPE_DIRECT_SEQUEUE_CONTROL	1028
#define BINDING_MSG_ELEMENT_TYPE_IEEE80211_INFO_ELEMENT 1029

#define BINDING_MSG_ELEMENT_TYPE_IEEE80211_MAC_OPERATE 1030//weiay 20080722
#define BINDING_MSG_ELEMENT_TYPE_IEEE80211_RATE_SET 1034
#define BINDING_MSG_ELEMENT_TYPE_STATION_SESSION_KEY	1038
#define BINDING_MSG_ELEMENT_TYPE_SET_TXP	1041
#define	BINDING_MSG_ELEMENT_TYPE_TX_POWER	1041
#define BINDING_MSG_ELEMENT_TYPE_WTP_QOS	1045
#define BINDING_MSG_ELEMENT_TYPE_IEEE80211_WTP_RADIO_CONFIGURATON 1046 //weiay 20080722

#define BINDING_MSG_ELEMENT_TYPE_UPDATE_WLAN 1044
#define BINDING_MSG_ELEMENT_TYPE_IEEE80211_RADIO_INFO	1048
#define	BINDING_MSG_ELEMENT_TYPE_RADIO_INFO                    1048       //pei add 0624
#define BINDING_MSG_ELEMENT_TYPE_IEEE80211_RADIO_TYPE_SET	1048 //added by weiay 20080714
#define BINDING_MSG_ELEMENT_TYPE_IEEE80211_WLAN_VLAN_INFO	1049       //pei add 0305
#define BINDING_MSG_ELEMENT_TYPE_WAPI_CER_INFO_ELEMENT	1050

#define BINDING_MSG_ELEMENT_TYPE_NEIGHBORDEAD_INTERVAL	1051

#define BINDING_MSG_ELEMENT_TYPE_WTP_IP_MAC_REPORT 1052 //wuwl add 20100126
#define BINDING_MSG_ELEMENT_TYPE_WTP_SNOOPING	1053 //wuwl add 20100126

#define BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET	2012 //wuwl add 20100311
#define BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_REPORT 2013 //wuwl add 20100415
#define BINDING_MSG_ELEMENT_TYPE_CHANGE_TUNNEL_MODE 2014
typedef struct
{
	unsigned char queueDepth;
	int cwMin;
	int cwMax;
	unsigned char  AIFS;
	unsigned char dot1PTag;
	unsigned char DSCPTag;	
} WTPQosValues;
typedef struct
{
	unsigned char radio_id;
	unsigned char wlan_id;
	unsigned short capabilities;
	unsigned char key_index;
	unsigned char key_status;	
	unsigned short key_length;
	char *key;/*legth 32*/	
	char *group_tsc;/*length 6*/	
	unsigned char qos;	
	unsigned char auth_type;	
	unsigned char mac_mode;	     
	unsigned char tunnel_mode; 
	unsigned char suppress_ssid;	
    char *ssid;	/*length 32*/
}AddWlanValues;
typedef struct{
 	unsigned char elem_id;  //221
 	unsigned char len;
 	unsigned char oui[3];
 	unsigned char oui_type;
 	unsigned char version[2]; /* little endian */
 	int proto;
 	int pairwise_cipher;     //alg
 	int group_cipher;
 	int key_mgmt;            //auth mode
 	int capabilities;
 	unsigned int num_pmkid;
 	//const unsigned char *pmkid;
 	int mgmt_group_cipher;
}WPA_IE;
typedef struct {                    //peiwenhui 0606
	unsigned char radio_id;
	unsigned char wlan_id;
	unsigned char flags;
	WPA_IE *wtp_ie;
}Ieee80211InfoEleValues;

typedef struct{
	unsigned char radio_id;
	unsigned char mac_length;
	unsigned char *mac_addr;
	unsigned char wlan_id;
	
}AddSTAValues;
typedef struct{
	unsigned char radio_id;
	unsigned char mac_length;
	unsigned char *mac_addr;
	unsigned char wlan_id;  //pei add 0306
}DeleteSTAValues;
typedef struct
{
	WTPQosValues* qosValues;
} bindingValues;

typedef struct
{
	unsigned char radioID;
	unsigned char channel;
	
} BindingChan;

typedef struct
{	
	unsigned char radioID;
	unsigned short TXP;
	unsigned short TXPOF;
	
} BindingTXP;
//added by weiay
typedef struct
{	
	unsigned char radioID;
	unsigned short rate;
	
} BindingRate;

typedef struct
{	
	unsigned char radioID;
	unsigned int radiotype;
	
} BindingRadioType;
//added end
typedef struct
{	
	unsigned char RadioID;
	unsigned char Reserved;
	unsigned short RTSThreshold;
	unsigned char Shortretry;
	unsigned char Longretry;
	unsigned short FragThreshold;
	unsigned int TxMSDULifetime;
	unsigned int RxMSDULifetime;
	
} BindingRadioOperate;

typedef struct
{	
	unsigned char RadioID;
	unsigned char IsShortPreamble;
	unsigned char BSSIDnums;
	unsigned char DTIMPeriod;
	char BSSID[6];
	unsigned short BeaconPeriod;
	unsigned int CountryCode;
	
} BindingRadioConfiguration;

typedef struct
{	
	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char state;   
	unsigned int distance;
	
}BindingAcktimeout;/*wcl add for RDIR-33*/
typedef struct
{	
	unsigned char RadioID;
	unsigned short guardinterval;
	unsigned short mcs;
	unsigned short cwmode;
	char mcs_list[32]; /*fengwenchao add 20120314 for requirements-407*/
	int mcs_count;  /*fengwenchao add 20120314 for requirements-407*/
	
} Binding11Nparameter;

typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char RadioID;
	unsigned char WlanID;
	unsigned char Able;
	unsigned char subframe;   //zhangshu add, 2010-10-09
	unsigned int AmpduLimit;
	
} Binding11nAmpduParameter;
typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char RadioID;
	unsigned char WlanID;
	unsigned char Mixed_Greenfield;
	
} Binding11nMixedGreenfield;
/* op*/
typedef enum{
	DHCP_SNOOPING = 1,
	STA_IP_MAC_REPORT = 2,
	Ampdu_op = 3,
	Puren_mixed_op = 4,
	Channel_Extoffset =5,
	Tx_chainmask = 6,
	Amsdu_op = 7,
    Rx_chainmask = 8,
    ACK_timeout = 9  /*wcl add for RDIR-33*/
}WTP_RADIO_SET;

/*type*/
typedef enum{
	WTP = 1,
	RADIO = 2,
	STA = 3,
	KEY = 4,
	WLAN = 5

}OP_TYPE;

struct Support_Rate_List{
	int Rate;
	struct  Support_Rate_List *next;
};

/* begin */
// TODO: book add 11n rate list , 2011-10-20
struct n_rate_info{
	int rate;
	int stream_num;
	int mcs;
	int cwmode;
	int guard_interval;
};

struct n_rate_list {
	struct n_rate_info 	rate_info;
	struct n_rate_list 	*next;
};

struct n_rate_table{
    int count;
	struct n_rate_list 	*rate_info_list;
};
/* end */



typedef struct
{
	unsigned char radio_id;
	unsigned char wlan_id;
}DeleteWlanValues;
typedef struct 
{
	unsigned char radio_id;
	unsigned char wlan_id;
	unsigned short capabilities;
	unsigned char key_index;
	unsigned char key_status;	
	unsigned short key_length;
	char *key;/*legth 32*/	
	
}UpdateWlanValues;
typedef struct
{
	unsigned char *mac;/*6 byte*/
	unsigned short int flags;
	unsigned char *Pairwise_TSC;/*6 byte*/
	unsigned char *Pairwise_RSC;/*6 byte*/
	unsigned char *key;
		
}STASessionKeyValues;


/*---------------------------*/

typedef struct {
	char RSSI;
	char SNR;
	int dataRate;
} CWBindingTransportHeaderValues;

typedef struct {
	CWProtocolMessage* frame;
	CWBindingTransportHeaderValues* bindingValues;
} CWBindingDataListElement;

extern const int gMaxCAPWAPHeaderSizeBinding;

CWBool CWAssembleDataMessage(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr, int PMTU, CWProtocolMessage *frame, CWBindingTransportHeaderValues *bindingValuesPtr, int is_crypted);
CWBool CWAssembleTransportHeaderBinding(CWProtocolMessage *transportHdrPtr, CWBindingTransportHeaderValues *valuesPtr);
CWBool CWBindingCheckType(int elemType);
CWBool CWParseTransportHeaderBinding(CWProtocolMessage *msgPtr, CWBindingTransportHeaderValues *valuesPtr);
CWBool CWAssembleAssignedWTPBssid(CWProtocolMessage *bindPtr, char *bssid,int crete_wlan_id,int radio_id);

#endif
