#ifndef _CAPWAP_H
#define _CAPWAP_H

#include "cvmx-config.h"
#include "cvmx.h"
#include "fwd_main.h"
#ifdef SDK_VERSION_2_2
#include "fastfwd-common-defs.h"
#else
#include "cvm-common-defs.h"
#endif


/* CAPWAP data tunnel relational ports */
#define CW_DAT_STD_PORT	5247
#define CW_DAT_TMP_PORT	8888
#define CW_DAT_AP_PORT	32769

/* CAPWAP control tunnel relational ports */
#define CW_CTL_STD_PORT	5246
#define CW_CTL_TMP_PORT	1234
#define CW_CTL_AP_PORT		32768

/* Inter-AC roaming port */
#define RAW_AC_PORT		8889
#define ROAM_AC_PORT		8890


#define IEEE80211_H_LEN						(cvm_common_roundup(sizeof(struct ieee80211_frame), 4))
#define IEEE80211_QOS_H_LEN				(cvm_common_roundup(sizeof(struct ieee80211_qosframe), 4))
#define IEEE80211_QOS_DIFF_LEN				((IEEE80211_QOS_H_LEN) - (IEEE80211_H_LEN))
#define LLC_H_LEN							(sizeof(struct ieee80211_llc))

#define IEEE80211_FC0_VERSION_MASK		0x03
#define IEEE80211_FC0_VERSION_0			0x00
#define IEEE80211_FC0_TYPE_MASK			0x0c
#define IEEE80211_FC0_TYPE_DATA			0x08
#define IEEE80211_FC0_SUBTYPE_QOS			0x80
#define IEEE80211_FC0_QOS_MASK			0x80

#define CW_FRAME_LEN		(ETH_H_LEN + IP_H_LEN + UDP_H_LEN + CW_H_LEN + \
		IEEE80211_H_LEN + LLC_H_LEN)
#define CW_802_3_FRAME_LEN	(ETH_H_LEN + IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN)

/* ICMP */
#define IP_IP_802_11_OFFSET       (IP_H_LEN + UDP_H_LEN + CW_H_LEN + IEEE80211_H_LEN + LLC_H_LEN)  /* From extern ip header to capwap 802.11 internal ip header */
#define IP_IP_802_11_QOS_OFFSET       (IP_H_LEN + UDP_H_LEN + CW_H_LEN + IEEE80211_QOS_H_LEN + LLC_H_LEN)  /* From extern ip header to capwap 802.11 internal ip header */
#define IP_IP_802_3_OFFSET        (IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN)  /* From extern ip header to capwap 802.3 internal ip header */

#define ETH_P_IP			0x0800
#define PPPOE_TYPE			0x8864
#define PPPOE_IP_TYPE		0x0021
#define PPPOE_H_LEN			8


/* IEEE802.11 relational structure */
struct ieee80211_frame {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
}__attribute__ ((packed));

struct ieee80211_qosframe {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
	unsigned char i_qos[2];
}__attribute__ ((packed));

struct ieee80211_frame_addr4 {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
	unsigned char i_addr4[MAC_LEN];
}__attribute__ ((packed));

struct ieee80211_qosframe_addr4 {
	unsigned char i_fc[2];
	unsigned char i_dur[2];
	unsigned char i_addr1[MAC_LEN];
	unsigned char i_addr2[MAC_LEN];
	unsigned char i_addr3[MAC_LEN];
	unsigned char i_seq[2];
	unsigned char i_addr4[MAC_LEN];
	unsigned char i_qos[2];
}__attribute__ ((packed));

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

struct ieee80211_llc {
	unsigned char llc_dsap;
	unsigned char llc_ssap;
	unsigned char llc_cmd;
	unsigned char llc_org_code[3];
	unsigned char llc_ether_type[2];
} __attribute__ ((packed));

union capwap_hd {
	struct {
		uint64_t dword_0_1;
		uint64_t dword_2_3;
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
}__attribute__ ((packed));

/* valid_bit */
#define CAPWAP_IS_EMPTY    0

#define MAX_CAPWAP_CACHE_NUM 8192   /*8K Capwap ËíµÀ*/
#define CAPWAP_CACHE_TBL_NAME "capwap_cache_tbl"
typedef struct  capwap_cache_tbl_s
{
	/* how many rules use this cache, add by zhaohan */
	int32_t use_num;

	/* Extern IP header */
	uint32_t dip;
	uint32_t sip;
	uint16_t dport;
	uint16_t sport;
	uint8_t tos;

	/*current entry is valid or not*/
	/*uint8_t    valid_bit;	*/

	/* CAPWAP header */
	uint8_t cw_hd[CW_H_LEN];
} capwap_cache_t ;


/**
 * Description:
 *  Get the type of capwap packet.
 *
 * Parameter:
 *  cw_h: CAPWAP header point.
 *
 * Return:
 *  PACKET_TYPE_CAPWAP_802_3
 *  PACKET_TYPE_CAPWAP_802_11
 *  PACKET_TYPE_UNKNOW
 *
 */
static inline int8_t get_capwap_type(union capwap_hd * cw_h)
{
	if( 0 == cw_h->m_t )
	{
		return PACKET_TYPE_CAPWAP_802_3;
	}

	if( 1 == cw_h->m_t )
	{
		if( 1 == cw_h->m_wbid )
		{
			return PACKET_TYPE_CAPWAP_802_11;
		}
	}
	return PACKET_TYPE_UNKNOW;
}

/**
 * Description:
 *  Decap rx udp packet, skip special udp packet.
 *
 * Parameter:
 *  uh: External UDP header
 *
 * Return:
 *  0: Successfully
 * -1: Failed
 *
 */
static inline int8_t  rx_udp_decap(cvm_common_udp_hdr_t *uh)
{
	/* CAPWAP data packet */
	if ((uh->uh_sport == CW_DAT_AP_PORT) && 
			((uh->uh_dport == CW_DAT_STD_PORT) || 
			 (uh->uh_dport == CW_DAT_TMP_PORT)))
	{
		return get_capwap_type((union capwap_hd *)((char *)uh + UDP_H_LEN));
	}
	else if ((uh->uh_sport == CW_CTL_AP_PORT) && 
			((uh->uh_dport == CW_CTL_STD_PORT) ||
			 (uh->uh_dport == CW_CTL_TMP_PORT)))
	{
		return PACKET_TYPE_UNKNOW;
	}

	/* Inter-AC roaming packet */
	else if ((uh->uh_sport == RAW_AC_PORT && uh->uh_dport == RAW_AC_PORT) ||
			(uh->uh_sport == ROAM_AC_PORT && uh->uh_dport == ROAM_AC_PORT))
	{
		return PACKET_TYPE_UNKNOW;
	}
	/* Common UDP packet */
	else
	{
		return PACKET_TYPE_ETH_IP;
	}
}

#endif
