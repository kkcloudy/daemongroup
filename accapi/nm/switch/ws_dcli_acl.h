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
* ws_dcli_acl.h
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_init_dbus.h"
#include "dbus/npd/npd_dbus_def.h"
#include "ws_returncode.h"
#include "ws_ec.h"
///////////////////////////////////////////////////////////////////
/* dcli_acl.h  version :v1.10  tangsiqi 2008-11-06*/
//////////////////////////////////////////////////////////////////

#ifndef __DCLI_ACL_H__
#define __DCLI_ACL_H__

#include <sys/types.h>
#include <sys/wait.h>


#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"

//#endif

#define CMD_SUCCESS 0
#define CMD_FAILURE -1




#define ACL_ANY_PORT (100*1024)
#define ACL_ANY_PORT_CHAR 255

#define STANDARD_ACL_RULE 0
#define EXTENDED_ACL_RULE 1
#define MAX_IP_STRLEN 16
#define MAX_EXT_RULE_NUM	512

#define ACCESS_PORT_TYPE 0
#define ACCESS_VID_TYPE  1

#define ACL_TRUE	1
#define ACL_FALSE   0
#define ALIAS_NAME_SIZE 		0x15
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define TIME_SPLIT_DASH 	'/'
#define TIME_SPLIT_SLASH	':'

#define ACL_DIRECTION_INGRESS 0
#define ACL_DIRECTION_EGRESS  1
#define ACL_DIRECTION_TWOWAY  2

#define ACL_TIME_NAME_EXIST    1
#define ACL_TIME_NAME_NOTEXIST 2
#define ACL_TIME_PERIOD_NOT_EXISTED 3
#define ACL_TIME_PERIOD_EXISTED 4


#define QOS_ERROR_NONE	-1
#define QOS_SUCCESS			        (QOS_ERROR_NONE+1)
#define QOS_FAIL                    (QOS_ERROR_NONE+2)
#define QOS_BAD_PARAM               (QOS_ERROR_NONE+3)
#define QOS_PROFILE_EXISTED			(QOS_ERROR_NONE + 4)
#define QOS_PROFILE_NOT_EXISTED 	(QOS_ERROR_NONE + 5)
#define QOS_POLICY_EXISTED			(QOS_ERROR_NONE + 6)
#define QOS_POLICY_NOT_EXISTED 		(QOS_ERROR_NONE + 7)
#define QOS_POLICY_MAP_BIND		    (QOS_ERROR_NONE + 8)
#define QOS_POLICER_NOT_EXISTED 	(QOS_ERROR_NONE + 9)
#define QOS_COUNTER_NOT_EXISTED 	(QOS_ERROR_NONE + 10)
#define QOS_POLICY_MAP_PORT_WRONG   (QOS_ERROR_NONE + 11)
#define QOS_POLICER_USE_IN_ACL      (QOS_ERROR_NONE + 12)
#define QOS_TRAFFIC_NO_INFO         (QOS_ERROR_NONE + 13)
#define QOS_PROFILE_IN_USE         (QOS_ERROR_NONE + 14)


#define ACL_GROUP_ERROR_NONE	0
#define ACL_GROUP_SUCCESS			(ACL_GROUP_ERROR_NONE)
#define ACL_GROUP_EXISTED			(ACL_GROUP_ERROR_NONE + 1)
#define ACL_GROUP_NOT_EXISTED 		(ACL_GROUP_ERROR_NONE + 16)
#define ACL_GROUP_PORT_NOTFOUND		(ACL_GROUP_ERROR_NONE + 3)
#define ACL_GROUP_RULE_NOTEXISTED	(ACL_GROUP_ERROR_NONE + 4)
#define ACL_GROUP_RULE_EXISTED		(ACL_GROUP_ERROR_NONE + 5)
#define NPD_DBUS_ACL_ERR_GENERAL	(ACL_GROUP_ERROR_NONE + 6)
#define ACL_GROUP_PORT_BINDED    	(ACL_GROUP_ERROR_NONE + 7)
#define ACL_GROUP_NOT_SHARE         (ACL_GROUP_ERROR_NONE + 8)
#define ACL_GLOBAL_NOT_EXISTED      (ACL_GROUP_ERROR_NONE + 9)
#define ACL_GLOBAL_EXISTED          (ACL_GROUP_ERROR_NONE + 10)
#define ACL_SAME_FIELD				(ACL_GROUP_ERROR_NONE + 11)
#define ACL_EXT_NO_SPACE			(ACL_GROUP_ERROR_NONE + 12)
#define ACL_GROUP_NOT_BINDED			(ACL_GROUP_ERROR_NONE + 13)
#define ACL_GROUP_VLAN_BINDED           (ACL_GROUP_ERROR_NONE + 14)
#define ACL_GROUP_EGRESS_ERROR          (ACL_GROUP_ERROR_NONE + 17)
#define EGRESS_ACL_GROUP_RULE_EXISTED  (ACL_GROUP_ERROR_NONE + 18)
#define ACL_GROUP_EGRESS_NOT_SUPPORT         (ACL_GROUP_ERROR_NONE + 19)
#define ACL_GROUP_WRONG_INDEX		 (ACL_GROUP_ERROR_NONE + 20)
#define ACL_ON_PORT_DISABLE         (ACL_GROUP_ERROR_NONE + 21)
#define ACL_POLICER_ID_NOT_SET      (ACL_GROUP_ERROR_NONE + 22)
#define ACL_GROUP_SAME_ID		    (ACL_GROUP_ERROR_NONE + 23)
#define ACL_RULE_TIME_NOT_SUPPORT         (ACL_GROUP_ERROR_NONE + 24)
#define ACL_RULE_INDEX_ERROR             (ACL_GROUP_ERROR_NONE + 25)
#define ACL_GROUP_INDEX_ERROR             (ACL_GROUP_ERROR_NONE + 26)
#define ACL_MIRROR_USE					(ACL_GROUP_ERROR_NONE + 27)

