#ifndef __NPD_DHCP_SNP_PKT_H__
#define __NPD_DHCP_SNP_PKT_H__
#include "dhcp_snp_tbl.h"
#include "dhcp_snp_com.h"
#include "dhcp_snp_options.h"

/*********************************************************
*	macro define													*
**********************************************************/
#define NPD_DHCP_SNP_REQUEST_TIMEOUT                 (60)
/* ip neigh command */
#define DHCPSNP_RTNL_CMD_IPNEIGH_ADD	RTM_NEWNEIGH
#define DHCPSNP_RTNL_CMD_IPNEIGH_DEL	RTM_DELNEIGH

#define DHCP_SNP_TRUE	1
#define DHCP_SNP_FALSE	0
	
#define ETH_ALEN 		 6
#define IP_ADDR_LEN     4

#define ETHER_ARP	0x0806
#define ETHER_IP	0x0800

#define IPVER4		0x4
#define IPVER6		0x6

#define IPPROTOCOL_TCP	0x6
#define IPPROTOCOL_UDP	0x11

#define DHCP_SERVER_PORT	0x43
#define DHCP_CLIENT_PORT	0x44

#define ARP_OPCODE_REQUEST	0x1
#define ARP_OPCODE_REPLY	0x2


/* 127.0.0.0 */
#define DHCP_SNP_LOOPBACK(x)	(((x) & htonl(0xff000000)) == htonl(0x7f000000))
/* 224.x.x.x - 239.255.255.255 */
#define DHCP_SNP_MULTICAST(x)	(((x) & htonl(0xf0000000)) == htonl(0xe0000000))
/* 240.x.x.x */
#define DHCP_SNP_BADCLASS(x)	(((x) & htonl(0xf0000000)) == htonl(0xf0000000))
/* 0.x.x.x */
#define DHCP_SNP_ZERONET(x)	(((x) & htonl(0xff000000)) == htonl(0x00000000))

/* 169.254.x.x */
#define DHCP_SNP_PRIVATE_NET(x)	(((x) & htonl(0xFFFF0000)) == htonl(0xA9FE0000))


/*********************************************************
*	struct define													*
**********************************************************/
	
typedef struct	
{
	unsigned char		dmac[ETH_ALEN]; 	/* destination eth addr */
	unsigned char		smac[ETH_ALEN]; 	/* source ether addr	*/
	unsigned short		etherType;
}ether_header_t;

/* ethenet ARP packet*/
typedef struct {
	unsigned short 	hwType;				/*hardware type: 0x0001-ethernet*/
	unsigned short 	protocol;			/* protocol type:0x0800-IP*/
	unsigned char 	hwSize;				/* hardware size*/
	unsigned char 	protSize;			/*protocol size*/
	unsigned short 	opCode;				/* 0x0001-request 0x0002-reply*/
	unsigned char  	smac[ETH_ALEN];	/* sender's MAC address*/
	unsigned char 	sip[IP_ADDR_LEN]; 	/* sender's ip address*/
	unsigned char 	dmac[ETH_ALEN];	/* target's MAC address*/
	unsigned char 	dip[IP_ADDR_LEN];	/* target's ip address*/
}arp_packet_t;

typedef struct
{
	unsigned char	   version:4;
	unsigned char	   hdrLength:4;
	unsigned char	   dscp:6;
	unsigned char	   ecn:2;
	unsigned short	   totalLen;
	unsigned short	   identifier;
	unsigned short	   flag:3;
	unsigned short	   fragOffset:13;
	unsigned char	   ttl;
	unsigned char	   ipProtocol;
	unsigned short	   checkSum;
	unsigned char	   sip[IP_ADDR_LEN];
	unsigned char	   dip[IP_ADDR_LEN];
}ip_header_t;

typedef struct 
{
	unsigned short source;
	unsigned short	dest;
	unsigned short len;
	unsigned short check;
}udp_header_t;


/*********************************************************
*	function declare												*
**********************************************************/


/**********************************************************************************
 *dhcp_snp_get_item_from_pkt()
 *
 *	DESCRIPTION:
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex
 *		NPD_DHCP_MESSAGE_T *packet
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL
 *
 ***********************************************************************************/
unsigned int dhcp_snp_get_item_from_pkt
(
	unsigned short vlanid,
	unsigned int ifindex,
	enum dhcp_packet_type type,
	NPD_DHCP_MESSAGE_T *packet,
	NPD_DHCP_SNP_USER_ITEM_T *user
);

/**********************************************************************************
 *dhcp_snp_discovery_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_discovery_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
);

/**********************************************************************************
 *dhcp_snp_offer_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 ***********************************************************************************/
#if 0
unsigned int dhcp_snp_offer_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
);
#endif
unsigned int dhcp_snp_offer_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp, 
	unsigned char * dhcp_buffr, 
	unsigned long buffr_len,
	int fd
);

/**********************************************************************************
 *dhcp_snp_request_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
  *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_request_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
);

/**********************************************************************************
 *dhcp_snp_ack_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *		struct dhcp_snp_listener *node
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_ack_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node	
);

/**********************************************************************************
 *dhcp_snp_nack_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_nack_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
);

/**********************************************************************************
 *dhcp_snp_release_process()
 *
 *	DESCRIPTION:
 *		release DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_release_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node	
);

/**********************************************************************************
 *dhcp_snp_inform_process()
 *
 *	DESCRIPTION:
 *		DHCP Inform packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_inform_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node	
);

unsigned short dhcp_snp_checksum
(
	void *addr,
	int count
);

/********************************************************************************************
 * 	dhcp_snp_packet_type_is_mc
 *
 *	DESCRIPTION:
 *             This function check out whether the packet is mcast or not.
 *
 *	INPUT:
 *             packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *               NONE
 *	RETURNS:
 *              DHCP_SNP_TRUE - indicate the packet is mcast packet
 *              DHCP_SNP_FALSE - indicate the packet is not mcast packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned long	dhcp_snp_packet_type_is_mc
(       
	unsigned char  *packetBuff
);

/**********************************************************************************
 *dhcp_snp_gratuitous_arp_process()
 *
 *	DESCRIPTION:
 *		Gratuitous ARP packet receive
 *
 *	INPUTS:
 *		vlanid - vlan id
 *		ifindex - interface ifindex
 *		packet  - packet buffer
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_gratuitous_arp_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	char *packet,
	struct dhcp_snp_listener *node	
);

unsigned int dhcp_snp_arp_request_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	char *packet,
	int fd
);


/**********************************************************************************
 *  dhcpsnp_notify_to_protal
 *
 *	DESCRIPTION:
 * 		when receive ACK, notify protal client IP and MAC
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0   ->  success
 *		-1  ->  failed
 *		
 **********************************************************************************/
int dhcp_snp_notify_to_protal(uint32_t userip, uint8_t *usermac);
/******************************************************************************
 * dhcp_snp_u32ip_check
 *  check u32 ip address
 *	INPUT:
 *		ipaddr		- u32 IPv4 address
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		0	-		valid
 *		-1	-		invalid ipv4 address
 ******************************************************************************/
int dhcp_snp_u32ip_check
(
	unsigned int ipaddr
);

/*********************************************************
*	extern Functions												*
**********************************************************/

#endif

