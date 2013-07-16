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
* wp_add_multi_radius.c
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"  

#include <netinet/in.h> 
#include "ws_eag_conf.h"
#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"
       
#ifndef ADD_MULTI_RADIUS
#define ADD_MULTI_RADIUS

#define SUBMIT_NAME 	"submit_add_multi_radius"

//#define MULTI_RADIUS_STATUS_FILE_PATH	"/opt/services/status/multi_radius_status.status"
#define MULTI_RADIUS_CONF_FILE_PATH	"/opt/services/conf/multiradius_conf.conf"

#endif

#define MAX_RADIUS_DOMAIN_LEN		64
#define MAX_RADIUS_KEY_LEN		128

int ip_input_is_legal(const char *str);
int port_input_is_legal(const char *str);

/***************************************************************
定义页面要用到的结构体
****************************************************************/


typedef struct{
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	
} STPageInfo;

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};

int eag_radius_policy_get_num(int policy_id);


/***************************************************************
申明回调函数
****************************************************************/
static int s_addMulitRadius_prefix_of_page( STPageInfo *pstPageInfo );
static int s_addMulitRadius_content_of_page( STPageInfo *pstPageInfo );


/***************************************************************
*USEAGE:	主函数
*Param:		
*Return:	
*			
*Auther:shao jun wu
*Date:2008-12-30 14:12:46
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	int ret = 0;
	STPageInfo stPageInfo;
	char url[256]="";

	DcliWInit();
	ccgi_dbus_init(); 
	
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret = init_portal_container(&(stPageInfo.pstPortalContainer));
	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	stPageInfo.lpublic=stPageInfo.pstPortalContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	//stPageInfo.username_encry=dcryption(stPageInfo.encry);
    if( WS_ERR_PORTAL_ILLEGAL_USER == ret )
    {
	    ShowErrorPage(search(stPageInfo.lpublic,"ill_user")); 	  /*用户非法*/
		release_portal_container(&(stPageInfo.pstPortalContainer));
		return 0;
	}
	//stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );
	stPageInfo.iUserGroup = stPageInfo.pstPortalContainer->iUserGroup;

	//stPageInfo.pstModuleContainer = MC_create_module_container();
	if( NULL == stPageInfo.pstPortalContainer )
	{
	    release_portal_container(&(stPageInfo.pstPortalContainer));
		return 0;
	}
	
	stPageInfo.fp = cgiOut;

	memset(plotid,0,sizeof(plotid));
	cgiFormStringNoNewlines("plotid", plotid, sizeof(plotid)); 
	
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);
	if (NULL == paraHead1) {
		return 0;
	}
	if(strcmp(plotid, "") == 0)
	{
		parameter.instance_id = paraHead1->parameter.instance_id;
		parameter.local_id = paraHead1->parameter.local_id;
		parameter.slot_id = paraHead1->parameter.slot_id;
		snprintf(plotid,sizeof(plotid)-1,"%d-%d-%d",parameter.slot_id, parameter.local_id, parameter.instance_id);
	}
	else
	{
		get_slotID_localID_instanceID(plotid, &parameter);
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, parameter.slot_id, DISTRIBUTFAG);
	fprintf(stderr, "----------------------------------------------plotid=%s\n", plotid);
