#ifndef _FASTFWD_H
#define _FASTFWD_H

#include "acl.h"
#include "capwap.h"

#define MEM_AUTO_FREE  		0
#define MEM_MANUAL_FREE 	1

#define PACKET_DSA_HEADER_LEN				8

#define	IP_IN_MANAGE(x)		(((uint32_t)(x) & 0xffff0000) == 0xA9FE0000) /* 169.254.0.0 */

/* Special IP address */
#define SPE_IP_ADDR(s,d) (CVM_IP_IN_MULTICAST((d)) || CVM_IP_IN_BADCLASS((d)) || \
						CVM_IP_IN_ZERONET((s)))

/* TCP control frame */
#define SPE_TCP_HDR(t)	(((t)->th_flags & CVM_COMMON_TCP_TH_SYN ) ||\
							((t)->th_flags & CVM_COMMON_TCP_TH_RST) || ((t)->th_flags & CVM_COMMON_TCP_TH_FIN))

/* Special IP Header */
#define SPE_IP_HDR(m) 	(((m)->ip_v != 4) || ((m)->ip_hl != 5) || \
	                      (((m)->ip_p != CVM_COMMON_IPPROTO_TCP) && \
						  ((m)->ip_p != CVM_COMMON_IPPROTO_UDP) && \
						  ((m)->ip_p != CVM_COMMON_IPPROTO_ICMP)))


#define FRAG_IP_PKT(m)   (((m)->ip_off & 0x3fff) != 0)
#define FRAGMENT_MORE(m)                (((m) & 0x2000) == 0x2000)
#define FRAGMENT_OFFSET(m)              ((m) & 0x1fff)
#define IS_PCI_PKT(work)    ((work)->word2.s.software)


#define PORTAL_PORT          2000
#define ACCESS_RADUIS_PORT   1812
#define ACCOUNT_RADUIS_PORT  1813
#define TELNET_PORT          23
#define SSH_PORT             22
#define RIP_PORT             520
#define DHCP_CLIENT_PORT     67
#define DHCP_SERVER_PORT     68

#define ETH_T_VRRP           0x70
#define ETH_T_8021X          0x888e
#define PACKET_TYPE_ETH_IP				1
#define PACKET_TYPE_CAPWAP_802_3		2
#define PACKET_TYPE_CAPWAP_802_11	3
#define PACKET_TYPE_ICMP 4
#define PACKET_TYPE_UNKNOW			-1

#define SE_MAGIC_NUM      0x12345678
#define FPA_INIT_WAIT   1
#define FPA_INIT_OK     2
#define IPD_EN_WAIT   3
#define IPD_EN_OK         4


/* set qos RED threshold. */
#define PACKET_DROP_PIP_HIGH	1000
#define PACKET_DROP_PIP_LOW	500

#define PACKET_DROP_PIP_QOS0_LOW    100
#define PACKET_DROP_PIP_QOS0_HIGH   300
#define PACKET_DROP_PIP_QOS1_LOW    300
#define PACKET_DROP_PIP_QOS1_HIGH   500
#define PACKET_DROP_PIP_QOS2_LOW    500
#define PACKET_DROP_PIP_QOS2_HIGH   700
#define PACKET_DROP_PIP_QOS3_LOW    700
#define PACKET_DROP_PIP_QOS3_HIGH   900
#define PACKET_DROP_PIP_QOS4_LOW    900
#define PACKET_DROP_PIP_QOS4_HIGH   1100
#define PACKET_DROP_PIP_QOS5_LOW    1100
#define PACKET_DROP_PIP_QOS5_HIGH   1300
#define PACKET_DROP_PIP_QOS6_LOW    1300
#define PACKET_DROP_PIP_QOS6_HIGH   1500
#define PACKET_DROP_PIP_QOS7_LOW    1500
#define PACKET_DROP_PIP_QOS7_HIGH   1700









/* abstract virtual port communication to control plane */
typedef enum
{
    VIRTUAL_PORT_INVALID,
    VIRTUAL_PORT_POW,
    VIRTUAL_PORT_PCIE,
    VIRTUAL_PORT_XAUI
}virtual_port_type;

typedef struct fwd_virtual_port_ops_s
{
	//int32_t    (*fwd_virtual_port_init)(cvmx_wqe_t* work);
	void    (*vp_pkt_to_control)(cvmx_wqe_t* work);
	void    (*vp_fccp_to_control)(cvmx_wqe_t* work);
}fwd_virtual_port_ops_t;  

typedef struct fwd_virtual_port_s
{
    uint32_t port_type;
    fwd_virtual_port_ops_t ops;
}fwd_virtual_port_t;

int32_t disable_fastfwd();
int32_t enable_fastfwd();
#endif

