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
* ws_dcli_vlan.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef WS_DCLI_VLAN_H
#define WS_DCLI_VLAN_H
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ws_init_dbus.h"
#include "ws_public.h"
#include "ws_ec.h"

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"

#define CMD_SUCCESS 0
#define CMD_FAILURE   -1
#define MAX_VLAN_NUM  4095

#define ALIAS_NAME_SIZE    0x15  /*定义vlan_name的长度*/



//#define NPD_DBUS_BUSNAME "aw.npd"

//#define NPD_DBUS_OBJPATH "/aw/npd"
//#define NPD_DBUS_INTERFACE "aw.npd"

    
/*#define vlan_out(x)  printf x  */

#define MAX_TOTAL_NUM 52
#define MAX_PORT_NUM 52
#define MAX_SOLT_NUM 4


#if 0
typedef struct {
    unsigned int trunkMbr[4];
}TRUNK_MEMBER_BMP;

typedef struct {
    unsigned int portMbr[2];
}PORT_MEMBER_BMP;
#endif

typedef struct {
	unsigned int port_total_num;
	unsigned int slot_num;
	unsigned int port_num;
}PRODUCT_PAGE_INFO;

struct vlan_info
{
    unsigned short  vlanId;
    char*           vlanName;
    char*           slot_port_tag[MAX_TOTAL_NUM];
    char*           slot_port_untag[MAX_TOTAL_NUM];
    unsigned short  tagornot;/*0初始值，1为untag，2为tag*/
    ///add 2009-3-10////
    unsigned short promisProt_tag[MAX_TOTAL_NUM];
    unsigned short promisProt_untag[MAX_TOTAL_NUM];
//  struct vlan_info*   next;
};

struct Vlanid_info
{
    int  vlanId;
    char vlanName[50];
    char untagport[20];
	char tagport[20];
	int  untagflag;
	int  tagflag;
	int  iftag;
	struct Vlanid_info *next;
};

struct vlan_info_simple
{
    
    unsigned short  vlanId;
    char*           vlanName;
    unsigned int    vlanStat;
};

typedef struct port_bmp_c{
    unsigned int low_bmp;
    unsigned int high_bmp;
}port_bmp_t_c;
typedef struct vlan_list_distributed_c{
    unsigned char vlanStat;
    unsigned char updown;
    unsigned char bond_slot[10];
    char vlanName[21];
	int vlanid;
    port_bmp_t_c untagPortBmp[10]; 
	port_bmp_t_c tagPortBmp[10];
	port_bmp_t_c qinq[10];
	unsigned char untagTrkBmp[127];
	unsigned char tagTrkBmp[127];
}vlan_list_t_c;

struct vlanlist_info_c
{
	vlan_list_t_c mem_vlan_list[4095];
	int vlannum;
};

struct vlan_info_detail
{
    unsigned char vlanStat;
    unsigned char updown;
    unsigned char bond_slot[10];
	unsigned char to_cpu_port;
	unsigned char qinq_state;
    char vlanName[21];
    port_bmp_t untagPortBmp[16]; 
	port_bmp_t tagPortBmp[16];
	port_bmp_t qinq[10];
	unsigned char untagTrkBmp[127];
	unsigned char tagTrkBmp[127];
	int vlanId;
	int portnum;
	int trunkid;
	int trunktag;/*untag:0,tag:1*/
	int tagnum;
	int untagnum;
	struct vlan_info_detail *next;
};



