#ifndef _STP_PARAM__H_
#define _STP_PARAM__H_

#define STP_NOT_ENABLED  0xff
#define STP_HAVE_ENABLED 0xfe
#define STP_PORT_HAVE_ENABLED 0xfc
#define STP_PORT_NOT_ENABLE 0xfd
#define STP_PORT_NOT_EXIST  0xfb
#define STP_Cannot_Find_Vlan 2


#define STP_DBUS_DEBUG(x) printf x
typedef struct vlan_ports_t {
	unsigned short vid;
	struct tagBITMAP ports;
}VLAN_PORTS_T;

typedef struct port_member{
    unsigned int portMbr[2];
}PORT_MEMBER_BMP;

struct port_duplex_mode{
    unsigned int port_index;
	unsigned int port_duplex_mode;
};

int  stp_param_set_stpm_enable();

int stp_param_set_stpm_disable();

void stp_param_set_running_mode(unsigned int type);

int stp_param_get_running_mode();

int stp_param_set_stpm_port_disable
(
	unsigned int port_index,
	int lkState,
	unsigned int speed, 
	unsigned int isWAN,
	unsigned int slot_no,
	unsigned int port_no
);

int stp_param_set_stpm_port_enable
(
	unsigned int port_index,
	int lkState,
	unsigned int speed, 
	unsigned int isWAN,
	unsigned int slot_no,
	unsigned int port_no
);

int add_ports_from_mstp(unsigned short vid,unsigned int port_index);

int del_ports_from_mstp(unsigned short vid,unsigned int port_index);

int stp_param_add_vlan_to_mstp(unsigned short vid,struct tagBITMAP ports);

int del_vlan_from_mstp(unsigned short vid);

int stp_param_set_vlan_to_instance (unsigned short vid, int mstid);

int stp_set_debug(int debug);

int stp_param_set_bridge_region_name(char* name);

int stp_param_set_bridge_revision(unsigned short value);

int stp_param_set_bridge_priority(int mstid,int prio);

int stp_param_reset_bridge_priority(int mstpid);

int stp_param_set_bridge_maxage(int maxage);

int stp_param_reset_bridge_maxage(int mstid);

int stp_param_set_bridge_fdelay(int value);

int stp_param_reset_bridge_fdelay(int mstid);

int stp_param_set_bridge_fvers(int fvers);

int stp_param_reset_bridge_fvers();

int stp_param_set_bridge_hello_time(unsigned int value);

int stp_param_reset_bridge_hello_time(int mstid);

int stp_param_set_bridge_max_hops(unsigned int value);

int stp_param_reset_bridge_max_hops();

int stp_param_set_port_non_stp(int mstpid,unsigned int port_index,unsigned int isEn);

int stp_param_set_port_past_cost(int mstpid,int port_index,int past_cost);

int stp_param_reset_port_past_cost(int mstpid,int port_index);

int stp_set_port_priority(int mstpid,int port_index,int prio);

int stp_param_reset_port_priority(int mstpid,int port_index);

int stp_param_set_port_p2p(int mstpid,int port_index,unsigned int value);

int stp_param_reset_port_p2p(int mstpid,int port_index);

int stp_param_set_port_edge(int mstpid,int port_index,unsigned char value);

int stp_reseet_port_edge(int mstpid,int port_index);

int stp_param_set_port_mcheck(int mstpid,int port_index,unsigned char value);

int stp_param_reset_port_mcheck(int mstpid,int port_index);

 int stp_param_set_bridge_nocfg (int mstpid);

int stp_param_set_port_cfg_defaultvalue (int mstpid, int port_index);

#if 0
int stp_bridge_get_cfg ( );

void stp_mgmt_print_bridge_id (UID_BRIDGE_ID_T* bridge_id, unsigned char cr);

void stp_show_rstp_port (BITMAP_T* ports_bitmap, int detail);
#endif

int stp_param_set_debug_value(unsigned int val_mask);

int stp_param_set_no_debug_value(unsigned int val_mask);

void stp_param_get_port_state(int mstid,unsigned int port_index,UID_STP_PORT_STATE_T* portInfo);

int stp_param_check_stp_state();

int stp_param_get_mstp_vlan_map_info(int mstid,unsigned short* map,int* count);

int stp_param_check_port_if_in_mst(int mstid,unsigned int port_index);

void stp_param_mstp_init_vlan_info(unsigned short vid,struct tagBITMAP ports);

int stp_param_save_running_cfg(char* buf,unsigned int bufLen);

#endif
