#ifndef _DHCP_SNP_LISTENER_H_
#define _DHCP_SNP_LISTENER_H_
#include <util/npd_list.h>
#include <sys/select.h>

#undef IF_NAMESIZE
#define IF_NAMESIZE 16

#define DHCP_SNP_PACKET_SIZE	1536

#define DHCP_SNP_SERVER_PORT	67
/*
 * dhcp snooping listener socket
 */
struct dhcp_snp_listener {
	struct list_head list;
	int fd;
	int no_arp;
	short ifflags;
	char ifname[IF_NAMESIZE];
};

/*
 * socket list
 */
struct dhcp_snp_sock {
	struct list_head sock_list;
	int count;
};

/*
 * packet listener operation type
 */
enum dhcpsnp_listener_op {
	DHCPSNP_LSTNER_ADD_E = 0,
	DHCPSNP_LSTNER_DEL_E,
	DHCPSNP_LSTNER_UPDATE_E,
	DHCPSNP_LSTNER_QUERY_E,	
	DHCPSNP_LSTNER_SET_NO_ARP,
	DHCPSNP_LSTNER_MAX
};

#define DHCPSNP_LSTNER_OPTYPE_DESC(op)	\
	(DHCPSNP_LSTNER_ADD_E == op) ? "add" : \
	((DHCPSNP_LSTNER_DEL_E == op) ? "delete" : \
	 ((DHCPSNP_LSTNER_UPDATE_E == op) ? "update" : \
	  ((DHCPSNP_LSTNER_QUERY_E == op) ? "query" : "malop")))

