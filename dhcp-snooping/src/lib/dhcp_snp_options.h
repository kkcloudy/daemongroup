#ifndef __NPD_DHCP_SNP_OPTIONS_H__
#define __NPD_DHCP_SNP_OPTIONS_H__

/*********************************************************
*	head files														*
**********************************************************/
#include "dhcp_snp_com.h"

/*********************************************************
*	macro define													*
**********************************************************/
#define NPD_DHCP_SNP_OPTION_FIELD		(0)
#define NPD_DHCP_SNP_FILE_FIELD			(1)
#define NPD_DHCP_SNP_SNAME_FIELD		(2)

#define NPD_DHCP_SNP_OPT_CODE			(0)
#define NPD_DHCP_SNP_OPT_LEN			(1)
#define NPD_DHCP_SNP_OPT_DATA			(2)


/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET			0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER			0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL			0x17
#define DHCP_MTU			0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1				0x3a
#define DHCP_T2				0x3b
#define DHCP_VENDOR			0x3c
#define DHCP_CLIENT_ID		0x3d
#define DHCP_OPTION_82		0x52
#define DHCP_CIRCUIT_ID		(0x1)
#define DHCP_REMOTE_ID		(0x2)


#define DHCP_END			0xFF

/* type of the DHCP packet */
enum dhcp_packet_type {
	NPD_DHCP_TYPE_ZERO = 0,
	NPD_DHCP_DISCOVER,
	NPD_DHCP_OFFER,
	NPD_DHCP_REQUEST,
	NPD_DHCP_DECLINE,
	NPD_DHCP_ACK,
	NPD_DHCP_NAK,
	NPD_DHCP_RELEASE,
	NPD_DHCP_INFORM,
	NPD_DHCP_TYPE_MAX = 0xFF
};

#define NPD_DHCP_SNP_REMOTEID_STR_LEN		(64)	/* length of user-defined remote-id string, 64	*/
#define NPD_DHCP_SNP_CIRCUITID_STR_LEN		(64)	/* length of user-defined circuit-id string, 64	*/

/*********************************************************
*	struct define													*
**********************************************************/
typedef enum {
	NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX = 0,			/* DHCP-Snooping format type of option 82: hex	*/
	NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII,			/* DHCP-Snooping format type of option 82: ascii	*/
	NPD_DHCP_SNP_OPT82_FORMAT_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_FROMAT_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT = 0,		/* DHCP-Snooping packet fill format type of option 82: extended	*/
	NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD,			/* DHCP-Snooping packet fill format type of option 82: standard	*/
	NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC = 0,		/* DHCP-Snooping remote-id type of option 82: system mac		*/
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME,			/* DHCP-Snooping remote-id type of option 82: system name		*/
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR,			/* DHCP-Snooping remote-id type of option 82: user define string	*/
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_REMOTEID_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT = 0,		/* DHCP-Snooping circuit-id type of option 82: default, vlan id + port index	*/
	NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR,			/* DHCP-Snooping circuit-id type of option 82: user define string	*/
	NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE = 0,		/* Config the configuration strategy for option 82: replace			*/
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP,				/* Config the configuration strategy for option 82: drop			*/
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP,				/* Config the configuration strategy for option 82: keep			*/
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_STRATEGY_TYPE;

typedef enum {
	NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD = 0,	/* Add dhcp-snooping static binding item to bind table			*/
	NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL,		/* Delete dhcp-snooping static binding item from bind table		*/
	NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_INVALID
}NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE;

/* 
*************************************************************
* struct defined use in option 82 fill
*************************************************************
*/
typedef struct NPD_DHCP_OPT82_SUBOPT_S{
	unsigned char subopt_type;
	unsigned char subopt_len;
}NPD_DHCP_OPT82_SUBOPT_T;

/* Circuit-ID */
/* use in extended format */
typedef struct NPD_DHCP_OPT82_CIRCUITID_S{
	unsigned char circuit_type;
	unsigned char circuit_len;
}NPD_DHCP_OPT82_CIRCUITID_T;

typedef struct NPD_DHCP_OPT82_CIRCUITID_DATA_S{
	unsigned short vlanid;
	unsigned short portindex;
}NPD_DHCP_OPT82_CIRCUITID_DATA_T;


/* Remote-ID */
/* use in extended format */
typedef struct NPD_DHCP_OPT82_REMOTEID_S{
	unsigned char remote_type;
	unsigned char remote_len;
}NPD_DHCP_OPT82_REMOTEID_T;

typedef struct NPD_DHCP_OPT82_REMOTEID_DATA_S{
	unsigned char mac[6];
}NPD_DHCP_OPT82_REMOTEID_DATA_T;


/* commond: 
	user defined string
*/
typedef struct NPD_DHCP_OPT82_COM_DATA_S{
	unsigned char data[NPD_DHCP_SNP_CIRCUITID_STR_LEN];
}NPD_DHCP_OPT82_COM_DATA_T;


/*********************************************************
*	function declare												*
**********************************************************/


/**********************************************************************************
 * dhcp_snp_opt82_enable
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
unsigned int dhcp_snp_opt82_enable
(
	void
);

/**********************************************************************************
 * dhcp_snp_opt82_disable
 *		set DHCP_Snooping enable option82 status, and init 
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
unsigned int dhcp_snp_opt82_disable
(
	void
);

/**********************************************************************************
 * dhcp_snp_check_opt82_status
 *		check DHCP_Snooping enable/disable option82 status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_EN_OPT82			- option82 status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_EN_OPT82		- option82 status is disable
 **********************************************************************************/
unsigned int dhcp_snp_check_opt82_status
(
	void
);

/**********************************************************************************
 * dhcp_snp_get_opt82_status
 *		get DHCP_Snooping enable/disable option82 status
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
unsigned int dhcp_snp_get_opt82_status
(
	unsigned char *status
);

/**********************************************************************************
 * dhcp_snp_set_opt82_status
 *		set DHCP_Snooping enable/disable option82 status
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
unsigned int dhcp_snp_set_opt82_status
(
	unsigned char isEnable
);

/**********************************************************************************
 * dhcp_snp_set_opt82_port_strategy
 *		set strategy of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char strategy_mode
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set strategy mode
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char strategy_mode
);

/**********************************************************************************
 * dhcp_snp_get_opt82_port_strategy
 *		get strategy of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char isTagged,
 *
 *	OUTPUT:
 *		unsigned int *strategy_mode
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_get_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char isTagged,
	unsigned char *strategy_mode
);

/**********************************************************************************
 * dhcp_snp_set_opt82_port_circuitid
 *		set circuitid of DHCP_Snooping option82 on special port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char circuitid_mode,
 *		char *circuitid_content
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set circuit-id mode
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_opt82_port_circuitid
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char circuitid_mode,
	char *circuitid_content
);

/**********************************************************************************
 * dhcp_snp_get_opt82_port_circuitid
 *		set circuitid of DHCP_Snooping option82 on special port
 *
 *	INPUT:
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned short vlanid,
 *
 *	OUTPUT:
 *		unsigned int *circuitid_mode,
 *		char *circuitid_content
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_get_opt82_port_circuitid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *circuitid_mode,
	char *circuitid_content
);

/**********************************************************************************
 * dhcp_snp_set_opt82_port_remoteid
 *		set remoteid of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *	 	unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char remoteid_mode,
 *		char *remoteid_content
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 * 	 	DHCP_SNP_RETURN_CODE_OK					- success
 *	 	DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set remote-id mode
 * 		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST 		- vlan not exist
 * 		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 * 		DHCP_SNP_RETURN_CODE_PARAM_NULL		 	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_opt82_port_remoteid
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char remoteid_mode,
	char *remoteid_content
);

/**********************************************************************************
 * dhcp_snp_get_opt82_port_remoteid
 *		get remoteid of DHCP_Snooping option82 on special port
 *
 *	INPUT:
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned short vlanid,
 *
 *	OUTPUT:
 *		unsigned int *remoteid_mode,
 *		char *remoteid_content
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_NOT_FOUND	- error
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 **********************************************************************************/
unsigned int dhcp_snp_get_opt82_port_remoteid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *remoteid_mode,
	char *remoteid_content
);

