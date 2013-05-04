#ifndef __DCLI_COMMON_STP_H__
#define __DCLI_COMMON_STP_H__

extern int dcli_debug_out;
#define DCLI_DEBUG(x) if(dcli_debug_out){printf x ;}

#define DCLI_STP_OK 0
#define DCLI_STP_INVALID_PARAM (DCLI_STP_OK+1)
#define DCLI_STP_NO_SUCH_MSTID (DCLI_STP_OK+2)

typedef enum {
  DisabledPort = 0,
  AlternatePort,
  BackupPort,
  RootPort,
  DesignatedPort,
  NonStpPort
} PORT_ROLE_T;

typedef enum{
	DISCARDING,
	LEARNING,
	FORWARDING
} PORT_STATE;

typedef struct{
		unsigned char local_port_no;
		unsigned int port_index;
}PORT_INFO;

typedef struct{
	unsigned char slot_no;
	unsigned char local_port_count;
	PORT_INFO port_no[6];
}SLOT_INFO;

typedef enum {
	DCLI_STP_M = 0,
	DCLI_MST_M, 
	DCLI_NOT_M
}DCLI_STP_RUNNING_MODE;

typedef struct{
	unsigned short vid;
	PORT_MEMBER_BMP untagbmp;
	PORT_MEMBER_BMP tagbmp;
}VLAN_PORTS_BMP;

int dcli_enable_g_stp_to_protocol
(
	struct vty* vty,
	DCLI_STP_RUNNING_MODE mode,
	unsigned int isEnable
);


/**********************************************************************************
 *  dcli_get_broad_product_id
 *
 *	DESCRIPTION:
 * 		get broad type
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
int dcli_stp_function_support(struct vty* vty, unsigned char *IsSuport);

int dcli_get_brg_g_state(struct vty* vty, int *stpmode);

int dcli_enable_stp_on_one_port_to_protocol
(
	struct vty*	vty,
	unsigned int port_index,
	unsigned int enable,
	int lkState,
	unsigned int speed,
	unsigned int isWAN,
	unsigned int slot_no,
	unsigned int port_no
);

int dcli_enable_stp_on_one_port_to_npd
(
	struct vty*	vty,	
	unsigned int mode,
	unsigned int port_index,
	unsigned int enable
);

 int dcli_get_one_port_index
(
	struct vty*		vty,
	unsigned char slot,
	unsigned char local_port,
	unsigned int* port_index
);

int dcli_get_all_ports_index
(
	struct vty*	 vty, 		
	PORT_MEMBER_BMP* portBmp
);

int show_slot_port_by_productid
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP* portBmp,
	unsigned int vid,
	unsigned int mstid
);

unsigned int dcli_get_one_port_info
(
	struct vty*	vty,
	unsigned int port_index,
	unsigned int portductid
);

unsigned int dcli_get_br_info
(
	struct vty* vty
);

int dcli_enable_g_stp_to_npd(struct vty* vty,unsigned int enable);

int dcli_get_port_index_link_state
(
	struct vty*   vty,
	unsigned int port_index,
	int* lkState,
	unsigned int *isWAN
);

int dcli_change_all_ports_to_bmp
(
	struct vty*				 vty,
	VLAN_PORTS_BMP* ports_bmp,
	unsigned int *num
);

int dcli_get_one_vlan_portmap
(
	struct vty*		vty,
	unsigned short vid,
	VLAN_PORTS_BMP* ports_bmp
);

int dcli_get_vlan_portmap
(
	struct vty*					vty,
	VLAN_PORTS_BMP** 	ports_bmp,
	unsigned int* 				count
);

int dcli_send_vlanbmp_to_mstp
(
	struct vty*				 vty,	
	VLAN_PORTS_BMP* ports_bmp
);

int dcli_get_mstp_one_port_info
(
	struct vty*	vty,
	int 				mstid,
	unsigned int port_index
);

int dcli_set_bridge_force_version
(
	struct vty* vty,
	unsigned int fversion
);

int dcli_stp_set_stpid_to_npd
(
	struct vty* vty,
	unsigned short vid,
	unsigned int mstid
);

int dcli_stp_set_port_prio_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
);

int dcli_stp_set_port_pathcost_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
);

int dcli_stp_set_port_edge_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
);

int dcli_stp_set_port_p2p_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
);

int dcli_stp_set_port_nonstp_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
);

/**********************************************************************************
 *  dcli_stp_set_stpid_to_npd
 *
 *	DESCRIPTION:
 * 		bind vlan id  and specifed stpid
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		CMD_SUCCESS
 *		CMD_FAILURE
 *
 **********************************************************************************/
int dcli_stp_set_stpid_to_npd
(
	struct vty* vty,
	unsigned short vid,
	unsigned int mstid
);
#endif
