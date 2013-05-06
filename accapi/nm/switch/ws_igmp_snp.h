/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* ws_igmp_snp.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef __DCLI_IGMP_SNP_H__
#define __DCLI_IGMP_SNP_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ws_init_dbus.h"
#include <dbus/dbus.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>
#include "ws_public.h"

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"


#define IGMP_SNP_OK			0
#define IGMP_SNP_ERR		(IGMP_SNP_OK + 1)

#define NPD_MAX_VLAN_ID 4095
#define IGMP_SNP_DISABLE 0xff

#define IGMP_SNP_DBUS_DEBUG(x) printf x
#define IGMP_SNP_DBUS_ERR(x) printf x

#define IGMP_SNP_STR "Config IGMP Snooping Protocol\n"
#define IGMP_STR_CMP_LEN	4


struct igmp_vlan
{
  	unsigned char ip0;
	unsigned char ip1;
	unsigned char ip2;
	unsigned char ip3;
	unsigned short  vidx;
	unsigned short vlanId;	
	
   struct igmp_vlan *next;
};


struct igmp_port
{
 
	unsigned int slot;
	unsigned int port;

	unsigned int un_slot;
	unsigned int un_port;

   struct igmp_port *next;
};

struct igmp_sum
{
  	
	unsigned short vlanId;
	
	struct igmp_port igp; // 加*的时候，再给分配地址
	
    struct igmp_sum *next;
};

#endif

#ifndef __DCLI_COMMON_IGMP_SNP_H__
#define __DCLI_COMMON_IGMP_SNP_H__


#define NPD_IGMP_SNP_SUCCESS 0
#define NPD_IGMP_SNP_ERRO_HW (NPD_IGMP_SNP_SUCCESS+1)
#define NPD_IGMP_SNP_ERRO_SW (NPD_IGMP_SNP_SUCCESS+2)
#define NPD_IGMP_SNP_VLAN_NOTEXIST (NPD_IGMP_SNP_SUCCESS+3)
#define NPD_IGMP_SNP_NOTENABLE	 (NPD_IGMP_SNP_SUCCESS+4)
#define NPD_IGMP_SNP_NOTENABLE_VLAN	(NPD_IGMP_SNP_SUCCESS+5)
#define NPD_IGMP_SNP_HASENABLE_VLAN	(NPD_IGMP_SNP_SUCCESS+6)
#define NPD_IGMP_SNP_NOTENABLE_PORT	(NPD_IGMP_SNP_SUCCESS+7)
#define NPD_IGMP_SNP_HASENABLE_PORT	(NPD_IGMP_SNP_SUCCESS+8)
#define NPD_IGMP_SNP_PORT_NOTEXIST	(NPD_IGMP_SNP_SUCCESS+9)
#define NPD_IGMP_SNP_NOTROUTE_PORT (NPD_IGMP_SNP_SUCCESS+10)
#define IGMP_SNOOP_ERR_NOT_ENABLE_GLB	0xff
#define IGMP_SNOOP_ERR_OUTOF_RANGE	(IGMP_SNOOP_ERR_NOT_ENABLE_GLB+1)
#define IGMP_SNOOP_ERR_SAMEVALUE	6		/* set same value to igmp-snoop times */

#define NPD_GROUP_SUCCESS 	0
#define NPD_GROUP_NOTEXIST (NPD_GROUP_SUCCESS+1)
#define NPD_ROUTE_PORT_NOTEXIST (NPD_GROUP_SUCCESS+2)
#define BRG_MC_FAIL 0xff

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

typedef struct{
    unsigned int   ports[2];
}CPSS_PORTS_BMP_STC;



#endif


#ifndef __DCLI_VLAN_H__
#define __DCLI_VLAN_H__


#define TRUE 1
#define FALSE 0
#define ALIAS_NAME_SIZE 		0x15
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'
#define SLOT_PORT_SPLIT_COMMA 	','

extern inline int parse_vlan_no(char* str,unsigned short* vlanId);
extern int param_first_char_check(char* str,unsigned int cmdtip);

