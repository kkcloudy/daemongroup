#ifndef __DCLI_FDB_H__
#define __DCLI_FDB_H__

#define MAX_VLANID 4094
#define MIN_VLANID 1
#define MIN_SLOT   1
#define MAX_SLOT   4
#define MIN_PORT   1
#define MAX_PORT   24
#define ALIAS_NAME_SIZE 0x15

#define CPU_PORT_VIRTUAL_SLOT	 0x1F
#define CPU_PORT_VIRTUAL_PORT	 0x3F /* for CPU port (*,63)*/
#define SPI_PORT_VIRTUAL_SLOT	 CPU_PORT_VIRTUAL_SLOT
#define SPI_PORT_VIRTUAL_PORT	 0x1A /* for SPI port (*,26)*/

enum{
	NPD_FDB_ERR_NONE =0,
	NPD_FDB_ERR_DBUS,
	NPD_FDB_ERR_GENERAL,
	NPD_FDB_ERR_BADPARA,
	NPD_FDB_ERR_OCCUR_HW,
	NPD_FDB_ERR_ITEM_ISMIRROR,
	NPD_FDB_ERR_NODE_EXIST ,
	NPD_FDB_ERR_NODE_NOT_EXIST,
	NPD_FDB_ERR_PORT_NOTIN_VLAN,
	NPD_FDB_ERR_VLAN_NONEXIST,
	NPD_FDB_ERR_SYSTEM_MAC,
	NPD_FDB_ERR_VLAN_NO_PORT,
	NPD_FDB_ERR_HW_NOT_SUPPORT,
	NPD_FDB_ERR_MAX
};

enum{
	port_type =0,
	trunk_type,
	vidx_type,
	vid_type
};

typedef struct
{
    unsigned char       arEther[6];
}ETHERADDR;


typedef struct {
	 unsigned short 	 vlanid;
	 unsigned char		 ether_mac[6];
	 unsigned int		 inter_type;
	 unsigned int		 value;
	 unsigned int        type_flag;
	 unsigned int        dev;
	 unsigned int 		 asic_num;
	 unsigned int        slot_id;
}DCLI_FDB_DBG;

__inline__ int parse_mac_addr(char* str,ETHERADDR* macAddr) ;
__inline__ int parse_agetime(char* str,unsigned short* agingtime);

int dcli_fdb_show_running_config( struct vty *vty);

#endif
