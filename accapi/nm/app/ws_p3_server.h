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

#ifndef _WS_P3_SERVER_H
#define _WS_P3_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*file paths*/
#define	P3_XML_FPATH	    "/opt/services/option/pppoe_option"

#define P3_CONF_FPATH		"/opt/services/conf/pppoe.conf"
#define P3_STATUS_FPATH     "/opt/services/status/pppoe_status.status"
#define P3_INIT             "sudo /opt/services/init/pppoe_init"
#define P3_CONFXMLT   		"/opt/www/htdocs/pppoe.xml"

/*需要操作到的配置文件和命令*/
#define P3_SERVER   "/etc/ppp/radius/servers"
#define P3_RCONF    "/etc/radiusclient/radiusclient.conf" 
#define P3_SERSH    "/etc/init.d/pppoe-server.sh"
#define P3_OPTION   "/etc/ppp/options"
#define P3_SUDO     "sudo /etc/init.d/pppoe-server.sh"
#define P3_HP       "/etc/hosts"

/*节点定义*/
#define P3_ROOT		"root"
#define P3_STATUS 	"lstatus"

/*radius*/
#define P3_RADIUS   "radius"
#define P3_SERIP    "serip"
#define P3_PWD      "serdef"  //modify
#define P3_AUTH     "auth"
#define P3_AUTH_PORT   "auport"
#define P3_ACCT     "acct"
#define P3_ACCT_PORT   "acport"

#define P3_DEF      "def"
#define P3_LOGIN    "log"
#define P3_DIC      "dic" /*文件名后不能加上空格*/
#define P3_SER      "ser" /*文件名后不能加上空格*/

/*hosts*/
#define P3_HOST     "host"
#define P3_HOSTIP   "hostip"
#define P3_HOSTNAME "hostname"
#define P3_HOSTDEF  "hostdef"

/*interface*/
#define P3_INF      "interface"
#define P3_MAX      "max"
#define P3_BASE     "base"
#define P3_MYIP     "myip"
#define P3_PORT     "p3if"  //modify

/*dns*/
#define P3_DNS      "dns"
#define P3_HIP      "hostip"
#define P3_BIP      "backip"
#define P3_DEFAULT  "defu"
#define P3_MASK     "mask"
#define P3_LCP      "lcp"
#define P3_LFILE    "logfile"
#define P3_PLUG     "plugin"

//定义结构体，三个结构体，最后定义一个总的结构体来存储这三个结构体的内容

typedef struct {   

	char serip[20];	
	char passwd[20];

	char auth[20];
	char auport[20];
	char acct[20];
	char acport[20];
	
	char def[256];
	
	char log[256];
	char dict[100];
	char ser[100];
	
}ST_P3_RADIUS;  //radius info

typedef struct {   

	char max[20];
	char base[20];
	char myip[20];
	char port[30];
	
}ST_P3_INF; //interface info

typedef struct {   

	char hostdef[60];
	char hostip[30];
	char hostname[30];
		
}ST_P3_HOST; //host

typedef struct {   

	char hostip[20];
	char backip[20];
	char defu[128];
	char mask[128];
	char lcp[128];
	char logfile[20];
	char plugin[40];
	
}ST_P3_DNS;  //dns info

//总的结构体信息  
typedef struct {

    int rnum;
	ST_P3_RADIUS s_radius;
	
    int ifnum;
	ST_P3_INF s_inf;

	int dnum;
	ST_P3_DNS s_dns;

	int hnum;
	ST_P3_HOST s_host;
	
}ST_P3_ALL;



extern int read_p3_xml(char * name, ST_P3_ALL *sysall);

extern int write_p3_conf( ST_P3_ALL *sysall);

extern int add_p3_node(char *fpath,char * node_name,char * value,char *content);

extern int add_p3_node_attr(char *fpath,char * node_name,char * value,char *content,char *attribute);

extern int p3_del(char *fpath,char *node_name,char *attribute,char *key);

extern int find_p3_node(char * fpath,char * node_name,char * content,char *logkey);

extern int mod_p3_node(char * fpath,char * node_name,char * content,char *newc);

extern int ser_p3_node();

extern void file_p3_exsit();
#endif

