
#ifndef _RDC_INTERFACE_H
#define _RDC_INTERFACE_H

#include <dbus/dbus.h>
#include <stdint.h>

/*define hansi type!*/

#define RADIUS_SECRETSIZE               128	/* No secrets that long */

typedef enum{ 
	HANSI_REMOTE = 0,
	HANSI_LOCAL
} HANSI_TYPE;

#define RDC_TIMEOUT_DEFAULT 				12
#define MAX_RADIUS_SRV_NUM				16

struct rdc_base_conf {
	int status;
	uint32_t nasip;
	uint32_t timeout;
};

/*add for RDC response DM message when no user-session was found*/
struct radius_srv_coa {
	uint32_t auth_ip;
	uint16_t auth_port;
	char auth_secret[RADIUS_SECRETSIZE];
	uint32_t auth_secretlen;
};

struct rdc_coa_radius_conf{
	int current_num;
	struct radius_srv_coa radius_srv[MAX_RADIUS_SRV_NUM];
};
/*end */

int
rdc_intf_set_nasip(DBusConnection *connection,
						int hansitype, int insid,
						uint32_t nasip);

int
rdc_intf_set_timeout(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t timeout);

int 
rdc_intf_set_status(DBusConnection *connection,
							int hansitype, int insid,
							int status);

int
rdc_intf_get_base_conf(DBusConnection *connection,
						int hansitype, int insid,
						struct rdc_base_conf *baseconf);

int
rdc_add_radius( DBusConnection *connection, 
							int hansitype, int insid, 					
							unsigned long auth_ip,
							unsigned short auth_port,
							char *auth_secret );
int
rdc_del_radius( DBusConnection *connection, 
							int hansitype, int insid, 
							unsigned long auth_ip,
							unsigned short auth_port,
							char *auth_secret );

int
rdc_get_radius_conf(DBusConnection *connection, 
							int hansitype, int insid, 
							char *domain,
							struct rdc_coa_radius_conf *radiusconf );

int
rdc_add_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter);

int
rdc_del_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter);

int
rdc_log_all_packetconn( DBusConnection *connection,
							int hansitype,int insid);

int
rdc_log_all_sockclient( DBusConnection *connection,
							int hansitype,int insid);

#endif

