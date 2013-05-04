#ifndef _DHCP_SNP_LOG_H_
#define _DHCP_SNP_LOG_H_

enum {
	SYSLOG_DBG_DEF     = 0x0,	/* default value*/
	SYSLOG_DBG_DBG     = 0x1,	/*normal */
	SYSLOG_DBG_WAR     = 0x2,	/*warning*/
	SYSLOG_DBG_ERR     = 0x4,	/* error*/
	SYSLOG_DBG_EVT     = 0x8,	/* event*/
	SYSLOG_DBG_PKT_REV = 0x10,  /*packet receive*/
	SYSLOG_DBG_PKT_SED = 0x20,  /*packet send*/
	SYSLOG_DBG_PKT_ALL = 0x30,  /*packet send and receive*/
	SYSLOG_DBG_ALL = 0xFF	/* all*/
};


#define DHCP_SNP_STATIC_BUFFER_SIZE			(256)


#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"


/**********************************************************************************
 *	dhcp_snp_log_config_debug_level
 * 
 *	set dhcp snooping debug level
 *
 *  INPUT:
 *		level  - debug level value 
 *		enable - enable or disable flag
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
int dhcp_snp_log_config_debug_level
(
	unsigned int level,
	unsigned int enable
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
void log_debug
(
	char *format,
	...
);


/**********************************************************************************
 *	log_info
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
void log_info
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
void log_error
(
	char *format,
	...
);

/**********************************************************************************
 *	log_warn
 * 
 *	output the daemon warnings to /var/log/system.log
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
void log_warn
(
	char *format,
	...
);

/**********************************************************************************
 *	syslog_ax_dhcp_snp_pkt_rx
 * 
 *	output the daemon warnings to /var/log/system.log
 *
 *	INPUT:
 *		format - 
 *	
 *	OUTPUT:
 *	 NULL
 *
 *	RETURN:
 *	 NULL
 *	 
 *
 **********************************************************************************/
void syslog_ax_dhcp_snp_pkt_rx
(
	char *format,
	...
);

/**********************************************************************************
 *	npd_syslog7_notice
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
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
void dhcp_snp_syslog7_notice
(
	char *format,
	...
);


/* alias */
#define syslog_ax_dhcp_snp_dbg log_debug
#define syslog_ax_dhcp_snp_err log_error
#define syslog_ax_dhcp_snp_warn log_warn

#define syslog_ax_dbus_dbg	log_debug
#define syslog_ax_dbus_err	log_error


/*****************************************************************************
 *	mac2str
 * 
 *	mac to strig
 *
 *  INPUT:
 *		haddr - mac address 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 static_buferr - mac string
 * 	 NULL
 *
 ****************************************************************************/
inline char *mac2str(
	unsigned char *haddr
);

/*****************************************************************************
 *	u32ip2str
 * 
 *	IPv4 address to string (EXP: 0x0a01010a -> 10.1.1.10)
 *
 *  INPUT:
 *		u32_ipaddr - IPv4 address 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 char * - ipv4 address string
 * 	 NULL - failed
 *
 ****************************************************************************/
inline char *u32ip2str
(
	unsigned int u32_ipaddr
);



#endif
