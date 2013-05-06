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
* ws_dcli_qos.h
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
#include <string.h>
#include <dbus/dbus.h>
#include "ws_init_dbus.h"

#include <sys/types.h>
#include <sys/wait.h>

#include "ws_ec.h"
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"

#define CMD_SUCCESS 0
#define CMD_FAILURE -1
#define Mapping_num 128
#define MAX_QOS_PROFILE 127
#define MAX_MAP_QOS 127
#define Policy_Map_Num 1024
#define Policer_Num 256


#ifndef _DCLI_QOS_H_
#define _DCLI_QOS_H_
#define MAX_EXT_RULE_NUM	512
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


#define ACL_DIRECTION_INGRESS 0
#define ACL_DIRECTION_EGRESS  1
#define ACL_DIRECTION_TWOWAY  2

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
#define ACL_RULE_EXT_ONLY	(ACL_GROUP_ERROR_NONE + 28)
#define ACL_RULE_TIME_NOT_SUPPORT         (ACL_GROUP_ERROR_NONE + 24)
#define ACL_RULE_INDEX_ERROR             (ACL_GROUP_ERROR_NONE + 25)
#define ACL_GROUP_INDEX_ERROR             (ACL_GROUP_ERROR_NONE + 26)
#define ACL_MIRROR_USE					(ACL_GROUP_ERROR_NONE + 27)
#define ACL_RULE_EXT_ONLY					(ACL_GROUP_ERROR_NONE + 28)
#define ACL_UNBIND_FRIST					(ACL_GROUP_ERROR_NONE + 29)
#define ACL_ADD_EQUAL_RULE					(ACL_GROUP_ERROR_NONE + 30)
#define ACL_RANGE_NOT_EXIST					(ACL_GROUP_ERROR_NONE + 31)
#define ACL_UDP_VLAN_RULE_ENABLE				(ACL_GROUP_ERROR_NONE + 32)
#define ACL_PORT_NOT_SUPPORT_BINDED				(ACL_GROUP_ERROR_NONE + 33)
#define QOS_SCH_GROUP_IS_SP	1024		


#define SLOT_PORT_SPLIT_DASH 		'-'
#define SLOT_PORT_SPLIT_SLASH		'/'


#define QOS_POLICER_ENABLE	1
#define QOS_POLICER_DISABLE 0
#define QOS_BAD_PTR					(QOS_ERROR_NONE + 100)


#define NPD_DBUS_ERROR_NO_QOS_MODE		18
#define NPD_DBUS_ERROR_HYBRID_FLOW		21
#define NPD_DBUS_ERROR_ALREADY_FLOW         16
#define NPD_DBUS_ERROR_ALREADY_HYBRID      17
#define NPD_DBUS_ERROR_ALREADY_PORT		14
#define NPD_FAIL -1

#define CMD_WARNING 1

#define QOS_FAIL_TRAFFIC 256
#define BCM_DEVICE_NOT_SUPPORT		555
///acl
#define STANDARD_ACL_RULE 0
#define EXTENDED_ACL_RULE 1


void dcli_qos_init();

extern unsigned long dcli_str2ulong_QOS
(
	char *str
);

struct qos_info
{
	unsigned int profileindex;  /*1-127*/
	unsigned int dp;	/*0-1*/
	unsigned int up;	/*0-7*/
	unsigned int tc;	/*0-7*/
	unsigned int dscp;  /*0-63*/
};
struct mapping_info
{
	char * mapping_des;
	unsigned int flag[Mapping_num];
	unsigned int profileindex[Mapping_num];
};

struct policy_map_info
{
	unsigned int policy_map_index;
	char * droppre;
	char * trustMem;
	char * modiUp;
	char * modiDscp;
	char * remaps;		

	/////////add by kehao ,02/21/2011 ///////
	unsigned int slot_no;
	unsigned int port_no;
	/////////////////////////////////////
};

struct policer_info
{
	unsigned int policer_index;
	char * policer_state;
	unsigned long cir;
	unsigned long cbs;
	char * CounterState;
	unsigned int CounterIndex;
	char * Out_Profile_Action;
	unsigned int Remap_QoSProfile;
	char * Policer_Mode;
	char * Policing_Packet_Size;
};

struct query_info
{
	unsigned int QID;
	char * Scheduling_group;
	unsigned int weight;
};

struct Shapping_info
{
	char *  Port_Shaping_status;
	long port_maxrate;
	unsigned int port_burstsize;
	unsigned int QOS_ID;
	char * Shaping_status;
	long maxrate;
	unsigned int burstsize;
};

struct counter_info
{
	unsigned int	   index;
	unsigned long    inprofile;
	unsigned long 	   outprofile;
};


typedef struct
{
	unsigned int flag;
	unsigned int profileIndex;
}DCLI_QOS_REMAP_SHOW_STC;
#endif


extern int show_qos_mode(char * mode);
//extern int config_qos_mode(char * type);
extern int get_one_port_index_QOS(char * slotport,unsigned int* port_index);
extern int set_qos_profile(char * qosindex,struct list * lcontrol);
extern int set_qos_profile_atrribute(char * qosIndex,char * dp,char * up,char * tc,char * dscp,struct list * lcontrol);	/*返回0表示成功，返回-1表示失败，返回-2表示非法输入，返回-3表示Fail to config qos profile*/