/*这里封装一些宏*/
#define NPD_TRUNK_ERR_NONE      NPD_SUCCESS
#define NPD_VLAN_ERR_NONE           NPD_SUCCESS
#define NPD_VLAN_ERR_GENERAL        (NPD_VLAN_ERR_NONE + 10)        //general failure
#define NPD_VLAN_BADPARAM           (NPD_VLAN_ERR_NONE + 12)    //bad parameters
#define NPD_VLAN_EXISTS         (NPD_VLAN_ERR_NONE + 13)        //vlan have been created already
#define NPD_VLAN_NAME_CONFLICT  (NPD_VLAN_ERR_NONE + 14)
#define NPD_VLAN_NOTEXISTS      (NPD_VLAN_ERR_NONE + 15)        //vlan does not exists
#define NPD_VLAN_ERR_HW         (NPD_VLAN_ERR_NONE + 16)    //vlan error when operation on HW
#define NPD_VLAN_PORT_EXISTS        (NPD_VLAN_ERR_NONE + 17)        //port already exists in vlan
#define NPD_VLAN_PORT_NOTEXISTS (NPD_VLAN_ERR_NONE + 18)        // port is not a member of vlan
#define NPD_VLAN_PORT_MEMBERSHIP_CONFLICT   (NPD_VLAN_ERR_NONE + 19)        //port can NOT be Untag member of different vlans. 
#define NPD_VLAN_L3_INTF            (NPD_VLAN_ERR_NONE + 20)        //vlan is L3 interface
#define NPD_VLAN_PORT_TAG_CONFLICT  (NPD_VLAN_ERR_NONE +21)
#define NPD_TRUNK_MEMBER_NONE		(NPD_VLAN_ERR_NONE +22)
#define NPD_VLAN_PORT_PROMISCUOUS_MODE_ADD2_L3INTF  (NPD_VLAN_ERR_NONE + 23)  /*promiscuous mode port add to l3 interface */
#define NPD_VLAN_PORT_DEL_PROMISCUOUS_PORT_DELETE (NPD_VLAN_ERR_NONE + 24) /*del promiscuous port but default is l3 intf*/
#define NPD_VLAN_SUBINTF_EXISTS     (NPD_VLAN_ERR_NONE + 26)    /* sub intf exists*/
#define NPD_VLAN_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN (NPD_VLAN_ERR_NONE + 28)  /*promis try to add to other vlan(not vlan 1)*/
#define NPD_VLAN_PORT_SUBINTF_EXISTS     (NPD_VLAN_ERR_NONE + 29)  /* port sub interface exists*/

#define NPD_VLAN_ERR_MAX            (NPD_VLAN_ERR_NONE + 255)
#define NPD_VLAN_PORT_L3_INTF       (NPD_VLAN_ERR_NONE + 61)	
#define NPD_VLAN_TRUNK_EXISTS       (NPD_VLAN_ERR_NONE + 63)
#define NPD_VLAN_TRUNK_NOTEXISTS    (NPD_VLAN_ERR_NONE + 64)

#define NPD_VLAN_TRUNK_CONFLICT     (NPD_VLAN_ERR_NONE + 65)  // vlan can not add more than one trunk 
#define NPD_VLAN_PORT_TRUNK_MBR     (NPD_VLAN_ERR_NONE + 66)  // port belong to trunk ,it can NOT add to vlan as port
#define NPD_VLAN_TRUNK_MBRSHIP_CONFLICT (NPD_VLAN_ERR_NONE + 67)
#define NPD_VLAN_NOT_SUPPORT_IGMP_SNP (NPD_VLAN_ERR_NONE + 70)
#define NPD_VLAN_IGMP_ROUTE_PORTEXIST (NPD_VLAN_ERR_NONE + 71)
#define NPD_VLAN_IGMP_ROUTE_PORTNOTEXIST (NPD_VLAN_ERR_NONE + 72)
#define NPD_VLAN_ARP_STATIC_CONFLICT (NPD_VLAN_ERR_NONE + 73)

#define NPD_TRUNK_NOTEXISTS     (NPD_TRUNK_ERR_NONE + 24)


#define DEFAULT_VLAN_ID         0x1
#define NPD_PORT_L3INTF_VLAN_ID 0xfff
#define NPD_MAX_VLAN_ID 4095


