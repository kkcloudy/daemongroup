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
* wp_dhcpsumary.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* @autelan.com
*
* DESCRIPTION: 
*
*
*******************************************************************************/
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
* ws_conf_engine.h
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

/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/accapi/nm/app/eag/ws_conf_engine.h,v $
*$Author: shaojunwu $
*$Date: 2010/10/26 03:33:54 $
*$Revision: 1.2 $
*$State: Exp $
*
*
*/

#ifndef _WS_CONF_ENGINE_H
#define _WS_CONF_ENGIEN_H

#include <stdio.h>
#include <sys/wait.h>

#define SCRIPT_FILE_PATH	"sudo /opt/services/init/eag_init"
#define EAG_STATUS_FILE		"/opt/services/status/eag_status.status"

#define MAX_CONF_NAME_LEN	32
#define MAX_CONF_VALUE_LEN	128
#define MAX_SHOW_KEY_LEN	32


#define RTN_ERR				-1
#define RTN_ERR_FILE_OPEN	(RTN_ERR-1)	

#define NASID_CONF_PATH "/opt/services/conf/nasidvlan_conf.conf"
#define NASID_MAX_NUM 	512
#define MAX_INDEX_LEN 	32

typedef struct {
	char index[MAX_INDEX_LEN];
	char start_vlan[MAX_INDEX_LEN];
	char end_vlan[MAX_INDEX_LEN];
	char nas[MAX_INDEX_LEN];
	int  nas_port_id;
}NAS_VLAN_ID;


typedef struct st_conf_item{
	char conf_name[MAX_CONF_NAME_LEN];
	char conf_value[MAX_CONF_VALUE_LEN];
	char show_key[MAX_SHOW_KEY_LEN];
	int  show_flag;//当这个值为1时才会显示出来。
	char *error;
	//这些回调函数都是由上层代码设置的。
	int  (*output_html_obj)( struct st_conf_item *this, void *param );//输出input或者其它输入对象的html代码
	int  (*get_value_user_input)( struct st_conf_item *this, void *param );//得到相应的用户输入，并进行错误检查,返回错误id
}t_conf_item;

//装载conf文件
int load_conf_file( char *file_path, t_conf_item pt_conf_item[], int max_num ,char *plot_id);

//保存conf文件
int save_conf_file( char *file_path, t_conf_item pt_conf_item[], int max_num );

//获得服务状态
int eag_services_status();
//开启服务
int eag_services_start();
//关闭服务
int eag_services_stop( );
//重启服务
int eag_services_restart();
//idle time 
int idle_tick_status();

#endif
