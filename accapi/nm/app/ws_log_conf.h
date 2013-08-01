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
* ws_log_conf.h
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
#ifndef _WS_LOG_CONF_H
#define _WS_LOG_CONF_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*存储信息临时文件*/
#define temppath "/var/run/apache2/log.txt"

#define VTYSH_CLI_LOG_CONFIG_FILE "/var/run/cli_log.conf"

/*file paths*/
#define	XML_FPATH	    "/opt/services/option/syslog_option"
#define CONF_FPATH		"/opt/services/conf/syslog_conf.conf"
#define STATUS_FPATH    "/opt/services/status/syslog_status.status"
#define XML_SYS_D       "/opt/www/htdocs/syslog.xml"
#define CONF_SYS_D      "/etc/syslog-ng/syslog-ng.conf"

/*ntp file paths*/
#define	NTP_XML_FPATH	    "/opt/services/option/ntp_option"
#define NTP_CONF_FPATH		"/etc/ntp.conf"
#define NTP_STATUS_FPATH    "/opt/services/status/ntp_status.status"
#define NTP_CONF_BK         "/opt/services/conf/ntp_conf.conf"
#define XML_NTP_D           "/opt/www/htdocs/ntp.xml"
#define SEM_ACTIVE_MASTER_SLOT_ID_PATH "/dbm/product/active_master_slot_id"

#define CONF_ENABLE  "1"
#define CONF_DISABLE "0"

#define IP_TXT          "/mnt/ip.txt"

/*service operations*/
#define S_PATH		"sudo  /opt/services/init/syslog_init"
#define S_START		"start"
#define S_RESTART	"restart"
#define S_STOP		"stop"
/*ntp service operations*/
#define N_PATH		"sudo /opt/services/init/ntp_init"
#define N_START		"start"
#define N_RESTART	"restart"
#define N_STOP		"stop"


/*节点定义*/
#define XML_NODE_ROOT		"root"

#define NODE_OPT            "options"
#define NODE_SOURCE         "source"
#define NODE_MARKZ          "markz"
#define NODE_SYSIP			"sysipz"
#define NODE_SYSPORT        "sysport"
#define NODE_PROTOCOLZ      "proz"
#define NODE_FLEVEL         "flevel"

#define NODE_ATT            "attribute"
#define NODE_CONTENT        "content"
#define NODE_VALUE          "value"
#define NODE_VIEWS          "views"
#define NODE_INFOS          "infos"
#define NODE_DIP            "df_sysip"
#define NODE_ENABLES        "enablez"
#define NODE_INDEXZ         "indexz"
#define F_FLOOD				"f_synflood"
#define F_EAGLOG			"f_eag"
#define IF_SYNFLOOD			"ifsynflood"
#define IF_EAGLOG			"ifeaglog"
#define F_PDEBUG         	"f_portal_debug"
#define DF_PDEBUG           "df_userconsole"
#define F_CLILOG			"f_cli"
#define DF_CLILOG			"df_cli"
#define NODE_LOG            "log"
#define NODE_DES            "des"
#define NODE_FILTER         "filter"

#define CH_KEYZ             "keyz"
#define CH_SOURCE           "source"
#define CH_FILTER           "filter"
#define CH_DEST             "dest"
#define NODE_LSTATUS        "lstatus"
#define NODE_SAVECYCLE      "savecycle"
#define NODE_EXEC			"execn"
#define NODE_LSERV			"locserv"

/*NTP节点定义*/
#define NTP_SERV    "server"
#define NTP_DRIFT   "drift"
#define NTP_REST    "rest"
#define NTP_BROD    "broad"
#define NTP_RSER    "reser"
#define NODE_IPV4   "nipv4"
#define NODE_IPV6   "nipv6"

/*server and sub node*/
#define NTP_SERVZ    "serverz"
#define NTP_SIPZ     "sipz"
#define NTP_SMASKZ    "smaskz"
#define NTP_SERVDF    "serdefult"

/*client and sub node*/
#define NTP_CLIZ    "clientz"
#define NTP_CIPZ    "cipz"
#define NTP_CPERZ   "cperz"
#define NTP_UPCLIZ  "upclientz"
#define NTP_UPFLAG  "upflag"
#define NTP_CLIDF   "cldefult"

/*ntp public node*/
#define NTP_TIMEF   "ntimef"
#define NTP_SLOTID  "nslotid"