//初始化完毕

	
//处理表单


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_RADIUS);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addMulitRadius_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addMulitRadius_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"add_multi_radius"),9);
	sprintf(url,"%s?UN=%s","wp_add_multi_radius.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,7);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_multi_radius.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_addMulitRadius_prefix_of_page( STPageInfo *pstPageInfo )
{
	FILE * pp = pstPageInfo->fp;
	struct list * radius_public = pstPageInfo->lpublic;
	struct list * radius_auth = pstPageInfo->lauth;

	
	char domain_name[256],radius_server_type[256],radius_server_ip[32],radius_server_port[32],radius_server_key[256],radius_server_portal[32],
			charging_server_ip[32],charging_server_port[32],charging_server_key[256],
			backup_radius_server_ip[32],backup_radius_server_port[32],backup_radius_server_key[256],backup_radius_server_portal[32],
			backup_charging_server_ip[32],backup_charging_server_port[32],backup_charging_server_key[256],swap_octets[256],strip_domain_name[256];
	char class_to_bandwidth[10] = {0};
	int ret = 0;
	unsigned long auth_ip=0;
	unsigned short auth_port=0;
	unsigned long acct_ip=0;
	unsigned short acct_port=0;
	
	unsigned long backup_auth_ip=0;
	unsigned short backup_auth_port=0;
	unsigned long backup_acct_ip=0;
	unsigned short backup_acct_port=0;
	struct in_addr inaddr;

	memset(domain_name, 0, sizeof(domain_name));		
	memset(radius_server_type, 0, sizeof(radius_server_type));		
	memset(radius_server_ip, 0, sizeof(radius_server_ip));
	memset(radius_server_port, 0, sizeof(radius_server_port));
	memset(radius_server_key, 0, sizeof(radius_server_key));
	memset(radius_server_portal, 0, sizeof(radius_server_portal));
	memset(charging_server_ip, 0, sizeof(charging_server_ip));
	memset(charging_server_port, 0, sizeof(charging_server_port));
	memset(charging_server_key, 0, sizeof(charging_server_key));
	memset(backup_radius_server_ip, 0, sizeof(backup_radius_server_ip));
	memset(backup_radius_server_port, 0, sizeof(backup_radius_server_port));
	memset(backup_radius_server_key, 0, sizeof(backup_radius_server_key));
	memset(backup_radius_server_portal, 0, sizeof(backup_radius_server_portal));
	memset(backup_charging_server_ip, 0, sizeof(backup_charging_server_ip));
	memset(backup_charging_server_port, 0, sizeof(backup_charging_server_port));
	memset(backup_charging_server_key, 0, sizeof(backup_charging_server_key));
	memset(swap_octets, 0, sizeof(swap_octets));
	memset(strip_domain_name, 0, sizeof(strip_domain_name));
	memset(class_to_bandwidth, 0, sizeof(class_to_bandwidth));

	if( (cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess) && (pstPageInfo->iUserGroup == 0) )
	{
		cgiFormStringNoNewlines("domain_name",domain_name, sizeof(domain_name));
		//cgiFormStringNoNewlines("radius_server_type",radius_server_type, sizeof(radius_server_type));
		cgiFormStringNoNewlines("radius_server_ip",radius_server_ip, sizeof(radius_server_ip));
		cgiFormStringNoNewlines("radius_server_port",radius_server_port, sizeof(radius_server_port));
		cgiFormStringNoNewlines("radius_server_key",radius_server_key, sizeof(radius_server_key));
		//cgiFormStringNoNewlines("radius_server_portal",radius_server_portal, sizeof(radius_server_portal));
		cgiFormStringNoNewlines("charging_server_ip",charging_server_ip, sizeof(charging_server_ip));
		cgiFormStringNoNewlines("charging_server_port",charging_server_port, sizeof(charging_server_port));
		cgiFormStringNoNewlines("charging_server_key",charging_server_key, sizeof(charging_server_key));
		cgiFormStringNoNewlines("backup_radius_server_ip",backup_radius_server_ip, sizeof(backup_radius_server_ip));
		cgiFormStringNoNewlines("backup_radius_server_port",backup_radius_server_port, sizeof(backup_radius_server_port));
		cgiFormStringNoNewlines("backup_radius_server_key",backup_radius_server_key, sizeof(backup_radius_server_key));
		//cgiFormStringNoNewlines("backup_radius_server_portal",backup_radius_server_portal, sizeof(backup_radius_server_portal));
		cgiFormStringNoNewlines("backup_charging_server_ip",backup_charging_server_ip, sizeof(backup_charging_server_ip));
		cgiFormStringNoNewlines("backup_charging_server_port",backup_charging_server_port, sizeof(backup_charging_server_port));
		cgiFormStringNoNewlines("backup_charging_server_key",backup_charging_server_key, sizeof(backup_charging_server_key));
	 	//cgiFormStringNoNewlines("swap_octets",swap_octets, sizeof(swap_octets));
	 	cgiFormStringNoNewlines("strip_domain_name",strip_domain_name, sizeof(strip_domain_name));
		cgiFormStringNoNewlines("class_to_bandwidth",class_to_bandwidth, sizeof(class_to_bandwidth));
	 	
		/////////////处理数据///////////////////
		if(!strcmp(domain_name,"") || strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1)
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (radius_server_ip != NULL && strcmp(radius_server_ip, "") != 0 && !ip_input_is_legal(radius_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (charging_server_ip != NULL && strcmp(charging_server_ip, "") != 0 && !ip_input_is_legal(charging_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_radius_server_ip != NULL && strcmp(backup_radius_server_ip, "") != 0 && !ip_input_is_legal(backup_radius_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_charging_server_ip != NULL && strcmp(backup_charging_server_ip, "") != 0 && !ip_input_is_legal(backup_charging_server_ip))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (radius_server_port != NULL && strcmp(radius_server_port, "") != 0 && !port_input_is_legal(radius_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (charging_server_port != NULL && strcmp(charging_server_port, "") != 0 && !port_input_is_legal(charging_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_radius_server_port != NULL && strcmp(backup_radius_server_port, "") != 0 && !port_input_is_legal(backup_radius_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (backup_charging_server_port != NULL && strcmp(backup_charging_server_port, "") != 0 && !port_input_is_legal(backup_charging_server_port))
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}
		if (strlen(radius_server_key) > MAX_RADIUS_KEY_LEN-1
			|| strlen(charging_server_key) > MAX_RADIUS_KEY_LEN-1
			|| strlen(backup_radius_server_key) > MAX_RADIUS_KEY_LEN-1
			|| strlen(backup_charging_server_key) > MAX_RADIUS_KEY_LEN-1)
		{
			ShowAlert( search(radius_public, "input_illegal"));
			return 0;
		}

		memset(&inaddr,0,sizeof(struct in_addr));
		inet_aton(radius_server_ip, &inaddr);
		auth_ip = ntohl(inaddr.s_addr);
		auth_port = atoi(radius_server_port);

		memset(&inaddr,0,sizeof(struct in_addr));
		inet_aton( charging_server_ip,&inaddr);
		acct_ip = ntohl(inaddr.s_addr);
		acct_port = atoi(charging_server_port);

		memset(&inaddr,0,sizeof(struct in_addr));
		inet_aton( backup_radius_server_ip,&inaddr);
		backup_auth_ip = ntohl(inaddr.s_addr);
		backup_auth_port = atoi(backup_radius_server_port);

		memset(&inaddr,0,sizeof(struct in_addr));
		inet_aton( backup_charging_server_ip,&inaddr);
		backup_acct_ip = ntohl(inaddr.s_addr);
		backup_acct_port = atoi(backup_charging_server_port);

		ret = eag_add_radius(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									domain_name,
									auth_ip,
									auth_port,
									radius_server_key,
									acct_ip,
									acct_port,
									charging_server_key,
									backup_auth_ip,
									backup_auth_port,
									backup_radius_server_key,
									backup_acct_ip,
									backup_acct_port,
									backup_charging_server_key );
		int remove_domain_switch = atoi(strip_domain_name);
		ret = eag_set_remove_domain_switch(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									domain_name,
									remove_domain_switch);	
					
		int class_bandwidth = atoi(class_to_bandwidth);
		ret = eag_set_class_to_bandwidth_switch(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									domain_name,
									class_bandwidth);

		fprintf( pp, "<script type='text/javascript'>\n" );
		fprintf( pp, "window.location.href='wp_multi_radius.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry, plotid);
		fprintf( pp, "</script>\n" );
		
	}
	return 0;		
}

static int s_addMulitRadius_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * radius_auth = pstPageInfo->lauth;
	struct list * radius_public = pstPageInfo->lpublic;
	
	fprintf(fp,"<table border=0 width=500 cellspacing=0 cellpadding=0>\n");
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"plot_idz"));
	fprintf(fp,"<td width=250>");
	fprintf(fp,"<select name=plot_id onchange=plotid_change(this)>\n");
	instance_parameter *pq = NULL;
	char temp[10] = { 0 };
	
	for (pq=paraHead1;(NULL != pq);pq=pq->next)
	{
		memset(temp,0,sizeof(temp));
		snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
		
		if (strcmp(plotid, temp) == 0)
			fprintf(cgiOut,"<option value='%s' selected>%s</option>\n",temp,temp);
		else	       
			fprintf(cgiOut,"<option value='%s'>%s</option>\n",temp,temp);
	}
	fprintf(fp,"</select>");
	fprintf(fp,"</td>"\
				"</tr>\n");
    fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_add_multi_radius.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );

	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"domain_name"));
	fprintf(fp,"<td width=250><input type=text name=domain_name size=15></td>");
	fprintf(fp,"</tr>\n");
	
	fprintf(fp,"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"radius_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=radius_server_ip size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"radius_server_port"));
	fprintf(fp,"<td width=250><input type=text name=radius_server_port size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"radius_server_key"));
	fprintf(fp,"<td width=250><input type=text name=radius_server_key size=15></td>"\
				"</tr>\n");

	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(radius_auth,"charging_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=charging_server_ip size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"charging_server_port"));
	fprintf(fp,"<td width=250><input type=text name=charging_server_port size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"charging_server_key"));
	fprintf(fp,"<td width=250><input type=text name=charging_server_key size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_radius_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=backup_radius_server_ip size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_radius_server_port"));
	fprintf(fp,"<td width=250><input type=text name=backup_radius_server_port size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_radius_server_key"));
	fprintf(fp,"<td width=250><input type=text name=backup_radius_server_key size=15></td>"\
				"</tr>\n");
	
	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(radius_auth,"backup_charging_server_ip"));
	fprintf(fp,"<td width=250><input type=text name=backup_charging_server_ip size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_charging_server_port"));
	fprintf(fp,"<td width=250><input type=text name=backup_charging_server_port size=15></td>"\
				"</tr>\n"\
				"<tr height=30 align=left>");	
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"backup_charging_server_key"));
	fprintf(fp,"<td width=250><input type=text name=backup_charging_server_key size=15></td>"\
				"</tr>\n");

	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"strip_domain_name"));
	fprintf(fp,"<td width=250><input type=checkbox name=strip_domain_name value=1 size=15></td>"\
				"</tr>\n");
				
	fprintf(fp,"<td width=250>%s</td>",search(radius_auth,"class_to_bandwidth"));
	fprintf(fp,"<td width=250><input type=checkbox name=class_to_bandwidth value=1 size=15></td>"\
				"</tr>\n");
	
	fprintf(fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",plotid);				
	fprintf(fp,	"</table>");
	fprintf(fp,"<input type=hidden name=UN value=\"%s\">",pstPageInfo->encry);				
	return 0;
}

int ip_input_is_legal(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return 0;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return 0;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return 0;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return 0;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return 0;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return 1;
		else
			return 0;
	}
	else
		return 0;		
}

int port_input_is_legal(const char *str)
{
	const char *p = NULL;
	int port;
	
	if (NULL == str || '\0' == str[0] || '0' == str[0])
		return 0;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;
		
	port = atoi(str);
	if (port < 1 || port > 65535)
		return 0;
	
	return 1;
}

int eag_radius_policy_get_num(int policy_id)
{
	char tempz[20] = "";
	int flag = -1;
	struct st_radiusz chead = {0};
	int num = 0;
	
	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTR_N, policy_id);
	memset(&chead, 0, sizeof(chead));

	if (access(MULTI_RADIUS_F, 0) != 0)
	{
		return num;
	}
	flag = read_radius_xml(MULTI_RADIUS_F, &chead, &num, tempz);
	if(0 == flag && num > 0)
	{
		Free_radius_info(&chead);
	}

	return num;
}