#define ALIAS_NAME_LEN_ERROR    0x1
#define ALIAS_NAME_HEAD_ERROR   (ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR   (ALIAS_NAME_LEN_ERROR+2)
#define SLOT_PORT_SPLIT_DASH        '-'
#define SLOT_PORT_SPLIT_SLASH       '/'
#define SLOT_PORT_SPLIT_COMMA       ','


#define ETH_PORT_NUM_MAX        0x6
#define EXTENDED_SLOT_NUM_MAX   0x4
#define SLOT_LLEGAL(slot_no)     ((slot_no)>0&&(slot_no)<=4)
#define PORT_LLEGAL(port_no)     ((port_no)>0&&(port_no)<=24)
#define SPLIT_LEGAL(str_chr)     (SLOT_PORT_SPLIT_COMMA == str_chr || SLOT_PORT_SPLIT_SLASH == str_chr || SLOT_PORT_SPLIT_DASH == str_chr)           
#define CHAR_NOT_NUMBER(str_chr) (str_chr<'0'||str_chr>'9')   

#define PRODUCT_SELECTOR_SWITCH 1

typedef struct  {
       unsigned char slot;
       unsigned char port;
}SLOT_PORT_S;





#define DCLI_CREATE_ERROR       1

#define DCLI_INTF_DIS_ROUTING_ERR 19
#define DCLI_INTF_EN_ROUTING_ERR 20
#define DCLI_INTF_EXISTED 21
#define DCLI_NOT_CREATE_ROUTE_PORT_SUB_INTF 23
#define DCLI_NOT_CREATE_VLAN_INTF_SUB_INTF 24
#define DCLI_ONLY_RUN_IN_VLAN 25
#define DCLI_ALREADY_ADVANCED 26
#define DCLI_NOT_ADVANCED 27
#define DCLI_PARENT_INTF_NOT_EXSIT 28


#define DCLI_SET_FDB_ERR 0xff

#define _DCLI_PVE_H
#define MAX_VLANID 4094
#define MIN_VLANID 1
#define MIN_SLOT   1
#define MAX_SLOT   4
#define MIN_PORT   1
#define MAX_PORT   6
#define ENABLE     1
#define DISABLE    2
#define PVE_ERR   -1
#define MAX_TRUNK 127
#define MIN_TRUNK 1
#define MAX_PVLAN 31
#define MIN_PVLAN 1
#define PVE_MAX_PORT    64

typedef enum{
    PVE_FAIL=0,
    PVE_TRUE=1,
}PVE_BOOL;


struct VlanMembrship
{   
unsigned char isMbr;    
unsigned char slotNum;
unsigned char portNum;  
unsigned char isTag;
};

/*struct Pvlan_info
{
    unsigned int 
}*/

typedef struct
    {
    unsigned short  vlanId;	
    char*           vlanName;	
    unsigned int    untagMbrBmp;	
    unsigned int    tagMbrBmp;
    unsigned int vlanStat;
    }vlanList;