#define NPD_VLAN_ERR_NONE			NPD_SUCCESS
#define NPD_VLAN_ERR_GENERAL		(NPD_VLAN_ERR_NONE + 10)		//general failure
#define NPD_VLAN_BADPARAM			(NPD_VLAN_ERR_NONE + 12)  	//bad parameters
#define NPD_VLAN_EXISTS			(NPD_VLAN_ERR_NONE + 13)		//vlan have been created already
#define NPD_VLAN_NAME_CONFLICT	(NPD_VLAN_ERR_NONE + 14)
#define NPD_VLAN_NOTEXISTS		(NPD_VLAN_ERR_NONE + 15)		//vlan does not exists
#define NPD_VLAN_ERR_HW			(NPD_VLAN_ERR_NONE + 16) 	//vlan error when operation on HW
#define NPD_VLAN_PORT_EXISTS		(NPD_VLAN_ERR_NONE + 17)		//port already exists in vlan
#define NPD_VLAN_PORT_NOTEXISTS	(NPD_VLAN_ERR_NONE + 18)		// port is not a member of vlan
#define NPD_VLAN_PORT_MEMBERSHIP_CONFLICT	(NPD_VLAN_ERR_NONE + 19)		//port can NOT be Untag member of different vlans. 
#define NPD_VLAN_L3_INTF			(NPD_VLAN_ERR_NONE + 20)		//vlan is L3 interface
#define NPD_VLAN_PORT_TAG_CONFLICT	(NPD_VLAN_ERR_NONE +21)
#define NPD_TRUNK_MEMBER_NONE		(NPD_VLAN_ERR_NONE +22)
#define NPD_VLAN_PORT_PROMISCUOUS_MODE_ADD2_L3INTF  (NPD_VLAN_ERR_NONE + 23)  //promiscuous mode port add to l3 interface 
#define NPD_VLAN_PORT_DEL_PROMISCUOUS_PORT_TO_DEFAULT_VLAN_INTF (NPD_VLAN_ERR_NONE + 24) //del promiscuous port but default is l3 intf
#define NPD_VLAN_SUBINTF_EXISTS     (NPD_VLAN_ERR_NONE + 26)    // sub intf exists
#define NPD_VLAN_ERR_MAX			(NPD_VLAN_ERR_NONE + 255)
#define NPD_VLAN_PORT_L3_INTF		(NPD_VLAN_ERR_NONE + 61)
#define NPD_VLAN_TRUNK_EXISTS		(NPD_VLAN_ERR_NONE + 63)		
#define NPD_VLAN_TRUNK_NOTEXISTS	(NPD_VLAN_ERR_NONE + 64)		
#define NPD_VLAN_TRUNK_CONFLICT		(NPD_VLAN_ERR_NONE + 65)  // vlan can not add more than one trunk 
#define NPD_VLAN_PORT_TRUNK_MBR		(NPD_VLAN_ERR_NONE + 66)  // port belong to trunk ,it can NOT add to vlan as port
#define NPD_VLAN_TRUNK_MBRSHIP_CONFLICT	(NPD_VLAN_ERR_NONE + 67)
#define NPD_VLAN_NOT_SUPPORT_IGMP_SNP (NPD_VLAN_ERR_NONE + 70)
#define NPD_VLAN_IGMP_ROUTE_PORTEXIST (NPD_VLAN_ERR_NONE + 71)
#define NPD_VLAN_IGMP_ROUTE_PORTNOTEXIST (NPD_VLAN_ERR_NONE + 72)
#define NPD_VLAN_ARP_STATIC_CONFLICT (NPD_VLAN_ERR_NONE + 73)
#define	DEFAULT_VLAN_ID			0x1
#define	NPD_PORT_L3INTF_VLAN_ID	0xfff
#define NPD_MAX_VLAN_ID 4095

#define	DEFAULT_VLAN_ID			0x1
#define	NPD_PORT_L3INTF_VLAN_ID	0xfff
#define NPD_MAX_VLAN_ID 4095

#define ETH_PORT_NUM_MAX		0x6
#define EXTENDED_SLOT_NUM_MAX	0x4

#define CMD_FAILURE -1
#define CMD_SUCCESS 0

#define IGMP_UNKNOW_PORT    1
#define IGMP_ERR_PORT       2
#define IGMP_CON_ADMIN      3
#define IGMP_BADPARAM       4
#define IGMP_NOTEXISTS      5
#define IGMP_PORT_EXISTS    6
#define IGMP_PORT_NOTEXISTS 7
#define IGMP_PORT_NOTMEMBER 8
#define IGMP_TAG_CONFLICT   9
#define IGMP_INTERFACE      10
#define IGMP_NOT_TRUNK      11
#define IGMP_HAS_ARP        12
#define IGMP_PVLAN          13
#define IGMP_L3             14
#define IGMP_NOT_DELETE     15
#define IGMP_NOT_ADD        16
#define IGMP_NOT_DEL_L3     17
#define IGMP_HW             18
#define IGMP_ERR_PORT_TAG   19



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
	unsigned int	untagMbrBmp;
	unsigned int	tagMbrBmp;
}vlanList;

