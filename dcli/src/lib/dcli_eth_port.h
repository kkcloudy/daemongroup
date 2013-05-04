#ifndef __DCLI_ETH_PORT_H__
#define __DCLI_ETH_PORT_H__

#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'
#define SLOT_PORT_SPLIT_COMMA 	','
#define ETHERNET_STR "Config ethernet information\n"
#define MAX_VLANID 4094
#define MIN_VLANID 1
#define MIN_SLOT   1
#define MAX_SLOT   4
#define MIN_PORT   1
#define MAX_PORT   6
#define NAM_ERR_HW 7
#define DCLI_SHOW_ARP_DETAIL_INFO 10
#define BUFFER_MODE_SHARED 0
#define BUFFER_MODE_DIVIDED 1

#define DCLI_ETH_PORT_ALREADY_RUN_THIS_MODE 55
#define DCLI_ETH_PORT_ALREADY_IN_L3_VLAN   56
#define DCLI_DEFAULT_VLAN_IS_L3_VLAN    57
#define DCLI_ETH_PORT_HAVE_SUBIF        58

#define	PORT_STORM_CONTROL_STREAM_DLF            0x01
#define	PORT_STORM_CONTROL_STREAM_MCAST          0x02
#define	PORT_STORM_CONTROL_STREAM_BCAST          0x04
#define	PORT_STORM_CONTROL_STREAM_UCAST          0x08  
#define	PORT_STORM_CONTROL_STREAM_ALL          (PORT_STORM_CONTROL_STREAM_DLF | \
	                                            PORT_STORM_CONTROL_STREAM_MCAST | \
	                                            PORT_STORM_CONTROL_STREAM_MCAST| \
	                                            PORT_STORM_CONTROL_STREAM_UCAST) 
	                                            
#define DCLI_ERROR_NONE          0 
#define DCLI_ERROR_DBUS         (DCLI_ERROR_NONE + 1)

typedef enum{
  DCLI_ETH_PORT_STREAM_PPS_E,
  DCLI_ETH_PORT_STREAM_BPS_E,
  DCLI_ETH_PORT_STREAM_MAX_E
}DCLI_ETH_PORT_STREAM_MODE_E;
__inline__ int parse_slotport_no
(
	char *str,
	unsigned char *slotno,
	unsigned char *portno
);

int parse_ve_slot_cpu_tag_no
(
    char *str,unsigned char *slotno,
    unsigned char *cpu_no,
    unsigned char *cpu_port_no,
    unsigned int *tag1, unsigned int *tag2
);	

int dcli_eth_port_mode_config
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no,
    unsigned int mode
);

struct ethport_list_s
{
	char slot_no;
	char port_no;
	char media_type;
	unsigned int attr_map;
	unsigned int mtu;
};


#endif
