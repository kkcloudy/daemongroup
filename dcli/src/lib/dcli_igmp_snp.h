#ifndef __DCLI_IGMP_SNP_H__
#define __DCLI_IGMP_SNP_H__

#define NPD_MAX_VLAN_ID 4095
#define IGMP_SNP_DISABLE 0xff

#define ISMLD 1
#define ISIGMP 2

#define TURE_5612E 1
#define FALSE_5612E 0

#define IGMP_SNP_DBUS_DEBUG(x) printf x
#define IGMP_SNP_DBUS_ERR(x) printf x

#define IGMP_SNP_STR "Config IGMP Snooping Protocol\n"
#define IGMP_STR_CMP_LEN	4

__inline__ int parse_slotport_no(char * str, unsigned char * slotno, unsigned char * portno);


typedef enum {
	VLAN_LIFETIME = 1,
	GROUP_LIFETIME,
	ROBUST_VARIABLE,
	QUERY_INTERVAL,
	RESP_INTERVAL,
	RXMT_INTERVAL
}IGMP_SNP_TIMER_TYPE;

int dcli_igmp_snp_time_show_running_config(unsigned char Ismld);
extern DBusConnection *dcli_dbus_connection_igmp;
extern int igmp_dist_slot; 
#endif