#define NTP_DEF     "default"

#define NTP_NM      "nomodify"
#define NTP_DI      "default ignore"
#define NTP_RING    "127.127.1.0"
#define NTP_PRE     "prefer"
#define NODE_RING   "sring"

/*跟value值也是一样的*/
#define L_WARN     "warn"
#define L_CRIT     "crit"
#define L_NOTICE   "notice"
#define L_ERR      "err"
#define L_INFO     "info"
#define L_ALL      "all"
#define L_IP       "ip"
#define L_DEBUG    "debug"

/*字符串定义*/
#define S_SOUR  "source"
#define S_FILT  "filter"
#define S_DEST  "destination"

#define IPMASK_LEN   32
#define TIMESTR_LEN  20
#define SID_LEN      64
#define NTPOSTR_LEN  20

#define OPT_ADD 	1
#define OPT_DEL 	0
#define FIL_OPT		1
#define DES_OPT		2
#define LOG_OPT     3

typedef struct {   

	char rname[128];	
	char content[256];
	
}ST_DES_INFO;

typedef struct {   
    char views[10];
	char infos[128];
	char fname[128];	
	char content[256];
	
}ST_FILTER_INFO;

typedef struct {   
    char keyz[128];
	char source[128];	
	char filter[128];
	char dest[128];
	
}ST_LOG_INFO;

typedef struct {   
	
	char *content;
	
}ST_OPT_INFO;

typedef struct {   

	char suname[128];
	char content[256];
	
}ST_SU_INFO;


typedef struct {
	
    int des_num;	
	ST_DES_INFO desinfo[10];	

	int f_num;
	ST_FILTER_INFO finfo[10];

	int log_num;
	ST_LOG_INFO loginfo[10];

	int opt_num;
	ST_OPT_INFO optinfo[10];

	int su_num;
	ST_SU_INFO suinfo[10];
	
	
}ST_SYS_ALL;


typedef struct {

char key[256];

}ST_LOG_KEY;


typedef struct {
char content[256];
}ST_IP_CONTENT;

struct filter_st
{
	char *valuez;
	char *viewz;
	char *contentz;
	char *infos;  
	int fnum;
	struct filter_st *next;
};

struct dest_st
{
	char *valuez;
	char *contentz;
	char *sysipz;
	char *sysport;
	char *proz;
	char *timeflag;
	char *indexz;
	char *flevel;
	int dnum;
	struct dest_st *next;
};

struct log_st
{
	char *flag;/*for mib!!*/
	char *keyz;
	char *filterz;
	char *sourcez;
	char *des;
	char *enablez;
	char *timeflag;
	char *sysipz;
	char *sysport;
	char *indexz;
	struct log_st *next;
};
struct opt_st
{
	char *contentz;
	struct opt_st *next;
};

struct source_st
{
    char *valuez;
	char *contentz;
	struct source_st *next;
};

typedef struct{
	struct opt_st     optst;
	int optnum;
	struct source_st  sourst;
	int sournum;
	struct filter_st  filtst;
	int filtnum;
	struct dest_st    destst;
	int destnum;
	struct log_st     logst;
	int lognum;
	int serv_flag;
	int synflood;
	int eaglog;
}SYSLOGALL_ST;

struct  syslogrule_st
{
    char udpstr[10];
	char ipstr[32];
	char portstr[10];
	char filter[50];/*desc*/
	char flevel[10];/*1,2...*/
	char id[10];
	char rulename[20];
	char keyword[64];
};

struct  syslogdes_st
{
	char ipstr[32];
	char portstr[10];
	char rulename[20];
};
/*-----ntp------*/
struct clientz_st
{
    char clitipz[IPMASK_LEN];
	char timeflag[TIMESTR_LEN];
	char ifper[NTPOSTR_LEN];
	char slotid[SID_LEN];
	struct clientz_st *next;
};
struct  serverz_st
{
    char servipz[IPMASK_LEN];
	char maskz[IPMASK_LEN];
	char timeflag[TIMESTR_LEN];
	char slotid[SID_LEN];
	struct serverz_st *next;
};
typedef struct{
	struct serverz_st  serst;
	int optnum;
	struct clientz_st  clist;
	struct clientz_st  upclist;
	struct serverz_st  sdefault;
	struct clientz_st  cdefault;
	int sournum;	
	char ipv4[NTPOSTR_LEN];
	char ipv6[NTPOSTR_LEN];
	char cront[NTPOSTR_LEN];
	char ntpv[NTPOSTR_LEN];
	char sring[NTPOSTR_LEN];
	char upflag[10];
}NTPALL_ST;


