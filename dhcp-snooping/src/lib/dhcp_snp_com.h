#ifndef __NPD_DHCP_SNP_COM_H__
#define __NPD_DHCP_SNP_COM_H__

/*********************************************************
*	head files														*
**********************************************************/
#include <sys/types.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <dbus/dbus.h>

#include "dhcp_snp_listener.h"

/*********************************************************
*	macro define													*
**********************************************************/
#define	NPD_DHCP_SNP_ENABLE					(1)			/* DHCP-Snooping global status, enable		*/
#define	NPD_DHCP_SNP_DISABLE				(0)			/* DHCP-Snooping global status, disenable		*/

#define	NPD_DHCP_SNP_OPT82_ENABLE			(1)			/* DHCP-Snooping option82, enable			*/
#define	NPD_DHCP_SNP_OPT82_DISABLE			(0)			/* DHCP-Snooping option82, disenable		*/

#define	NPD_DHCP_SNP_INIT_0					(0)			/* init int/short/long variable				*/

#define NPD_DHCP_MESSAGE_TYPE				(0x35)		/*DHCP option 53, flag of message type		*/

#define NPD_DHCP_BOOTREQUEST				(1)
#define NPD_DHCP_BOOTREPLY					(2)

#define NPD_DHCP_SNP_OPTION_LEN				(1500)		/* length of DHCP-Snooping option			*/

/*show running cfg mem*/
#define NPD_DHCP_SNP_RUNNING_CFG_MEM		(1024*1024)

#define DHCP_SNP_ENABLE 1
#define DHCP_SNP_DISABLE 0

#define DHCP_SNP_SET_UDF_FLAG		(0x1)
#define DHCP_SNP_CLR_UDF_FLAG		(0x0)
#define DHCP_SNP_ARP_PROXY_ENABLE		(1)			
#define DHCP_SNP_ARP_PROXY_DISABLE		(0)

/*********************************************************
*	struct define													*
**********************************************************/
typedef enum {
	NPD_DHCP_SNP_PORT_MODE_NOTRUST = 0,					/* DHCP-Snooping trust mode of port: no trust	*/
	NPD_DHCP_SNP_PORT_MODE_NOBIND,						/* DHCP-Snooping trust mode of port: trust but no bind	*/
	NPD_DHCP_SNP_PORT_MODE_TRUST,						/* DHCP-Snooping trust mode of port: trust	*/
	NPD_DHCP_SNP_PORT_MODE_INVALID
}NPD_DHCP_SNP_PORT_MODE_TYPE;

#if 0
typedef struct NPD_DHCP_SNP_PORT_S
{
	unsigned char trust_mode;			/* DHCP-Snooping trust mode of port				*/
	unsigned char opt82_strategy;		/* DHCP-Snooping option82 strategy type of port	*/
	unsigned char opt82_circuitid;		/* DHCP-Snooping option82 circuitid type of port	*/
										/* DHCP-Snooping option82 circuitid content of port */
	unsigned char opt82_circuitid_str[64];
	unsigned char opt82_remoteid;		/* DHCP-Snooping option82 remoteid type of port	*/
										/* DHCP-Snooping option82 remoteid content of port */
	unsigned char opt82_remoteid_str[64];

}NPD_DHCP_SNP_PORT_T;
#endif

typedef struct NPD_DHCP_MESSAGE_S
{
	unsigned char op;
	unsigned char htype;
	unsigned char hlen;
	unsigned char hops;
	unsigned int xid;
	unsigned short secs;
	unsigned short flags;
	unsigned int ciaddr;
	unsigned int yiaddr;
	unsigned int siaddr;
	unsigned int giaddr;
	unsigned char chaddr[16];
	unsigned char sname[64];
	unsigned char file[128];
	unsigned int cookie;
	unsigned char options[NPD_DHCP_SNP_OPTION_LEN]; /* 312 - cookie */ 
} NPD_DHCP_MESSAGE_T;

typedef struct NPD_DHCP_OPTION_S{
  unsigned char	code;
  unsigned char	leng;
  unsigned char	value[];
}NPD_DHCP_OPTION_T;


typedef struct NPD_UDP_DHCP_PACKET_S
{
	struct ethhdr eth;
	struct iphdr ip;
	struct udphdr udp;
	NPD_DHCP_MESSAGE_T data;
}NPD_UDP_DHCP_PACKET_T;

typedef struct NPD_DHCP_SNP_SHOW_ITEM_S
{
	unsigned int   bind_type;
	unsigned char  chaddr[12];
	unsigned int   ip_addr;
	unsigned short vlanId;
	unsigned int   ifindex;
	unsigned int   lease_time;
}NPD_DHCP_SNP_SHOW_ITEM_T;

