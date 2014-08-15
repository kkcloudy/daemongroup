
#ifndef _PDC_INTERFACE_H
#define _PDC_INTERFACE_H

#include <dbus/dbus.h>
#include <stdint.h>

/*define hansi type!*/
typedef enum{ 
	HANSI_REMOTE = 0,
	HANSI_LOCAL
} HANSI_TYPE;

#define PDC_DEFAULT_PORTAL_PORT 	2000
#define MAX_PDC_MAP_NUM			128
#define MAX_PDC_IPV6_MAP_NUM 128

#define PORTAL_PROTOCOL_MOBILE		0
#define PORTAL_PROTOCOL_TELECOM		1

struct pdc_base_conf {
	int status;
	uint32_t nasip;
	uint16_t port;
	int portal_protocol;
};

struct pdc_intf_map {
	uint32_t userip;
	uint32_t usermask;
	uint8_t eag_slotid;
	uint8_t eag_hansitype;
	uint8_t eag_hansiid;
	uint32_t eag_ip;
	uint16_t eag_port;
};

struct pdc_intf_ipv6_map {
	uint32_t userip[4];
	int prefix_length;
	uint8_t eag_slotid;
	uint8_t eag_hansitype;
	uint8_t eag_hansiid;
	uint32_t eag_ip;
	uint16_t eag_port;
};


struct pdc_map_conf {
	uint32_t num;
	struct pdc_intf_map map[MAX_PDC_MAP_NUM];
};

struct pdc_ipv6_map_conf {
	uint32_t num;
	struct pdc_intf_ipv6_map map[MAX_PDC_IPV6_MAP_NUM];
};


int
pdc_intf_set_nasip(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t nasip);

int
pdc_intf_set_port(DBusConnection *connection,
							int hansitype, int insid,
							uint16_t port);
int 
pdc_intf_set_status(DBusConnection *connection,
							int hansitype, int insid,
							int status);

int
pdc_intf_get_base_conf(DBusConnection *connection,
						int hansitype, int insid,
						struct pdc_base_conf *baseconf);

int
pdc_intf_add_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip, uint32_t usermask,
							int  eag_slotid, int eag_insid);

int
pdc_intf_del_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip, uint32_t usermask);

int
pdc_intf_modify_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip, uint32_t usermask,
							int  eag_slotid, int eag_insid);

int
pdc_intf_show_maps(DBusConnection *connection,
							int hansitype, int insid,
							struct pdc_map_conf *map_conf);

int
pdc_intf_add_ipv6_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t useripv6[4], int prefix_length,
							int  eag_slotid, int eag_insid);

int
pdc_intf_del_ipv6_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t useripv6[4], int prefix_length);

int
pdc_intf_modify_ipv6_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t useripv6[4], int prefix_length,
							int  eag_slotid, int eag_insid);

int
pdc_intf_show_ipv6_maps(DBusConnection *connection,
							int hansitype, int insid,
							struct pdc_ipv6_map_conf *map_conf);

int
pdc_intf_set_portal_protocol(DBusConnection *connection,
							int hansitype, int insid,
							int32_t portal_protocol);

int
pdc_log_all_blkmem(DBusConnection *connection,
							int hansitype, int insid);

int
pdc_log_all_thread(DBusConnection *connection,
							int hansitype, int insid);

int
pdc_set_userconn( DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip,
							uint8_t eag_slotid,
							uint8_t eag_hansitype,
							uint8_t eag_hansiid );

int
pdc_log_userconn( DBusConnection *connection,
							int hansitype, int insid );

int
pdc_add_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter);

int
pdc_del_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter);

#endif

