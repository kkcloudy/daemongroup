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
* ws_fdb.h
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
#ifndef __DCLI_FDB_H__
#define __DCLI_FDB_H__


#define MAX_VLANID 4094
#define MIN_VLANID 1
#define MIN_SLOT   1
#define MAX_SLOT   4
#define MIN_PORT   1
#define MAX_PORT   24
#define ALIAS_NAME_SIZE 0x15
#define CMD_FAILURE -1
#define CMD_SUCCESS 0
#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'
#define SLOT_PORT_SPLIT_COMMA 	','

#define CPU_PORT_VIRTUAL_SLOT	 0x1F
#define CPU_PORT_VIRTUAL_PORT	 0x3F // for CPU port (*,63)
#define SPI_PORT_VIRTUAL_SLOT	 CPU_PORT_VIRTUAL_SLOT
#define SPI_PORT_VIRTUAL_PORT	 0x1A // for SPI port (*,26)

#define PageNumber 10

enum{

	NPD_FDB_ERR_NONE =0, /////!
	NPD_FDB_ERR_DBUS,
	NPD_FDB_ERR_GENERAL, /////!
	NPD_FDB_ERR_BADPARA,
	NPD_FDB_ERR_OCCUR_HW,
	NPD_FDB_ERR_ITEM_ISMIRROR,
	NPD_FDB_ERR_NODE_EXIST ,   /////!
	NPD_FDB_ERR_NODE_NOT_EXIST,  /////!
	NPD_FDB_ERR_PORT_NOTIN_VLAN, /* 3->4*/ /////!
	NPD_FDB_ERR_VLAN_NONEXIST, /*1.5->5*/ /////!
	NPD_FDB_ERR_SYSTEM_MAC, /////!
	NPD_FDB_ERR_VLAN_NO_PORT,
	NPD_FDB_ERR_HW_NOT_SUPPORT,
	NPD_FDB_ERR_MAX   /////!
};

enum{
	port_type =0,
	trunk_type,
	vidx_type,
	vid_type
};


struct fdb_all
{
  unsigned char  show_mac[6];
  unsigned short vlanid;
  unsigned int type_flag;
  unsigned int dcli_flag;

  unsigned int trans_value1;
  unsigned int trans_value2;

  char* type;
  char* vname;

 
  struct fdb_all *next;
};

struct fdb_blacklist
{
  unsigned char  show_mac[6];
  unsigned short vlanid;

  unsigned char dmac ;
  unsigned char smac ;	
	
  struct fdb_blacklist *next;
};

struct fdb_limit
{
  unsigned short vlanid;

  unsigned char slotNum;
  unsigned char portNum;
  unsigned int fdbnumber;
  
  struct fdb_limit *next;
};

struct fdb_mac_vid
{
  unsigned short vlanid;
  unsigned int trans_value1;
  unsigned int trans_value2;
  unsigned char  show_mac[6];
  unsigned short dcli_flag;  
};


int parse_vlan_no_fdb(char* str,unsigned short* vlanId);
inline int parse_agetime(char* str,unsigned short* agingtime);
inline int parse_int_parse(char* str,unsigned int* shot);
int mac_format_check_fdb(char* str,int len);
int parse_vlan_string(char* input);


extern int config_fdb_agingtime(char *agingtime);

extern int show_fdb_agingtime(int *agingtime);

extern int config_fdb_agingtime_default();

extern int create_fdb_static(char *mac, char *vlanid, char *portno);

extern int create_fdb_static_vlanname(char *mac, char *vlan_name, char *portno);

extern int create_fdb_static_vlanname_trunk(char *mac, char *vlan_name, char *trunkid);

extern int create_fdb_static_vlanid_trunk(char * mac,char * vlan_id,char * trunkid);

extern int delete_fdb_blacklist_by_vid(char *mactype, char *mac, char *vlanid);

extern int delete_fdb_blacklist_by_vlanname(char *mactype, char *mac, char *vlan_name);

extern int delete_fdb_blacklist_by_vlanname(char *mactype, char *mac, char *vlan_name);

extern int create_fdb_blacklist_by_vid(char * mactype,char * mac,char * vlanid);

extern int create_fdb_blacklist_by_vlanname(char *mactype, char *mac, char *vlan_name);

extern int delete_fdb_static_by_vlanname(char *mac, char *vlan_name);

extern int delete_fdb_static_by_vid(char *mac, char *vlanid);

extern int delete_fdb_static_by_vlanid(char * vlan_id);

extern int delete_fdb_static_by_port(char *portno);

extern int delete_fdb_by_vid(char *vlan_id);

extern int delete_fdb_by_port(char *portno);

extern int delete_fdb_by_trunk(char *trunk_id);

extern int show_fdb_by_mac_vid(
char *mac,
char *vlan_id,
struct fdb_mac_vid *macvid
);

extern int get_fdb_count();


extern int show_fdb    //显示所有的fdb列表
(
struct fdb_all *head,
int *fdb_num
);

extern int show_fdb_static  //显示静态的fdb列表
(
struct fdb_all *head,
int *fdb_num
);

extern int show_fdb_dynamic  //显示动态的fdb列表
(
struct fdb_all *head,
int *fdb_num
);

extern int show_fdb_blacklist  //显示黑名单列表
(
struct fdb_blacklist *head,
int *bl_num
);

extern int show_fdb_by_vid  //显示指定vlan的列表
(
char *vlan_id,
struct fdb_all *head,
int *vid_num
);

extern int show_fdb_by_vlanname  //显示vlan名字的列表，可更改一下
(
char *vlan_name,
struct fdb_all *head,
int *vid_num
);

extern int show_fdb_by_port  //显示vlan的端口的信息列表
(
struct fdb_all *head,
int *p_num,
char *portno
);
 
extern int show_fdb_by_mac  //根据mac显示fdb列表
(
char *mac,
struct fdb_all *head,
int *m_num
);

extern void Free_fdb_all(struct fdb_all *head);

extern void Free_fdb_blacklist(struct fdb_blacklist *head);

extern void Free_fdb_limit(struct fdb_limit *head);

extern int config_fdb_by_port(char *portno, char *count);

extern int config_fdb_by_vlan(char *vlan_id, char *count);

extern int config_fdb_by_vlan_port(char *vlan_id, char *port, char *count);


extern int show_fdb_number_limit();
//void dcli_fdb_show_running_config();

#endif   



#ifdef __cplusplus
extern "C"
{
#endif
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
//#include <zebra.h>
#include <dbus/dbus.h>

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>

///#include "command.h"
//#include "if.h"

//#include "dcli_fdb.h"
//#include "dcli_main.h"
#include "npd/nbm/npd_bmapi.h"


//extern DBusConnection *dcli_dbus_connection;
extern DBusConnection *ccgi_dbus_connection;

//extern int toupper(int c);

#ifdef __cplusplus
}
#endif

