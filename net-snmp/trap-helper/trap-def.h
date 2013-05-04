/* trap-def.h */

#ifndef TRAP_DEF_H
#define TRAP_DEF_H

#ifdef  __cplusplus
extern "C" {
#endif

#define MAC_LEN					6
#define MAC_STR_LEN				18
#define MAX_MAC_OID 			187
#define NETID_NAME_MAX_LENTH 	21

#define APPLICATION_NAME		"snmptrap"  /*this can't be change*/

//trap type 
#define TRAP_TYPE_ENVIRO		"environment"
#define TRAP_TYPE_DEVICE		"Device"
#define TRAP_TYPE_COMMUNIC 		"communication"
#define TRAP_TYPE_PROCESS_ERR	"processing_error"
#define TRAP_TYPE_PROCESS_SUC	"processing_success"
#define TRAP_TYPE_QOS			"QOS"

//trap level 
#define TRAP_LEVEL_MAJOR		"major"
#define TRAP_LEVEL_CRITIC		"critical"
#define	TRAP_LEVEL_SECONDARY	"secondary"
#define TRAP_LEVEL_GENERAL		"general"

/*  common oid  */
#define TRAP_ENTERPRISE_OID				".31656"
#define TRAP_PRODUCT_OID				".6.1"

#define TRAP_BEFORE_ENTERPRISES_OID		".1.3.6.1.4.1"
#define TRAP_AP_OID						".4"
#define TRAP_AC_OID						".5"

#define TRAP_FREQUENCY_OID		".3"
#define TRAP_TYPE_OID			".4"
#define TRAP_LEVEL_OID			".5"
#define TRAP_EVENT_TIME_OID		".6"
#define TRAP_STATUS_OID			".7"
#define TRAP_TITLE_OID			".8"
#define TRAP_CONTENT_OID		".9"
//add by liusheng
#define TRAP_AC_IP_OID                      ".2.2.1"
#define TRAP_AC_POWER_OFF_INDEX_OID         ".16"
#define AC_INTERFACE_IP_CHANGE_STATE_OID    ".17"
#define AC_INTERFACE_OID                         ".2.2.20.1.1"                //error
#define AC_INTERFACE_IP_OID                    ".2.2.20.1.2"                   
#define AC_BACKUP_STAUTS_OID                ".2.1.1.38"
#define AC_BACKUP_NETWORK_IP_OID            ".2.1.1.39"
//add by liusheng end 
#define TRAP_INSTANCE_OID ".2.29.1.1"  //full oid .1.3.6.1.4.1.31656.6.1.2.29.1.1
#define TRAP_RADIO_INFO_OID ".4.5"


#define WTP_NET_ELEMENT_CODE_OID			".1.2.5.1.3"
#define AC_NET_ELEMENT_CODE_OID				".2.2.3"

/*  end of common oid  */

/* parameter oid */
#define EI_WLANID_TRAP_DES		".2.7.1.1"
#define EI_WTPID_TRAP_DES		".2.11.2.1.1"
#define EI_SN_TRAP_DES			".1.1.1.1.11"
#define EI_MAC_TRAP_DES			".1.2.5.1.9"
#define EI_ESSID_TRAP_DES		".1.5.4.1.4"
#define EI_STA_MAC_TRAP_DES		".1.8.1.1.1"
#define EI_CHANNEL_MAC_TRAP_DES				".1.7.3.1.1"
#define EI_WTP_CPU_USAGE_TRAP_DES			".1.1.2.1.5"
#define EI_WTP_CPU_THRESHOLD_TRAP_DES		".1.1.2.1.6"
#define EI_WTP_MEM_USAGE_TRAP_DES			".1.1.2.1.11"
#define EI_WTP_MEM_THRESHOLD_TRAP_DES		".1.1.2.1.12"
#define EI_WTP_TEMP_USAGE_TRAP_DES			".1.1.2.1.17"
#define EI_AC_IP_ADDR_TRAP_DES				".2.2.1"
#define EI_AC_MAC_TRAP_DES				".2.2.5"
#define EI_AP_RSSI_TRAP_DES		".1.8.1.1.15"
#define EI_ROGUE_STA_CHANNEL_MAC_TRAP_DES	".1.13.5.1.11"
#define EI_WTP_SYN_ATTACK_TYPE			".1.13.5.1.3"
#define EI_WTP_RSSI					".1.13.5.1.10"
#define EI_STA_USER_IPADDR			".1.17.2.1.1"
#define EI_STA_USER_NAME			".1.17.2.1.2"


#define EI_AC_CPU_USAGE_TRAP_DES			".2.1.2.13"
#define EI_AC_MEM_USAGE_TRAP_DES			".2.1.2.8"
#define EI_AC_TEMP_USAGE_TRAP_DES			".2.1.2.20"
#define EI_AC_BAND_USAGE_TRAP_DES			".2.3.1.17"
#define EI_AC_DROP_USAGE_TRAP_DES			".2.3.1.18"
#define EI_AC_USER_ONLINE_USAGE_TRAP_DES	".2.3.1.19"
#define EI_AC_RADIUS_REQUEST_RATE_TRAP_DES	".2.3.1.20"
#define EI_AC_DHCP_USAGE_TRAP_DES			".2.6.3.4"
#define EI_AC_DHCP_IP_POOL_NAME_DES			".2.6.5.1.10"

#define EI_AC_DHCP_MAX_USAGE_TRAP_DES		".2.6.3.5"

#define EI_AC_PORTAL_URL					".2.2.6"
#define EI_AC_RADIUS_AUTH_IP				".2.14.6.1.2"
#define EI_AC_RADIUS_AUTH_PORT				".2.14.6.1.3"
#define EI_AC_RADIUS_ACCT_IP				".2.14.7.1.2"
#define EI_AC_RADIUS_ACCT_PORT				".2.14.7.1.3"



#ifdef  __cplusplus
}
#endif

#endif		/* TRAP_DEF_H */


