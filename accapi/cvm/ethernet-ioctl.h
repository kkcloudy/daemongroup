/*
 *	filename: ethernet-ioctl.h
 *	Auther :sush@autelan.com
 *	13/May/2008
 *
 * */

#ifndef __ETHERNET_IOCTL_H__
#define __ETHERNET_IOCTL_H__

#define DEV_NAME "/dev/oct0"


/**
  * Structure for encapsulating param to ioctl.
  */
typedef struct ve_dev_priv
{
	int portNum;
	int devNum;
	char name[15];
}ve_netdev_priv;


typedef struct cvm_updown_times
{
	int pip_port;
	unsigned int up_times;
	unsigned int down_times;
	int 		 reseved_1;
	int 		 reseved_2;
}cvm_port_updown_times;

typedef struct cvm_link_timestamp
{
	int 			pip_port;
	unsigned long long	timestamp;
	int 			reseved_1;
}cvm_link_timestamp;
/*added by huxuefeng 20100205start*/
typedef struct
{
	int phy_addr;
	int location;
	int read_reg_val;
	int write_reg_val;
}phy_read_write_t;

/*added by huxuefeng 20100205end*/


/**
  * Structure for encapsulating param passing to ioctl.
  * add by baolc
  */
#define VE_VLAN_IFNAME_SIZE      16
#define VE_MARVELL_DEV_COUNT     32
typedef struct ve_vlan_param_data_s
{
	unsigned short 				vid;		/* VLAN id */
	char						intfName[VE_VLAN_IFNAME_SIZE];	/* interface name */
	unsigned int                untagPortBitmap[VE_MARVELL_DEV_COUNT];  /* untagged bit map, 2 device */
	unsigned int                tagPortBitmap[VE_MARVELL_DEV_COUNT];    /* tagged bit map, 2 device */
}ve_vlan_param_data_s; /* vlan node */


#if 0
/**
  * Structures for vlan-port table.Or described it as interface info table.
  * add by baolc
  */
typedef struct ve_vlan_s
{
	unsigned short 				vid;		/* VLAN id */
	char						intfName[VE_VLAN_IFNAME_SIZE];	/* interface name */
	unsigned int                untagPortBitmap[VE_MARVELL_DEV_COUNT];  /* untagged bit map, 2 device */
	unsigned int                tagPortBitmap[VE_MARVELL_DEV_COUNT];    /* tagged bit map, 2 device */
	
	unsigned int                ifIndex;    /* interface index */
	spinlock_t                  lock;       /* spinlock for this vlan node */
}ve_vlan_s; /* vlan/interface info node */

#endif


/**
  * Param structure for manage fdb tables.
  */
typedef struct ve_fdb_param_data_s
{
	unsigned short    vid;      /* vlan id */
	int               devNum;   /* device number */
	int               portNum;  /* port number */
	unsigned char     DMAC[6];   /* mac */
}ve_fdb_param_data_s;



/*
  * Structure of the FDB broadcast array.
  * luoxun 2008-06-13
  */
typedef struct{
		int devNum;
		int portNum;
}DevPort;

typedef struct FDB_broadcast_array{
	int tagcount;
	int untagcount;
	DevPort tag[32];
	DevPort untag[32];
}FDB_BROADCAST_ARRAY;


/**
 * Status statistics for a port
 */