typedef struct {
	unsigned short vidx;
	unsigned int   groupMbrBmp;
}groupList;


typedef struct{
    unsigned int vlanlife;
	unsigned int grouplife;
	unsigned int robust;
	unsigned int queryinterval;
	unsigned int respinterval;	
	unsigned int hostime;
}igmp_timer;


extern int parse_timeout(char* str,unsigned int* timeout);

extern int show_igmp_snp_timer
(
	unsigned int vlanlife,
	unsigned int grouplife,
	unsigned int robust,
	unsigned int queryinterval,
	unsigned int respinterval,	
	unsigned int hostime
);


extern int param_first_char_check_new(char* str,unsigned int cmdtip);


extern int igmp_snooping_able(char *able);

extern int config_igmp_snooping(int type, char *time);

extern int show_igmp_snp_time_interval();

extern int show_igmp_snp_time_interval_new(igmp_timer *test);

extern int show_igmp_snp_group_count();

extern int iShow_igmp_vlan_count(int *vlan_count);

extern int config_igmp_snp_npd_vlan_ccgi(int vid,char *able);

extern int config_igmp_snp_npd_port(int vid,char *able, char *port);

extern int igmp_snp_mcroute_port_add_del(int vid,int cmd,char *port);

extern int show_vlan_igmp_snp_mcgroup_list(int vid,struct igmp_vlan *igmp_head,int *igmp_num);

extern int show_group_member_slot_port
(
    unsigned int product_id,
	PORT_MEMBER_BMP mbrBmp

);

extern unsigned int ltochararry
(
	unsigned int Num,
	unsigned char *c0,
	unsigned char *c1,
	unsigned char *c2,
	unsigned char *c3
);

extern int show_igmp_snp_mcgroup_router_port
(
    int vid,
    struct igmp_port *head,
	int *num
);

extern int show_vlan_slot_port
(
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp,
	struct igmp_port *head,
	int *num
);

extern int dcli_igmp_snp_check_status(unsigned char* stats);

extern int dcli_igmp_snp_check_status(unsigned char* stats);

extern int dcli_igmp_snp_vlan_portmbr_status_check
(  
    unsigned short vlanId,
	unsigned int slotno,
	unsigned int portno,
	unsigned int* stats
);

extern int dcli_enable_disable_igmp_one_vlan
(
	unsigned short vlanId,
	unsigned int enable
);

extern int dcli_enable_disable_igmp_one_port
(
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char port_no,
	unsigned char enable
);

extern int dcli_enable_disable_igmp_mcrouter_port
(
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char port_no,
	unsigned char enDis
);

extern int dcli_show_igmp_snp_one_mcgroup
(
	unsigned short vid,
	unsigned short vidx
);


extern int dcli_get_slot_port_by_gindex
(
	unsigned int port_index,
	unsigned char *slot_no,
	unsigned char *local_port_no
);
extern int show_igmpsnp_vlan_member_info
(
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp

);
extern int show_igmp_groulist();

extern int show_igmp_vlan_list
(
    struct igmp_sum *head,
	int *num,
	int *port_num
);

extern int show_vlan_member_slot_port_new
(
	unsigned int product_id,
	unsigned int untagBmp,
	unsigned int tagBmp
);

extern void Free_igmp_head(struct igmp_vlan *head);

extern void Free_igmp_port(struct igmp_port *head);

extern void Free_igmp_sum(struct igmp_sum *head);

extern int dcli_show_igmp_snp_mcgroup_list
(
	unsigned short vlanId,
	struct igmp_vlan *head,
	int *num
);

extern int show_vlan_slot_port_new
(
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp,
	struct igmp_port *head,
	int *num
);

/////////  关于添加和删除端口的操作
extern int add_delete(char * addordel,char * slot_port_no,char * Tagornot,unsigned short vID);

extern int show_vlan_port_member
(
	char * VID,
	struct igmp_port *head,
	int *num
);
#endif

