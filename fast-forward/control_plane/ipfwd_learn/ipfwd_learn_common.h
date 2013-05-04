#ifndef _IPFWD_LEARN_COMMON_H_
#define _IPFWD_LEARN_COMMON_H_

#include <linux/version.h>
#include <linux/kernel.h>
#include <asm/unistd.h>


/* Log level */
#define EMERG_LVL			0
#define ALERT_LVL			1
#define CRIT_LVL			2
#define ERR_LVL				3
#define WARNING_LVL		4
#define NOTICE_LVL			5
#define INFO_LVL			6
#define ICMP_DEBUG			7
#define DEBUG_LVL			8
#define DEBUG_IPFWD_LVL	    DEBUG_LVL

#define IPFWD_LEARN_RETURN_OK    0
#define IPFWD_LEARN_RETURN_FAIL -1

#ifndef BUILDNO_VERSION2_1
#define __packed			__attribute__ ((packed))
#define roundup(x, y)		((((x)+((y)-1))/(y))*(y))  /* byte align */
#endif

#define FUNC_DISABLE 0
#define FUNC_ENABLE  1

/* base define */
#define MAC_LEN				6
#define ETH_H_LEN			14
#define ETH_T_LEN			2
#define VLAN_PROTO_LEN		2
#define VLAN_HLEN			4
#define IP_H_LEN			20
#define UDP_H_LEN			8
#define CW_H_LEN			16

#define IP_PROTOCOL_ICMP            0x1
#define IP_PROTOCOL_TCP             0x6
#define IP_PROTOCOL_UDP             0x11

#define ETH_P_IP			0x0800

/* RPA relational */
#define PPPOE_TYPE			0x8864
#define PPPOE_IP_TYPE		0x0021
#define PPPOE_H_LEN			8


#define PROTO_STR(t)  ((t) == 0x6 ? "TCP" : ((t) == 0x11 ? "UDP" : ((t) == 0x1 ? "ICMP" : "Unknown")))


/* ethernet headers*/
typedef struct eth_hdr_s {
    uint8_t	 h_dest[6];	   /*destination eth addr */
    uint8_t  h_source[6];	   /* source ether addr	*/
    uint16_t h_vlan_proto;              /* Should always be 0x8100 */
}eth_hdr_t;

typedef struct vlan_eth_hdr_s {
    uint8_t h_dest[6];
    uint8_t h_source[6];
    uint16_t        h_vlan_proto;
    uint16_t        h_vlan_TCI;
    uint16_t        h_eth_type;
}vlan_eth_hdr_t;


#define IP_FMT(m)	\
				((uint8_t*)&(m))[0], \
				((uint8_t*)&(m))[1], \
				((uint8_t*)&(m))[2], \
				((uint8_t*)&(m))[3]

#define MAC_FMT_DEFINE(m)  \
				((uint8_t*)(m))[0], \
				((uint8_t*)(m))[1], \
				((uint8_t*)(m))[2], \
				((uint8_t*)(m))[3], \
				((uint8_t*)(m))[4], \
				((uint8_t*)(m))[5]
				

/* RPA relational */
#define RPA_ENET_ETHER_ADDR_LEN   6
#define RPA_COOKIE            0x8fff
#define IS_RPA                0x1234
#define IS_VLAN_RPA           0x1235
#define NOT_RPA               0x4567
#define RPA_HEAD_LEN(m)     (((m)==IS_RPA) ? 18:22)
#define IS_RPA_PKT(m) (((m)==IS_RPA) || ((m)==IS_VLAN_RPA))

typedef struct {
	unsigned char  ether_dhost[RPA_ENET_ETHER_ADDR_LEN];
	unsigned char  ether_shost[RPA_ENET_ETHER_ADDR_LEN];
    unsigned short  cookie;
	unsigned char  type; 
	unsigned char  dnetdevNum;
	unsigned char  snetdevNum;
	unsigned char  d_s_slotNum; 
}rpa_eth_head_t;

typedef struct {
	unsigned char  ether_dhost[RPA_ENET_ETHER_ADDR_LEN];
	unsigned char  ether_shost[RPA_ENET_ETHER_ADDR_LEN];
	unsigned short vlan_type;
	unsigned short vlan_tag;
    unsigned short cookie;
	unsigned char  type; 
	unsigned char  dnetdevNum;
	unsigned char  snetdevNum;
	unsigned char  d_s_slotNum; /* dst port and src slot number */
}rpa_vlan_eth_head_t;


