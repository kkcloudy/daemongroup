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
* ws_trunk.h
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
#ifndef _WS_TRUNK_H
#define _WS_TRUNK_H



#define ALIAS_NAME_SIZE 		0x15
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define VID_MAX_NUM  0xFFF 
#define VID_SPLIT_DASH 		'-'
#define VID_SPLIT_SLASH		','
#define VID_LLEGAL(vid)     ((vid)>0&&(vid)<VID_MAX_NUM)
#define NPD_VLAN_ERR_NONE			NPD_SUCCESS


#define TRUNK_PORT_EXISTS_GTDB	0xf		//port already exists in trunkDB
#define TRUNK_PORT_MBRS_FULL	(TRUNK_PORT_EXISTS_GTDB+1)
#define NPD_TRUNK_ERR_NONE		NPD_SUCCESS
#define NPD_TRUNK_ERR_GENERAL	(NPD_TRUNK_ERR_NONE + 20)		//general failure
#define NPD_TRUNK_BADPARAM			(NPD_TRUNK_ERR_NONE + 22)  		//bad parameters
#define NPD_TRUNK_EXISTS			(NPD_TRUNK_ERR_NONE + 23)		//vlan have been created already
#define NPD_TRUNK_NOTEXISTS		(NPD_TRUNK_ERR_NONE + 24)			//vlan does not exists
#define NPD_TRUNK_ERR_HW			(NPD_TRUNK_ERR_NONE + 26) 		//vlan error when operation on HW
#define NPD_TRUNK_PORT_EXISTS		(NPD_TRUNK_ERR_NONE + 27)		//port already exists in vlan
#define NPD_TRUNK_PORT_NOTEXISTS	(NPD_TRUNK_ERR_NONE + 28)		// port is not a member of vlan
#define NPD_TRUNK_PORT_MEMBERSHIP_CONFLICT	(NPD_TRUNK_ERR_NONE + 29)		//port can NOT be Untag member of different vlans. 
#define NPD_TRUNK_NAME_CONFLICT		(NPD_TRUNK_ERR_NONE + 30)	
#define NPD_TRUNK_MEMBERSHIP_CONFICT		(NPD_TRUNK_ERR_NONE + 31)
#define NPD_TRUNK_ALLOW_ERR				(NPD_TRUNK_ERR_NONE +32)
#define NPD_TRUNK_REFUSE_ERR				(NPD_TRUNK_ERR_NONE +33)
#define NPD_TRUNK_MEMBER_ADD_ERR	(NPD_TRUNK_ERR_NONE + 34)
#define NPD_TRUNK_MEMBER_DEL_ERR	(NPD_TRUNK_ERR_NONE + 35)
#define NPD_TRUNK_GET_ALLOWVLAN_ERR	(NPD_TRUNK_ERR_NONE + 36)
#define NPD_TRUNK_NO_MEMBER			(NPD_TRUNK_ERR_NONE + 37)
#define NPD_TRUNK_SET_TRUNKID_ERR	(NPD_TRUNK_ERR_NONE + 38)
#define NPD_TRUNK_DEL_MASTER_PORT	(NPD_TRUNK_ERR_NONE + 39)		//master port NOT allowd to delete.
#define NPD_TRUNK_PORT_ENABLE		(NPD_TRUNK_ERR_NONE + 40)		//port enalbe in trunk
#define NPD_TRUNK_PORT_NOTENABLE	(NPD_TRUNK_ERR_NONE + 41)		// port disable in trunk
#define NPD_TRUNK_ALLOW_VLAN		(NPD_TRUNK_ERR_NONE + 42)
#define NPD_TRUNK_NOTALLOW_VLAN		(NPD_TRUNK_ERR_NONE + 43)
#define NPD_TRUNK_LOAD_BANLC_CONFLIT (NPD_TRUNK_ERR_NONE + 44)		//trunk load balance mode same to original
#define NPD_TRUNK_VLAN_TAGMODE_ERR (NPD_TRUNK_ERR_NONE + 45)		
#define NPD_TRUNK_PORT_L3_INTF		(NPD_TRUNK_ERR_NONE + 61)
#define NPD_VLAN_TRUNK_CONFLICT		(NPD_VLAN_ERR_NONE + 65)  // vlan can not add more than one trunk 


#define	DEFAULT_TRUNK_ID			0x0								//NOT member of any trunk


#define ETH_PORT_NUM_MAX		0x6
#define EXTENDED_SLOT_NUM_MAX	0x4
#define TRUNK_MEMBER_NUM_MAX		0x8
#define TRUNK_CONFIG_SUCCESS	0x0
#define	TRUNK_CONFIG_FAIL		0xff

#define SLOT_PORT_SPLIT_DASH 		'-'
#define SLOT_PORT_SPLIT_SLASH		'/'

/*src/npdsuite/npd/src/app/npd_vlan.h*/
#define NPD_VLAN_NOTEXISTS		(NPD_VLAN_ERR_NONE + 15)		//vlan does not exists
#define NPD_VLAN_L3_INTF			(NPD_VLAN_ERR_NONE + 20)		//vlan is L3 interface
#define NPD_VLAN_TRUNK_EXISTS		(NPD_VLAN_ERR_NONE + 63)




struct port_profile
{
  unsigned int slot;
  unsigned int port;
  unsigned int tmpVal;
  struct port_profile *next;
};

