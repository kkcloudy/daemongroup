#ifndef __DCLI_TRUNK_H__
#define __DCLI_TRUNK_H__

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  !FALSE
#endif

__inline__ int parse_trunk_no(char * str, unsigned short * trunkId);
__inline__ int parse_vid_list(char* ptr,int* count,short vid[]);
extern int parse_slotno_localport(char* str,unsigned int*slotno,unsigned int *portno) ;

#define ALIAS_NAME_SIZE 		0x15
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define VID_MAX_NUM  0xFFF 
#define VID_SPLIT_DASH 		'-'
#define VID_SPLIT_SLASH		','
#define VID_LLEGAL(vid)     ((vid)>0&&(vid)<VID_MAX_NUM)


#define TRUNK_PORT_EXISTS_GTDB	0xf		/*port already exists in trunkDB*/
#define TRUNK_PORT_MBRS_FULL	(TRUNK_PORT_EXISTS_GTDB+1)
#define NPD_TRUNK_ERR_NONE		NPD_SUCCESS
#define NPD_TRUNK_ERR_GENERAL	(NPD_TRUNK_ERR_NONE + 20)		/*general failure*/
#define NPD_TRUNK_BADPARAM			(NPD_TRUNK_ERR_NONE + 22)  		/*bad parameters*/
#define NPD_TRUNK_EXISTS			(NPD_TRUNK_ERR_NONE + 23)		/*vlan have been created already*/
#define NPD_TRUNK_NOTEXISTS		(NPD_TRUNK_ERR_NONE + 24)			/*vlan does not exists*/
#define NPD_TRUNK_ERR_HW			(NPD_TRUNK_ERR_NONE + 26) 		/*vlan error when operation on HW*/
#define NPD_TRUNK_PORT_EXISTS		(NPD_TRUNK_ERR_NONE + 27)		/*port already exists in trunk*/
#define NPD_TRUNK_PORT_NOTEXISTS	(NPD_TRUNK_ERR_NONE + 28)		/* port is not a member of trunk*/
#define NPD_TRUNK_PORT_MEMBERSHIP_CONFLICT	(NPD_TRUNK_ERR_NONE + 29)		/*port can NOT be Untag member of different vlans.*/ 
#define NPD_TRUNK_NAME_CONFLICT		(NPD_TRUNK_ERR_NONE + 30)	
#define NPD_TRUNK_MEMBERSHIP_CONFICT		(NPD_TRUNK_ERR_NONE + 31)
#define NPD_TRUNK_ALLOW_ERR				(NPD_TRUNK_ERR_NONE +32)
#define NPD_TRUNK_REFUSE_ERR				(NPD_TRUNK_ERR_NONE +33)
#define NPD_TRUNK_MEMBER_ADD_ERR	(NPD_TRUNK_ERR_NONE + 34)
#define NPD_TRUNK_MEMBER_DEL_ERR	(NPD_TRUNK_ERR_NONE + 35)
#define NPD_TRUNK_GET_ALLOWVLAN_ERR	(NPD_TRUNK_ERR_NONE + 36)
#define NPD_TRUNK_NO_MEMBER			(NPD_TRUNK_ERR_NONE + 37)
#define NPD_TRUNK_SET_TRUNKID_ERR	(NPD_TRUNK_ERR_NONE + 38)
#define NPD_TRUNK_DEL_MASTER_PORT	(NPD_TRUNK_ERR_NONE + 39)		/*master port NOT allowd to delete.*/
#define NPD_TRUNK_PORT_ENABLE		(NPD_TRUNK_ERR_NONE + 40)		/*port enalbe in trunk*/
#define NPD_TRUNK_PORT_NOTENABLE	(NPD_TRUNK_ERR_NONE + 41)		/* port disable in trunk*/
#define NPD_TRUNK_ALLOW_VLAN		(NPD_TRUNK_ERR_NONE + 42)
#define NPD_TRUNK_NOTALLOW_VLAN		(NPD_TRUNK_ERR_NONE + 43)
#define NPD_TRUNK_LOAD_BANLC_CONFLIT (NPD_TRUNK_ERR_NONE + 44)		/*trunk load balance mode same to original*/
#define NPD_TRUNK_VLAN_TAGMODE_ERR (NPD_TRUNK_ERR_NONE + 45)
#define NPD_TRUNK_PORT_LINK_DOWN		    (NPD_TRUNK_ERR_NONE + 46)
#define NPD_TRUNK_UNSUPPORT         (NPD_TRUNK_ERR_NONE + 47)
#define NPD_TRUNK_PORT_CONFIG_DIFFER (NPD_TRUNK_ERR_NONE + 48) /*port has a differ configuration with master port*/
#define NPD_TRUNK_PORT_L3_INTF		(NPD_TRUNK_ERR_NONE + 61)

#define	DEFAULT_TRUNK_ID			0x0								/*NOT member of any trunk*/


#define ETH_PORT_NUM_MAX		0x6
#define EXTENDED_SLOT_NUM_MAX	0x4
#define TRUNK_MEMBER_NUM_MAX		0x8
#define TRUNK_CONFIG_SUCCESS	0x0
#define	TRUNK_CONFIG_FAIL		0xff

typedef enum {
	LOAD_BANLC_SRC_DEST_MAC = 0 ,
	LOAD_BANLC_SOURCE_DEV_PORT ,
	LOAD_BANLC_SRC_DEST_IP,
	LOAD_BANLC_TCP_UDP_RC_DEST_PORT ,
	LOAD_BANLC_MAC_IP,
	LOAD_BANLC_MAC_L4,
	LOAD_BANLC_IP_L4 ,
	LOAD_BANLC_MAC_IP_L4,
	LOAD_BANLC_MAX
}trkLoadBanlcMode;

int show_trunk_member_slot_port
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP mbrBmp_sp,
	PORT_MEMBER_BMP disMbrBmp_sp
);
unsigned int dcli_trunk_clear_trunk_arp
(
    struct vty * vty,
    unsigned int trunkId,
    unsigned int isStatic
);


#endif

