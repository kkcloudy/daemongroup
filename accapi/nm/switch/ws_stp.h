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
* ws_stp.h
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
//ws_stp.h
#ifndef _WS_DCLI_RSTP_H
#define _WS_DCLI_RSTP_H
#endif

//#include <zebra.h>
#include <dbus/dbus.h>

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//#include "ws_ec.h"



//#include "dcli_stp.h"
//#include "command.h"
//#include "dcli_common_stp.h"
#include "ws_init_dbus.h"
#include "ws_public.h"

//#define STP_FULL_DEBUG	

//extern DBusConnection *dcli_dbus_connection;
extern DBusConnection *ccgi_dbus_connection;


extern char* stp_port_role[7];

extern char* stp_port_state[5];









/*stp cfg default value*/

#define DEF_BR_PRIO 32768  //默认DEFAULT
#define MIN_BR_PRIO 0
#define MAX_BR_PRIO 61440

#define DEF_BR_HELLOT   2
#define MIN_BR_HELLOT   1
#define MAX_BR_HELLOT   10

#define DEF_BR_MAXAGE   20
#define MIN_BR_MAXAGE   6
#define MAX_BR_MAXAGE   40

#define DEF_BR_FWDELAY  15
#define MIN_BR_FWDELAY  4
#define MAX_BR_FWDELAY  30

#define DEF_BR_REVISION  0
#define MIN_BR_REVISION  0
//#define MAX_BR_REVISION  61440

#define DEF_REMAINING_HOPS 20
#define MIN_REMAINING_HOPS 6
#define MAX_REMAINING_HOPS 40

#define MIN_MST_ID 0
//#define MAX_MST_ID 64

#define STP_FORCE_VERS  2 /* NORMAL_RSTP */
#define MST_FORCE_VERS 3 /*NORMAL_MSTP*/

/* port configuration */
#define DEF_PORT_PRIO   128
#define MIN_PORT_PRIO   0
#define MAX_PORT_PRIO   240 /* in steps of 16 */

//#define ADMIN_PORT_PATH_COST_AUTO   20
#define DEF_ADMIN_NON_STP   NPD_FALSE
#define DEF_ADMIN_EDGE      NPD_TRUE
#define DEF_LINK_DELAY      3 /* see edge.c */
#define P2P_AUTO  2
#define DEF_P2P        P2P_AUTO




#define STP_DISABLE 0xff
#define STP_HAVE_ENABLED 0xfe
#define STP_PORT_NOT_ENABLED 0xfd
#define STP_PORT_HAVE_ENABLED 0xfc
#define STP_PORT_NOT_LINK 0xfb

#define STP_DBUS_DEBUG(x) printf x
#define STP_DBUS_ERR(x) printf x


#define STP_DEBUG_FLAG_ALL       0xFF
#define STP_DEBUG_FLAG_DBG       0x1
#define STP_DEBUG_FLAG_WAR       0x2
#define STP_DEBUG_FLAG_ERR       0x4
#define STP_DEBUG_FLAG_EVT       0x8
#define STP_DEBUG_FLAG_PKT_REV   0x10
#define STP_DEBUG_FLAG_PKT_SED   0x20
#define STP_DEBUG_FLAG_PKT_ALL   0x30
#define STP_DEBUG_FLAG_PROTOCOL  0x40


#define STP_STR "Config Spanning-Tree Protocol\n"
#define MST_STR "Specify mst instance id <0-63>\n"

#define PRINTF_RSTP_NOT_ENABLED "RSTP hasn't enabled\n"
#define PRINTF_RSTP_HAVE_ENABLED  "RSTP is already enabled\n"
#define PRINTF_MSTP_NOT_ENABLED "MSTP hasn't enabled\n"
#define PRINTF_MSTP_HAVE_ENABLED  "MSTP is already enabled\n"
#define PRINTF_PORT_NOT_ENABLED "This port hasn't enabled\n"
#define PRINTF_PORT_HAVE_ENABLED  "This port is already enabled\n"

