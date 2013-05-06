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
* ws_eag_conf.h
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

#ifndef _WS_EAG_CONF_H
#define _WS_EAG_CONF_H
       
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//////define node name
#define  HS_PLOT_ID        "HS_PLOT_ID"
#define	 HS_STATUS         "HS_STATUS" 
#define	 HS_STATUS_KICK    "HS_STATUS_KICK" 
#define	 HS_DEBUG_LOG      "HS_DEBUG_LOG" 
#define	 HS_WANIF          "HS_WANIF"
#define	 HS_LANIF          "HS_LANIF"
#define	 HS_NETWORK        "HS_NETWORK"
#define	 HS_NETMASK        "HS_NETMASK"
#define	 HS_UAMLISTEN      "HS_UAMLISTEN"
#define	 HS_UAMPORT        "HS_UAMPORT"
#define	 HS_NAS_PT         "HS_NAS_PT"
#define	 HS_RADIUS_PT      "HS_RADIUS_PT"
#define	 HS_PORTAL_PT      "HS_PORTAL_PT"
#define	 HS_WWV_PT      	"HS_WWV_PT"
#define  HS_RADACCTINTERVAL "HS_RADACCTINTERVAL"
#define  HS_DEFIDLETIMEOUT "HS_DEFIDLETIMEOUT"
#define  HS_IDLEFLOW		"HS_IDLEFLOW"
#define  HS_VRRPID         "HS_VRRPID"
#define  HS_PPI_PORT       "HS_PPI_PORT"
#define  HS_MAX_HTTPRSP       "HS_MAX_HTTPRSP"

#define PLOTID_ZEAO    "0"
#define PLOTID_ONE     "1"
#define PLOTID_TWO     "2"
#define PLOTID_THREE   "3"
#define PLOTID_FOUR    "4"
#define PLOTID_FIVE    "5"

#define MTD_N     "default"
#define MTR_N     "s"
#define MTP_N     "p"
#define MTN_N     "n"
#define MTC_N     "eag"
#define MTW_N     "w"

#define ATT_Z     "attr"
#define VALUE_Z   "value"

#define MULTI_RADIUS_F  "/opt/services/conf/radiusmt_conf.conf"
#define MULTI_PORTAL_F  "/opt/services/conf/portalmt_conf.conf"
#define MULTI_NAS_F     "/opt/services/conf/nasmt_conf.conf"
#define MULTI_EAG_F     "/opt/services/conf/eagmt_conf.conf"
#define MULTI_WWV_F     "/opt/services/conf/wwvmt_conf.conf"

#define MULTI_RADIUS_STATUS  "/opt/services/status/radiusmt_status.status"
#define MULTI_PORTAL_STATUS  "/opt/services/status/portalmt_status.status"
#define MULTI_NAS_STATUS     "/opt/services/status/nasmt_status.status"
#define MULTI_EAG_STATUS     "/opt/services/status/eagmt_status.status"
#define MULTI_WWV_STATUS     "/opt/services/status/wwvmt_status.status"
//optionz values
#if 0
struct multiradius
{
	char *content; 
	int  mt_num;
	struct multiradius *next;
};
#endif

struct  substringz
{
	char *substr;
	struct substringz *next;
};
struct optionz
{
    char content0[20];
	char content1[20]; 
	char content2[20];  
	char content3[20]; 
	char content4[20];  
	char content5[20]; 
};

#define NTYPE   "ntype"
#define NSTART  "nstart"
#define NEND    "nend"
#define NNASID  "nnasid"
#define NCOVER  "ncover"
//nas
struct st_nasz
{
	char ntype[32];
	char nstart[32];
	char nend[32];
	char nnasid[32];
	char ncover[32];
	struct st_nasz *next;
};
#define RDOMAIN  "rdomain"
#define RRADST   "rradst"
#define RRIP     "rrip"
#define RRPORT   "rrport"
#define RRKEY    "rrkey"
#define RRPORTAL "rrportal"
#define RCIP     "rcip"
#define RCPORT   "rcport"
#define RCKEY    "rckey"
#define RBIP     "rbip"
#define RBPORT   "rbport"
#define RBKEY    "rbkey"
#define RBPORTAL "rbportal"
#define RBCIP    "rbcip"
#define RBCPORT  "rbcport"
#define RBCKEY   "rbckey"
#define R_SWAP_OCTETS   "r_swap_octets"


#define RADIUS_SERVER_TYPE_DEFAULT	"default"
#define RADIUS_SERVER_TYPE_RJ		"sam-rj"
#define RADIUS_SERVER_TYPE_HW		"huawei" /*add by tangsiqi for huawei radius*/

#define PORTAL_XMLF         "/opt/services/conf/portalmt_conf.conf"

