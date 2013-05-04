#ifndef __DCLI_VLAN_H__
#define __DCLI_VLAN_H__

#include "command.h"

#define TRUE 1
#define FALSE 0
#define ALIAS_NAME_SIZE 		0x15
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'
#define SLOT_PORT_SPLIT_COMMA 	','

__inline__ int parse_vlan_no(char* str,unsigned short* vlanId);
int param_first_char_check(char* str,unsigned int cmdtip);

#define NPD_VLAN_ERR_NONE			NPD_SUCCESS
#define NPD_VLAN_ERR_GENERAL		(NPD_VLAN_ERR_NONE + 10)		/*general failure*/
#define NPD_VLAN_BADPARAM			(NPD_VLAN_ERR_NONE + 12)  	/*bad parameters*/
#define NPD_VLAN_EXISTS			(NPD_VLAN_ERR_NONE + 13)		/*vlan have been created already*/
#define NPD_VLAN_NAME_CONFLICT	(NPD_VLAN_ERR_NONE + 14)
#define NPD_VLAN_NOTEXISTS		(NPD_VLAN_ERR_NONE + 15)		/*vlan does not exists*/
#define NPD_VLAN_ERR_HW			(NPD_VLAN_ERR_NONE + 16) 	/*vlan error when operation on HW*/
#define NPD_VLAN_PORT_EXISTS		(NPD_VLAN_ERR_NONE + 17)		/*port already exists in vlan*/
#define NPD_VLAN_PORT_NOTEXISTS	(NPD_VLAN_ERR_NONE + 18)		/* port is not a member of vlan*/
#define NPD_VLAN_PORT_MEMBERSHIP_CONFLICT	(NPD_VLAN_ERR_NONE + 19)		/*port can NOT be Untag member of different vlans.*/ 
#define NPD_VLAN_L3_INTF			(NPD_VLAN_ERR_NONE + 20)		/*vlan is L3 interface*/
#define NPD_VLAN_PORT_TAG_CONFLICT	(NPD_VLAN_ERR_NONE +21)
#define NPD_TRUNK_MEMBER_NONE		(NPD_VLAN_ERR_NONE +22)
#define NPD_VLAN_PORT_PROMISCUOUS_MODE_ADD2_L3INTF  (NPD_VLAN_ERR_NONE + 23)  /*promiscuous mode port add to l3 interface */
#define NPD_VLAN_PORT_DEL_PROMISCUOUS_PORT_DELETE (NPD_VLAN_ERR_NONE + 24) /*del promiscuous port but default is l3 intf*/
#define NPD_VLAN_SUBINTF_EXISTS     (NPD_VLAN_ERR_NONE + 26)    /* sub intf exists*/
#define NPD_VLAN_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN (NPD_VLAN_ERR_NONE + 28)  /*promis try to add to other vlan(not vlan 1)*/
#define NPD_VLAN_PORT_SUBINTF_EXISTS     (NPD_VLAN_ERR_NONE + 29)  /* port sub interface exists*/
#define NPD_VLAN_ERR_MAX			(NPD_VLAN_ERR_NONE + 255)
#define NPD_VLAN_PORT_L3_INTF		(NPD_VLAN_ERR_NONE + 61)
#define NPD_VLAN_TRUNK_EXISTS		(NPD_VLAN_ERR_NONE + 63)		
#define NPD_VLAN_TRUNK_NOTEXISTS	(NPD_VLAN_ERR_NONE + 64)		
#define NPD_VLAN_TRUNK_CONFLICT		(NPD_VLAN_ERR_NONE + 65)  /* vlan can not add more than one trunk */
#define NPD_VLAN_PORT_TRUNK_MBR		(NPD_VLAN_ERR_NONE + 66)  /* port belong to trunk ,it can NOT add to vlan as port*/
#define NPD_VLAN_TRUNK_MBRSHIP_CONFLICT	(NPD_VLAN_ERR_NONE + 67)
#define NPD_VLAN_NOT_SUPPORT_IGMP_SNP (NPD_VLAN_ERR_NONE + 70)
#define NPD_VLAN_IGMP_ROUTE_PORTEXIST (NPD_VLAN_ERR_NONE + 71)
#define NPD_VLAN_IGMP_ROUTE_PORTNOTEXIST (NPD_VLAN_ERR_NONE + 72)
#define NPD_VLAN_ARP_STATIC_CONFLICT (NPD_VLAN_ERR_NONE + 73)
#define	DEFAULT_VLAN_ID			0x1
#define	NPD_PORT_L3INTF_VLAN_ID	0xfff
#define NPD_MAX_VLAN_ID 4095

#define ETH_PORT_NUM_MAX		0x6
#define EXTENDED_SLOT_NUM_MAX	0x4
/* for distributed os, redefine the legal slot & port */
#if 1
#define SLOT_LLEGAL(slot_no)     ((slot_no)>0&&(slot_no)<=16)
#define PORT_LLEGAL(port_no)     ((port_no)>0&&(port_no)<=64)
#else
#define SLOT_LLEGAL(slot_no)     ((slot_no)>0&&(slot_no)<=4)
#define PORT_LLEGAL(port_no)     ((port_no)>0&&(port_no)<=24)
#endif
#define SPLIT_LEGAL(str_chr)     (SLOT_PORT_SPLIT_COMMA == str_chr || SLOT_PORT_SPLIT_SLASH == str_chr || SLOT_PORT_SPLIT_DASH == str_chr)           
#define CHAR_NOT_NUMBER(str_chr) (str_chr<'0'||str_chr>'9')   

typedef struct {
	unsigned int trunkMbr[4];
}TRUNK_MEMBER_BMP;

typedef struct {
	unsigned int portMbr[2];
}PORT_MEMBER_BMP;

typedef struct  {
       unsigned char slot;
       unsigned char port;
}SLOT_PORT_S;

struct VlanMembrship
{
	unsigned char isMbr;
	unsigned char slotNum;
	unsigned char portNum;
	unsigned char isTag;
};

typedef struct {
	unsigned short 	vlanId;
	char* 			vlanName;
	PORT_MEMBER_BMP	untagMbrBmp;
	PORT_MEMBER_BMP	tagMbrBmp;
	TRUNK_MEMBER_BMP untagTrkBmp;
	TRUNK_MEMBER_BMP tagTrkBmp;
	unsigned int vlanStat;
}vlanList;

typedef struct {
	unsigned short vidx;
    PORT_MEMBER_BMP   groupMbrBmp;
}groupList;

typedef enum
{
    VLAN_FILTER_UNKOWN_UC = 0,
    VLAN_FILTER_UNREG_NONIP_MC ,
    VLAN_FILTER_UNREG_IPV4_MC ,
    VLAN_FILTER_UNREG_IPV6_MC ,
    VLAN_FILTER_UNREG_NONIPV4_BC ,
   	VLAN_FILTER_UNREG_IPV4_BC ,
    VLAN_FILTER_TYPE_MAX,
} VLAN_FLITER_ENT;

int parse_slotno_localport(char* str,unsigned int*slotno,unsigned int *portno);
int parse_slotno_localport_include_slot0(char* str,unsigned int *slotno,unsigned int *portno) ;

/**************************************
*show_vlan_member_slot_port
*Params:
*		portmbrBmp0 - uint bitmap
*		portmbrBmp1 - uint bitmap
*
*Usage: show vlan membership with
*		in slot&port number.
**************************************/
int show_vlan_member_slot_port
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp,
	unsigned int * promisPortBmp
);

#endif