typedef struct
{
    unsigned int    dropped_octets;         /**< Inbound octets marked to be dropped by the IPD */
    unsigned int    dropped_packets;        /**< Inbound packets marked to be dropped by the IPD */
    unsigned int    pci_raw_packets;        /**< RAW PCI Packets received by PIP per port */
    unsigned int    octets;                 /**< Number of octets processed by PIP */
    unsigned int    packets;                /**< Number of packets processed by PIP */
    unsigned int    multicast_packets;      /**< Number of indentified L2 multicast packets.
                                            Does not include broadcast packets.
                                            Only includes packets whose parse mode is
                                            SKIP_TO_L2 */
    unsigned int    broadcast_packets;      /**< Number of indentified L2 broadcast packets.
                                            Does not include multicast packets.
                                            Only includes packets whose parse mode is
                                            SKIP_TO_L2 */
    unsigned int    len_64_packets;         /**< Number of 64B packets */
    unsigned int    len_65_127_packets;     /**< Number of 65-127B packets */
    unsigned int    len_128_255_packets;    /**< Number of 128-255B packets */
    unsigned int    len_256_511_packets;    /**< Number of 256-511B packets */
    unsigned int    len_512_1023_packets;   /**< Number of 512-1023B packets */
    unsigned int    len_1024_1518_packets;  /**< Number of 1024-1518B packets */
    unsigned int    len_1519_max_packets;   /**< Number of 1519-max packets */
    unsigned int    fcs_align_err_packets;  /**< Number of packets with FCS or Align opcode errors */
    unsigned int    runt_packets;           /**< Number of packets with length < min */
    unsigned int    runt_crc_packets;       /**< Number of packets with length < min and FCS error */
    unsigned int    oversize_packets;       /**< Number of packets with length > max */
    unsigned int    oversize_crc_packets;   /**< Number of packets with length > max and FCS error */
    unsigned int    inb_packets;            /**< Number of packets without GMX/SPX/PCI errors received by PIP */
    unsigned long long    inb_octets;             /**< Total number of octets from all packets received by PIP, including CRC */
    unsigned short    inb_errors;             /**< Number of packets with GMX/SPX/PCI errors received by PIP */
    unsigned int    mcast_l2_red_packets;   /**< Number of packets with L2 Multicast DMAC
                                             that were dropped due to RED.
                                             The HW will consider a packet to be an L2
                                             multicast packet when the least-significant bit
                                             of the first byte of the DMAC is set and the
                                             packet is not an L2 broadcast packet.
                                             Only applies when the parse mode for the packets
                                             is SKIP-TO-L2 */ 
    unsigned int    bcast_l2_red_packets;   /**< Number of packets with L2 Broadcast DMAC
                                             that were dropped due to RED.
                                             The HW will consider a packet to be an L2
                                             broadcast packet when the 48-bit DMAC is all 1's.
                                             Only applies when the parse mode for the packets
                                             is SKIP-TO-L2 */ 
    unsigned int    mcast_l3_red_packets;   /**< Number of packets with L3 Multicast Dest Address
                                             that were dropped due to RED.
                                             The HW considers an IPv4 packet to be multicast
                                             when the most-significant nibble of the 32-bit
                                             destination address is 0xE (i.e it is a class D
                                             address). The HW considers an IPv6 packet to be 
                                             multicast when the most-significant byte of the
                                             128-bit destination address is all 1's.
                                             Only applies when the parse mode for the packets
                                             is SKIP-TO-L2 and the packet is IP or the parse
                                             mode for the packet is SKIP-TO-IP */ 
    unsigned int    bcast_l3_red_packets;   /**< Number of packets with L3 Broadcast Dest Address
                                             that were dropped due to RED.
                                             The HW considers an IPv4 packet to be broadcast
                                             when all bits are set in the MSB of the
                                             destination address. IPv6 does not have the 
                                             concept of a broadcast packets.
                                             Only applies when the parse mode for the packet
                                             is SKIP-TO-L2 and the packet is IP or the parse
                                             mode for the packet is SKIP-TO-IP */ 
} cvmx_pip_port_status_t;


typedef struct
{
    unsigned int    packets;
    unsigned long long    octets;
    unsigned long long    doorbell;
} cvmx_pko_port_status_t;


typedef struct port_counter_s
{
	char                   ifName[21];
	int                    clear;
    cvmx_pip_port_status_t pip_stats;
    cvmx_pko_port_status_t pko_stats;
}port_counter_t;

typedef struct macAddr_data_s
{
    unsigned int slot_id;
	unsigned char hwaddr[6];
}macAddr_data_t;

/**
  * Command number for ioctl.
  */
#define CVM_IOC_MAGIC 242
#define CVM_IOC_MAXNR 32

#define CVM_IOC_REG_                _IOWR(CVM_IOC_MAGIC,1,ve_netdev_priv) /* read values*/
#define CVM_IOC_UNREG_              _IOWR(CVM_IOC_MAGIC,2,ve_netdev_priv) /* read values*/

/*add by baolc*/
#define CVM_IOC_REG_INTF_           _IOWR(CVM_IOC_MAGIC, 3, ve_vlan_param_data_s) /* read values*/
#define CVM_IOC_UNREG_INTF_         _IOWR(CVM_IOC_MAGIC, 4, ve_vlan_param_data_s) /* read values*/
#define CVM_IOC_UPDATE_INTF_        _IOWR(CVM_IOC_MAGIC, 5, ve_vlan_param_data_s) /* read values*/


/*cmd for manage tables, add by baolc*/
#define CVM_IOC_DISPLAY_INTF_TABLE  _IOWR(CVM_IOC_MAGIC, 6, int) 
#define CVM_IOC_DISPLAY_FDB_TABLE   _IOWR(CVM_IOC_MAGIC, 7, int) 
#define CVM_IOC_FDB_ADD_ITEM        _IOWR(CVM_IOC_MAGIC, 8, ve_fdb_param_data_s) 
#define CVM_IOC_FDB_DEL_ITEM        _IOWR(CVM_IOC_MAGIC, 9, ve_fdb_param_data_s) 

