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
* capture.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

#ifndef _WS_DCLI_PORTCONF_H

#include "ws_nm_status.h"

#define NPD_SUCCESS 0
#define NPD_FAIL -1

#define TRUE 	1
#define FALSE 	0

#define DCLI_INTF_CHECK_MAC_ERR		5

#define _WS_DCLI_PORTCONF_H

#define WS_SUCCESS 0
#define WS_FAIL 1
#define WS_NO_SUCH_PORT 2
#define WS_RESET 3
#define WS_EXEC_COMM_FAIL 4
#define WS_FAIL_GET_ARG 5
#define WS_NOT_SUPPORT 6
#define WS_BAD_VALUE  7
#define WS_OUT_RANGE 8
#define WS_ADMIN_DIS 10
#define WS_ERR_UNKNOW 	11
#define WS_DEL_FROM_VLAN_FIRST 	12

#define DEFAULT 0xff
#define ADMIN (1<<0)
#define SPEED (1<<1)
#define AUTONEGT (1<<2)
#define AUTONEGTS (1<<3)
#define AUTONEGTD (1<<4)
#define AUTONEGTF (1<<5)
#define DUMOD (1<<6)
#define FLOWCTRL (1<<7)
#define BACKPRE (1<<8)
#define LINKS (1<<9)
#define CFGMTU (1<<10)

#define VTY_SHUTDOWN 			"/usr/bin/vty_shutdown.sh"
#define VTY_SHUTDOWN_ERR_TMP    "/var/run/vty_shutdown_err_tmp"


#define NPD_DBUS_SUCCESS 0
#define NPD_DBUS_ERROR_NO_SUCH_PORT 2
#define DCLI_ETH_PORT_ALREADY_RUN_THIS_MODE 55
#define NPD_DBUS_ERROR_UNSUPPORT  6

#define DCLI_ETH_PORT_HAVE_SUBIF        58

extern char *p_slot_status_str[MODULE_STAT_MAX];
extern char *p_eth_port_type_str[ETH_MAX];
extern char *p_link_status_str[2];
extern char *p_doneOrnot_status_str[2];
extern char *p_onoff_status_str[2];
extern char *p_duplex_status_str[2];
extern char *p_eth_speed_str[ETH_ATTR_SPEED_MAX];
extern char *p_eth_media_str[3];


struct arp_nexthop_profile
{
   unsigned int ipaddr;
   unsigned char mac[6];
   unsigned char slot_no;
   unsigned char port_no;
   unsigned char isTrunk;
   unsigned char trunkId;
   unsigned short vid;
   unsigned char isTagged;
   unsigned char isStatic;
   unsigned char isValidz;
   unsigned int refCnt;
   struct arp_nexthop_profile *next;
};
typedef struct{
unsigned short slot;
unsigned short plot;
unsigned long  long int rx_goodbytes;
unsigned long  long int rx_goodbytes_mib;
unsigned long  long int rx_badbytes;
unsigned long  long int rx_badbytes_mib;
unsigned long  long int rx_uncastpkts;
unsigned long  long int rx_bcastpkts;
unsigned long  long int rx_mcastpkts;
unsigned long  long int rx_fcframe;
unsigned long  long int rx_fifooverruns;
unsigned long  long int rx_underSizeframe;
unsigned long  long int rx_fragments;
unsigned long  long int rx_overSizeframe; 
unsigned long  long int rx_jabber;
unsigned long  long int rx_errorframe;
unsigned long  long int rx_BadCrc;
unsigned long  long int rx_collision;
unsigned long  long int rx_late_collision;
unsigned long  long int rx_sent_deferred;

unsigned long  long int tx_sent_deferred;
unsigned long  long int tx_goodbytes;
unsigned long  long int tx_uncastframe;
unsigned long  long int tx_excessiveCollision;
unsigned long  long int tx_mcastframe;
unsigned long  long int tx_bcastframe;
unsigned long  long int tx_sentMutiple;
unsigned long  long int tx_fcframe;
unsigned long  long int tx_crcerror_fifooverrun;

unsigned long  long int k1024tomax;
unsigned long  long int k512to1023;
unsigned long  long int k256to511;
unsigned long  long int k128to255;
unsigned long  long int k65to127;
unsigned long  long int k64oct;
unsigned int linkupcount;
unsigned int linkdowncount;
}port_flow;

