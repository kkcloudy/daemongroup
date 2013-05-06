#ifndef _PPPOE_INTERFACE_DEF_H
#define _PPPOE_INTERFACE_DEF_H

#define DEV_MAX_NUM			128
#define DEV_DESC_LEN		32

#define CONFIGCMD_SIZE		2048

#define PPPOE_NAMELEN		64
#define USERNAMESIZE		256		/* Max length of username */
#define USERPASSWDSIZE		16
#define RADIUS_SECRETSIZE	128		/* No secrets that long */
#define ACCT_SESSIONIDSIZE	32
#define IPADDRSTRSIZE		16

#define DEFAULT_MAX_SESSIONID			0xfffe	/* 0xffff can not be used*/

#define DEFAULT_ACCT_INTERVAL			900
#define DEFAULT_RADIUS_RETRY_INTERVAL	4
#define DEFAULT_RADIUS_RETRY_TIMES		1
#define DEFAULT_VICE_RADIUS_RETRY_TIMES	1

#define DEFAULT_STATUPDATE_TIMEOUT		60			
#define DEFAULT_ECHOUPDATE_TIMEOUT		20			
#define DEFAULT_ACCTUPDATE_TIMEOUT		900			

#define DEFAULT_SESSTERM_TIMEOUT		3
#define DEFAULT_SESSSYSCALL_TIMEOUT		3
#define DEFAULT_SESSRESTRANS_TIMEOUT	3
#define DEFAULT_SESSCONFIG_TIMEOUT		10
//#define DEFAULT_SESSRADIUS_TIMEOUT		30

#define DEFAULT_BACKUP_CONNECT_INTERVAL	10
#define DEFAULT_BACKUP_ECHO_INTERVAL	10
#define DEFAULT_BACKUP_ECHO_TIMES		6


struct radius_server {
	unsigned short port;
	unsigned int ip;
	unsigned int secretlen;
	char secret[RADIUS_SECRETSIZE];
};

struct radius_srv {
	struct radius_server auth;
	struct radius_server backup_auth;
	
	struct radius_server acct;
	struct radius_server backup_acct;
};

struct pppoeDevBasicInfo {
	unsigned int dev_id;
	unsigned int state;
	unsigned int ipaddr;
	unsigned int mask;
	char ifname[IFNAMSIZ];
	char base_ifname[IFNAMSIZ];
	char dev_desc[DEV_DESC_LEN];
};

struct pppoeUserInfo {
	unsigned int sid;
	unsigned int ip;
	unsigned int sessTime;

	unsigned char mac[ETH_ALEN];
	char username[USERNAMESIZE];
};

struct pfm_table_entry {	
	unsigned int opt;		/* opt 0:add pfm entry, opt 1:del pfm entry */
	unsigned int opt_para;
	unsigned short protocol;
	char ifname[IFNAMSIZ];

    char src_ipaddr[IPADDRSTRSIZE];
    unsigned int src_port;
    
    char dest_ipaddr[IPADDRSTRSIZE];
    unsigned int dest_port;

	unsigned int sendto;	
	unsigned int slot_id;	
};

#endif