/* IEEE802.11 relational structure */
struct ieee80211_llc {
	unsigned char llc_dsap;
	unsigned char llc_ssap;
	unsigned char llc_cmd;
	unsigned char llc_org_code[3];
	unsigned char llc_ether_type[2];
} __packed;

#define LLC_H_LEN   (sizeof(struct ieee80211_llc))


/* IEEE802.11 relational structure */
struct ieee80211_frame {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
}__packed;

struct ieee80211_qosframe {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
	unsigned char i_qos[2];
}__packed;
 
struct ieee80211_frame_addr4 {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
	unsigned char i_addr4[MAC_LEN];
}__packed;

struct ieee80211_qosframe_addr4 {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
	unsigned char i_addr4[MAC_LEN];
	unsigned char i_qos[2];
}__packed;

struct ieee80211_fc_field{
	unsigned char fc_proto_ver;
	unsigned char fc_type;
	unsigned char fc_subtype;
	unsigned char fc_ds;
	unsigned char fc_frag;
	unsigned char fc_retry;
	unsigned char fc_pwr;
	unsigned char fc_moredata;
	unsigned char fc_protect;
	unsigned char fc_order;
};

#define IEEE80211_FC0_VERSION_MASK		0x03
#define IEEE80211_FC0_VERSION_0			0x00
#define IEEE80211_FC0_TYPE_MASK			0x0c
#define IEEE80211_FC0_TYPE_DATA			0x08
#define IEEE80211_FC0_SUBTYPE_QOS		    0x80
#define IEEE80211_FC0_QOS_MASK			0x80

#define IEEE80211_H_LEN						(roundup(sizeof(struct ieee80211_frame), 4))
#define IEEE80211_QOS_H_LEN				(roundup(sizeof(struct ieee80211_qosframe), 4))
#define IEEE80211_QOS_DIFF_LEN				((IEEE80211_QOS_H_LEN) - (IEEE80211_H_LEN))


/* CAPWAP  relational */
union capwap_hd {
	struct {
		uint32_t dword0;
		uint32_t dword1;
		uint32_t dword2;
		uint32_t dword3;
	} u;
#define m_dword0 u.dword0
#define m_dword1 u.dword1
#define m_dword2 u.dword2
#define m_dword3 u.dword3

	struct {
		/* dword0 */
		uint32_t pmb:8;
		uint32_t hlen:5;
		uint32_t rid:5;
		uint32_t wbid:5;
		uint32_t t:1;
		uint32_t f:1;
		uint32_t l:1;
		uint32_t w:1;
		uint32_t m:1;
		uint32_t k:1;
		uint32_t flags:3;

		/* dword1 */
		uint16_t fragid;
		uint16_t offset:13;
		uint16_t rsvd0:3;

		/* dword2 */
		uint8_t wlanid;
		uint8_t rsvd1;
		uint16_t rsvd2;

		/* dword3 */
		uint32_t rsvd3;
	} s;
#define m_pmb s.pmb
#define m_hlen s.hlen
#define m_rid s.rid
#define m_wbid s.wbid
#define m_t s.t
#define m_f s.f
#define m_l s.l
#define m_w s.w
#define m_m s.m
#define m_k s.k
#define m_flags s.flags
#define m_fragid s.fragid
#define m_offset s.offset
#define m_wlanid s.wlanid
}__packed;


/* CAPWAP control tunnel relational ports */
#define CW_CTL_STD_PORT	5246
#define CW_CTL_TMP_PORT	1234
#define CW_CTL_AP_PORT		32768

/* Inter-AC roaming port */
#define RAW_AC_PORT		8889
#define ROAM_AC_PORT		8890

/*dhcp server or clien port*/
#define DHCP_SERVER_PORT  67
#define DHCP_CLIEN_PORT   68

#define CW_DAT_STD_PORT	5247
#define CW_DAT_TMP_PORT	8888
#define CW_DAT_AP_PORT	32769


#define IP_IP_802_11_OFFSET       (IP_H_LEN + UDP_H_LEN + CW_H_LEN + IEEE80211_H_LEN + LLC_H_LEN)  /* From extern ip header to capwap 802.11 internal ip header */
#define IP_IP_802_3_OFFSET        (IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN)  /* From extern ip header to capwap 802.3 internal ip header */
#define CW_FRAME_LEN		(ETH_H_LEN + IP_H_LEN + UDP_H_LEN + CW_H_LEN + \
							IEEE80211_H_LEN + LLC_H_LEN)
#define CW_802_3_FRAME_LEN	(ETH_H_LEN + IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN)




