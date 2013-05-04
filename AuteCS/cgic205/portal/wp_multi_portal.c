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
* wp_multi_portal.c
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

#ifndef MULTI_PORTAL
#define MULTI_PORTAL

#define SUBMIT_NAME "submit_multi_portal"
#define MULTI_PORTAL_CONF_PATH "/opt/services/conf/multiportal_conf.conf"
#define MULTI_PORTAL_STATUS_PATH "/opt/services/status/multiportal_status.status"



//#define MAX_INDEX_LEN 32

#endif


//#define SUBMIT_NAME "submit_nasid_by_vlan"


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
static int s_multiPortal_prefix_of_page( STPageInfo *pstPageInfo );
static int s_multiPortal_content_of_page( STPageInfo *pstPageInfo );



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

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_multiPortal_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_multiPortal_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_multiPortal_prefix_of_page( STPageInfo *pstPageInfo )
{
	char buf[]="start\n";
	char del_rule[10] = "";
	char index[32] = "";
	FILE * fp = pstPageInfo->fp;
	fprintf(fp, "<style type=text/css>"\
	 	 		"#div1{ width:58px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  			"#div2{ width:56px; height:15px; padding-left:3px; padding-top:3px}"\
	  			"#link{ text-decoration:none; font-size: 12px}</style>" );
	FILE * fp1 =NULL ;
	if( (fp1 = fopen(MULTI_PORTAL_STATUS_PATH,"w+")) != NULL )
	{

		fwrite(buf,strlen(buf),1,fp1);
		fclose(fp1);
	}
	fprintf(fp,	"<script type=\"text/javascript\">"\
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
	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_portal.cgi?UN=%s';\n", pstPageInfo->encry );
		fprintf( fp, "</script>\n" );
	}
	cgiFormStringNoNewlines( "DELRULE", del_rule, 10 );
	if( !strcmp(del_rule, "delete") )
	{
		cgiFormStringNoNewlines( "INDEX", index, 32 );
		char * ptr ;
		int i_index = strtoul( index, &ptr, 0 );
		
		//fprintf( stderr, "index=%s",index );
		FILE * fp = NULL;
		int  i, revnum=0, location=-1;
		
		char a1[256],a2[256],a3[256],a4[256],a5[256];
		int temp=-1;
		int del_loc = 0;
		char * revinfo[128];
		char buf[1024];
		if( (fp = fopen(MULTI_PORTAL_CONF_PATH,"r+")) != NULL )
		{
			//fprintf(stderr,"fp=%x",fp);//输出到出错文档?
			//读取操作
			memset( buf, 0, sizeof(buf));
			while( fgets(buf, sizeof(buf), fp) )//
			{
				//memset( a1, '\0', sizeof(a1));
				//memset( a2, '\0', sizeof(a2));
				//memset( a3, '\0', sizeof(a3));
				//memset( a4, '\0', sizeof(a4));
				del_loc ++;
				revinfo[revnum] = (char *)malloc(sizeof(buf));
				memset( revinfo[revnum], 0, sizeof(buf) );
				strncpy( revinfo[revnum], buf, strlen(buf) );
				//fprintf( stderr, "buf=%s",buf );
				//sscanf( revinfo[revnum], "%[^=]=%[^=]=%[^=]=%[^\n]\n",a1,a2,a3,a4);
				//sscanf( revinfo[revnum], "%[^=]=%[^=]=%[^=]=%[^\n]\n",a1,a2,a3,a4);
				//fprintf( stderr, "%s--%s",a1,a2);
				if( del_loc == i_index )
				{
					location = revnum;
				}
				
				memset( buf, 0, sizeof(buf) );
				revnum ++ ;
			}
			fclose(fp);
			fp = fopen( MULTI_PORTAL_CONF_PATH, "w+" );
			////写回操作
			for( i=0; i<revnum; i++ )
			{
				if( i == location )
					continue;
				
				fwrite( revinfo[i], strlen(revinfo[i]), 1, fp );

				free(revinfo[i]);//add by wk

			}
			fclose(fp);
		}
	}
	return 0;
}