extern int delete_qos_profile(char * qosindex,struct list * lcontrol);
extern int set_dscp_to_profile(char * dscpindex,char * profileindex,struct list * lcontrol);
extern int del_dscp_to_profile(char * dscpindex,struct list * lcontrol);
extern int set_dscp_to_dscp(char * dscpsrc,char * dscpdst,struct list * lcontrol);
extern int del_dscp_to_dscp(char * dscpsrc,struct list * lcontrol);
extern int set_up_to_profile(char * upindex,char * profileindex,struct list * lcontrol);
extern int del_up_to_profile(char * upindex,struct list * lcontrol);
extern int create_policy_map(char * policyID,struct list * lcontrol);
extern int del_policy_map(char * policyID,struct list * lcontrol);
extern int config_policy_map(char * policyID,struct list * lcontrol);
extern int allow_qos_marker(char * enable,char * policyID,struct list * lcontrol);
extern int set_default_up(char * upindex,char * policyID,struct list * lcontrol);
extern int set_default_qos(char * qosprofile,char * policyindex,struct list * lcontrol);
extern int trust_l2_up(char *enable,char * policyindex,struct list * lcontrol);
extern int trust_l3_dscp(char * enabledscp,char * enableremap,char * policyindex,struct list * lcontrol);
extern int trust_l2andl3_mode(char * upable,char * dscpenable,char *remapenbale,char * policyindex,struct list * lcontrol);
extern int untrust_mode(char * policyindex,struct list * lcontrol);
extern int bind_policy_port(char * policyindex,unsigned int index);
extern int unbind_policy_port(char * policyindex,unsigned int index,struct list * lcontrol);
extern int config_acl_ingress_policy(char * ruleindex,char * QoS_profile,char * enable,char * source_up,char * source_dscp,char * policerID,struct list * lcontrol);
extern int config_acl_egress_policy(char * ruleindex,char * egress_up,char * egress_dscp,char * source_up,char * source_dscp,char * policerID,struct list * lcontrol);
extern int show_qos_profile(struct qos_info qos_all[],int * qospronum,struct list * lcontrol);
extern int show_policy_map(struct policy_map_info policy_map_all[],int * policyNum,struct list * lcontrol);
extern int show_port_qos(char * portNo,struct list * lcontrol);
extern struct mapping_info show_remap_table_byindex(int * mappingNum,int * upnum,int * dscpnum,int * dscpselfnum,struct list * lcontrol);
extern int set_policer(char * policerId);
extern int set_cir_cbs(char * cirparam,char * cbsparam,char * policyindex,struct list * lcontrol);
extern int set_out_profile(char * policyindex);
extern int set_out_profile_action(char * action,char * policyindex,struct list * lcontrol);
extern int set_out_profile_remap(char * profile,char * policyindex,struct list * lcontrol);
extern int set_color_mode(char * colormode,char * policyindex);
extern int counter_policer(char * counterindex,char * enable,char * policyindex);
extern int set_policer_enable(char * policerId,char * enable);
extern int set_strict_mode(char * mode);
extern int set_meter_loose_mode(char * loose_mru);
extern int set_counter_attr(char * counterindex,char * Inprofile,char * Outprofile);
extern int show_counter(char * counterindex,struct counter_info * all_info);
extern int show_policer(struct policer_info policer_all[],int * num);
extern int delete_policer(char * policerId);
extern int set_queue_scheduler(char * queuemode);
extern int set_wrr_queue(char * group,char * TCrange,char * Weight,unsigned int  queueindex);
extern int show_queue(struct query_info query_all[]);
extern int set_traffic_shape(char * maxrateparam,char * burstsize,unsigned int portindex,char * kORm,struct list * lcontrol);
extern int set_traffic_queue_attr(char * queueindex,char * maxrateparam,char * burstsize,unsigned int portindex,char * kORm,struct list * lcontrol);
extern int show_traffic_shape(struct Shapping_info shapping_all[],unsigned int portindex);
extern int del_traffic_shape(unsigned int portindex);
extern int del_queue_traffic(char * queueIdparam,unsigned int portindex);
extern int config_qos_base_acl_traffic(char * index,char * rule);
extern int configQosMode(char * type);
extern int show_qos_base_acl_traffic();/*返回0表示成功，返回-1表示失败，返回-2表示No QoS profile exists*/
extern int delete_append(char *aclvalue);/*返回0表示成功，返回-1表示失败，返回-2表示Delete acl to qos profile map not exist返回-3表示Can't delete this acl since it is bound to group*/
                                 /*返回-4表示Can't delete this acl since it is bound togroup 返回-5表示Fail to delete acl to qos profile map table*/
extern int config_policer_range(char *spid,char *epid,char *pindex);/*0:succ,-1:fail,-2:config policer fail */
extern int delete_policer_range(char *spid,char *epid);/*0:succ,-1:fail,-2:policer %d not existed,-3:Since policer is in use,can not delete!,-4:Delete policer fail*/