/* Stream type */
#define TCP_TYPE            1
#define UDP_TYPE            2
#define CW_TYPE             3
#define ETH_TYPE            4
#define CW_802_3_STREAM     5
#define CW_802_11_STREAM    CW_TYPE
#define ETH_ICMP            6
#define CW_802_3_ICMP       7
#define CW_802_11_ICMP      8
#define NOT_ICMP            9
#define CW_UNKNOWN_STREAM   -1


/* Special input interface */
#define SPE_IN_IF(m)		( ((m)[0] == 'v') || ((m)[0] == 'w') || ((m)[0] == 'r') || \
	                           ((m)[0] == 'b') || (((m)[0] == 'e') && ((m)[1] == 'b')))

#define SPE_IN_IF_86(m)		( ((m)[0] == 'w') || ((m)[0] == 'r') || ((m)[0] == 'b') || \
	                           (((m)[0] == 'e') && ((m)[1] == 'b')))

/* Special output interface */
#define SPE_OT_IF(m) ((m)[6] == '.')

/* obc device */		
#define IS_OBC_DEV(m)  ( ((m)[0] == 'o') && ((m)[1] == 'b') && ((m)[2] == 'c') )


/* Special IP Header */
#define SPE_IP_HDR(m) 	(((m)->version != 4) || ((m)->ihl != 5) || \
						(((m)->frag_off & 0x1fff) != 0) || \
						(((m)->protocol != IP_PROTOCOL_TCP) && \
						  ((m)->protocol != IP_PROTOCOL_UDP) && \
						  ((m)->protocol != IP_PROTOCOL_ICMP)))

/* TCP control frame */
#define SPE_TCP_HDR(t)	(((t)->syn) || ((t)->fin) || ((t)->rst))

#ifdef BUILDNO_VERSION2_1
/* Some random defines to make it easier in the kernel.. */
#define LOOPBACK(x)	(((x) & htonl(0xff000000)) == htonl(0x7f000000))
#define MULTICAST(x)	(((x) & htonl(0xf0000000)) == htonl(0xe0000000))
#define BADCLASS(x)	(((x) & htonl(0xf0000000)) == htonl(0xf0000000))
#define ZERONET(x)	(((x) & htonl(0xff000000)) == htonl(0x00000000))
#define LOCAL_MCAST(x)	(((x) & htonl(0xFFFFFF00)) == htonl(0xE0000000))
#endif
#define IP_IN_MANAGE(x)	(((x) & htonl(0xffff0000)) == htonl(0xA9FE0000)) /* 169.254.0.0 */

/* Special IP address */
#define SPE_IP_ADDR(s,d) (MULTICAST((d)) || BADCLASS((d)) || \
					ZERONET((d)) || BADCLASS((s)) || ZERONET((s)) || \
					IP_IN_MANAGE((s)) || IP_IN_MANAGE((d))) 	/* filter 169.254.0.0 */

/*special udp port ,such as dhcp server port,dhcp clien port*/
#define SPE_UDP_PORT(m) 	(((m)->source == DHCP_SERVER_PORT) || ((m)->dest == DHCP_SERVER_PORT) || \
                                ((m)->source == DHCP_CLIEN_PORT) || ((m)->dest == DHCP_CLIEN_PORT))
        


/*
   IP forwarding log print function
 */
extern int log_level;
#define log(LVL, format, args...) 	\
	do {		\
		if (unlikely(LVL <= log_level)){		\
			switch(LVL) {	\
				case EMERG_LVL: \
						printk(KERN_EMERG format, ##args);	\
				break;	\
				case ALERT_LVL:	\
						printk(KERN_ALERT format, ##args);	\
				break;	\
				case CRIT_LVL:	\
						printk(KERN_CRIT format, ##args);		\
				break;	\
				case ERR_LVL:	\
						printk(KERN_ERR format, ##args);		\
				break;	\
				case WARNING_LVL:	\
							printk(KERN_WARNING format, ##args);	\
				break;	\
				case NOTICE_LVL:	\
							printk(KERN_NOTICE  format, ##args);	\
				break;	\
				case INFO_LVL:	\
						printk(KERN_INFO  format, ##args);	\
				break;	\
				case ICMP_DEBUG:	\
							printk(KERN_DEBUG format, ##args);	\
				break;	\
				case DEBUG_LVL:	\
						printk(KERN_DEBUG format, ##args);	\
				break;	\
				default:	\
						break;	\
			}	\
		}	\
	} while(0)


#endif
