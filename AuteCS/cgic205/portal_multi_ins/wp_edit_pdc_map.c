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
#include "eag/pdc_interface.h"
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

static unsigned int
mask2binary(unsigned int mask){
	unsigned int mask_ret = 0xffffffff;
	if (0 == mask)
	 {
	    return 0;
	}
	mask_ret = (mask_ret << (32 - mask));
	return mask_ret;
}

static unsigned int
binary2mask(unsigned int mask){
	int i = 0;
	if (0xffffffff == mask){
	    return 32;
	}
	for (i = 0; i < 32; i++)
	{
		if ( 0 == mask << i){
			return i;
		}

	}
}

/******************************************************************
上面的都是页面调用是需要用到的api，下面才是和页面相关的东西
******************************************************************/
#define SUBMIT_NAME		"submit_set_pdc_conf"


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
static int s_pdc_prefix_of_page( STPageInfo *pstPageInfo );
static int s_pdc_content_of_page( STPageInfo *pstPageInfo );
static int wp_pdc_do_command( STPageInfo *pstPageInfo );


static int
wp_pdc_add_map(STPageInfo *pstPageInfo)
{
	int ret = 0;
	char usermap[32]={0};
	char eaghansi[32] = {0};
	char id[16] = {0};
	char tmpstr[32] = {0};
	uint32_t userip = 0;
	uint32_t usermask = 0;
	int eag_slotid = 0;
	int eag_insid = 0;
	char *pmap = usermap;

	cgiFormStringNoNewlines("usermap", usermap, sizeof(usermap));
	cgiFormStringNoNewlines("eaghansi", eaghansi, sizeof(eaghansi));

	debug_printf(stderr, "------------------------------usermap=%s, eaghansi=%s\n", usermap, eaghansi);

	if (strcmp(usermap, "") == 0 && strcmp(eaghansi, "") == 0) {
		return 0;
	}
	
	ret = ip_address_format2ulong((char**)&pmap,&userip,&usermask);
	if (ret != 0 || usermask > 32) {
		ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));				  
		return 0;
	}
	usermask = mask2binary(usermask);

	ret = sscanf(eaghansi, "%d-%d", &eag_slotid, &eag_insid);
	if (ret != 2 || eag_slotid > 10 || eag_insid > 16 
		|| eag_slotid < 0 || eag_insid < 0) {
		ShowAlert(search(pstPageInfo->lpublic, "input_illegal"));				  
		return 0;
	}

	ret = pdc_intf_add_map(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									userip, 
									usermask,
									eag_slotid,
									eag_insid);
	return ret;
}

static int
wp_pdc_del_map(STPageInfo *pstPageInfo)
{
	int ret = 0;
	char usermap[32]={0};
	char *pmap = usermap;
	uint32_t userip = 0;
	uint32_t usermask = 0;

	debug_printf(stderr, "---------------------------------wp_pdc_del_map\n");
	
	cgiFormStringNoNewlines("usermap",usermap,sizeof(usermap));
	debug_printf(stderr, "--------------------------------usermap=%s\n", usermap);
	ret = ip_address_format2ulong((char**)&pmap,&userip,&usermask);
	if (ret != 0 || usermask > 32) {
		ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));				  
		return 0;
	}
	
	debug_printf(stderr, "--------------------------------userip=%u, usermask=%d\n", userip, usermask);
	usermask = mask2binary(usermask);
	debug_printf(stderr, "--------------------------------userip=%u, usermask=%x\n", userip, usermask);

	ret = pdc_intf_del_map(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									userip, 
									usermask);
	return ret;
}

static int
wp_pdc_set_map(STPageInfo *pstPageInfo)
{
	int ret = 0;
	char usermap[32]={0};
	char eaghansi[32] = {0};
	char *pmap = usermap;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	int eag_slotid = 0;
	int eag_insid = 0;

	cgiFormStringNoNewlines("usermap", usermap, sizeof(usermap));
	cgiFormStringNoNewlines("eaghansi", eaghansi, sizeof(eaghansi));
	debug_printf(stderr, "------------------------------usermap=%s, eaghansi=%s\n", usermap, eaghansi);
	ret = ip_address_format2ulong((char**)&pmap,&userip,&usermask);
	if (ret != 0 || usermask > 32) {
		ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));				  
		return 0;
	}
	debug_printf(stderr, "--------------------------------userip=%u, usermask=%d\n", userip, usermask);
	usermask = mask2binary(usermask);
	debug_printf(stderr, "--------------------------------userip=%u, usermask=%x\n", userip, usermask);

	ret = sscanf(eaghansi, "%d-%d", &eag_slotid, &eag_insid);
	if (ret != 2 || eag_slotid > 10 || eag_insid > 16 
		|| eag_slotid < 0 || eag_insid < 0) {
		ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));				  
		return 0;
	}

	ret = pdc_intf_modify_map(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									userip, 
									usermask,
									eag_slotid,
									eag_insid);
	return ret;
}



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
	char url[256]="";

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
	debug_printf(stderr, "----------------------------------------------plotid=%s\n", plotid);
