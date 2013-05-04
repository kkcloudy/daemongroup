#ifndef _PPPOE_METHOD_DEF_H
#define _PPPOE_METHOD_DEF_H

enum {
	PPPOE_METHOD_NONE,
	PPPOE_METHOD_INSTANCE_SHOW_VRRP_INFO,
	PPPOE_METHOD_INSTANCE_NOTITY_VRRP_BACKUP,
	PPPOE_METHOD_INSTANCE_BACKUP_TASK,	
	PPPOE_METHOD_SESSION_WIRELESS_INFO_SHOW,
	PPPOE_METHOD_SESSION_STAT_UPDATE,
	PPPOE_METHOD_SESSION_KICK_BY_SESSION_ID,
	PPPOE_METHOD_SESSION_KICK_BY_MAC,
	PPPOE_METHOD_SESSION_ONLINE_SYNC,
	PPPOE_METHOD_SESSION_OFFLINE_SYNC,
	PPPOE_METHOD_SESSION_UPDATE_SYNC,
	PPPOE_METHOD_SESSION_CLEAR,
	PPPOE_METHOD_SESSION_CLEAR_V2,
	PPPOE_METHOD_SESSION_SYNC,
	PPPOE_METHOD_SESSION_SYNC_FINISHED,
	PPPOE_METHOD_NUMS,
};

struct instance_para {
	uint32 slot_id;
	uint32 local_id;
	uint32 instance_id;
};

struct vrrp_instance_info {
	uint32 state;
	uint32 local_id;
	uint32 instance_id;

	uint32 heartlink_local_ip;
	uint32 heartlink_opposite_ip;
};

struct instance_sync {
	uint16 code;
	uint16 dev_id;
};

struct session_info {	
	uint32 slot_id;
	uint32 local_id;
	uint32 instance_id;
	
	uint32 sid;
	uint8 mac[ETH_ALEN];
	char ifname[IFNAMSIZ];	
};
	
struct session_wireless_info {
	struct session_info info;
	
	uint32 wtp_id;
	uint32 radio_g_id;
	uint32 bssindex;
	uint32 vlan_id;
	uint32 auth_type;
	uint8 wtpmac[ETH_ALEN];

	uint8 wlan_id;
	uint8 security_id;
	uint8 radio_l_id;	
};

#endif
