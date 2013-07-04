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
* wp_white_list.c
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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include <dbus/dbus.h>
#include <dbus/npd/npd_dbus_def.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include "ws_public.h"



#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"


#include "ws_user_manage.h"
#include "user_manage.h"

#include "ws_conf_engine.h"

#include "ws_init_dbus.h"
#include "ws_eag_auto_conf.h"
#include "ws_returncode.h"

#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/rdc_interface.h"
#include "ws_dcli_vrrp.h"
#include "ws_eag_conf.h"
#include "ws_dbus_list_interface.h"


#define _DEBUG	1

#if _DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif



//定义全局变量


static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};



//定义静态函数


/*convert the ip address to unsigned long int data*/
static unsigned long  ip2ulong(char *str)
{
	char *separate=".";
	char *token = NULL;
	unsigned long ip_long[4]; 
	unsigned long ip = 0;
	int i = 1;
	
	token=strtok(str,separate);
	if(NULL != token){
	    ip_long[0] = strtoul(token,NULL,10);
	}
	while((token!=NULL)&&(i<4))
	{
		token=strtok(NULL,separate);
		if(NULL != token){
		    ip_long[i] = strtoul(token,NULL,10);
		}
		i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	return ip;
}

static int rdc_ins_running_state(DBusConnection * conn, int hansitype, int insid)
{
	int ret = 0;
	struct rdc_base_conf baseconf;		
	memset(&baseconf, 0, sizeof(baseconf));
	
	ret = rdc_intf_get_base_conf(conn, hansitype, insid, &baseconf);
	if ((EAG_RETURN_OK == ret) && (1 == baseconf.status))
		ret = 1;
	else
		ret = 0;
	return ret;
}


/******************************************************************
上面的都是页面调用是需要用到的api，下面才是和页面相关的东西
******************************************************************/
#define SUBMIT_NAME		"submit_set_rdc_conf"


/***************************************************************
定义页面要用到的结构体
****************************************************************/
#define MAX_URL_LEN         256


typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;/*解析control.txt文件的链表头*/
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	
	int formProcessError;
} STPageInfo;

/***************************************************************
申明回调函数
****************************************************************/
static int s_rdc_prefix_of_page( STPageInfo *pstPageInfo );
static int s_rdc_content_of_page( STPageInfo *pstPageInfo );
static int wp_rdc_do_command( STPageInfo *pstPageInfo );


