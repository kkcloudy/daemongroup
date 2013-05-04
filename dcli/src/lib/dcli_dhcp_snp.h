#ifndef __DCLI_DHCP_SNP_H__
#define __DCLI_DHCP_SNP_H__

/*********************************************************
*	head files														*
**********************************************************/
#include <sysdef/npd_sysdef.h>

/*********************************************************
*	macro define													*
**********************************************************/
#define DCLI_DHCP_SNP_STR					"DHCP-Snooping information\n"
#define DCLI_DHCP_SNP_OPT82_STR				"DHCP-Snooping option 82\n"
#define DCLI_DHCP_SNP_OPT82_FORMAT_STR		"DHCP-Snooping storage format of option 82\n"
#define DCLI_DHCP_SNP_OPT82_PKT_FORMAT_STR	"DHCP-Snooping fill format of option 82\n"
#define DCLI_DHCP_SNP_OPT82_REMOTEID_STR	"DHCP-Snooping remote-id field of option 82\n"
#define DCLI_DHCP_SNP_OPT82_CIRCUITID_STR	"DHCP-Snooping circuit-id field of option 82\n"
#define DCLI_DHCP_SNP_OPT82_STRATEGY_STR	"DHCP-Snooping the configuration strategy for option 82\n"
#define DCLI_DHCP_SNP_ADD_BINDING_STR		"Add DHCP-Snooping binding item\n"
#define DCLI_DHCP_SNP_DEL_BINDING_STR		"Delete DHCP-Snooping binding item\n"
#define DCLI_DHCP_SNP_DYNAMIC_BINDING_STR	"Dynamic DHCP-Snooping binding item\n"
#define DCLI_DHCP_SNP_STATIC_BINDING_STR	"Static DHCP-Snooping binding item\n"
#define DCLI_DHCP_SNP_BINDING_STR			"DHCP-Snooping binding item\n"
#define DCLI_DHCP_SNP_MAC_STR				"Config MAC address\n"
#define DCLI_DHCP_SNP_MAC_ADDRESS_STR		"MAC address format as HH:HH:HH:HH:HH:HH\n"
#define DCLI_DHCP_SNP_IP_STR				"Config IP address\n"
#define DCLI_DHCP_SNP_IP_ADDRESS_STR		"IP address format as A.B.C.D\n"


#define DCLI_DHCP_SNP_INIT_0				(0)			/* init int/short/long variable, 0				*/

#define DCLI_DHCP_SNP_ENABLE				(1)			/* enable								*/
#define DCLI_DHCP_SNP_DISABLE				(0)			/* disable								*/

#define DCLI_DHCP_SNP_MAC_ADD_LEN			(6)			/* length of mac							*/
#define DCLI_DHCP_SNP_REMOTEID_STR_LEN		(64)		/* length of user-defined remote-id string, 64	*/
#define DCLI_DHCP_SNP_CIRCUITID_STR_LEN		(64)		/* length of user-defined circuit-id string, 64	*/

#define DHCP_SNP_ARP_PROXY_ENABLE		(1)			
#define DHCP_SNP_ARP_PROXY_DISABLE		(0)
/*********************************************************
*	struct define													*
**********************************************************/
typedef enum {
	DCLI_DHCP_SNP_PORT_MODE_NOTRUST = 0, 				/* DHCP-Snooping trust mode of port: no trust	*/
	DCLI_DHCP_SNP_PORT_MODE_NOBIND,						/* DHCP-Snooping trust mode of port: trust but no bind	*/
	DCLI_DHCP_SNP_PORT_MODE_TRUST,						/* DHCP-Snooping trust mode of port: trust	*/
	DCLI_DHCP_SNP_PORT_MODE_INVALID
}DCLI_DHCP_SNP_PORT_MODE_TYPE;

typedef enum {
	DCLI_DHCP_SNP_OPT82_FORMAT_TYPE_HEX = 0,			/* DHCP-Snooping format type of option 82: hex	*/
	DCLI_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII,				/* DHCP-Snooping format type of option 82: ascii	*/
	DCLI_DHCP_SNP_OPT82_FORMAT_TYPE_INVALID
}DCLI_DHCP_SNP_OPT82_FORMAT_TYPE;

typedef enum {
	DCLI_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT = 0,		/* DHCP-Snooping packet fill format type of option 82: extended	*/
	DCLI_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD,			/* DHCP-Snooping packet fill format type of option 82: standard	*/
	DCLI_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_INVALID
}DCLI_DHCP_SNP_OPT82_FILL_FORMAT_TYPE;

typedef enum {
	DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC = 0,		/* DHCP-Snooping remote-id type of option 82: system mac		*/
	DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME,			/* DHCP-Snooping remote-id type of option 82: system name		*/
	DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR,			/* DHCP-Snooping remote-id type of option 82: user define string	*/
	DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID
}DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE;

