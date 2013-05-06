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
* ws_user_limit.h
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
#ifndef _WS_USER_LIMIT_H
#define _WS_USER_LIMIT_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*存储信息临时文件*/
#define LIMIT_XML_PATH   "/opt/services/option/limit_option"
#define LIMIT_CONF_PATH  "/etc/security/limits.conf"
#define LIMIT_STAT_PATH  "/opt/services/status/limit_status.status"
#define LIMIT_INIT       "/opt/services/init/limit_init"

/*节点定义*/
#define XML_LIMIT_ROOT		"root"
#define LIMIT_NODE          "node"

#define LIMIT_NODE_ATT      "attribute"
#define LIMIT_NODE_NAME     "name"
#define LIMIT_NODE_STATE    "state"
#define LIMIT_NODE_RULER    "ruler"
#define LIMIT_NODE_DEVICE   "device"
#define LIMIT_NODE_NUMBER   "number"


//存储时用到的链表
typedef struct {   

	char state[50];	
	char name[50];
	char ruler[50];
	char device[50];
	char number[50];
	
}ST_LIMIT_INFO;

typedef struct {
	
    int l_num;	
	ST_LIMIT_INFO limit_info[10];	
	
}ST_LIMIT_ALL;


/*函数部分，可分为给上层用的，和内部调用的。*/

extern int write_limit_config( ST_LIMIT_ALL *sysall, char *file_path) ;

//读取全部的信息
extern int read_limit_xml(char * name, ST_LIMIT_ALL *sysall);

//指定属性节点
extern int mod_limit_node(char * fpath,char * node_name,char *attribute,char *ruler,ST_LIMIT_INFO  newc);

//指定属性节点
extern int find_limit_node(char * fpath,char * node_name,char *attribute,char *ruler, ST_LIMIT_INFO *logkey);

#endif

