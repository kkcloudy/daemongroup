
#ifndef __WIFI_H__
#define __WIFI_H__

#include <linux/netdevice.h>
#include "dba/dba.h"
#define WIFI_DEBUG    9
#define WIFI_WARNING  5
#define WIFI_ERROR    1

#define WIFI_STA_ACL_SUPPORT 0

#define VRID_MAX 35

#define WTP_MAX_NR				(4096+1)
#define BSS_MAX_NR_PER_WTP	128
#define MAC_LEN		6
#define IPv6_LEN	16
#define ETH_LEN		16
#define DEFAULT_LEN	256
#define ESSID_LEN 32
/*256 Wlan interfaces + 32768 Bss interfaces*/
#define WIFI_INTERFACE_NUM		(256 + (WTP_MAX_NR * BSS_MAX_NR_PER_WTP))

#define WIFI_OK 	(0)
#define WIFI_ERR	(-1)
#define L_RADIO_NUM 4
#define L_BSS_NUM   32
#define _4MB            (0x400000)

extern int wifi_eth_debug;

typedef enum{
	IEEE802_11_MGMT = 10,
	IEEE802_11_EAP	= 11,
	IEEE802_3_EAP	= 12,
	IEEE_OTHER	= 64
}DataType;

/**
  * Command number for ioctl.
  */
#define WIFI_IOC_MAGIC 243
#define WIFI_IOC_MAXNR 0x16

#define WIFI_IOC_IF_CREATE		_IOWR(WIFI_IOC_MAGIC, 1, struct interface_basic_INFO) // read values
#define WIFI_IOC_IF_DELETE    _IOWR(WIFI_IOC_MAGIC, 2, struct interface_basic_INFO) // read values
#define WIFI_IOC_MMAP  		_IOWR(WIFI_IOC_MAGIC, 3, uint64_t) // read values
#define WIFI_IOC_VRRP_MMAP _IOWR(WIFI_IOC_MAGIC, 4, sh_mem_t)
#define WIFI_IOC_GET_V6ADDR  _IOWR(WIFI_IOC_MAGIC, 5, dev_ipv6_addr_t)
#define WIFI_IOC_IF_UPDATE 	_IOWR(WIFI_IOC_MAGIC, 6, struct interface_INFO)//update ip port bssid
#define WIFI_IOC_WSM_SWITCH		_IOWR(WIFI_IOC_MAGIC, 7, unsigned int)//update ip port bssid
#define WIFI_IOC_ASD_SWITCH 	_IOWR(WIFI_IOC_MAGIC, 8, unsigned int) /*ht add for wifi and asd communicate switch,110308*/
#define WIFI_IOC_HANSISTATE_UPDATE 	_IOWR(WIFI_IOC_MAGIC, 9, unsigned int) /*wuwl add control data packet*/
#define WIFI_IOC_ASD_THRED_ID 	_IOWR(WIFI_IOC_MAGIC, 10, unsigned int) /*wuwl add asd pid notice to wifi for netlink*/
#define WIFI_IOC_ADD_STA		_IOWR(WIFI_IOC_MAGIC, 11, struct asd_to_wifi_sta) 
#define WIFI_IOC_DEL_STA		_IOWR(WIFI_IOC_MAGIC, 12, struct asd_to_wifi_sta)
#define WIFI_IOC_BATCH_IF_CREATE  	_IOWR(WIFI_IOC_MAGIC, 13, struct interface_batch_INFO) // read values
#define WIFI_IOC_BATCH_IF_DELETE	_IOWR(WIFI_IOC_MAGIC, 14, struct interface_batch_INFO) // read values
#if WIFI_STA_ACL_SUPPORT
#define WIFI_IOC_SET_NFMARK 	_IOWR(243, 15, struct wifi_nf_info) // caojia add for sta acl function 
#define WIFI_IOC_GET_NFMARK 	_IOWR(243, 16, struct wifi_nf_info) // caojia add for sta acl function 
#endif


/* #define kthread_16 */

#define MASTTER_STATE	0	
#define BACKUP_STATE	1	
#define DISABLE_STATE	2	/* disable*/




extern int wifi_eth_debug;

extern struct sock *nl_sk;

struct dba_result_w {
	unsigned int module_type;
	int result;
	unsigned int len;	/* length of data */
	//void *data;	
	char data[256];
};

typedef struct dba_result_w dba_result_t_w;

/**
 * This is the definition of the wifi-ethernet driver's private
 * driver state stored in dev->priv.
 */