static const char *filterlist[] = {
	"f_at_least_crit",
	"f_at_least_err",
	"f_at_least_warn",
	"f_at_least_notice",
	"f_at_least_info",
	"f_debug",
};
struct timez_st
{
    char nowstr[256];
	char utcstr[256];
	char hwclockstr[256];
	char slotid[SID_LEN];
};

/*函数部分，可分为给上层用的，和内部调用的*/

extern int read_filter(char * name,char * node_name, ST_SYS_ALL *sysall); //读xml内容并存储

extern int write_config( ST_SYS_ALL *sysall, char *file_path ); //写入到conf文件中

extern int add_node(char * name,char * node_name); //添加指定节点

extern int mod_log_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc);


extern int start_syslog(); //启动服务

extern int stop_syslog();  //停止服务
 
extern int restart_syslog(); //重启服务

extern int is_file_exist( char *filtpath ); //判断文件存在否

/*取出指定节点内容*/
extern int find_log_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,ST_LOG_KEY *logkey);

/*截取处理字符串*/
extern int  cut_up(char *dstring,ST_LOG_KEY *keys,char *ruler);

extern int ip_save(char *fpath,char *value); //存储ip字符串

extern int  line_num(char *fpath,char *cmd); //对文件的相应计算

extern int get_ip(char *fpath,ST_IP_CONTENT * ip); //从文件中粘字符串

extern int check_abc_d(char *ff); //检查字符串中是否含字母 

extern int if_subs(char *zstring,char *subs); //判断一个是否是另一个的子串，是就返回2，不是返回0

extern int read_ntp(char * name, ST_SYS_ALL *sysall);  //读ntp的 xml文件

extern int write_ntp( ST_SYS_ALL *sysall, char *file_path); //读ntp的conf文件

extern int restart_ntp(); //ntp重启服务

extern int start_ntp(); //启动ntp服务

extern int stop_ntp();  //停止ntp服务

extern int add_ntp_node(char *fpath,char * node_name,char * value,char *content); //添加一个相应的节点

extern int add_ntp_node_new(char *fpath,char * node_name,char * value,char *content,char *attribute);

extern int ntp_del(char *fpath,char *node_name,char *attribute,char *key);  //删除某个属性的节点

extern int find_ntp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey);

extern int mod_ntp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc);

extern int  cut_up_ip(char *dstring,char *ip);

extern char *replace_ip(char *strbuf, char *sstr, char *dstr);

extern int  cut_up_port(char *dstring,char *port); //获取端口

extern char *first_port(char *dstring);

extern int get_cli_syslog_state();

extern inline void reset_sigmask();

/*xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，newc，更改的新内容*/
extern int mod_second_xmlnode(char * fpath,char * node_name,char * content,char *newc,int flagz);

/*查找指定节点，fpath，xml文件路径，nodename，二级xml节点名，content，三级xml节点名，keyz，三级xml节点名下内容，flagz，标记xml*/
extern int find_second_xmlnode(char * fpath,char * node_name,char * content,char *keyz,int *flagz);

/*xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，newc，更改的新内容*/
extern int get_second_xmlnode(char * fpath,char * node_name,char * content,char *gets,int flagz);

/*添加二级节点并内容fpath:xml file path,node_name:second level node,content:sub second node name,keyz:node content*/
extern int add_second_xmlnode(char * fpath,char * node_name,ST_SYS_ALL *sysall);

extern void Free_read_dest_xml(struct dest_st *head);
extern int read_dest_xml(struct dest_st *chead,int *confnum);

extern void Free_read_log_xml(struct log_st *head);
extern int read_log_xml(struct log_st *chead,int *confnum);

extern void Free_read_filter_xml(struct filter_st * head);
extern int read_filter_xml(struct filter_st * chead, int * confnum);

extern void Free_read_opt_xml(struct opt_st * head);
extern int read_opt_xml(struct opt_st * chead, int * confnum);

extern void Free_read_source_xml(struct source_st * head);
extern int read_source_xml(struct source_st * chead, int * confnum);