#define ACL_NORMAL_MALLOC_ERROR_NONE	 (ACL_GROUP_ERROR_NONE + 100)
#define ACL_ADD_EQUAL_RULE					(ACL_GROUP_ERROR_NONE + 30)



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
#define NPD_VLAN_ERR_MAX			(NPD_VLAN_ERR_NONE + 255)
#define NPD_VLAN_PORT_L3_INTF		(NPD_VLAN_ERR_NONE + 61)	
#define NPD_VLAN_TRUNK_EXISTS		(NPD_VLAN_ERR_NONE + 63)
#define NPD_VLAN_TRUNK_NOTEXISTS	(NPD_VLAN_ERR_NONE + 64)
#define	DEFAULT_VLAN_ID			0x1

#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)




#define ETH_PORT_NUM_MAX		0x6
#define EXTENDED_SLOT_NUM_MAX	0x4

#define SLOT_PORT_SPLIT_DASH 		'-'
#define SLOT_PORT_SPLIT_SLASH		'/'

#define DCLI_CREATE_ERROR 		1

#define DCLI_INTF_DIS_ROUTING_ERR 19
#define DCLI_INTF_EN_ROUTING_ERR 20
#define DCLI_INTF_EXISTED 21


#define DCLI_SET_FDB_ERR 0xff
#define ruleNum  1024 

#define CMD_WARNING 1

#define ICMP_WARNING 260

struct group_info
{
	unsigned int groupIndex;
	unsigned int groupType;//0 为ingress,1为egress
	unsigned int ruleNumber;
	unsigned int ruleindex[ruleNum];
};

struct acl_groupone_info
{
	unsigned int groupIndex;
	unsigned int bind_by_port_count;
	unsigned int bind_by_slot[ruleNum];
	unsigned int bind_by_port[ruleNum];
	unsigned int vlan_count;
	unsigned int bind_by_vlan[ruleNum];
	unsigned int acl_count;
	unsigned int  index[ruleNum];
};

struct acl_info
{
	unsigned int ruleIndex; //规则号
	unsigned int groupIndex; //规则号
	char * ruleType;	//规则类型,分standard,extend 2种
	char * protype;
	char * dip;		//目地IP,带子网位数的IP
	char * sip;    //源IP,带子网位数的IP
	unsigned long srcport; 
	unsigned long dstport;
	unsigned int icmp_code;
	unsigned int icmp_type;
	char * actype;
	unsigned char SubQosMakers;   //0  -  enable ,  1  - disable
	char * dmac;
	char * smac;
	unsigned int vlanid;
	char * source_port;
	char * redirect_port;
	char * analyzer_port;
	 unsigned int  policerId;
	 unsigned int  up;   //  0  -- none , 2 - UP 
	 unsigned int dscp;
	 unsigned int egrUP;
	 unsigned int egrDSCP;
	 unsigned int  modifyUP;
	 unsigned int  modifyDSCP;
	 unsigned int qosprofileindex;
	 char * upmm;
	 char * dscpmm;
	 unsigned int precedence;
	 unsigned char appendIndex;
	 unsigned char dipv6[16];
	 unsigned char sipv6[16];
	 unsigned int nextheader;
};



unsigned long dcli_ip2ulong
(
	char *str
);


int timeRange_time_check_illegal
(	 
	unsigned int sm,
	unsigned int sd,
	unsigned int sh,
	unsigned int smt	
);
int timeRange_time_hour_check_illegal
(	 
	unsigned int sh,
	unsigned int sm 
);
int timeRange_name_legal_check
(
	char* str,
	unsigned int len
);

int timeRange_absolute_deal
 (
 	char *str,
 	unsigned int *startyear,
 	unsigned int *startmonth,
 	unsigned int *startday,
 	unsigned int *starthour,
 	unsigned int *startminute
 );