//新增加的
#define PRINTF_STP_Small_Bridge_Priority "The bridge priority is small\n"
#define PRINTF_STP_Large_Bridge_Priority "The bridge priority is large\n"
#define PRINTF_STP_Small_Max_Hops        "The max hops is small\n"
#define PRINTF_STP_Large_Max_Hops        "The max hops is large\n"
#define PRINTF_STP_Small_Hello_Time      "The hello time is small\n"
#define PRINTF_STP_Large_Hello_Time      "The hello time is large\n"
#define PRINTF_STP_Small_Max_Age         "The max age is small\n"
#define PRINTF_STP_Large_Max_Age         "The max age is large\n"
#define PRINTF_STP_Small_Forward_Delay   "The forward delay is small\n"
#define PRINTF_STP_Large_Forward_Delay   "The forward delay is large\n"
#define PRINTF_STP_Forward_Delay_And_Max_Age_Are_Inconsistent  "The forward delay and the max age should be contend with: Max-Age<=2*(Forward-Delay-1)\n"
#define PRINTF_STP_Hello_Time_And_Max_Age_Are_Inconsistent     "The hello time and the max age should be contend with:2*（hello-time+1）<=Max-Age\n"
#define PRINTF_STP_Hello_Time_And_Forward_Delay_Are_Inconsistent "The hello time and the forward delay should be contend with: 2*(hello-time +1)<=2*(forward-delay - 1)\n"



#define NPD_VLAN_ERR_HW_STP			(0 + 16) 	//vlan error when operation on HW
#define NPD_VLAN_NOTEXISTS_STP		(0 + 15)		//vlan does not exists
#define NPD_DBUS_ERROR_NO_SUCH_VLAN_CCGI 4

/*Dcli_stp.h */
enum { 
  STP_OK = 0,                                     
  STP_ERROR,                                      
  STP_Cannot_Find_Vlan,      
  STP_Imlicite_Instance_Create_Failed,          
  STP_Small_Bridge_Priority,                    
  STP_Large_Bridge_Priority,                  
  STP_Small_Hello_Time,                      
  STP_Large_Hello_Time,                      
  STP_Small_Max_Age,                     
  STP_Large_Max_Age,                         
  STP_Small_Forward_Delay,                
  STP_Large_Forward_Delay,               
  STP_Small_Max_Hops,                   
  STP_Large_Max_Hops,                  
  STP_Forward_Delay_And_Max_Age_Are_Inconsistent,
  STP_Hello_Time_And_Max_Age_Are_Inconsistent, 
  STP_Hello_Time_And_Forward_Delay_Are_Inconsistent,
  STP_Vlan_Had_Not_Yet_Been_Created,           
  STP_Port_Is_Absent_In_The_Vlan,             
  STP_Big_len8023_Format,                     
  STP_Small_len8023_Format,               
  STP_len8023_Format_Gt_Len,            
  STP_Not_Proper_802_3_Packet,              
  STP_Invalid_Protocol,                    
  STP_Invalid_Version,                      
  STP_Had_Not_Yet_Been_Enabled_On_The_Vlan,   
  STP_Cannot_Create_Instance_For_Vlan,       
  STP_Cannot_Create_Instance_For_Port,      
  STP_Invalid_Bridge_Priority,           
  STP_There_Are_No_Ports,               
  STP_Cannot_Compute_Bridge_Prio,          
  STP_Another_Error,                    
  STP_Nothing_To_Do,                     
  STP_BRIDGE_NOTFOUND,                
  STP_CREATE_PORT_FAIL,                  
  STP_PORT_NOTFOUND,              
  STP_LAST_DUMMY                      
};

/*Dcli_common_stp.h*/
#ifndef __DCLI_COMMON_STP_H__
#define __DCLI_COMMON_STP_H__
#endif

extern int dcli_debug_out;
//#define DCLI_DEBUG(x) if(dcli_debug_out){printf x ;}

#define DCLI_STP_OK 0
#define DCLI_STP_INVALID_PARAM (DCLI_STP_OK+1)
#define DCLI_STP_NO_SUCH_MSTID (DCLI_STP_OK+2)

#define CGI_STP_MODE 0
#define CGI_MST_MODE 1
//extern struct vty *vty;
#define NPD_SUCCESS 0
#define NPD_FAIL -1
#define CGI_SLOT_PORT_SPLIT_DASH '-'
#define CGI_SLOT_PORT_SPLIT_SLASH '/'

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