//radius
struct st_radiusz
{
	char domain_name[128];
	char radius_server_type[32];
	char radius_server_ip[50];
	char radius_server_port[32];
	char radius_server_key[128];
	char radius_server_portal[32];
	char charging_server_ip[50];
	char charging_server_port[32];
	char charging_server_key[128];
	char backup_radius_server_ip[50];
	char backup_radius_server_port[32];
	char backup_radius_server_key[128];
	char backup_radius_server_portal[32];
	char backup_charging_server_ip[50];
	char backup_charging_server_port[32];
	char backup_charging_server_key[128];
	char swap_octets[32];
	struct st_radiusz *next;
};
//#define PSSID  "pssid" /* don't use this */
#define P_KEYWORD   "pssid" /* it is not just SSID now ,it is key word -- wk*/
#define PIP    "pip"
#define PPORT  "pport"
#define PWEB   "pweb"
#define PACN   "pacn"
#define PNPORT "pnport"
#define PJOM   "pjom"
#define PDOMAIN   "pdomain"
#define P_TYPE   "ptype"
#define P_ADVERTISE_URL "p_advertise_url"
#if 0

//portal
struct st_portalz
{
	//char pssid[32];
	char p_keyword[32];
	char p_type[32];
	char pip[32];
	char pport[32];
	char pweb[128];
	char pnport[32];
	char pjom[32];
	char pdomain[32];
	char pacname[32];
	struct st_portalz *next;
};
#endif

struct st_portalz
{
	//char pssid[32];/* don't use this */
	char *p_keyword;
	char *p_type;
	char *pip;
	char *pport;
	char *pweb;
	char *pnport;
	char *pjom;
	char *pdomain;
	char *pacname;
	char *advertise_url;
	struct st_portalz *next;
};
//eag
struct st_eagz
{
	char eag_start[20];
	char space_start[20];
	char debug_log[20];
	char db_listen[32];
	char listen_port[20];
	char nasid[10];
	char radiusid[10];
	char portalid[10];
	char wwvid[10];
	char timeout[10];
	char vrrpid[10];
	char ppi_port[10];
	char max_httprsp[10];
	char def_acct_interval[10];
	char max_idle_flow[10];
	struct st_eagz *next;
};

#define WLANSIDZ "wlansidz"
#define WLANEIDZ "wlaneidz"
#define WTPSIDZ  "wtpsidz"
#define WTPEIDZ  "wtpeidz"
#define VLANIDZ  "vlanidz"
//wlan wtp vlan
struct st_wwvz
{
   char wlansidz[10];
   char wlaneidz[10];
   char wtpsidz[10];
   char wtpeidz[10];
   char vlanidz[10];
   struct st_wwvz *next;
};


extern int add_eag_node_attr(char *fpath,char * node_name,char *attribute,char *ruler);

extern int mod_eag_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc);

extern int if_xml_file_z(char * fpath);

extern int get_eag_node_attr(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey);

extern int get_eag_node(char * fpath,char * node_name,char *content);

extern int delete_eag_onelevel(char *fpath,char *node_name,char *attribute,char *ruler);

extern int delete_eag_seclevel(char *fpath,char *node_name,char *attribute,char *ruler,char *secnode);

extern void Free_radius_info(struct st_radiusz *head);

extern void Free_eag_info(struct st_eagz *head);

extern void Free_portal_info(struct st_portalz *head);

extern void Free_nas_info(struct st_nasz *head);

extern int read_radius_xml(char *fpath,struct st_radiusz *chead,int *confnum,char *node_name);

extern int read_portal_xml(char *fpath,struct st_portalz *chead,int *confnum,char *node_name);

extern int read_nas_xml(char *fpath,struct st_nasz *chead,int *confnum,char *node_name);

extern int read_eag_xml(char *fpath,struct st_eagz *chead,int *confnum,char *node_name);

extern int read_optionz_xml(char *fpath,struct optionz *chead,int *confnum,char *node_sep);

extern int get_eag_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct st_eagz *cq);

extern int get_radius_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct st_radiusz *cq);

extern int get_nas_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct st_nasz *cq);

extern int if_design_node(char *fpath,char *node_name,char *attribute,char *ruler);

extern int if_design_node_second(char *fpath,char *node_name,char *attribute,char *ruler,char *content);

extern int read_wwvz_xml(char *fpath,struct st_wwvz *chead,int *confnum,char *node_name);

extern void Free_wwvz_info(struct st_wwvz *head);

extern int create_eag_xml(char *xmlpath);

extern int add_portal_server_mib(char *xmlpath,char *nodename,char *pssid,char *purl,char *acname,char *portal_server_portal,char *noteport,char *domain,char *type);

extern void Free_get_portal_struct(struct st_portalz *f1);


#endif


