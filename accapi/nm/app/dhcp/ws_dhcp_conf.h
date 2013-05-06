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
* ws_dhcp_conf.h
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
#ifndef _WS_DHCP_CONF_H
#define _WS_DHCP_CONF_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "dbus/npd/npd_dbus_def.h"
#include "dbus/dhcp/dhcp_dbus_def.h"
#include "ws_init_dbus.h"

#define DHCPFILESIZE    2048
///dhcpv4
#define DHCP_XML        "/opt/services/conf/dxml_conf.conf"
#define DHCP_CONF_PATH  "/opt/services/conf/dhcp_conf.conf"
#define DHCP_XML_STATE  "/opt/services/status/dxml_status.status"

//dhcpv4 second level node and children nodes option60
#define DHCP_OPT_NAME    "optname"
#define DHCP_OPT_USRN    "usrn"
#define DHCP_OPT_IDEN    "iden"
#define DHCP_OPT_VENDER  "vender"
#define DHCP_OPT_HEX     "opthex"
          
//dhcpÌí¼Ó
#define DHCP_NODE_CONF  "conf"
#define DHCP_NODE_ATTR  "attr"
#define DHCP_LEASE_TIME "ltime"
#define DHCP_TIMETICK   "timetickz"
#define DHCP_DEF_GW     "defgw"
#define DHCP_SERVER     "server"
#define DHCP_SERVER_N   "sername"
#define DHCP_NET_MASK   "netmask"
#define DHCP_ADDR_POOL  "pool"
#define DHCP_POOLNAME   "poolname"

#define DHCP_SUBNET_Z   "subnet"
#define DHCP_CONFNAME_Z "confname"
#define DHCP_IFACTIVE   "ifactive"
#define DHCP_OPTYPEZ    "optype"                     

//dhcpv4 second level node and children nodes and bind ip£¬mac
#define DHCP_BIND_NODE  "bindn"
#define DHCP_BIND_IPMAC "bindip"
#define DHCP_BIND_MAC   "bindmac"

//////dhcpv4 global paremeter
#define DHCP_GB_PARAM   "gbparam"
#define DHCP_GB_PINGC   "pingc"

///dhcpv6
#define DHCPVS_XML        "/opt/services/conf/dvsxml_conf.conf"
#define DHCPVS_CONF_PATH  "/opt/services/conf/dhcpvs_conf.conf"
#define DHCPVS_XML_STATE  "/opt/services/status/dvsxml_status.status"
#define DHCPVS_OPT_PATH   "/opt/services/option/dhcpvs_option"
#define DHCPVS_INITZ      "sudo /opt/services/init/dhcpvs_init"
#define DHCPVS_START      "start"
#define DHCPVS_STOP       "stop"
#define DHCPVS_RESTART    "restart"

///dhcpv6 second level node and children nodes(three level nodes)
#define DHCPVS_SC_CONF     "sconf"
#define DHCPVS_TC_POOL     "spool"
#define DHCPVS_TC_LEASE    "slease"
#define DHCPVS_TC_SUBNET   "ssubnet"
#define DHCPVS_TC_PREFIX   "sprefix"
#define DHCPVS_TC_AUTH     "sauth"
#define DHCPVS_TC_NSERVER  "snserver"
#define DHCPVS_TC_DSEARCH  "ssearch"
#define DHCPVS_TC_NETMASK "snetmask"


#define DHCPIPNETLEN    30
#define DHCPCOMLEN      10


//dhcpv6 confs           
struct dvsconfz
{
  char *spool; //big space
  char slease[50];
  char ssubnet[50];
  char snserver[50];
  char ssearch[50];
  char snetmask[50];
  char sauth[10];  
  char sprefix[10];
  struct dvsconfz *next;
};

//dhcpv4
struct  substringz
{
 char *substr;
 struct substringz *next;
};

//dhcpv4 option60
struct optionsix
{
  char classname[30];
  char iden[30];
  char vendor[30];
  char opthex[30];
  
  int optnum;
  struct optionsix *next;
};

//dhcpv4 conf            
struct confinfo
{
  char confname[30];
  char netmask[30];
  char gateway[30];
  char optype[10];
  char dnsname[30];
  char lease[30];
  char subnetz[30];  
  char ifactive[10];
  char *hexip;
  char *sip;
  char *dnserver;  
  char *pools;  
  int confnum;
  struct confinfo *next;
};