#define WAIT_FOREVER_BEGIN \
	while(1) {

#define WAIT_FOREVER_END	}

/*
 * anti arp spoofing interface node
 */
struct anti_arp_spoof_node {
	struct list_head list;
	char ifname[IF_NAMESIZE];
};

/*
 * interface list
 */
struct anti_arp_spoof_info {
	struct list_head intf_list;
	int count;
};

/*
 * dhcp ebtables operation type
 */
enum dhcpsnp_ebt_op {
	DHCPSNP_EBT_ADD_E = 0, 
	DHCPSNP_EBT_DEL_E,
	DHCPSNP_EBT_MAX
};

/**********************************************************************************
 * dhcp_snp_listener_init
 *	Initialize DHCP snooping packet socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_init(void);

/**********************************************************************************
 * dhcp_snp_listener_handle
 *	Handle DHCP snooping packet listener add/delete/update operation
 *
 *	INPUT:
 *		listener - packet listener
 *		op_type - operation type( 0 - add, 1 - delete, 2 - update)
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_handle
(
	struct dhcp_snp_listener *listener,
	enum dhcpsnp_listener_op op_type
);

/**********************************************************************************
 * dhcp_snp_listener_init
 *	Initialize DHCP snooping packet socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_open
(
	char *ifname
);

/**********************************************************************************
 * dhcp_snp_listener_unware
 *	Release DHCP snooping packet socket and stop packet listener
 *
 *	INPUT:
 *		ifname - interface to release packet listener
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_close
(
	char *ifname
);

/********************************************************************************************
 * 	dhcp_snp_listener_query
 *
 *	DESCRIPTION:
 *             This function check out whether dhcp snooping is enabled on interface or not.
 *
 *	INPUT:
 *             ifname - interface name
 *
 *	OUTPUT:
 *               NONE
 *
 *	RETURNS:
 *              GT_TRUE - indicate the packet is IPv4 packet
 *              GT_FALSE - indicate the packet is not IPv4 packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned int	dhcp_snp_listener_query
(       
	unsigned char  *ifname
);

/**********************************************************************************
 * dhcp_snp_listener_save_cfg
 *	Handle DHCP snooping packet listener show running-config
 *
 *	INPUT:
 *		listener - packet listener
 *		op_type - operation type( 0 - add, 1 - delete, 2 - update)
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_PARAM_NULL - if show string buffer null
 **********************************************************************************/
int dhcp_snp_listener_save_cfg
(
	unsigned char *buffer,
	unsigned int length
);

/********************************************************************************************
 * 	nam_packet_type_is_Dhcp
 *
 *	DESCRIPTION:
 *             This function check out whether the packet is dhcp or not.
 *
 *	INPUT:
 *             packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *               NONE
 *	RETURNS:
 *              GT_TRUE - indicate the packet is IPv4 packet
 *              GT_FALSE - indicate the packet is not IPv4 packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned char	dhcp_snp_listener_type_check
(       
	unsigned char  *packetBuff
);

/********************************************************************************************
 * 	dhcp_snp_listener_gratuitous_arp_check
 *
 *	DESCRIPTION:
 *             This function check out whether the packet is gratuitous arp or not.
 *
 *	INPUT:
 *             packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *               NONE
 *	RETURNS:
 *              DHCP_SNP_TRUE - indicate the packet is gratuitous packet
 *              DHCP_SNP_FALSE - indicate the packet is not gratuitous packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned char	dhcp_snp_listener_gratuitous_arp_check
(       
	unsigned char  *packetBuff
);

/********************************************************************************************
 * 	dhcp_snp_dump_packet_detail
 *
 *	DESCRIPTION:
 * 		This function dump all bits of packet buffer 
 *
 *	INPUT:
 *		buffer - points to the packet's header
 *		bufflen - buffer length
 *		
 *	OUTPUT:
 * 		NULL
 *
 *	RETURNS:
 *		NULL
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
void dhcp_snp_listener_dump_detail
(
	unsigned char *buffer,
	unsigned long buffLen
);

/**********************************************************************************
 * dhcp_snp_socket_init
 *	Scan socket list to update FD_SET
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_socket_init
(
	void
);

/**********************************************************************************
 * dhcp_snp_socket_global_enable
 *	Scan socket list to global enable socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_socket_global_enable
(
	void
);

/**********************************************************************************
 * dhcp_snp_socket_init
 *	Scan socket list to update FD_SET
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_socket_global_disable
(
	void
);

/**********************************************************************************
 * dhcp_snp_listener_read_sock
 *	Scan socket list to verify if packet arrived
 *
 *	INPUT:
 *		fds - FD set
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_read_sock
(
	fd_set fds
);

/**********************************************************************************
 * dhcp_snp_listener_thread_main
 *	Main routine for packet listener handle thread
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
void * dhcp_snp_listener_thread_main
(
	void *arg
);

/**********************************************************************************
 * anti_arp_spoof_init
 *	Initialize anti ARP spoof interface list
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int anti_arp_spoof_init(void);

/**********************************************************************************
 * dhcp_snp_handle_intf_ebtables
 *		set DHCP_Snooping enable/disable interface ebtables
 *
 *	INPUT:
 *		ifname - interface name
 *		isEnable - enable or disable flag
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_handle_intf_ebtables
(
	char *ifname,
	unsigned char isEnable
);

/**********************************************************************************
 * dhcp_snp_listener_handle_ebr_ebtables
 *	Handle ebr interface to add ebtables rules as follows:
 *	  1 - permit all dhcp packet from all interfaces under ebr
 *		(rules: ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 67 --ip-sport 68 -j ACCEPT
 				ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 68 --ip-sport 67 -j ACCEPT)
 *	  2 - deny all other packets
 *		(rules: ebtabls -A -i ethx-y -j DROP)
 *
 *	INPUT:
 *		ebrname - bridge interface name
 *		op - add or delete
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
int dhcp_snp_listener_handle_ebr_ebtables
(
   	char *ebrname,
	enum dhcpsnp_ebt_op op
);

/**********************************************************************************
 * dhcp_snp_listener_handle_intf_ebtables
 *	Handle interface to add ebtables rules as follows:
 *	  1 - permit all dhcp packet from all interfaces under ebr
 *		(rules: ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 67 --ip-sport 68 -j ACCEPT
 				ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 68 --ip-sport 67 -j ACCEPT)
 *	  2 - deny all other packets
 *		(rules: ebtabls -A -i ethx-y -j DROP)
 *
 *	INPUT:
 *		ifname - interface(except ebr) name
 *		op - add or delete
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
int dhcp_snp_listener_handle_intf_ebtables
(
   	char *ifname,
	enum dhcpsnp_ebt_op op
);

/**********************************************************************************
 * dhcp_snp_listener_handle_host_ebtables
 *	Handle host ebtables rules as follows:
 *	  1 - permit all packet from all interfaces under ebr
 *		(rules: ebtables -I/-D FORWARD -s xx:xx:xx:xx:xx:xx -p ip --ip-src x.x.x.x -j ACCEPT
 *			   ebtables -I/-D FORWARD -d xx:xx:xx:xx:xx:xx -p ip --ip-dst x.x.x.x -j ACCEPT)
 *
 *	INPUT:
 *		mac - host mac address
 *		ipaddr - ip address
 *		op - add or delete
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
int dhcp_snp_listener_handle_host_ebtables
(
   unsigned char *mac,
   unsigned int ipaddr,
   enum dhcpsnp_ebt_op op
);

/********************************************************************************************
 * 	dhcp_snp_listener_query
 *
 *	DESCRIPTION:
 *             This function check out whether dhcp snooping is enabled on interface or not.
 *
 *	INPUT:
 *             ifname - interface name
 *
 *	OUTPUT:
 *               NONE
 *
 *	RETURNS:
 *              GT_TRUE - indicate the packet is IPv4 packet
 *              GT_FALSE - indicate the packet is not IPv4 packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned int dhcp_snp_listener_handle_arp
(   
	unsigned int isenable,
	unsigned char  *ifname
);

/**********************************************************************************
 * check_dhcp_snp_listener
 *	Scan socket list return count of listener node
 *
 *	INPUT:
 *		
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		ret		- count of listener node
 **********************************************************************************/
unsigned int check_dhcp_snp_listener
(
	void
);


#endif
