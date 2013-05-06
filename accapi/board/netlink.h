#ifndef __NETLINK_U_LIB_H_
#define __NETLINK_U_LIB_H_

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#define SUCCESS  0
#define ERROR   -1
#define MAX_PAYLOAD         1024 /* maximum payload size*/
#define NETLINK_OCTEON      21
#define NETLINK_DISTRIBUTED 24
#define NETLINK_KSEM        23
#define NETLINK_RPA 25


#define ACTIVE_STDBY_SWITCH_EVENT 5

enum netlink_type
{
    LOCAL_BOARD,
	CROSS_BOARD,
    OVERALL_UNIT,
    NL_TYPE_MAX
};

enum netlink_msg_type
{
	BOARD_STATE_NOTIFIER_EVENT,
	SYSTEM_STATE_NOTIFIER_EVENT,
	ASIC_ETHPORT_UPDATE_EVENT,
	OCTEON_ETHPORT_LINK_EVENT,
	ASIC_VLAN_SYNC_ENVET,
	ACTIVE_STDBY_SWITCH_OCCUR_EVENT,
	ACTIVE_STDBY_SWITCH_COMPLETE_EVENT,
	POWER_STATE_NOTIFIER_EVENT,
	FAN_STATE_NOTIFIER_EVENT,
	ASIC_VLAN_NOTIFIER_ENVET,
	BASE_MAC_ADDRESS_CHANGE_EVENT,
	RPA_BROADCAST_MASK_ACTION_EVENT,
	RPA_DEBUG_CMD_EVENT,
	NL_MSG_TYPE_MAX
};

enum module_type
{
	COMMON_MODULE,
	SEM_MODULE,
	NPD_MODULE,
	MOUDLE_TYPE_MAX
};

typedef enum
{
    HOTPLUG_MODE,
    COMMAND_MODE,
    PANIC_MODE,
    SWITCH_MODE_MAX
}active_mcb_switch_type_t;

enum power_state
{
	POWER_ON,
	POWER_OFF,
	POWER_INSERTED,		
	POWER_REMOVED,
	VCC_ALARM,
	POWER_STATE_MAX
};

enum fan_state
{
	FAN_REMOVED,
	FAN_INSERTED,
	FAN_ALARM,
	FAN_NORMAL,
	FAN_STATE_MAX
};

typedef enum
{
	LOCAL_BROADCAST, /* Broadcast active mcb switch netlink message on local board which will switch to new active mcb */
	OTHER_BOARD_BROADCAST, /* Broadcast active mcb switch netlink message on the other boards */
	ALL_BROADCAST /* Broadcast active mcb switch netlink message on all boards */
}active_mcb_switch_netlink_broadcast_domin_t;

typedef struct mcb_switch_arg_s
{
	active_mcb_switch_type_t switch_type;
	int current_active_mcb_slot_id;
	active_mcb_switch_netlink_broadcast_domin_t broadcast_domain;
}mcb_switch_arg_t;

typedef struct kernel_nlmsg_s
{
    int action_type;
    int ipgport;
}kernel_nlmsg_t;

typedef struct nl_msg_head{
	int type;
	int object;
    int count;
	int pid;
}nl_msg_head_t;

typedef struct board_active_info{
	int action_type;
    int slotid;
}board_active_info_t;

typedef struct product_state_info_s{
	int action_type;
    int slotid;
}product_state_info_t;

typedef struct asic_port_update_s{
	unsigned char devNum;
	unsigned char virportNum;
	unsigned short trunkId;
	int action_type;
	int slot_id;
	int port_no;
	int eth_l_index;
}asic_port_update_t;

typedef struct port_bitmap_s{
    unsigned int low_bmp;
    unsigned int high_bmp;
}port_bitmap_t;
typedef struct asic_vlan_sync_s{
	int action_type;
	int slot_id;
	int vlan_id;
	unsigned int bond_slot[16];
	char vlan_name[21];
    port_bitmap_t untagPortBmp[16]; 
	port_bitmap_t tagPortBmp[16];	
}asic_vlan_sync_t;

typedef struct active_stdby_switch_s{
	int action_type;
    int active_slot_id;
}active_stdby_switch_t;

typedef struct power_state_info_t{
	int action_type;
    int power_id;
}power_state_info_t;

typedef struct fan_state_info_s{
	int action_type;
    int fan_id;
}fan_state_info_t;

typedef unsigned char mac_addr_t;

enum rpa_mask_action_type
{
	MASK_GET,
	MASK_SET,
	MASK_NOTIFY,
	DEFAULT_MASK_NOTIFY
};
typedef struct rpa_mask_action_s {
	int action_type;
	unsigned int mask;
}rpa_mask_action_t;

typedef enum rpa_debug_cmd {
	RPA_SHOW_DEV_INDEX_TABLE,
	CMD_MAX
}rpa_debug_cmd_t;

typedef struct netlink_msg_s{	
	int msgType;
	union{
	    product_state_info_t productInfo;
		kernel_nlmsg_t       kernelInfo;
		asic_port_update_t   portInfo;
		board_active_info_t  boardInfo;
		asic_vlan_sync_t     vlanInfo;
		active_stdby_switch_t swInfo; 
		power_state_info_t   powerInfo;
		fan_state_info_t     fanInfo;
		mac_addr_t base_mac_addr[6];
		rpa_mask_action_t broadcast_mask_action;
		rpa_debug_cmd_t debug_cmd_type;
	}msgData;	
}netlink_msg_t;


extern void nam_thread_create(char	*name,unsigned (*entry_point)(void*),void	*arglist,unsigned char accessChip,unsigned char needJoin);
extern int npd_netlink_init(void);
extern void npd_netlink_recv_thread(void);
extern int npd_asic_ehtport_update_notifier(unsigned int eth_g_index);
extern int npd_asic_vlan_sync(unsigned int act,unsigned int slot);
extern int npd_asic_creat_vlan(unsigned short vlanId, char *vlanName);
extern int npd_asic_vlan_notifier(unsigned int vlanId, unsigned int slotId);
extern int npd_asic_bond_vlan_to_slot(unsigned short vlanId, unsigned short slotId,unsigned int bond_info);
extern int npd_netlink_send(char *msgBuf, int len);
extern int npd_update_slotx_vlan_info(int slot, int state);
extern int npd_init_second_stage(void);
extern int npd_set_dev_map_table(void);

extern int sem_netlink_init();
extern void product_state_notifier(int action_type);
extern void sem_netlink_recv_thread(void);
extern int sem_netlink_send(char *msgBuf, int len);
extern int mcb_active_standby_switch_notifier(active_mcb_switch_type_t switch_type, int msg_type, 
	int active_master_slot_id, active_mcb_switch_netlink_broadcast_domin_t broadcast_domain);


#endif
