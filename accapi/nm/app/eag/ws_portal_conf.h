#ifndef _WS_PORTAL_CONF_H
#define _WS_PORTAL_CONF_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*存储信息临时文件*/
#define PORTAL_XMLFZ        "/opt/services/conf/mtportal_conf.conf"
/*节点定义*/
#define NODE_PORTAL_SECD    "psecd"
#define NODE_PORTAL_SSID    "pssid"
#define NODE_PORTAL_IP      "pip"
#define NODE_PORTAL_PORT    "pport"
#define NODE_PORTAL_WEB	    "pweb"
#define NODE_PORTAL_DOMAIN  "pdom"
#define NODE_PORTAL_URL     "purl"

#define NODE_WTP_INDEXZ     "windex"
#define NODE_WTP_PSKZ       "wpsk"
#define NODE_WTP_F          "wtpf"
#define NODE_WTP_XMLF       "/var/run/apache2/wtppsk.xml"

typedef struct {
	char valuez[20];	
	char contentz[256];	
}ST_PORTAL_INFO;

typedef struct {	
    int portal_num;	
	ST_PORTAL_INFO porinfo[10];		
}ST_PORTAL_ALL;

struct portal_st
{
	char *pssid;
	char *pip;
	char *pport;
	char *pweb;
	char *pdom;
	char *purl;
	int pnum;
	struct portal_st *next;
};

struct wtpf_st
{
	char *windex;
	char *wpsk;
	int wnum;
	struct wtpf_st *next;
};

extern void Free_read_portal_xml(struct portal_st *head);
extern int read_portal_xmlz(char *xml_fpath,struct portal_st *chead,int *confnum);
extern int get_second_xmlnode_portal(char * fpath,char * node_name,char * content,char *gets,int flagz);
extern int mod_second_xmlnode_portal(char * fpath,char * node_name,char * content,char *newc,int flagz);
extern int find_second_xmlnode_portal(char * fpath,char * node_name,char * content,char *keyz,int *flagz);
extern int add_second_xmlnode_portal(char * fpath,char * node_name,ST_PORTAL_ALL *sysall);
extern int del_second_xmlnode_portal(char * fpath,char * node_name,int flagz);
extern int add_portal_server( char *xml_fpath, char *ssid, char *ip, int port, char *webz,char *domainz);
extern void if_portal_exist();
extern int add_portal_server_url( char *xml_fpath, char *ssid, char *purl);

/*wtp wapi value*/
extern void if_wtpxml_exist(char *xmlpath);
extern int add_wtp_psk( char *xml_fpath, int index, char *psk);

 #endif

