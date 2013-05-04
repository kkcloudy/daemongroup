
#ifndef __WIFI_TX_H__
#define __WIFI_TX_H__

#include "wifi.h"
#define CWSetField32(src,start,len,val)					src |= ((~(0xFFFFFFFF << len)) & val) << (32 - start - len)
#define CWGetField32(src,start,len)					((~(0xFFFFFFFF<<len)) & (src >> (32 - start - len)))
#define IEEE80211_RETRIEVE(dest, src, offset, len) memcpy((unsigned char*)(dest),((unsigned char*)(src)+offset), (len))
#define		CW_PROTOCOL_VERSION					0

#define 	CW_TRANSPORT_HEADER_VERSION_START			0
#define 	CW_TRANSPORT_HEADER_VERSION_LEN				4

#define		CW_TRANSPORT_HEADER_TYPE_START				4
#define		CW_TRANSPORT_HEADER_TYPE_LEN				4

// Radio ID number (for WTPs with multiple radios)
#define 	CW_TRANSPORT_HEADER_RID_START				13
#define 	CW_TRANSPORT_HEADER_RID_LEN				5

// Length of CAPWAP tunnel header in 4 byte words 
#define 	CW_TRANSPORT_HEADER_HLEN_START				8
#define 	CW_TRANSPORT_HEADER_HLEN_LEN				5

// Wireless Binding ID
#define 	CW_TRANSPORT_HEADER_WBID_START				18
#define 	CW_TRANSPORT_HEADER_WBID_LEN				5

// Format of the frame
#define 	CW_TRANSPORT_HEADER_T_START				23
#define 	CW_TRANSPORT_HEADER_T_LEN				1

// Is a fragment?
#define 	CW_TRANSPORT_HEADER_F_START				24
#define 	CW_TRANSPORT_HEADER_F_LEN				1

// Is NOT the last fragment?
#define 	CW_TRANSPORT_HEADER_L_START				25
#define 	CW_TRANSPORT_HEADER_L_LEN				1

// Is the Wireless optional header present?
#define 	CW_TRANSPORT_HEADER_W_START				26
#define 	CW_TRANSPORT_HEADER_W_LEN				1

// Is the Radio MAC Address optional field present?
#define 	CW_TRANSPORT_HEADER_M_START				27
#define 	CW_TRANSPORT_HEADER_M_LEN				1

// Is the message a keep alive?
#define 	CW_TRANSPORT_HEADER_K_START				28
#define 	CW_TRANSPORT_HEADER_K_LEN				1

// Set to 0 in this version of the protocol
#define 	CW_TRANSPORT_HEADER_FLAGS_START				29
#define 	CW_TRANSPORT_HEADER_FLAGS_LEN				3

// ID of the group of fragments
#define 	CW_TRANSPORT_HEADER_FRAGMENT_ID_START			0
#define 	CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN			16

// Position of this fragment in the group 
#define 	CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START		16
#define 	CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN			13

// Set to 0 in this version of the protocol
#define 	CW_TRANSPORT_HEADER_RESERVED_START			29
#define 	CW_TRANSPORT_HEADER_RESERVED_LEN			3

extern unsigned long long vm[VRID_MAX];
#define IEEE8023_DEST_MAC_START		0
#define IEEE8023_SRC_MAC_START			6
#define IEEE8023_MAC_LEN				6
#define IEEE8023_TYPE_START				12
#define IEEE8023_TYPE_LEN				2

struct ieee80211_frame {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
};
	
 struct ieee80211_qosframe {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
	unsigned char i_qos[2];
	unsigned char i_align[2];
};

struct ieee80211_llc {
 unsigned char llc_dsap;
 unsigned char llc_ssap;
 unsigned char llc_cmd;
 unsigned char llc_org_code[3];
 unsigned char llc_ether_type[2];
} ;

struct capwap_head {
 unsigned int capwap1;
 unsigned int capwap2;
 unsigned int capwap3;
 unsigned int capwap4;
} ;

struct capwap_head_802_3 {
 unsigned int capwap1;
 unsigned int capwap2;
 unsigned char maclen;
 unsigned char bssid[MAC_LEN];
 unsigned char rsv;
} ;
int wifi_assemble_capwap(struct sk_buff *skb, wifi_dev_private_t* priv,unsigned char dataflag);
int wifi_assemble_udp_ip(struct sk_buff *skb, wifi_dev_private_t* priv);
void skb_dump(char *name, struct sk_buff *skb);
int wifi_xmit(struct sk_buff *skb, struct net_device *dev);

#endif