//初始化完毕

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_PDC);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_pdc_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_pdc_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"pdc_conf"),WP_EAG_PDC);
	sprintf(url,"%s?UN=%s","wp_pdc_conf.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,WP_EAG_PDC);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_pdc_conf.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );
	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	free_instance_parameter_list(&paraHead1);
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_pdc_prefix_of_page( STPageInfo *pstPageInfo )
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
		wp_pdc_do_command(pstPageInfo);

		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_pdc_conf.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
		fprintf( cgiOut, "</script>\n" );
	}

	return 0;		
}

static int s_pdc_content_of_page( STPageInfo *pstPageInfo )
{
	int ret = 0;
	struct pdc_map_conf map_conf;
	memset(&map_conf, 0, sizeof(map_conf));
	char usermap[32]={0};
	char eaghansi[32] = {0};
	char *pmap = usermap;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	int i = 0;
	if( NULL == pstPageInfo )
	{
		return -1;
	}
	

	cgiFormStringNoNewlines("usermap",usermap,sizeof(usermap));
	debug_printf(stderr, "--------------------------------usermap=%s\n", usermap);
	ret = ip_address_format2ulong((char**)&pmap,&userip,&usermask);
	if (ret != 0 || usermask > 32) {
		ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));				  
		return 0;
	}
	
	debug_printf(stderr, "--------------------------------userip=%u, usermask=%d\n", userip, usermask);
	usermask = mask2binary(usermask);
	debug_printf(stderr, "--------------------------------userip=%u, usermask=%x\n", userip, usermask);

	
	ret = pdc_intf_show_maps(ccgi_connection, parameter.local_id, parameter.instance_id, &map_conf);
	if (0 != ret) {
		goto return_line;
	}

	for (i = 0; i < map_conf.num; i++) {
		if (userip == map_conf.map[i].userip && usermask == map_conf.map[i].usermask) {
			break;
		}
	}
	if (i > map_conf.num) {
		goto return_line;
	}

	usermask = binary2mask(map_conf.map[i].usermask);
	snprintf(eaghansi, sizeof(eaghansi), "%u-%u", map_conf.map[i].eag_slotid, map_conf.map[i].eag_hansiid);

	fprintf(cgiOut,	"	<table>\n");
	fprintf(cgiOut,	"		<tr>\n"\
			"			<td colspan='3'>\n"\
			"				<table>\n"\
			"					<tr>\n"\
			"						<td>ID:</td>\n"\
			"						<td>%s</td>\n"\
			"					</tr>\n", plotid);
#if 0
	fprintf(cgiOut,	"					<tr height=30 align=left>\n");
	fprintf(cgiOut,	"					<td width=150>%s</td>\n", search(pstPageInfo->lauth, "add_pdcmap"));
	fprintf(cgiOut,	"					</tr>\n");
#endif	
	fprintf(cgiOut,	"					<tr height=30 align=left>\n");
	fprintf(cgiOut,	"					<td width=150>User Subnet:</td>\n");
	fprintf(cgiOut,	"					<td width=250>%s</td>\n", usermap);
	fprintf(cgiOut,	"					</tr>\n");
	
	fprintf(cgiOut,	"					<tr height=30 align=left>\n");
	fprintf(cgiOut,	"					<td width=150>EAG ID:</td>\n");
	fprintf(cgiOut,	"					<td width=200><input type=text name=eaghansi value=\"%s\" size=10><font color=red>(SLOTID-INSID)</td>\n", eaghansi);
	fprintf(cgiOut,	"					</tr>\n");
	
	fprintf(cgiOut,	"				</table>\n"\
			"			</td>\n");
	fprintf(cgiOut, "			<td width='50'>&nbsp;</td>\n");
	fprintf(cgiOut, "		</tr>\n");
	fprintf(cgiOut, "	</table>\n");
	
return_line:	
	fprintf(cgiOut,"<input type=hidden name=UN value=%s>", pstPageInfo->encry);
	fprintf(cgiOut,"<input type=hidden name=plotid value=%s>", plotid);
	fprintf(cgiOut,"<input type=hidden name=usermap value=%s>", usermap);
	return 0;
}

static int wp_pdc_do_command(STPageInfo *pstPageInfo)
{
	return wp_pdc_set_map(pstPageInfo);
}

