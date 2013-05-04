#ifndef __PPPOE_SNP_DBUS_H__
#define __PPPOE_SNP_DBUS_H__

#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <omapip/omapip_p.h>
#include <syslog.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/rtnetlink.h>
#include <bits/sockaddr.h>


#define SET_FLAG		1
#define CLR_FLAG		0

#define ENABLE			1
#define DISABLE			0

#define DHCP_DIRECT_BROADCAST_ENABLE		(1)
#define DHCP_DIRECT_BROADCAST_DISABLE		(0)

#define PPPOE_SNP_LOG_DEBUG			0x01
#define PPPOE_SNP_LOG_INFO			0x02
#define PPPOE_SNP_LOG_ERROR			0x04	
#define PPPOE_SNP_LOG_DEFAULT		0x00
#define PPPOE_SNP_LOG_ALL			(PPPOE_SNP_LOG_DEBUG | PPPOE_SNP_LOG_INFO | PPPOE_SNP_LOG_ERROR)


#define SIOCGIFUDFFLAGS	0x893d		/* get udf_flags			*/
#define SIOCSIFUDFFLAGS	0x893e		/* set udf_flags			*/
#define IFF_PPP_SNP		0x40		/* pppoe snooping */

#define MRU_MIN			60
#define MRU_MAX			1492

struct pppoe_snp_cfg {
	char ifname[IFNAMSIZ];
	unsigned short mru;
	struct pppoe_snp_cfg *next;
};


DBusMessage * 
dba_dbus_server_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * 
pppoe_snp_dbus_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * 
dhcp_pppoe_snooping_iface_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage*
pppoe_snp_dbus_set_debug_state
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);


int pppoe_snp_set_udf_flag(char *ifname, unsigned int flag,	unsigned int set_flag);
int pppoe_snp_iface_add(struct pppoe_snp_cfg *iface);
int pppoe_snp_iface_del(char *ifname);

#endif
