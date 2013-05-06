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
* ws_version_param.h
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
*******************************************************************************/
#ifndef _WS_VERSION_PARAM_H
#define _WS_VERSION_PARAM_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define V_XML     "/opt/www/htdocs/ApMonitor/version.xml"
#define ISS_XML   "/opt/www/htdocs/ApMonitor/issue.xml"

#define DOWN_LOC  "locexport"
#define DOWN_WEB  "webexport"
#define UP_TYPE_W "webupload"
#define UP_TYPE_L "locupload"
#define T_HTTP    "http"
#define T_IMG     "img"
#define T_CONF    "conf"
#define T_LOG     "log"
#define T_FTP     "ftp"

#define R_ST   "stype"
#define R_FT   "ftype"
#define R_PRO  "pro"
#define R_RP   "rport"
#define R_RUN  "run"
#define R_FN   "fname"
#define R_RI   "rip"
#define R_UN   "uname"
#define R_PS   "pword"
#define R_FC   "fail"

//版本升级参数
typedef struct {
   char  sendtype[20];               //传送文件类型
   char  filetype[20];              //文件类型 1，2，3
   char  protocal[20];              //使用协议，可写定
   char  routeport[20];             //远程服务器端口
   char  running[20];              //运行状态
   
   char filename[64];        //文件名字    
   char routeip[64];               //远程服务器地址，可是指定的网络资源地址   
   char username[64];              //远程服务器用户名
   char password[64];              //远程服务器密码
   char failcause[128];            //失败原因
}ST_VERSION_PARAM;


/*conf node define*/
#define CONF_XML_S "/mnt/conf_xml.conf"
#define CONF_VERSION  "version"
#define CONF_FILESIZE "filesize"
#define CONF_TEMP_S  "/var/run/apache2/conf_xml.conf"


/*add or delete conf node and find one conf node*/
extern int del_conf_node(char *fpath,char *node_name);

extern int add_conf_node(char *fpath,char * node_name,char * value);

extern int find_conf_node(char * fpath,char * node_name,char *outkey);


extern int get_version_param(ST_VERSION_PARAM *version, char *xml_path);  //获取版本升级参数

extern int set_version_param(ST_VERSION_PARAM version, char *xml_path);//设置版本升级参数

extern int set_one_node(char *xml_path,char *node,char *newc); //设置指定节点的值，其他的不变

extern int file_ftp(char *ip,char *username,char *passwd,char *dir,char *filename,int type);//上传或下载文件

extern int get_file_bytesum(char *fpath);

extern int add_mibs_node_attr_z(char *fpath,char * node_name,char *attribute,char *ruler);

extern int mod_mibs_node_z(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc);

extern int create_mibs_xml_z(char *xmlpath);

extern int get_mibs_node_z(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey);


#endif