/**********************************************************************************
 * dhcp_snp_get_opt82_suboption
 *		get option82's sub-option with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsinged int subid
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL			- not get the option
 *		not NULL 		- sub option of the option 82
 **********************************************************************************/
NPD_DHCP_OPTION_T *dhcp_snp_get_opt82_suboption
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int subcode
);

/**********************************************************************************
 * dhcp_snp_check_opt82
 *		check the packet if have option 82 with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_FOUND		- have the option
 *		DHCP_SNP_RETURN_CODE_NOT_FOUND	- not have the option
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 **********************************************************************************/
unsigned int dhcp_snp_check_opt82
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code
);

/**********************************************************************************
 * dhcp_snp_remove_opt82
 *		remove option 82 the packet if have option 82 with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsigned int opt_len
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_remove_opt82
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int opt_len,
	unsigned int *del_len
);

/**********************************************************************************
 * dhcp_snp_cat_opt82_sting
 *		cat option 82 string with Circuit-ID sting and Remote-ID string.
 *
 *	INPUT:
 *		char *circuitid_str,
 *		char *remoteid_str,
 *
 *	OUTPUT:
 *		char *opt82_str,
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_cat_opt82_sting
(
	char *circuitid_str,
	char *remoteid_str,
	char *opt82_str
);

/**********************************************************************************
 * dhcp_snp_find_opt82_add_position
 *		get option82's add position.
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *
 *	OUTPUT:
 *		unsigned int *position
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success, get the position
 *		DHCP_SNP_RETURN_CODE_ERROR 		- fail, not get the position
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_find_opt82_add_position
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int *position
);

/**********************************************************************************
 * dhcp_snp_attach_opt82_string
 *		attach option 82 string to the packet with bounds checking (warning, not aligned).
 *		add an option string to the options (an option string contains an option code, * length, then data) 
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsigned char *string
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_attach_opt82_string
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	char *string
);

/**********************************************************************************
 * dhcp_snp_end_option
 *		return the position of the 'end' option (no bounds checking) 
 *
 *	INPUT:
 *		unsigned char *optionptr
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		unsigned int		- end position of the option
 *
 **********************************************************************************/
unsigned int dhcp_snp_end_option
(
	unsigned char *optionptr
); 

/**********************************************************************************
 * dhcp_snp_attach_opt82
 *		attach DHCP_Snooping option82 to dhcp packet
 *
 *	INPUT:
 *		unsigned char *packet
 *		unsigned int ifindex,
 *		unsigned char isTagged,
 *		unsigned short vlanid
 *
 *	OUTPUT:
 *		unsigned long *bufflen		- packet len
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_attach_opt82
(
	unsigned char *packet,
	unsigned int ifindex,
	unsigned char isTagged,
	unsigned short vlanid,
	unsigned long *bufflen
);


/*********************************************************
*	extern Functions												*
**********************************************************/

#endif