typedef enum {
	DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT = 0,		/* DHCP-Snooping circuit-id type of option 82: default, vlan id + port index	*/
	DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR,			/* DHCP-Snooping circuit-id type of option 82: user define string	*/
	DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID
}DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE;


typedef enum {
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE = 0,		/* Config the configuration strategy for option 82: replace			*/
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP,				/* Config the configuration strategy for option 82: drop			*/
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP,				/* Config the configuration strategy for option 82: keep			*/
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID
}DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE;

typedef enum {
	DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD = 0,	/* Add dhcp-snooping static binding item to bind table			*/
	DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL,		/* Delete dhcp-snooping static binding item from bind table		*/
	DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_INVALID
}DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE;

/*********************************************************
*	function declare												*
**********************************************************/


/**********************************************************************************
 * dcli_dhcp_snp_mac_ascii_to_hex
 * 
 * 	mac address in Hex format
 *
 *	INPUT:
 *		chaddr	- string of mac
 *		size		- macAddr buffer size
 *	
 *	OUTPUT:
 *		macAddr - mac address will be returned back
 *
 * 	RETURN:
 *		-1 - if mac address buffer size too small.
 *		-2 - illegal character found.
 *		-3 - parameter is null
 *		0 - if no error occur
 *
 *	NOTATION:
 *		mac address is a ASCII code formatted MAC address, such as 
 *		"001122334455" stands for mac address 00:11:22:33:44:55 or 00-11-22-33-44-55
 *		
 **********************************************************************************/
int dcli_dhcp_snp_mac_ascii_to_hex
(
    unsigned char *chaddr,
    unsigned char *macAddr,
    unsigned int  size
);

/**********************************************************************************
 * dcli_dhcp_snp_remoteid_string_legal_check
 * 
 *	INPUT:
 *		str	- remote-id string user entered from vtysh. 
 *		 len	- Length of remote-id string 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		1	--string too long
 *		2	--illegal char on head of string
 *		3	--unsupported char in string
 *		DHCP_SNP_RETURN_CODE_OK 		-- success
 *		DHCP_SNP_RETURN_CODE_ERROR		-- fail
 **********************************************************************************/
unsigned int dcli_dhcp_snp_remoteid_string_legal_check
(
	char *str,
	unsigned int len
);

/**********************************************************************************
 * dcli_dhcp_snp_circuitid_string_legal_check
 * 
 *	INPUT:
 *		str	- circuit-id string user entered from vtysh. 
 *		 len	- Length of circuit-id string 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		1	--string too long or too short
 *		2	--illegal char on head of string
 *		3	--unsupported char in string
 *		DHCP_SNP_RETURN_CODE_OK 		-- success
 *		DHCP_SNP_RETURN_CODE_ERROR		-- fail
 **********************************************************************************/
unsigned int dcli_dhcp_snp_circuitid_string_legal_check
(
	char *str,
	unsigned int len
);

/**********************************************************************************
 * dcli_dhcp_snp_check_global_status()
 *	DESCRIPTION:
 *		check DHCP-Snooping enable/disable global status 
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		unsigned char* stats		- check result
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_check_global_status
(
	unsigned char* stats
);
/**********************************************************************************
 * dcli_dhcp_snp_wan_check_global_status()
 *	DESCRIPTION:
 *		check DHCP-Snooping enable/disable global status 
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		unsigned char* stats		- check result
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_wan_check_global_status
(
	struct vty *vty,
	unsigned char* stats
);

/**********************************************************************************
 * dcli_dhcp_snp_config_port()
 *	DESCRIPTION:
 *		config DHCP-Snooping port's trust mode 
 *
 *	INPUTS:
 *	 	unsigned short vlanId,			- vlan id
 *	 	unsigned char slot_no,			- slot no of port
 *	 	unsigned char local_port_no,		- port no of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_config_port
(
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char local_port_no,
	unsigned char trust_mode
);

/**********************************************************************************
 * dcli_dhcp_snp_show_bind_table()
 *	DESCRIPTION:
 *		show DHCP-Snooping bind table
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_bind_table
(
	struct vty* vty
);

/**********************************************************************************
 * dcli_dhcp_snp_show_bind_table()
 *	DESCRIPTION:
 *		show DHCP-Snooping trust and no-bind ports
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_trust_ports
(
	struct vty* vty
);

/*********************************************************
*	extern Functions												*
**********************************************************/
	char *dcli_dhcp_snp_show_running_config2
	(
		struct vty *vty,
		int slotid
	);

char * 
dcli_dhcp_snp_show_running_hansi_cfg
(
	unsigned int slot_id,unsigned int InstID, unsigned int islocaled 
);


#endif