typedef struct NPD_DHCP_SNP_SHOW_TRUST_PORTS_S
{
	unsigned short vlanId;
	unsigned int slot_no;
	unsigned int port_no;
	unsigned int tag_mode;
	unsigned int trust_mode;
}NPD_DHCP_SNP_SHOW_TRUST_PORTS_T;

typedef struct NPD_DHCP_SNP_ETHER_HEAD_S
{
	unsigned char  dmac[6];		/* destination eth addr	*/
	unsigned char  smac[6];		/* source ether addr	*/
	unsigned short etherType;
}NPD_DHCP_SNP_ETHER_HEAD_T;


#define SIOCGIFUDFFLAGS	0x893d		/* get udf_flags			*/
#define SIOCSIFUDFFLAGS	0x893e		/* set udf_flags			*/
#define IFF_UDF_NOARP	0x08		/* no arp flag */
#define IFF_ARP_DORP 	0x04


/*********************************************************
*	function declare												*
**********************************************************/


/**********************************************************************************
 * dhcp_snp_enable
 *		set DHCP_Snooping enable global status, and init 
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
unsigned int dhcp_snp_enable
(
	void
);

/**********************************************************************************
 * dhcp_snp_disable
 *		set DHCP_Snooping enable global status, and init 
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
unsigned int dhcp_snp_disable
(
	void
);

/**********************************************************************************
 * dhcp_snp_check_global_status
 *		check DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_ENABLE_GBL		- global status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL	- global status is disable
 **********************************************************************************/
unsigned int dhcp_snp_check_global_status
(
	void
);

/**********************************************************************************
 * dhcp_snp_get_global_status
 *		get DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		unsigned char *status
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_get_global_status
(
	unsigned char *status
);

/**********************************************************************************
 * dhcp_snp_set_global_status
 *		set DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		unsigned char isEnable
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_set_global_status
(
	unsigned char isEnable
);

/**********************************************************************************
 * dhcp_snp_set_intf_arp
 *		set DHCP_Snooping enable/disable interface anti-arp-spoofing
 *
 *	INPUT:
 *		unsigned char isEnable
 *		char *ifname
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_set_intf_arp
(
	unsigned char isEnable,
	char *ifname
);

#if 0
/**********************************************************************************
 * dhcp_snp_reset_all_vlan_port
 *		reset DHCP_Snooping attribute on all vlan's ports
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_reset_all_vlan_port
(
	void
);


/**********************************************************************************
 * dhcp_snp_set_port_trust_mode
 *		set port is DHCP_Snooping trust/no-bind/notrust
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan ID
 *		unsigned int g_eth_index,		- port index
 *		unsigned char tagMode,			- tag mode of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_port_trust_mode
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char trust_mode
);
#endif
/**********************************************************************************
 * dhcp_snp_get_option
 *		get an option with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL			- not get the option
 *		not NULL 		- string of the option
 **********************************************************************************/
void *dhcp_snp_get_option
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code
);

#if 0
/**********************************************************************************
 * dhcp_snp_is_trusted
 *		check the port is trust/no-bind port, and get trust mode
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan ID
 *		unsigned int ifindex,			- port global index
 *		unsigned char isTagged,		- tag flag of port
 *
 *	OUTPUT:
 *		unsigned char *trust_mode		- trust mode of port
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- found the port which trust or no-bind
 *		DHCP_SNP_RETURN_CODE_ERROR					- not found the port which trust or no-bind
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_is_trusted
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char isTagged,
	unsigned char *trust_mode
);

/**********************************************************************************
 * dhcp_snp_get_vlan_trust_port
 *		get trust/no-bind ports of the vlan
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan ID
 *		unsigned char tagMode,			- tag mode
 *
 *	OUTPUT:
 *		unsigned int *cnt,
 *		NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK				- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL		- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST	- vlan is not exist
 *
 **********************************************************************************/
unsigned int dhcp_snp_get_vlan_trust_port
(
	unsigned short vlanId,
	unsigned char tagMode,
	unsigned int *cnt,
	NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
);


/**********************************************************************************
 * dhcp_snp_get_trust_ports
 *		get port is trust/no-bind port, and get trust mode
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		unsigned int *trust_ports_cnt,
 *		NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 **********************************************************************************/
unsigned int dhcp_snp_get_trust_ports
(
	unsigned int *trust_ports_cnt,
	NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
);

/**********************************************************************************
 * dhcp_snp_remove_vlan_broadcast_ports
 *		remove port from vlan's broadcast ports
 *
 *	INPUT:
 *		unsigned short ifindex,
 *		unsigned char isTagged,
 *		struct vlan_ports *tagPorts,
 *		struct vlan_ports *untagPorts
 *	
 *	OUTPUT:
 *		struct vlan_ports *tagPorts,
 *		struct vlan_ports *untagPorts
 *
 * 	RETURN:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL
 *		DHCP_SNP_RETURN_CODE_OK
 *		
 **********************************************************************************/