/*cmd for manage tables, add by sush*/
#define CVM_IOC_INTF_UP 			_IOWR(CVM_IOC_MAGIC, 10, ve_vlan_param_data_s) 
#define CVM_IOC_INTF_DOWN 			_IOWR(CVM_IOC_MAGIC, 11, ve_vlan_param_data_s) 
#define CVM_IOC_DEV_UP 				_IOWR(CVM_IOC_MAGIC, 12, ve_netdev_priv) 
#define CVM_IOC_DEV_DOWN 			_IOWR(CVM_IOC_MAGIC, 13, ve_netdev_priv) 
#define CVM_IOC_GET_UPDOWN_TIMES 	_IOWR(CVM_IOC_MAGIC, 14, cvm_port_updown_times) 

/*added by huxuefeng20100205start*/
#define CVM_IOC_PHY_READ			_IOWR(CVM_IOC_MAGIC, 15, phy_read_write_t)
#define CVM_IOC_PHY_WRITE			_IOWR(CVM_IOC_MAGIC, 16, phy_read_write_t)
/*added by huxuefeng 20100205end*/

#define CVM_IOC_ETH_LINK_TIMESTAMP 	_IOWR(CVM_IOC_MAGIC, 17, cvm_link_timestamp)

#define CVM_IOC_ADV_EN_             _IOWR(CVM_IOC_MAGIC,18,ve_netdev_priv) 
#define CVM_IOC_ADV_DIS_            _IOWR(CVM_IOC_MAGIC,19,ve_netdev_priv) 

#define CVM_IOC_ADV_EN_INTF_        _IOWR(CVM_IOC_MAGIC,20,ve_vlan_param_data_s) 
#define CVM_IOC_ADV_DIS_INTF_       _IOWR(CVM_IOC_MAGIC,21,ve_vlan_param_data_s) 

#define CVM_IOC_GET_ETH_STAT        _IOWR(CVM_IOC_MAGIC, 22, port_counter_t) 
#define CVM_IOC_GET_ETHER_HWADDR    _IOWR(CVM_IOC_MAGIC, 23, macAddr_data_t)


/*#define VE_VLAN_IFNAME_SIZE      32*/
#define VE_VLAN_TABLE_MAX_NUM    (4*1024)
#define VE_MAX_PORT_NUM          27



#define VE_TRUE   1
#define VE_FALSE  0

#define VE_SUCCESS   0
#define VE_FAILED    (-1)

#define VE_IS_TAG_PORT   1
#define VE_IS_UNTAG_PORT 0
#define VE_ERR_PORT    (-1)


/**
  * Function for query tag or untag  by portnum and vid.
  * param(out):  intfIdx    interface's index, use 0 if no need.
  * return:  VE_IS_TAG_PORT or VE_IS_UNTAG_PORT or VE_ERR_PORT
  */
int ve_query_port_istag(unsigned short vid, int devNum, int portNum, int* intfIdx);


/**
  * Function for query interface index  by portnum and vid.
  * return:  interface index or VE_ERR_PORT
  */
int ve_query_port_intf_index(unsigned short vid, int devNum, int portNum);



/*********************************************************************/
/** for ioctl use ********************************************************/


/**
  * Init vlan/interface info table.
  * add by baolc
  * 2008-06-11
  * return: VE_FAILED for failure, VE_SUCCESS for success
  */
int ve_init_intf_info_table(void);


/**
  * Destroy vlan/interface info table.
  * add by baolc
  * 2008-06-12
  * return: VE_FAILED for failure, VE_SUCCESS for success
  */
int ve_destroy_intf_info_table(void);



/**
  * Create vlan/interface info(add data into vlan/interface info table).
  * add by baolc
  * 2008-06-11
  * return: VE_FAILED for failure, VE_SUCCESS for success
  */
int ve_add_intf_info_by_vid(unsigned short vlanId, struct ve_vlan_param_data_s* vlan_data, unsigned int intfIdx);


/**
  * Delete vlan/interface info from vlan/interface info table.
  * add by baolc
  * 2008-06-11
  * return: VE_FAILED for failure, VE_SUCCESS for success
  */
int ve_delete_intf_info_by_vid(unsigned short vlanId);


/**
  * Update vlan's port map.
  * add by baolc
  * 2008-06-11
  * return: VE_FAILED for failure, VE_SUCCESS for success
  */
int ve_update_intf_port_map(unsigned short vlanId, struct ve_vlan_param_data_s* vlan_data);


/**
  * Deprecated!!No use!!
  * Get idle index.
  * add by baolc
  */
int ve_get_intf_idle_index(void);

int ve_query_all_port(FDB_BROADCAST_ARRAY* b_array, unsigned short vlanId);
void ve_display_all_port(FDB_BROADCAST_ARRAY* b_array, unsigned short vid);


#endif