struct Trunklist
{
  unsigned short TrunkId;
  unsigned char tagMode;
  struct Trunklist *next;
};

    
typedef struct {
    unsigned short vidx;
    unsigned int   groupMbrBmp;
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


struct vlan_ports_collection
{
	unsigned int have_port;
	unsigned int port_min;
	unsigned int port_max;
};

extern inline int parse_vlan_no(char* str,unsigned short* vlanId);
extern inline int vlan_name_legal_check(char* str,unsigned int len);
extern inline int param_first_char_check(char* str,unsigned int cmdtip);
extern inline int parse_param_no(char* str,unsigned int* param);        			

extern void Free_show_vlan_list_slot(struct vlan_info_detail *head);
/*返回1时调用Free_show_vlan_list_slot()释放空间*/
extern int show_vlan_list_slot(struct vlan_info_detail *head,int *vlannum);/*返回0表示失败，返回1表示成功*/
																	   		 /*返回-1表示get_product_info() return -1,Please check dbm file*/
																	   		 /*返回-2表示Failed to open*/
																	   		 /*返回-3表示Failed to mmap for g_vlanlist[]*/
																			 /*返回-4表示malloc error*/

extern int show_vlan_list(struct vlan_info_simple vlan_all[], int vlan_port_num[],int* vNum);
extern int create_vlan(DBusConnection *connection,unsigned short vID,char *vName);
extern int delete_vlan(DBusConnection *connection,unsigned short vID);
extern int addordelete_port(DBusConnection *connection,char * addordel,char * slot_port_no,char * Tagornot,unsigned short vID,char * lan);
																			/*返回0表示失败，返回1表示成功*/
																			/* 返回-1表示operation failed，返回-2表示Unknown portno format*/
																			/*返回-3表示Bad slot/port number，返回-4表示Bad tag parameter*/
																			/*返回-5表示Error occurs in Parse eth-port Index or devNO.& logicPortNo*/
																			/*返回-6表示Port already member of the vlan*/
																			/*返回-7表示Port is not member of the vlan*/
																			/*返回-8表示Port already untagged member of other active vlan*/
																			/*返回-9表示Port Tag-Mode can NOT match*/
																			/*返回-10表示Occcurs error,port is member of a active trunk*/
																			/*返回-11表示Occcurs error,port has attribute of static arp*/	
																			/*返回-12表示Occcurs error,port is member of a pvlan*/
																			/*返回-13表示There are protocol-vlan config on this vlan and port*/
																			/*返回-14表示Can't del an advanced-routing interface port from this vlan*/
																			/*返回-15表示get master_slot_id error，返回-16表示get get_product_info return -1*/
																			/*返回-17表示Please check npd on MCB slot，返回-18表示vlan_list add/delete port Error*/
extern int show_vlan_ByID(struct vlan_info *vlan_info_byID, unsigned short vID,int* untagport_num,int* tagport_num);
extern int setVID(unsigned short vidOld,char * vidnew);
extern int createIntfForVlan(unsigned int vID);
extern int deleteIntfForVlan(unsigned short vID);
extern int execute_dcli_command (const char *command, int argc,char* argv1,char* argv2,char* argv3,char* argv4);
extern int deleteIntfForVlanNoShow(unsigned short vID);
extern int config_vlan_advanced_routing(unsigned int vID);
extern int config_vlan_filter(unsigned short VID,char * status,char * filterParam);
extern int addordel_trunk(char * addordel,unsigned short VID,char * trunk_id,char * tagornot ,struct list * lcontrol);
extern int show_trunklis_by_vlanId(char * vlanID,struct Trunklist * trunkdta,int * num);
extern void Free_vlan_trunk(struct Trunklist * head);
extern int Product_Adapter_for_page(PRODUCT_PAGE_INFO * pstProductInfo,char * product_name);
extern int show_vlan_member_slot_port(unsigned int product_id_param,PORT_MEMBER_BMP untagBmp_param,PORT_MEMBER_BMP tagBmp_param,unsigned int * promisPortBmp_param);
extern int show_vlanid_portlist(char *vid,struct Vlanid_info *head);
extern void Free_vlanid_info(struct Vlanid_info * head);
extern int show_vlan_member_slot_port_link
(
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp,
	unsigned int * promisPortBmp,
    struct Vlanid_info *head
);
extern int ccgi_vlanlist_add_del_port
(
DBusConnection *connection,
char * addordel,
char * slot_port_no,
char * Tagornot,
unsigned short vID
);
extern int show_current_vlan_port_member(struct vlanlist_info_c *head);
extern void get_vlan_ports_collection(struct vlan_ports_collection *ports);
extern int get_vlan_port_member_tagflag(int vlan_id,char *port,int *tag_flag);/*返回0表示失败，返回1表示成功*/
																					/*返回-1表示Failed to open file，返回-2表示Failed to mmap*/
																					/*返回-3表示vlan is NOT exists，返回-4表示Failed to munmap*/
																					/*返回-5表示close shm_vlan failed*/

#endif

