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
* wp_add_multi_portal.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION: 
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


#ifndef ADD_MULTI_PORTAL
#define ADD_MULTI_PORTAL

#define SUBMIT_NAME 	"submit_add_multi_portal"

#define MULTI_PORTAL_CONF_PATH "/opt/services/conf/multiportal_conf.conf"


#endif
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



/***************************************************************
申明回调函数
****************************************************************/
static int s_addMulitPortal_prefix_of_page( STPageInfo *pstPageInfo );
static int s_addMulitPortal_content_of_page( STPageInfo *pstPageInfo );


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
	STPageInfo stPageInfo;
	char url[256]="";
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	stPageInfo.username_encry=dcryption(stPageInfo.encry);
    if( NULL == stPageInfo.username_encry )
    {
	    ShowErrorPage(search(stPageInfo.lpublic,"ill_user")); 	  /*用户非法*/
		return 0;
	}
	stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );

	//stPageInfo.pstModuleContainer = MC_create_module_container();
	init_portal_container(&(stPageInfo.pstPortalContainer));
	if( NULL == stPageInfo.pstPortalContainer )
	{
		return 0;
	}
	stPageInfo.lpublic=stPageInfo.pstPortalContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	
	stPageInfo.fp = cgiOut;
//初始化完毕

	
//处理表单


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 5 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addMulitPortal_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addMulitPortal_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"add_multi_portal"),6);
	sprintf(url,"%s?UN=%s","wp_add_multi_portal.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,6);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_multi_portal.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_addMulitPortal_prefix_of_page( STPageInfo *pstPageInfo )
{
	//FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	
	char ssid[256], ipaddr[256], port[256], web_page[256],domain[256];

	
	//////////memset////////////////
//	memset( ssid, 0, sizeof(ssid));
//	memset( ipaddr, 0, sizeof(ipaddr));
//	memset( port, 0, sizeof(port));
//	memset( web_page, 0, sizeof(web_page));

	memset( ssid, '\0', sizeof(ssid));
	memset( ipaddr, '\0', sizeof(ipaddr));
	memset( port, '\0', sizeof(port));
	memset( web_page, '\0', sizeof(web_page));
	memset( domain, '\0', sizeof(domain));

	//web_page[0]='\0';
	

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{

		cgiFormStringNoNewlines("ssid",ssid, sizeof(ssid));
		cgiFormStringNoNewlines("ipaddr",ipaddr, sizeof(ipaddr));
		cgiFormStringNoNewlines("port",port, sizeof(port));
		cgiFormStringNoNewlines("web_page",web_page, sizeof(web_page));
		cgiFormStringNoNewlines("domain",domain, sizeof(domain));
	

		
		/////////////处理数据///////////////////
		FILE * fp = NULL;
		char content[1024];
		memset( content, 0, sizeof(content));
		sprintf( content, "%s=%s=%s=%s=%s\n",ssid,ipaddr,port,web_page,domain);

		//check length of SSID
		if(strlen(ssid) > 32)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_too_long") );
			return 0;
		}
		if(strlen(ssid) == 0)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_too_long") );
			return 0;
		}
		int i = 0;
		while( ssid[i] != '\0')
		{
			if( (ssid[i] >= '0' && ssid[i] <= '9') ||	(ssid[i] >= 'a' && ssid[i] <= 'z') ||(ssid[i] >= 'A' && ssid[i] <= 'Z')
												|| ssid[i] == '-' || ssid[i] == '_' )
			{
				i++;
			}else
			{
				ShowAlert( search(portal_auth, "multi_portal_name_error") );
				return 0;
			}
		}


		//check IP
		unsigned int ip0=0,ip1=0,ip2=0,ip3=0;	
		int iRet;
		iRet = sscanf( ipaddr,"%u.%u.%u.%u", &ip0, &ip1, &ip2, &ip3 );			
		if( 4 == iRet )
		{
			if( ip0 <= 0 || ip0 > 255 || ip1 < 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 || ip3 < 0 || ip3 > 255 )
			{
				ShowAlert( search(portal_auth, "ip_error") );
				return 0;
			}
		}else
		{
			ShowAlert( search(portal_auth, "ip_error") );
			return 0;
		}

		//check port
		/*
		char *port_temp;
		char *p;
		char port_str[10];
		int port_num;
		
		port_temp = strdup(port);
		//debug_printf( stderr, "is_legal_port  begin port = %s\n", port );
		p = strtok( port_temp, "," );
		while( NULL != p )
		{
			debug_printf( stderr, "is_legal_port  p = %s\n", p );
			strcpy( port_str, p );
			port_num = atoi(port_str);
			if( port_num <= 0 || port_num > 65535 )
			{
				free( port_temp );
				return 0;	
			}
			p = strtok( NULL, "," );
		}
		debug_printf( stderr, "is_legal_port  end port = %s\n", port );
		free( port_temp );
		return 1;
		*/



	
		//fprintf(stderr,"get submit,content=%s",content);
		//fprintf(stderr,"content=%s",content);
		if( NULL != (fp = fopen(MULTI_PORTAL_CONF_PATH,"a+")) )
			{
				fwrite(content, strlen(content), 1, fp);
				//fflush(fp);
				fclose(fp);
				ShowAlert( search(portal_auth, "add_multi_portal_suc") );
				//ShowAlert( search(portal_public, "input_illegal") );

			}
		
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_portal.cgi?UN=%s';\n", pstPageInfo->encry);
		fprintf( fp, "</script>\n" );
		
		//fprintf(stderr,"111");
	}
	return 0;		
}

static int s_addMulitPortal_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	//struct list * portal_public = pstPageInfo->lpublic;
	//struct list * portal_auth = pstPageInfo->lauth;

	fprintf(fp,	"<table border=0 width=280 cellspacing=0 cellpadding=0>"\
				"<tr height=30 align=left>"\
				"<td width=100>ESSID</td>"\
				"<td width=180><input type=text name=ssid size=15></td>"\
				"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>IP</td>"\
				"<td width=180><input type=text name=ipaddr size=15></td>"\
				"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>Port</td>"\
				"<td width=180><input type=text name=port size=15></td>"\
				"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>Web</td>"\
				"<td width=180><input type=text name=web_page size=15></td>"\
				"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>Domain</td>"\
				"<td width=180><input type=text name=domain size=15></td>"\
				"</tr>"\
				);
	fprintf(fp,	"</table>");
	
	return 0;
}




