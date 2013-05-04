#ifndef _STP_OUT_H__
#define _STP_OUT_H__

/* In the best case: clean all Learning entries with
 the vlan_id and the port (if 'exclude'=0) or for all ports,
 exclude the port (if ''exclude'=1). If 'port'=0, delete all
 entries with the vlan_id, don't care to 'exclude'  */


#define P2P_YES              1
#define P2P_NO               0

#if 0
typedef enum{
      ETH_ATTR_DUPLEX_FULL = 0,
	 ETH_ATTR_DUPLEX_HALF
}DUPLEX_MODE;
#endif

typedef enum {
  LT_FLASH_ALL_PORTS_EXCLUDE_THIS,
  LT_FLASH_ONLY_THE_PORT
} LT_FLASH_TYPE_T;

typedef enum
{
    STP_PORT_SPEED_10_E = 0,
    STP_PORT_SPEED_100_E,
    STP_PORT_SPEED_1000_E,
    STP_PORT_SPEED_10000_E,
    STP_PORT_SPEED_12000_E,
    STP_PORT_SPEED_2500_E,
    STP_PORT_SPEED_5000_E
}STP_PORT_SPEED_ENT;

int
stp_to_flush_lt (IN int port_index, IN int vlan_id,
                  IN LT_FLASH_TYPE_T type, IN char* reason);

void /* for bridge id calculation */
stp_to_get_port_mac (IN int port_index, OUT unsigned char* mac);

unsigned long
stp_to_get_port_oper_speed (PORT_T *port);

int /* 1- Up, 0- Down */
stp_to_get_port_link_status (IN int port_index);

int /* 1- Full, 0- Half */
stp_to_get_duplex (PORT_T *port);

#ifdef STRONGLY_SPEC_802_1W
int
stp_to_set_learning (IN int port_index, IN int vlan_id, IN int enable);

int
stp_to_set_forwarding (IN int port_index, IN int vlan_id, IN int enable);
#else
/*
 * In many kinds of hardware the state of ports may
 * be changed with another method
 */
int
stp_to_set_port_state (IN int port_index, IN int vlan_id, IN RSTP_PORT_STATE state);
#endif

int
stp_to_set_hardware_mode (int vlan_id, UID_STP_MODE_T mode);

int
stp_to_tx_bpdu (IN int port_index, IN int vlan_id,
                 IN unsigned char* bpdu,
                 IN size_t bpdu_len,            
				IN unsigned int isWAN,
				IN unsigned int slot_no,
				IN unsigned int port_no);

const char *
stp_to_get_port_name (IN int port_index);

int
stp_to_get_init_stpm_cfg (IN int vlan_id,
                           INOUT UID_STP_CFG_T* cfg);


int
stp_to_get_init_port_cfg (IN int vlan_id,
                           IN int port_index,
                           INOUT UID_STP_PORT_CFG_T* cfg);


#endif /* _STP_OUT_H__ */