unsigned int dhcp_snp_remove_vlan_broadcast_ports
(
	unsigned short ifindex,
	unsigned char isTagged,
	struct vlan_ports *tagPorts,
	struct vlan_ports *untagPorts
);
#endif
/**********************************************************************************
 * dhcp_snp_packet_rx_process()
 *	DESCRIPTION:
 *		receive packets for DHCP Snooping rx 
 *
 *	INPUTS:
 *		unsigned long numOfBuff		- number of buffer
 *		unsigned char *packetBuffs[]	- array of buffer
 *		unsigned long buffLen[]			- array of buffer length
 *		unsigned int interfaceId			- port Or TrunkId has been transfer to eth_g_index
 *		unsigned char isTagged			- tag flag of port
 *		unsigned short vid				- vlanid
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		GT_FAIL			- build fail
 *		GT_NO_RESOURCE	- alloc memory error
 *		GT_BAD_SIZE		- packet length is invalid
 *		GT_OK			- build ok
 *		totalLen			- receive packet length
***********************************************************************************/
unsigned int dhcp_snp_packet_rx_process
(
	unsigned long numOfBuff,
	unsigned char *packetBuffs[],
	unsigned long buffLen[],
	unsigned int ifindex,
	unsigned char isTagged,
	unsigned short vid, 
	struct dhcp_snp_listener *node
);


/**********************************************************************************
 * dhcp_snp_save_global_cfg
 *		get string of DHCP Snooping show running global config
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
unsigned int dhcp_snp_save_global_cfg
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
);

#if 0
/**********************************************************************************
 * dhcp_snp_save_vlan_cfg
 *		get string of DHCP Snooping show running global config
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void dhcp_snp_save_vlan_cfg
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
);
#endif

/**********************************************************************************
 * dhcp_snp_dbus_enable_global_status
 *		set DHCP-Snooping enable/disable global status
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_enable_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_check_global_status
 *		check and get DHCP-Snooping enable/disable global status
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ERROR
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_check_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

#if 0
/**********************************************************************************
 * dhcp_snp_dbus_enable_vlan_status
 *		enable/disable DHCP-Snooping on special vlan
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_enable_vlan_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_config_port
 *		config DHCP-Snooping trust mode of port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_show_bind_table
 *		show DHCP-Snooping bind table
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_show_static_bind_table
 *		show DHCP-Snooping static bind table
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_static_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_show_trust_ports
 *		show DHCP-Snooping trust and no-bind ports
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_trust_ports
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_enable_opt82
 *		enable/disable DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_enable_opt82
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
); 

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_format_type
 *		set storage format type of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_format_type
 *		set fill format type of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_fill_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_remoteid_content
 *		set remote-id type and content of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_remoteid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
); 

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_port_strategy
 *		set DHCP-Snooping option 82 strategy mode of port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- option 82 strategy already setted same value
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 * 
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_port_strategy
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_port_circuitid_content
 *		set circuit-id type and content of  DHCP-Snooping option82 on special port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- option 82  circuit-id mode already setted same value
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 *		DHCP_SNP_RETURN_CODE_ERROR					- fail
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_port_circuitid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_port_remoteid_content
 *		set remote-id type and content of  DHCP-Snooping option82 on special port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_port_remoteid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_add_del_static_binding
 *		add/delete dhcp-snooping static-binding item to/from bind table
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_add_del_static_binding
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_add_binding
 *		add dhcp-snooping binding item to bind table
 *		inner interface in dhcp snooping, for save content of bindint table
 *		by command "write" before reboot
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_add_binding
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);
#endif
/**********************************************************************************
 * dhcp_snp_dbus_show_running_global_config
 *		DHCP Snooping show running global config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_running_global_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

#if 0
/**********************************************************************************
 * dhcp_snp_dbus_show_running_vlan_config
 *		DHCP Snooping show running vlan config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_running_vlan_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_save_bind_table
 *		get string of DHCP Snooping show running save bind table
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void dhcp_snp_save_bind_table
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
);
#endif
/**********************************************************************************
 * dhcp_snp_dbus_config_intf
 *		config DHCP-Snooping enable status on interface
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_intf
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
); 

/**********************************************************************************
 * dhcp_snp_dbus_config_debug_level
 *		config DHCP-Snooping debug level
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_debug_level
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_config_ipneigh
 *		config DHCP-Snooping ip neigh operation flag
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ERROR				- fail
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_ipneigh
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_save_intf_cfg
 *		get string of interface DHCP Snooping show running config
 *
 *	INPUT:
 *		buf - show running string buffer
 *		bufLen - buffer length
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void dhcp_snp_save_intf_cfg
(
	unsigned char *buf,
	unsigned int bufLen
);

/*********************************************************
*	extern Functions												*
**********************************************************/
#endif