/***************************************************************
*USEAGE:	得到已经使用的id列表
*Param:		p_used_id -> 存放id列表的结构体
*Return:	0 -> success
*			!= 0 -> failure。
*Auther:shao jun wu
*Date:2008-12-30 14:12:46
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	int ret = 0;
	STPageInfo stPageInfo;
	char opt[10]={0};

	DcliWInit();
	ccgi_dbus_init();
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret = init_portal_container(&(stPageInfo.pstPortalContainer));
	stPageInfo.lpublic=stPageInfo.pstPortalContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	
	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
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

 	
	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_RDC);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_rdc_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_rdc_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ACTION, "wp_rdc_conf.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	release(stPageInfo.lauth);
	release(stPageInfo.lpublic);
	
	free_instance_parameter_list(&paraHead1);
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_rdc_prefix_of_page( STPageInfo *pstPageInfo )
{
	char *error_message=NULL;

	if( NULL == pstPageInfo )
	{
		return -1;
	}
 	
 	fprintf( cgiOut, "<style type=text/css>.a3{width:30px;border:0px; text-align:center}</style>" );
 	fprintf( cgiOut, "<script language=javascript src=/ip.js></script>\n" );
 	fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );
	fprintf( cgiOut, "<style type=text/css>"\
	 	 		"#div1{ width:58px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  			"#div2{ width:56px; height:15px; padding-left:3px; padding-top:3px}"\
	  			"#link{ text-decoration:none; font-size: 12px}</style>" );


	fprintf( cgiOut, "<script type=\"text/javascript\">"\
				"function popMenu(objId)"\
				"{"\
			   		"var obj = document.getElementById(objId);"\
			   		"if (obj.style.display == 'none')"\
			   		"{"\
				 		"obj.style.display = 'block';"\
			   		"}"\
			   		"else"\
			   		"{"\
				 		"obj.style.display = 'none';"\
			   		"}"\
		   		"}"\
		   	"</script>");


	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess)
	{
		wp_rdc_do_command(pstPageInfo);
	}
	return 0;		
}

static int s_rdc_content_of_page( STPageInfo *pstPageInfo )
{
	int i = 0;
	static instance_parameter *pq = NULL;
	char temp[10] = {0};
	char menu[21]="";
	int ret = 0;
	int cl = 0;
	
	if( NULL == pstPageInfo )
	{
		return -1;
	}
	struct rdc_base_conf baseconf;
	memset(&baseconf, 0, sizeof(baseconf));
	ret = rdc_intf_get_base_conf(ccgi_connection, parameter.local_id, parameter.instance_id, &baseconf);
	if (0 != ret) {
		goto return_line;
	}
		
	fprintf(cgiOut,	"	<table>\n");
	fprintf(cgiOut,	"		<tr>\n"\
			"			<td colspan='3'>\n"\
			"				<table>\n"\
			"					<tr>\n"\
			"						<td>ID:</td>\n"\
			"						<td>\n"\
			"						<select name='plotid' style='width:100%%;height:auto' onchange='on_id_change(this);'>\n");

	for (pq=paraHead1; (NULL != pq); pq=pq->next)
	{
		memset(temp,0,sizeof(temp));
		snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
		
		if (strcmp(plotid, temp) == 0)
			fprintf(cgiOut,"<option value='%s' selected>%s</option>\n",temp,temp);
		else	       
			fprintf(cgiOut,"<option value='%s'>%s</option>\n",temp,temp);
	}	
	fprintf(cgiOut,	"						</select>\n" );
	fprintf(cgiOut,	"			<script type=text/javascript>\n"\
			"			function on_id_change(obj)\n"
			"			{\n"\
			"				window.location.href='wp_rdc_conf.cgi?UN=%s&plotid='+obj.value;\n"\
			"			}\n",pstPageInfo->encry );
	fprintf(cgiOut, "			</script>\n" );
	fprintf(cgiOut, "						</td>\n"\
			"					</tr>\n");

	//status
	fprintf(cgiOut,	"					<tr>");
	fprintf(cgiOut,	"					<td width=150>%s</td>",search(pstPageInfo->lauth, "rdc_status"));
	fprintf(cgiOut,	"					<td><input type=checkbox name=rdc_status value=1 %s></td>",(1 == baseconf.status) ? "checked" : "");
	fprintf(cgiOut,	"					</tr>");

	//nas ip 
	char instr[32] = {0};
	ccgi_ip2str( baseconf.nasip, instr, sizeof(instr) - 1);
	fprintf(cgiOut,	"					<tr>");
	fprintf(cgiOut,	"					<td width=150>%s</td>","NAS IP");
	fprintf(cgiOut,	"					<td><input type=text name=nasip value=%s></td>",instr);
	fprintf(cgiOut,	"					</tr>");
		
	
	fprintf(cgiOut,	"				</table>\n"\
			"			</td>\n"\
			"		</tr>\n"\
			"	</table>\n");
	
return_line:	
	fprintf(cgiOut,"<input type=hidden name=UN value=%s>", pstPageInfo->encry);
	return 0;
}

static int wp_rdc_do_command(STPageInfo *pstPageInfo)
{
	char rdc_status[10] = {0};
	char nasip[32] = {0};

	int status = 0;
	uint32_t ip = 0;

	int ret = 0;
	int isrun = 0;
	
	cgiFormStringNoNewlines("rdc_status", rdc_status, sizeof(rdc_status));
	cgiFormStringNoNewlines("nasip", nasip, sizeof(nasip));

	ip = ip2ulong(nasip);
	status = atoi(rdc_status);
	
	isrun  = rdc_ins_running_state(ccgi_connection, parameter.local_id, parameter.instance_id);

	if (!isrun) {
		ret = rdc_intf_set_nasip(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									ip);
		if (0 != ret) {
			goto error;
		}

		ret = rdc_intf_set_status(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									status);
	} else if (0 == status) {
		ret = rdc_intf_set_status(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									status);
	}
		
error:
	if (EAG_ERR_RDC_SERVICE_ALREADY_ENABLE == ret) {
		ShowAlert("Service already enable");
	} else if (0 != ret) {
		ShowAlert(search(pstPageInfo->lpublic, "log_err"));
	}
	return ret;
}