/*
typedef struct {
	unsigned int portMbr[2];
}PORT_MEMBER_BMP;
*/
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

typedef struct
{
	unsigned char   root_br_mac[6];
	unsigned char   design_br_mac[6];
	int      root_path_cost,design_br_version;
	unsigned short root_br_prio,design_br_prio;//short
	unsigned short  root_br_portId;//short
	unsigned short  root_br_maxAge,design_br_maxAge;//short
	unsigned short  root_br_hTime,design_br_hTime;//short
	unsigned short  root_br_fdelay,design_br_fdelay;//short
	unsigned char slot;
	unsigned char port;
}bridge_info;

typedef struct
{
	unsigned char  port_prio;
	unsigned int	port_cost;
	int 					port_role;
	int 					port_state;
	int 					port_lk;
	int 					port_p2p;
	int 					port_edge;
	unsigned short br_prio;
	unsigned char	mac[6];
	unsigned int	 br_cost;
	unsigned short br_dPort;
}port_info;


typedef struct
{
	unsigned char slot;
	unsigned char port;
}slot_port;

typedef struct     //******************桥自身信息
{
    char *pname;
	unsigned short revision;
	unsigned char   mac[6];
	unsigned int      br_version;
	unsigned int		count;
	unsigned short vid,*tmp,oldvid;
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;
	char map[10];
	char t1[10];	
}br_self_info;

typedef struct    //域消息
{
	unsigned char	mac[6];
	unsigned int		path_cost;
	unsigned short root_portId;
	unsigned short	br_prio;
	unsigned short	br_maxAge;
	unsigned short	br_hTime;
	unsigned short	br_fdelay;
	unsigned char	 br_hops;

}msti_info;
typedef struct  //根桥消息
{	
	char* pname;	
	unsigned char   mac[6];
	unsigned int		path_cost;
	unsigned short root_portId;	
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;

}cist_info;

#define CMD_SUCCESS 0
#define CMD_FAILURE -1
#define CMD_WARNING 1