static int s_multiPortal_content_of_page( STPageInfo *pstPageInfo )
{
	
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	int i, cl = 0;

	char menu[21]="";
	char i_char[10]="";

	/////////////读取数据/////////////////////////
	char buf[1024];
	int por_num = 0;
	char a1[256],a2[256],a3[256],a4[256],a5[256];
	
	FILE * fd = NULL ;
	
#if 1
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
					"<tr>"\
						"<td><a id=link href=wp_add_multi_portal.cgi?UN=%s>%s</a></td>", pstPageInfo->encry,search(portal_auth,"add_multi_portal") );
	fprintf(fp, "</tr>"\
				"</table>");
#endif


	fprintf(fp,	"<table border=0 width=493 cellspacing=0 cellpadding=0>"\
				"<tr height=30 align=left bgcolor=#eaeff9>"\
				"<th width=80>ESSID</th>"\
				"<th width=80>IP</th>"\
				"<th width=80>Port</th>"\
				"<th width=80>Web</th>"\
				"<th width=80>Domain</th>"\
				"<th width=13>&nbsp;</th>"\
				"</tr>");
	//"</tr>"\
	//"<tr height=30 align=left"\
	//"<th width=80>2</th>"\

	if( NULL == (fd = fopen(MULTI_PORTAL_CONF_PATH, "r")))
	{
		fprintf(fp,	"</table>");
		return 0;//无文件不显示
	}
	int locate = 0;
	memset(buf, 0, sizeof(buf));
	while( (fgets( buf, sizeof(buf), fd )) != NULL )
	{
		//fprintf(stderr,"buf=%s",buf);
		locate ++;
		#if 1
		if( !strcmp(buf,"") )
		{
			fclose( fd );
			fprintf(fp,	"</table>");
			return 0;//无数据不显示
		}
		#endif
		memset( a1, 0, sizeof(a1));
		memset( a2, 0, sizeof(a2));
		memset( a3, 0, sizeof(a3));
		memset( a4, 0, sizeof(a4));
		memset( a5, 0, sizeof(a5));
		sscanf(buf, "%[^=]=%[^=]=%[^=]=%[^=]=%[^\n]\n",a1,a2,a3,a4,a5);

		//fprintf(stderr,"buf=%s",buf);
		//fprintf(stderr,"index=%s-%s",a1,a2);

		memset(menu,0,21);
		strcpy(menu,"menulist");
		//sprintf(i_char,"%d",nas_num+1);
		//strcat(menu,i_char);
		char temp[5];
		
		memset(&temp,0,5);
		sprintf(temp,"%d",locate);
		strcat(menu,temp);
		
		fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
		//fprintf(fp,	"<td>%d</td>", locate);
		fprintf(fp,	"<td>%s</td>", a1);
		fprintf(fp,	"<td>%s</td>", a2);
		fprintf(fp,	"<td>%s</td>", a3);
		fprintf(fp,	"<td>%s</td>", a4);
		fprintf(fp,	"<td>%s</td>", a5);
		//fprintf(fp,	"<td>%s</td>", );
		fprintf(fp, "<td>");
       	fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(512-locate),menu,menu);
		fprintf(fp, "<img src=/images/detail.gif>"\
					"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
		fprintf(fp, "<div id=div1>");
		fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_multi_portal.cgi?UN=%s&DELRULE=%s&INDEX=%d target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" , locate ,search(portal_public,"delete"));
		fprintf(fp, "</div>"\
		  			"</div>"\
		   			"</div>");
	   	fprintf(fp,	"</td>");
		fprintf(fp,	"</tr>");
		
		memset(buf, '\0', sizeof(buf));
		cl = !cl;
		por_num++ ;
		
		
	}
	fclose(fd);			
	fprintf(fp,	"</table>");
	return 0;	
}


