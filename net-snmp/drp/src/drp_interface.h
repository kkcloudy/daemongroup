
#ifndef _DRP_INTERFACE_H_
#define _DRP_INTERFACE_H_

//#include <dbus/dbus.h>
#include <stdint.h>
#include "nm_list.h"

/*define hansi type!*/
#if 0
typedef enum{ 
	HANSI_REMOTE = 0,
	HANSI_LOCAL
} HANSI_TYPE;
#endif

#define DRP_DBUS_BUSNAME      "aw.drp"
#define DRP_DBUS_OBJPATH      "/aw/drp"
#define DRP_DBUS_INTERFACE    "aw.drp"

#define MAX_DBUS_OBJPATH_LEN		128
#define MAX_DBUS_BUSNAME_LEN		128
#define MAX_DBUS_INTERFACE_LEN	128
#define MAX_DBUS_METHOD_LEN		128

#define XML_DOMAIN_D                        "/opt/www/htdocs/dns_cache.xml"
#define XML_DOMAIN_PATH	   	      "/opt/services/option/domain_option"

#define	MAX_DOMAIN_IPADDR			16
#define	MAX_DOMAIN_NAME_LEN		256
#define	MAX_DOMAIN_CONFIG_NUM		128

#define DRP_CONFIG_FILE "/var/run/dns_cache.conf"
#define SLOT_MAX_NUM 	16

struct domain_param{
	char domain_name[MAX_DOMAIN_NAME_LEN];
	int index;
	unsigned long ipaddr;
};

typedef struct domain_param domain_pt;

struct domain_config{
	char domain_name[MAX_DOMAIN_NAME_LEN];
	int num;
	struct {
		int index;
		unsigned long ipaddr;
	}domain_ip[MAX_DOMAIN_IPADDR];
};

typedef struct domain_config domain_ct;

struct domain_configs{
	int num;
	struct domain_param domain[MAX_DOMAIN_CONFIG_NUM];
};

typedef struct domain_configs domain_cst;

int drp_check_ip_format(const char *str);

static int
drp_inet_atoi (const char *cp, unsigned int  *inaddr)
{
	int dots = 0;
	register u_long addr = 0;
	register u_long val = 0, base = 10;

	do
	{
		register char c = *cp;

		switch (c)
		{
			case '0': case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				val = (val * base) + (c - '0');
				break;
			case '.':
				if (++dots > 3)
					return 0;
			case '\0':
				if (val > 255)
					return 0;
				addr = addr << 8 | val;
				val = 0;
				break;
			default:
				return 0;
		}
	} while (*cp++) ;

	if (dots < 3)
		addr <<= 8 * (3 - dots);
	if (inaddr)
		*inaddr = htonl (addr);
	return 1;
}



#endif