struct port_list
{
	char port_no;
	unsigned char porttype;
	unsigned int attr_map;
	unsigned int mtu;
	unsigned int link_keep_time;
	struct port_list * next;
};
typedef struct port_list ETH_PORT_LIST;

struct slot_list
{	
	char slot_no;
	int port_num;
	ETH_PORT_LIST port;
	struct slot_list * next;
};
typedef struct slot_list ETH_SLOT_LIST;




extern int ccgi_port_admin_state(char *str_port_name, char *str_state);
extern int ccgi_port_link_state(char *str_port_name, char *str_link_mode);
extern int ccgi_port_speed_conf(char *str_port_name, char *str_speed);
extern int ccgi_port_auto_speed(char *str_port_name, char *str_autospe_state);
extern int ccgi_port_mode_conf( char *str_port_name, char *str_mode);	/*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示it is already this mode*/
																    		/*返回-3表示unsupport this command，返回-4表示execute command failed*/
extern int ccgi_port_auto_dup(char *str_port_name, char *str_autodup_state);
extern int ccgi_port_auto_flowctl(char *str_port_name, char *str_autoflow_state);
extern int ccgi_port_dupmode_conf(char *str_port_name, char *str_mode);
extern int ccgi_port_flowctl_conf(char *str_port_name, char *str_flowctl_mode);
extern int ccgi_port_backpre_conf(char *str_port_name, char *str_bp_mode);
extern int ccgi_port_mtu_conf(char *str_port_name, char *str_mtu);
extern int ccgi_port_medai_conf(char *str_port_name, char *str_media_mode);
extern void Free_arp_nexthop_head(struct arp_nexthop_profile *head);
extern int ccgi_port_default(char *str_port_name);
extern int ccgi_show_ethport_arp(char *str_port_name,struct arp_nexthop_profile *arp_head,int *arp_num);  	/*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示execute command failed*/
extern int ccgi_clear_ethport_arp(char *str_port_name);  	/*返回0表示失败，返回1表示成功，返回-1表示execute command failed*/
extern int ccgi_show_ethport_nexthop(char *str_port_name,struct arp_nexthop_profile *nexthop_head,int *nexthop_num);	   /*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示execute command failed*/

extern int ccgi_get_port_flow(unsigned int value,unsigned char type,port_flow *test,unsigned int slot,unsigned int plot);//端口流量获取
extern int ccgi__show_eth_port_portno_stat();  //显示函数来调用
extern int ccgi_get_slot_port_by_portindex(	unsigned int port_index,unsigned char* slot,unsigned char* port);

extern int get_port_buffer_mode( char *mode );
extern int set_port_buffer_mode( char *str_mode );
extern int set_ethport_ipg_value( char *str_port_name, char *value );
extern int show_eth_port_ipg(unsigned int value,unsigned char type);
/*返回0且slot_mun>0时，调用Free_ethslot_head()释放空间*/
extern int show_ethport_list(ETH_SLOT_LIST *head,int * slot_num);
extern int show_eth_port_stat(struct eth_port_counter_s *EthStat, DBusConnection * connection,unsigned int value,unsigned char type);

//extern void Free_ethport_head(ETH_PORT_LIST * head);
extern void Free_ethslot_head(ETH_SLOT_LIST * head);
extern int count_eth_port_num();
extern int ccgi_port_interface_state_conf(char *str_port_name, char *state);

extern int ccgi_eth_port_mode_config
(
    unsigned char slot_no,
    unsigned char port_no,
    unsigned int mode
);
#endif