#ifdef __cplusplus
extern "C"
{
#endif

extern int config_spanning_tree_mode(char *mode);


/********************************************************************
*	       name :	int config_spanning_tree(int able)
*	description :	start or stop RSTP function of bridge.
*	  parameter :	0 enable; 1 disable.
*		 return :	0 suc; -1 fail.
********************************************************************/


extern int config_spanning_tree(char *status,DBusConnection *connection);

/********************************************************************
*	       name :	int config_spanning_tree_ethport(int slot, int port, int able)
*	description :	start or stop RSTP function of designated port.
*	  parameter :	
*		 return :	0 suc; -1 fail.
********************************************************************/

extern int config_spanning_tree_ethport(char *port, char *status);

extern int config_spanning_tree_ethport_new(char *port, char *status);  //********


/*config spanning tree*/
extern int config_spanning_tree_pri(char *priority );


extern int config_spanning_tree_max_age(char *max_age);


extern int config_spanning_tree_hello_time(char *hellotime);


extern int config_spanning_tree_forward_delay(char *delaytime);


extern int config_spanning_tree_version(char *version);


extern int config_spanning_tree_default();



/*config spanning tree port*/
extern int config_spanning_tree_port_cost(char *port, char *cost);


extern int config_spanning_tree_port_pri(char *port, char *pri);


extern int config_spanning_tree_port_nonstp(char *port, char *mode);


extern int config_spanning_tree_port_p2p(char *port, char *mode);


extern int config_spanning_tree_port_edge(char *port, char *mode);


extern int config_spanning_tree_port_mcheck(char *port, char *mode);


extern int config_spanning_tree_port_default(char *port);



//MSTP
extern int config_spanning_tree_region_name(char *region_name);

extern int config_spanning_tree_revision(char *br_revision);

extern int config_spanning_tree_mstp_prio(char *instanceID, char *prio);

extern int config_spanning_tree_max_hops(char *max_hops);

extern int config_spanning_tree_path_cost_mstp(char *mstID, char *port, char *path_cast);

extern int config_spanning_tree_port_prio_mstp(char *mstID, char *port, char *prio);

extern int config_spanning_tree_default_mstp(char *mstID);

extern int config_spanning_tree_port_default_mstp(char *mstID, char *port);

extern int config_spanning_tree_map(char *vlanID, char *mstID);

extern int config_spanning_tree_all_instance(char *mstID);

extern int show_spanning_tree_one_instance(char *mstID);

extern int ccgi_get_br_self_info(int mstid, unsigned short ** pvid, unsigned int *num, int flag,br_self_info *test);//**************



/*show spanning tree information*/
//extern int show_spanning_tree(bridge_info *ptbridge_info);


extern int ccgi_get_broad_product_id(unsigned int *id);

extern int ccgi_get_brg_g_state(int *stpmode);

extern int ccgi_enable_g_stp_to_protocol(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int isEnable,
	DBusConnection *connection

);



extern int ccgi_set_bridge_force_version(
	unsigned int fversion,
	DBusConnection *connection
);

extern int ccgi_get_one_port_index(unsigned char slot,unsigned char local_port,unsigned int *port_index);

extern int ccgi_get_port_index_link_state(unsigned int port_index, int* lkState);


//extern int ccgi_enable_stp_on_one_port_to_protocol(unsigned int port_index,unsigned int enable,int lkState);

extern int ccgi_enable_stp_on_one_port_to_npd(unsigned int mode, unsigned int port_index, unsigned int enable);

extern int ccgi_stp_set_port_pathcost_to_npd(unsigned int mstid, unsigned int port_index, unsigned int value);

extern int ccgi_stp_set_port_prio_to_npd(unsigned int mstid, unsigned int port_index, unsigned int value);

extern int ccgi_stp_set_port_nonstp_to_npd(unsigned int mstid, unsigned int port_index, unsigned int value);

extern int ccgi_stp_set_port_p2p_to_npd(unsigned int mstid, unsigned int port_index, unsigned int value);

extern int ccgi_stp_set_port_edge_to_npd(unsigned int mstid, unsigned int port_index, unsigned int value);

extern int ccgi_get_all_ports_index(		
  PORT_MEMBER_BMP* portBmp

);

extern int ccgi_get_br_info(bridge_info *ptbridge_info);

extern int ccgi_get_one_port_info(unsigned int port_index, unsigned int portductid, port_info *ptport_info);



extern int ccgi_enable_g_stp_to_npd(unsigned int enable,DBusConnection *connection);

extern int ccgi_send_vlanbmp_to_mstp(VLAN_PORTS_BMP* ports_bmp);

extern int ccgi_get_vlan_portmap(VLAN_PORTS_BMP** ports_bmp, unsigned int* count);

extern int check_port_state(char *port);//检查端口状态

extern int ccgi_change_all_ports_to_bmp(VLAN_PORTS_BMP* ports_bmp, unsigned int *num);

extern int ccgi_get_mstp_one_port_info(int mstid,unsigned int vid, unsigned int port_index, unsigned int slot, unsigned int port);

extern int show_slot_port_by_productid(
	unsigned int product_id,
	PORT_MEMBER_BMP* portBmp,
	unsigned int vid,
	unsigned int mstid
);

extern int ccgi_get_msti_info_new(int mstid,msti_info *test);  //新的

extern int ccgi_get_cist_info_new(int mstid,cist_info *test);  //新的

extern int ccgi_get_msti_info(int mstid);

extern int ccgi_get_cist_info();

extern int ccgi_stp_set_stpid_to_npd(unsigned short vid, unsigned int mstid);

extern int ccgi_get_one_vlan_portmap(unsigned short vid, VLAN_PORTS_BMP* ports_bmp);

//*******************

int ccgi_get_port_index_speed(unsigned int port_index,unsigned int* speed);  //新增函数 

int ccgi_set_port_duplex_mode_to_stp(unsigned int port_index,unsigned int mode);

int ccgi_get_port_duplex_mode(unsigned int port_index,unsigned int* mode);

int ccgi_enable_stp_on_one_port_to_protocol(unsigned int port_index,unsigned int enable,int lkState,unsigned int speed);

extern int check_abc(char *ff);

extern int get_port_admin_state
(
	char *port_num,
	int *s_type
);
extern int ccgi_get_brg_g_state_slot(int *stpmode,DBusConnection *connection);


#ifdef __cplusplus
extern "C"
}
#endif