//dhcpv4 bind ip-mac
struct bindipmac
{
 char *bindinfo; 
 int bindnum; 
 struct bindipmac *next;

};

//dhcpv4 global params
struct gbparam
{
 char pingcheck[10];
 int gbnum; 
 struct gbparam *next;

};

//dhcpv4 gross struct
struct grossdhcp
{
  struct confinfo  *confmsg;
  struct bindipmac *bindmsg;
  struct optionsix *optmsg;

};

//dhcpv4 mibs api
struct pool_list
{
	char startipz[50];
	char endipz[50];
	int pool_num;
	struct pool_list * next;
};
typedef struct pool_list POOL_LIST;

//dhcpv4 mibs api
struct snmp_list
{	
	char leasez[50];
	char timetickz[50];
	char gatewayz[50];
	char netmask[50];
	char subnetz[50];
	char dnsz[256];
	int snmp_num;
	char stipz[50];
	char eipz[50];
	POOL_LIST pool;
	struct snmp_list * next;
};
typedef struct snmp_list SNMP_LIST;

struct snmp_single
{
  char leasez[50];
  char gatewayz[50];
  char subnetz[50];
  char startz[50];
  char endz[50];
};

struct dhcpleasestat
{
	char dlsubnet[DHCPIPNETLEN];
	char dlpoolname[DHCPIPNETLEN];
	char dluseage[DHCPCOMLEN];
	char dlcount[DHCPCOMLEN];
	char dlfree[DHCPCOMLEN];	
	int  dhcpreqnum;
	int dhcpresponse;
	unsigned long discovery_num;
	unsigned long offer_num;
	struct dhcpleasestat *next;
};

/*add or delete conf node and find one conf node*/

/* add xml node with attribute*/
extern int add_dhcp_node_attr(char *fpath,char * node_name,char *attribute,char *ruler);

/*find the designed xml node ,modify the node's content*/
extern int mod_dhcp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc);

/*if donnot have the xml file ,crreate it*/
extern int create_dhcp_xml(char *fpath);

/*find the designed xml node ,get the node's content*/
extern int get_dhcp_node_attr(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey);

/*find last node record the modify info*/
extern int get_dhcp_node(char * fpath,char * node_name,char *content);

//save datas
extern int string_link_list(struct substringz *head,int *subnum,char *source);


extern void Free_substringz_all(struct substringz *head);

extern void Free_bindipmac_info(struct bindipmac *head);

extern void Free_confinfo_info(struct confinfo *head);

extern void Free_optionsix_info(struct optionsix *head);

extern int read_option_info(struct optionsix *ohead,int *optnum);

extern int read_confinfo_xml(struct confinfo *chead,int *confnum);

extern int read_bindinfo_xml(struct bindipmac *bhead,int *bnum);

extern int write_dhcp_conf(char *fpath,struct optionsix *o_head,struct confinfo *c_head,struct bindipmac *b_head,struct gbparam *g_head);

extern int get_conf_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct confinfo *confs);

extern int if_xml_file(char * fpath);

extern int delete_dhcp_onelevel(char *fpath,char *node_name,char *attribute,char *ruler);

extern int read_gbparam_xml(struct gbparam *gbhead,int *gbnum);

extern void Free_gbparam_info(struct gbparam *head);

extern int string_linksep_list(struct substringz *head,int *subnum,char *source,char *sep);

extern int show_pool_list(SNMP_LIST *head,int * poolnumz);

extern void Free_poollist_head(SNMP_LIST * head);

///dhcpv6 function
extern int read_dvsconf_xml(char *fpath,struct dvsconfz *chead,int *confnum);
extern void Free_dvsconfz_info(struct dvsconfz*head);
extern int write_dhcpvs_conf(char *fpath,struct dvsconfz *c_head);
extern int get_dvsconf_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct dvsconfz *confs);
extern int start_dhcpvs();
extern int stop_dhcpvs();
extern int restart_dhcpvs();

extern int get_show_pool_list(int indexz,struct snmp_single *ssingle);
extern int get_show_xml_nodenum(char * fpath,char * node_name,int *nodenum);
extern int save_dhcp_lease();
extern void get_sub_net(char *soureip,char *maskbit,char *subz)	;
extern int mod_dnsz_service(char *subnetz,char *newdns,int indexz);
extern int save_dhcpconf();
extern int get_pool_info();
extern int get_dhcp_pool_info(struct dhcpleasestat *head);
#endif