int timeRange_time_deal
(
	char *str,
	unsigned int *hour,
	unsigned int *minute
);






extern int get_one_port_index(char * slotport,unsigned int* port_index);
extern int show_acl_allinfo(struct acl_info  acl_all[],int *aNum);
extern int show_group_ByRuleIndex(unsigned int ruleindex,unsigned int dir,unsigned int * returnGpIndex,unsigned int * returnGpType);
extern int show_group_list(unsigned int dir,struct group_info grpInfo[],unsigned int * groupNum,unsigned int baseNum);
extern int bind_acl_group(unsigned int dir,unsigned int nodeflag,unsigned int index,unsigned int groupNum);
extern int unbind_aclgrp_port(char * grptype,char * aclgrpIndex,int portindex,unsigned int nodetype,struct list * lcontrol);
extern int enable_aclgrp(char * grptype,char * enable,int portindex,unsigned int bindtype,struct list * lcontrol);
extern int acl_service_glabol_enable(char * enable_or_disable,struct list * lcontrol);
extern char *  show_acl_service();
//extern int show_aclrule_byGrpIndex(char * grpType,char * grpindex,int acl[],int * aclCount);
extern int add_rule_group(char * addordel,char * ruleindex,unsigned int Grpindex,unsigned int dir,struct list * lcontrol);
extern int delete_acl_rule(char * ruleindex);
extern int delete_acl_group(char * GrpNum,unsigned int dir);
extern int addacl_group(unsigned int dir,unsigned int groupIndex);
extern struct acl_info show_aclinfo_Byindex(unsigned int index);
extern int show_group_index(unsigned int dir,unsigned int grpNUm[]);
extern int addrule_trap_IP(unsigned int mode_type,char * ruleIndex,char * sipmask,char * dipmask);
extern int addrule_trap_UdpOrTcp(int mode_type,char * protocol,char * Index,char * dipmask,char * dPort,char * sipmask,char * sPort);
extern int addrule_trap_Icmp(int mode_type,char * Index,char * dipmask,char * sipmask,char * type,char * code);
extern int addrule_trap_ethernet(int mode_type,char * Index,char * dmac,char * smac);
extern int addrule_trap_arp(int mode_type,char * Index,char * smac,char * vlanid,char * slotport);
extern int addrule_PermitOrDeny_arp(int mode_type,char * Index,char * action ,char * smac,char * vlanid,char * slotport,char * policerID);
extern int addrule_PermitOrDeny_ethernet(int mode_type,char * Index,char * action,char * dmac ,char * smac,char * policerID);
extern int addrule_PermitOrDeny_icmp(int mode_type,char * Index,char * action,char * dipmask,char * sipmask,char * type,char * code,char * policerID);
extern int addrule_PermitOrDeny_ip(int mode_type,char * Index,char * action,char * dipmask,char * sipmask,char * policerID);
extern int addrule_PermitOrDeny_TcpOrUdp(int mode_type,char * Index,char * action,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * policerID);
extern int addrule_MirrorOrRedir_arp(int mode_type,char * Index,char * action,char * sPortNo,char * smac,char * vlanid,char * slotport,char * policerID);
extern int addrule_MirrorOrRedir_ip(int mode_type,char * Index,char * action,char * sPortNo,char * dipmask,char * sipmask,char * policerID);
extern int addrule_MirrorOrRedir_TcpOrUdp(int mode_type,char * Index,char * action,char * sPortNo,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * policerID);
extern int addrule_MirrorOrRedir_icmp(int mode_type,char * Index,char * action,char * sPortNo,char * dipmask,char * sipmask,char * type,char * code,char * policerID);
extern int addrule_MirrorOrRedir_ethernet(int mode_type,char * Index,char * action,char * sPortNo,char * dmac,char * smac,char * policerID);
extern int addrule_extend(char * Index,char * action,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * dmac,char * smac,char * vlanid,char * slotport,char * policerID);
extern int addrule_MirrorOrRedir_extend(char * Index,char * action,char * sPortNo,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * dmac,char * smac,char * vlanid,char * slotport,char * policerID);
extern int addrule_ingress_qos(char * ruleindex,char * qosIndex,char * sub_qos,char * source_up,char * source_dscp,char * policerID,struct list * lcontrol);
extern int addrule_egress_qos(char * ruleindex,char * egress_up,char * egress_dscp,char * source_up,char * source_dscp,struct list * lcontrol);
extern int show_acl_group_one(char* type,char * Index,struct  acl_groupone_info *p_acl_grpone_info);
extern int addrule_MirrorOrRedir_TcpOrUdp_extended(char * Index, char * rule, char * portslot, char * dipmask, char * dport, char * sipmask, char * sport, char * dmac, char * smac, char * vid, char * sourcePort, char * policerID);


#endif


