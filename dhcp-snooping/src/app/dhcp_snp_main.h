#ifndef _DHCP_SNP_MAIN_H
#define _DHCP_SNP_MAIN_H





/**********************************************************************************
 * dhcp_snp_dbus_init
 *	Initialize DHCP snooping dbus connection
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		TRUE		- success
 *	 	FALSE		- fail
 **********************************************************************************/
extern int dhcp_snp_dbus_init(void);

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
extern int dhcp_snp_listener_init(void);

/**********************************************************************************
 * dhcp_snp_netlink_init
 *	Initialize DHCP snooping netlink socket
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
extern int dhcp_snp_netlink_init(void);

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
extern void * dhcp_snp_listener_thread_main
(
	void *arg
);

/**********************************************************************************
 * dhcp_snp_dbus_thread_main
 *	Main routine for dbus message handle thread
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
extern void * dhcp_snp_dbus_thread_main
(
	void *arg
);

void * dhcp_snp_tbl_thread_aging
(
	void *arg
);

void * dhcp_snp_asd_interactive
(
	void *arg
);

/**********************************************************************************
 *	log_debug
 * 
 *	output the daemon debug info to /var/log/system.log
 *
 *  INPUT:
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
extern void log_debug
(
	char *format,
	...
);

/**********************************************************************************
 *	log_error
 * 
 *	output the daemon debug info to /var/log/system.log
 *
 *  INPUT:
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
extern void log_error
(
	char *format,
	...
);

extern int anti_arp_spoof_init(void);
void pid_write(char *name);
#endif
