#ifndef _WS_RADIUS_CONF_H
#define _WS_RADIUS_CONF_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*file paths*/

#define XML_RADIUS_PATH	   	           "/opt/services/option/radius_option"
#define XML_RADIUS_D       	           "/opt/www/htdocs/radiusd.xml"

#define CONF_RADIUS_D	                  "/opt/radius/raddb/radiusd.conf"
#define CONF_RADIUS_PATH     	           "/opt/services/conf/radius_conf.conf"

#define STATUS_RADIUS_PATH             "/opt/services/status/radius_status.status"
#define RAD_PATH                                 "/opt/services/init/radius_init"

#define XML_RADIUS_CLIENT_PATH      "/opt/services/option/radius_client_option"
#define XML_RADIUS_CLIENT_D            "/opt/www/htdocs/clients.xml"

#define CONF_RADIUS_CLIENT_D                "/opt/radius/raddb/clients.conf"         
#define CONF_RADIUS_CLIENT_PATH          "/opt/services/conf/clients_conf.conf"

#define  CONF_RADIUS_MYSQL_DIR             "/opt/services/conf"
#define  CONF_RADIUS_MYSQL_D                "/opt/radius/raddb/sql.conf"   
#define  CONF_RADIUS_MYSQL_PATH          "/opt/services/conf/sql_conf.conf"
///////////////////////////////////////////////////////////
#define  NODE_STATUS           "lstatus"
#define  NODE_LDAP               "ldap"
#define  NODE_SERVER           "server"
#define  NODE_IDEN                "identify"
#define  NODE_PASSWD          "password"
#define  NODE_BASEDN           "basedn"
#define  NODE_SECECT			"select"
//#define  NODE_FILTER            "filter"
#define  NODE_ACCESS_ATTR    "access_attr"
#define  NODE_CONNECT           "connect"
#define  NODE_TIMEOUT           "timeout"
#define  NODE_TIMELIMIT         "timelimit"
#define  NODE_NETTIME            "net_time"
#define  NODE_TLS                     "tls"
//#define  NODE_CONTENT         "content"
#define  NODE_CLIENT             "client"
#define  NODE_IP                      "ip"
#define  NODE_SECRET              "secret"
#define  NODE_NASTYPE            "nastype"
#define  NODE_ATTRIBUTE        "attribute"
#define EMPTY_CONTENT        ""
/*a struct to store info of the node "ldap" in xml file */
 struct  ldap_st
{
	char  *server;
	char  *identify;
	char  *password;
	char  *basedn;
	char   *filter_ldap;
	char   *access_attr;
	char   *connect ;
	char   *timeout;
	char   *timelimit;
	char    *net_time;
	char    *tls;
	char   * content;
	struct ldap_st *next;
};

struct  radius_client_st
{
	char *client_ip;
	char  *secret;
	char  *nastype;
	struct radius_client_st *next;
};
extern int start_radius(); //启动服务
extern int stop_radius();  //停止服务
extern int restart_radius(); //重启服务
//extern int is_file_exist( char *filtpath ); //判断文件存在否
extern void if_radius_exist();          
extern void if_radius_client_exist();
extern int   if_radius_enable();           
extern int   get_third_radius_xmlnode(char * fpath,char * node_name,char * content,char *gets);
extern int   find_third_radius_xmlnode(char * fpath,char * node_name,char * content,char *getc,int *flagz);
extern int   mod_first_radius_xmlnode(char *xmlpath,char *node_name,char *newc);
extern int   mod_third_radius_xmlnode(char * fpath,char * node_name,char * content,char *newc);
extern int   get_first_radius_xmlnode(char *xmlpath,char *node_name,char *newc);
extern  int  get_radius_second_node_attr(char *fpath,char * node_name,char *attribute,char *ruler);
extern int   add_radius_second_xmlnode(char * fpath,char * second_node_name,char * third_node_name,char *third_node_content);
extern int   add_radius_third_xmlnode(char * fpath,char * second_node_name,char * third_node_name,char *third_node_content);
extern int   add_second_node_attr(char *fpath,char * node_name,char *attribute,char *ruler);
extern  int  add_radius_client_third_node(char * fpath,char * node_name,char *attribute,char *ruler,char * third_node_name,char *third_node_content); 
extern int   read_module_ldap_xml(struct ldap_st *chead,int *ldapnum);
extern  void  free_read_ldap_xml(struct ldap_st *head,int *free_flag);
//extern  void  free_read_radius_client_xml(struct radius_client_st *head);
extern   void free_read_radius_client_xml(struct radius_client_st *head,int *free_flag);
extern  int   write_xml2config_radiusd(char *filepath,struct ldap_st *lst);
extern  int   delete_radius_client_second_xmlnode(char *fpath,char *node_name,char *attribute,char *ruler);
extern  int   delete_radius_client_third_xmlnodel(char *fpath,char *node_name,char *attribute,char *ruler,char *thirdnode);

#endif


