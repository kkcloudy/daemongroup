#ifndef __PPPOE_SNOOPING_H__
#define __PPPOE_SNOOPING_H__

#include <linux/types.h>
#include <linux/skbuff.h>

#include "dba/dba.h"


/* PPPoE Tags */
#define TAG_END_OF_LIST        0x0000
#define TAG_SERVICE_NAME       0x0101
#define TAG_AC_NAME            0x0102
#define TAG_HOST_UNIQ          0x0103
#define TAG_AC_COOKIE          0x0104
#define TAG_VENDOR_SPECIFIC    0x0105
#define TAG_RELAY_SESSION_ID   0x0110
#define TAG_SERVICE_NAME_ERROR 0x0201
#define TAG_AC_SYSTEM_ERROR    0x0202
#define TAG_GENERIC_ERROR      0x0203

/*
 * PPP Protocol field values.
 */
#define PPP_IP		0x21	/* Internet Protocol */
#define PPP_AT		0x29	/* AppleTalk Protocol */
#define PPP_IPX		0x2b	/* IPX protocol */
#define PP_VJC_COMP	0x2d	/* VJ compressed TCP */
#define PP_VJC_UNCOMP	0x2f	/* VJ uncompressed TCP */
#define PPP_MP		0x3d	/* Multilink protocol */
#define PPP_IPV6	0x57	/* Internet Protocol Version 6 */
#define PPP_COMPFRAG	0xfb	/* fragment compressed below bundle */
#define PPP_COMP	0xfd	/* compressed packet */
#define PPP_MPLS_UC	0x0281	/* Multi Protocol Label Switching - Unicast */
#define PPP_MPLS_MC	0x0283	/* Multi Protocol Label Switching - Multicast */
#define PPP_IPCP	0x8021	/* IP Control Protocol */
#define PPP_ATCP	0x8029	/* AppleTalk Control Protocol */
#define PPP_IPXCP	0x802b	/* IPX Control Protocol */
#define PPP_IPV6CP	0x8057	/* IPv6 Control Protocol */
#define PPP_CCPFRAG	0x80fb	/* CCP at link level (below MP bundle) */
#define PPP_CCP		0x80fd	/* Compression Control Protocol */
#define PPP_MPLSCP	0x80fd	/* MPLS Control Protocol */
#define PPP_LCP		0xc021	/* Link Control Protocol */
#define PPP_PAP		0xc023	/* Password Authentication Protocol */
#define PPP_LQR		0xc025	/* Link Quality Report protocol */
#define PPP_CHAP	0xc223	/* Cryptographic Handshake Auth. Protocol */
#define PPP_CBCP	0xc029	/* Callback Control Protocol */


/* PPPoE Tag */
struct pppoe_tag {
	__u16 tag_type;
	__u16 tag_len;
	char tag_data[0];
} __attribute__ ((packed));


struct pppoe_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8 ver : 4;
	__u8 type : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8 type : 4;
	__u8 ver : 4;
#else
#endif
	__u8 code;
	__u16 sid;
	__u16 length;
	struct pppoe_tag tag[0];
	/*
    unsigned char payload[ETH_DATA_LEN];
	*/
} __attribute__ ((packed));


/*
  A summary of the Link Control Protocol packet format is shown below.
  RFC-1661
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Data ...
   +-+-+-+-+
*/
/* LCP CODE */
#define PPPoE_LCP_CFG_REQ		0x01		/* Configure-Request */
#define PPPoE_LCP_CFG_ACK		0X02		/* Configure-Ack */
#define PPPoE_LCP_CFG_NAK		0X03		/* Configure-Nak */
#define PPPoE_LCP_CFG_REJ		0X04		/* Configure-Reject */
#define PPPoE_LCP_TER_REQ		0X05		/* Terminate-Request */
#define PPPoE_LCP_TER_ACK		0X06		/* Terminate-Ack */
#define PPPoE_LCP_CODE_REJ		0X07		/* Code-Reject */
#define PPPoE_LCP_PROTO_REJ		0X08		/* Protocol-Reject */
#define PPPoE_LCP_ECHO_REQ		0X09		/* Echo-Request */
#define PPPoE_LCP_ECHO_REP		0X0A		/* Echo-Reply */
#define PPPoE_LCP_DIS_REQ		0X0B		/* Discard-Request */

struct pppoe_proto {
	__u8 code;
	__u8 id;		/* Identifier */
	__u16 length;	/* including the Code, Identifier, Length and Data fields */
} __attribute__ ((packed));



/* 
A summary of the Configuration Option format is shown below.
    0                   1
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |    Length     |    Data ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
 
/* pppoe LCP type */
#define LCP_OPT_RES		0       /* RESERVED */
#define LCP_OPT_MRU		1       /* Maximum-Receive-Unit */
#define LCP_OPT_AUTH	3       /* Authentication-Protocol */
#define LCP_OPT_QUAL	4		/* Quality-Protocol */
#define LCP_OPT_MAGIC	5       /* Magic-Number */
#define LCP_OPT_PFC		7       /* Protocol-Field-Compression */
#define LCP_OPT_ACFC	8       /* Address-and-Control-Field-Compression */

struct pppoe_option {
	__u8 type;
	__u8 len;		/* including the Type, Length and Data fields */
	char data[0];
} __attribute__ ((packed));


#define MRU_MIN			60
#define MRU_MAX			1492
#define MRU_DEFAULT		1408

/* Header size of a PPPoE packet */
#define PPPOE_OVERHEAD 6  /* type, code, session, length */
#define HDR_SIZE (sizeof(struct ethhdr) + PPPOE_OVERHEAD)
#define MAX_PPPOE_PAYLOAD (ETH_DATA_LEN - PPPOE_OVERHEAD)
#define MAX_PPPOE_MTU (MAX_PPPOE_PAYLOAD - 2)


int parse_pppoe_session(struct sk_buff *skb, struct pppoe_hdr *pppoe_hdr, dba_result_t *res);
#endif