typedef struct {
	int wlanID;
	unsigned int BSSIndex;
	unsigned int vrid;
	unsigned int protect_type;
	unsigned int isIPv6;
	unsigned int acip;
	unsigned int apip;
	unsigned char acipv6[IPv6_LEN];
	unsigned char apipv6[IPv6_LEN];
	unsigned short acport;
	unsigned short apport;
	unsigned short seqNum;
	unsigned char bssid[MAC_LEN];
	unsigned char apmac[MAC_LEN];
	unsigned char acmac[MAC_LEN];
	unsigned char ifname[ETH_LEN];
	unsigned char WLANID;
	unsigned char wsmswitch;
	struct net_device_stats stats; /* Device statistics */
	unsigned int first;
	unsigned long timers;
	unsigned char Eap1XServerMac[MAC_LEN];	
	unsigned char Eap1XServerSwitch;
	unsigned short	vlanid;
	unsigned char vlanSwitch;
	struct dba_result_w res;
	unsigned char f802_3;//if 1---capwap+802.3,else 0---802.11
} wifi_dev_private_t;

#define BSS_HASH_SIZE 256
#define STA_HASH_SIZE 256
typedef struct HANSI_INFO
{
	unsigned int instId;	
	unsigned char hpre_state;
	unsigned char hstate;
	unsigned char vlanSwitch;
	unsigned int asd_pid;
	unsigned char dhcpoption82;
}HANSI_info;

struct wifi_bss_tbl{
	unsigned char BSSID[MAC_LEN];
	unsigned int BSSIndex;
	unsigned int vrid;
	unsigned int dev_index;
	wifi_dev_private_t  priv;
	unsigned char Eap1XServerMac[MAC_LEN];	
	unsigned char Eap1XServerSwitch;
	unsigned short	vlanid;
	unsigned char vlanSwitch;
	unsigned char roaming_flag;
	struct wifi_bss_tbl *next;
};
struct wifi_sta_tbl{
	struct wifi_sta_tbl *next;
	unsigned char STAMAC[MAC_LEN];
	unsigned char BSSID_Before[MAC_LEN];
	unsigned char BSSID[MAC_LEN];
	unsigned char roaming_flag;
#if WIFI_STA_ACL_SUPPORT
	unsigned int nfmark; // caojia add for sta acl function
#endif
};
struct asd_to_wifi_sta{
	unsigned char STAMAC[MAC_LEN];
	unsigned char BSSID_Before[MAC_LEN];
	unsigned char BSSID[MAC_LEN];
	unsigned char roaming_flag;
#if WIFI_STA_ACL_SUPPORT
	unsigned int nfmark; // caojia add for sta acl function
#endif
};

#if WIFI_STA_ACL_SUPPORT
/* caojia add for sta acl function */
struct wifi_nf_info
{
	unsigned char STAMAC[MAC_LEN];
	//unsigned char BSSID[MAC_LEN];
	unsigned int nfmark;
};
#endif


extern struct wifi_bss_tbl *wifi_bss_hash[BSS_HASH_SIZE];
extern struct wifi_sta_tbl *wifi_sta_hash[STA_HASH_SIZE];
extern unsigned int wsmswitch;
extern unsigned int asdswitch;
extern struct HANSI_INFO hansiInfo[VRID_MAX];

/* ethenet packet layer 3 header*/
struct ip_header_t {
	unsigned char	version:4;			/* ip protocol version: 4 for ipv4, 6 for ipv6*/
	unsigned char  	hdrLen:4;			/* ip header length in four-byte unit*/
	unsigned char	dscp:6;				/* Differentiated Services Codepoint (DSCP)*/
	unsigned char	ect:1;				/* ECN-capable Transport*/
	unsigned char 	ecnce:1;			/* ECN-CE*/
	unsigned short	totalLen;			/* total length*/
	unsigned short 	id;					/* identification*/
	unsigned char	flag;				/* don't fragment or more fragment*/
	unsigned char 	fragOffset;			/* fragment offset*/
	unsigned char	ttl;				/* Time To Live*/
	unsigned char  	protocol;			/* ip protocol*/
	unsigned short 	checksum;			/* ip checksum*/
	unsigned char	sip[4];				/* source ip address*/
	unsigned char  	dip[4];				/* destination ip address*/
};

/*
* Move from wsm_main.c Used by sending message to ASD.
*
*/
typedef struct
{
	unsigned int Type;
	unsigned int WTPID;
	unsigned int Radio_G_ID;
	unsigned int BSSIndex;
	unsigned int DataLen;
} DataMsgHead;

struct vlan_h{
	unsigned short h_vlan_proto;				/* Should always be 0x8100 */
	unsigned short h_vlan_TCI;				/* Encapsulates priority and VLAN ID */
};
extern unsigned int wifi_tx_switch;
extern unsigned int wifi_rx_switch;
extern unsigned short wifi_8021q_type;//default 0x8100
extern unsigned int wifi_ipv6_dr_sw;
int kernel_to_asd(struct sk_buff *skb,unsigned int vrid);
int handle_data_msg(struct sk_buff *skb);
void CWCaptrue_wifi(int n ,unsigned char *buffer);


#endif