extern int read_syslogall_st(char * xmlpath,SYSLOGALL_ST *sysall);
extern void Free_read_syslogall_st(SYSLOGALL_ST *sysall);
extern int write_config_syslogallst( SYSLOGALL_ST *sysall, char *file_path) ;

extern int del_second_xmlnode(char * fpath,char * node_name,int flagz);
extern int add_syslog_server( char *xml_fpath, int enable, char *ip, int port, int filter ,char *utcp,int indexz);
extern int mod_syslog_server( char *xml_fpath, char *timeflag, int enable, char *ipaddr, int port, int filter );
extern int del_syslog_server( char *xml_fpath, int idindex );
extern void save_syslog_file();
extern void if_syslog_exist();
extern int if_syslog_enable();
extern int add_syslog_serve_web( char *xml_fpath, char *enable, char *ip,char *port, char *filter ,char *utcp,char *flevel,char *indexz);
extern int if_dup_info(char *sysudp,char *sysip,char *port,char *flevel,char *timeflag);/*返回0表示不重复，返回1表示重复*/
extern int mod_first_xmlnode(char *xmlpath,char *node_name,char *newc);
extern int get_first_xmlnode(char *xmlpath,char *node_name,char *newc);
extern void if_ntp_exist();
extern int if_ntp_enable();/*add by kehao,01-10-2011*/
extern int add_ntp_server( char *xml_fpath, char *ipz, char *maskz);
extern int add_ntp_client( char *xml_fpath, char *ipz, char *perz);
extern void Free_read_ntp_server(struct serverz_st *head);
extern int read_ntp_server(char *xmlpath,struct serverz_st *chead,int *confnum);
extern void Free_read_ntp_client(struct clientz_st *head);
extern int read_ntp_client(char *xmlpath,struct clientz_st *chead,int *confnum);
extern int count_xml_node(char *xmlpath,int *nodenum);
extern int write_ntp_conf( NTPALL_ST *ntpall, char *file_path);
extern void save_ntp_conf();
extern int count_secondxml_node(char *xmlpath,char *node_name,int *nodenum);
extern int check_ip_format_func(char *str);
extern int mod_log_enables(char * fpath,char * node_name,char * content,char *newc);
extern void if_filter_flood();
extern int add_ntp_server_slotid( char *xml_fpath, char *ipz, char *maskz,int slotid);
extern int add_ntp_client_slotid( char *xml_fpath, char *ipz, char *perz,int slotid);
extern int set_default_inside_client_func(int p_masterid);
extern int ntp_serverip_duplication(char *ntp_serverip);
extern int ntp_clientip_duplication(char *ntp_clientip);
extern int add_upper_ntp( char *xml_fpath, char *ipz, char *perz,char* slotid);
extern int read_upper_ntp(char *xmlpath,struct clientz_st *chead,int *confnum);
extern void Free_read_upper_ntp(struct clientz_st *head);
extern int ntp_uppertip_duplication(char *ntp_clientip);
extern int set_master_default_server_func();
extern int ip_address_check(char *str);
extern int submask_address_check(char *str);
extern int ntp_upperserverip_duplication(char *ntp_serverip);
extern int add_ntp_server_default( char *xml_fpath, char *ipz, char *maskz);
extern int add_ntp_client_default( char *xml_fpath, char *ipz, char *perz);
extern void if_filter_eaglog();
extern int del_syslog_serve_web( char *xml_fpath, char *ip,char *port, char *filter ,char *utcp,char *flevel);
extern int del_log_node( char *xml_fpath, char *ip,char *port, char *filter ,char *utcp);
extern int del_des_node( char *xml_fpath, char *ip,char *port,char *utcp,char *flevel);
extern void if_filter_portaldebug();
extern void if_destination_portaldebug();
extern void if_log_portaldebug();
extern void add_log_portaldebug();
extern void if_filter_clilog();
extern void if_log_clilog();
extern void if_destination_clilog();
extern int add_filter(char *rulename,char *content);
extern int del_clilog_node( char *xml_fpath, char *ip,char *port, char *filter ,char *utcp,char *flevel);
extern int del_syslogruler(char *xml_fpath,char *node_name,char *subnode,char *rulename);
extern int add_destination_rule(char *rulename,char *ip ,char *port);
extern int add_log_rule(char *filter,char *desname);
#endif