struct vlan_profile
{
  unsigned short vlanId;
  char vlanName[NPD_TRUNK_IFNAME_SIZE];
  unsigned char	tagMode;
  struct vlan_profile *next;
};

#if 0
typedef struct {
	unsigned int portMbr[2];
}PORT_MEMBER_BMP;

#endif


struct trunk_profile
{
  unsigned short trunkId;
  unsigned char	trunkName[NPD_TRUNK_IFNAME_SIZE];
  unsigned char	loadBalanc[20];
  unsigned char masterFlag;
  unsigned int mSlotNo;
  unsigned int mPortNo;
  unsigned char tLinkState;
  unsigned int port_Cont;
  unsigned int vlan_Cont;
  PORT_MEMBER_BMP	mbrBmp_sp;  
  PORT_MEMBER_BMP disMbrBmp_sp;
  struct port_profile *portHead;
  struct vlan_profile *vlanHead;
  struct trunk_port_bmp	portsMbrBmp;
  trunk_port_list_s	*portList;
  trunk_vlan_list_s	*allowVlanList;
  struct trunk_profile *next;  
};


extern void Free_trunk_head( struct trunk_profile *head);
extern void Free_trunk_one( struct trunk_profile *trunk);
extern void Free_vlanlist_trunk_head( struct trunk_profile *head);
extern int show_trunk_list(struct trunk_profile *trunk_head, int *trunk_num);   /*失败返回0，成功返回1，返回-1表示no trunk*/
																			      /*返回-2表示Error occurs in Read trunk Entry in HW.，返回-3表示Op on trunk portlist Fail.*/
extern int show_trunk_byid(int id,struct trunk_profile *trunk);  /*传递要显示的trunk的id号，返回trunk的所有信息*/
																	 /*返回0表示失败，,返回1表示成功，返回-1表示trunk ID非法*/
																	 /*返回-2表示trunk not Exists，返回-3表示error*/
extern int show_trunk_vlanlist(int trunk_id,struct trunk_profile *trunk_head); /*失败返回0，成功返回1，返回-1表示Illegal trunk Id，返回-2表示trunk not exists*/
																				  /*返回-3表示Error: Op on Hw Fail，返回-4表示Error: Op on Getting trunk allow vlanlist*/
extern int create_trunk(char * id, char *trunk_name);   	/*返回0表示失败，返回1表示成功，返回-1表示trunk name can  not be list*/
														/*返回-2表示trunk name too long，返回-3表示trunk name begins with an illegal char*/
														/*返回-4表示trunk name contains illegal char，返回-5表示trunk id illeagal*/
														/*返回-6表示trunkID Already Exists，返回-7表示trunkName Already Exists*/
														/*返回-8表示error*/
extern int delete_trunk_byid(int id);   /*返回0表示 删除失败，返回1表示删除成功，返回-1表示Trunk id Illegal，返回-2表示trunk not exists，返回-3表示出错*/
extern int add_delete_trunk_port(int id,char *flag,char *slot_port);  /*flag="add"或"delete"，slot_port格式"1-1,1-2"，slot 范围1-4，port范围1-6，每个trunk最多add 8个port*/
                                                                          /*返回0表示失败，返回1表示成功，返回-1表示port not exists*/
                                                                          /*返回-2表示trunk not exists，返回-3表示 port was Already the member of this trunk*/
                                                                          /*返回-4表示port was not member of this trunk，返回-5表示 trunk port member id FULL*/
                                                                          /*返回-6表示This port is a member of other trunk，返回-7表示 This port is L3 interface*/
                                                                          /*返回-8表示This is the master port of trunk，返回-9表示error*/
extern int set_master_port(int id,char *slot_port);  /*slot_port格式"1-1,1-2"，slot 范围1-4，port范围1-6*/
													    /*返回0表示失败，返回1表示成功，返回-1表示Unknow portno format.，返回-2表示port not exists*/
														/*返回-3表示trunk not exists，返回-4表示 port was not member of this trunk*/
														/*返回-5表示This port is L3 interface，返回-6表示 error*/
extern int allow_refuse_vlan(int id,char *flag,char *vlan_id,char *mod);  /*flag="allow"或"refuse"，vlan_id范围1-4094*，mod="tag"或"untag"*/
                                                                             /*返回0表示失败，返回1表示成功，返回-1表示Unknow vlan format.，返回-2表示vlan not exists*/
                                                                             /*返回-3表示vlan is L3 interface，返回-4表示 vlan Already allow in trunk*/
                                                                             /*返回-5表示vlan Already allow in other trunk，返回-6表示 There exists no member in trunk*/
                                                                             /*返回-7表示Vlan NOT allow in trunk，返回-8表示tagMode error in vlan，返回-9表示error*/
extern int set_port_state(int id,char *slot_port,char *state);  /*slot_port格式"1-1,1-2"，slot 范围1-4，port范围1-6，每个trunk最多add 8个port，state="enable"或"disable"*/
																  /*返回0表示失败，返回1表示成功，返回-1表示Unknow portno format.，返回-2表示port was not member of this trunk*/
																  /*返回-3表示port already enable，返回-4表示 port not enable，返回-5表示error*/
extern int set_load_balance(char *mode);	/*mode列表:based-port|based-mac|based-ip|based-L4|mac+ip|mac+L4|ip+L4|mac+ip+L4*/
		          					        /*返回0表示失败，返回1表示成功，返回-1表示there no trunk exist*/
										    /*返回-2表示load-balance Mode same to corrent mode*/

#endif
