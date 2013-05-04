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
* wp_nasid_byvlan.c
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

#define SUBMIT_NAME "submit_nasid_by_vlan"

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
static int s_nasidbyvlan_prefix_of_page( STPageInfo *pstPageInfo );
static int s_nasidbyvlan_content_of_page( STPageInfo *pstPageInfo );





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


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 4 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_nasidbyvlan_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_nasidbyvlan_content_of_page, &stPageInfo );

	
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


static int s_nasidbyvlan_prefix_of_page( STPageInfo *pstPageInfo )
{
	char del_rule[10] = "";
	char index[32] = "";
	FILE * fp = pstPageInfo->fp;
	fprintf(fp, "<style type=text/css>"\
	 	 		"#div1{ width:58px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  			"#div2{ width:56px; height:15px; padding-left:3px; padding-top:3px}"\
	  			"#link{ text-decoration:none; font-size: 12px}</style>" );

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
		fprintf( fp, "window.location.href='wp_authentication.cgi?UN=%s';\n", pstPageInfo->encry );
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
		
		char a1[33],a2[33],a3[33],a4[33];
		memset( a1, 0, 33 );
		memset( a2, 0, 33 );
		memset( a3, 0, 33 );
		memset( a4, 0, 33 );
		int del_loc = 0;
		char * revinfo[NASID_MAX_NUM];
		char buf[128];
		if( (fp = fopen(NASID_CONF_PATH,"r+")) != NULL )
		{
			fprintf(stderr,"fp=%x",fp);
			//读取操作
			while( fgets(buf, 128, fp) )
			{
				del_loc ++;
				revinfo[revnum] = (char *)malloc(128);
				memset( revinfo[revnum], 0, 128 );
				strncpy( revinfo[revnum], buf, strlen(buf) );
				//fprintf( stderr, "buf=%s",buf );
				sscanf( revinfo[revnum], "%15[a-zA-Z]=%20[0-9a-zA-Z.-]=%20[0-9a-zA-Z.-]=%32[a-z0-9.]",a1,a2,a3,a4 );
				fprintf( stderr, "a1=%s--del_loc=%d--i_index=%d--a=%s-a2=%s-a3=%s-a4=%s",a1, del_loc, i_index, a1, a2, a3, a4 );
				if( del_loc == i_index )
				{
					location = revnum;
				}

				memset( buf, 0, 128 );
				revnum ++ ;

			}
			fclose(fp);
			fp = fopen( NASID_CONF_PATH, "w+" );
			////写回操作
			for( i=0; i<revnum; i++ )
			{
				if( i == location )
					continue;
				
				fwrite( revinfo[i], strlen(revinfo[i]), 1, fp );

			}
			fclose(fp);
			//ShowAlert(search());
			
		}
	}
	return 0;		
}

static int s_nasidbyvlan_content_of_page( STPageInfo *pstPageInfo )
{
	
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	int i, cl = 0;

	char menu[21]="";
	char i_char[10]="";

	/////////////读取数据/////////////////////////
	char buf[256];
	int nas_num = 0;
	NAS_VLAN_ID nas_info;
	
	FILE * fd = NULL ;
	
#if 1
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
				   "<tr>"\
				   "<td><a id=link href=wp_addnasid.cgi?UN=%s>%s</a></td>", pstPageInfo->encry,search(portal_auth,"add_nasid") );
	fprintf(fp, "</tr>"\
				  "</table>");
#endif


	fprintf(fp,	"<table border=0 width=573 cellspacing=0 cellpadding=0>"\
				"<tr height=30 align=left bgcolor=#eaeff9>"\
				"<th width=80>INDEX</th>"\
				"<th width=80>TYPE</th>"\
				"<th width=120>%s</th>", search(portal_auth,"start") );
	fprintf(fp,	"<th width=120>%s</th>", search(portal_auth,"end") );
	fprintf(fp,	"<th width=80>NAS</th>"\
				"<th width=80>%s</th>", search(portal_auth,"syntaxis") );
	fprintf(fp, "<th width=13>&nbsp;</th>"\
				"</tr>");
	
	if( NULL == (fd = fopen(NASID_CONF_PATH, "r")))
	{
		fprintf(fp,	"</table>");
		return 0;//无文件不显示
	}
	int locate = 0;
	while( (fgets( buf, 256, fd )) != NULL )
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
		memset( &(nas_info), 0, sizeof(nas_info) );
		
		sscanf(buf, "%15[a-zA-Z]=%20[0-9a-zA-Z.-]=%20[0-9a-zA-Z.-]=%32[a-z0-9.]=%d", nas_info.index, nas_info.start_vlan, nas_info.end_vlan, nas_info.nas, &nas_info.nas_port_id );
		fprintf(stderr,"buf=%s",buf);
		fprintf(stderr,"index=%s-%s-%s-%s",nas_info.index, nas_info.start_vlan, nas_info.end_vlan, nas_info.nas );

		memset(menu,0,21);
		strcpy(menu,"menulist");
		sprintf(i_char,"%d",nas_num+1);
		strcat(menu,i_char);
		
		fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
		fprintf(fp,	"<td>%d</td>", locate);
		fprintf(fp,	"<td>%s</td>", nas_info.index);
		fprintf(fp,	"<td>%s</td>", nas_info.start_vlan);
		fprintf(fp,	"<td>%s</td>", nas_info.end_vlan);
		fprintf(fp,	"<td>%s</td>", nas_info.nas);
		fprintf(fp,	"<td>%d</td>", nas_info.nas_port_id);
		fprintf(fp, "<td>");
       	fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(NASID_MAX_NUM-nas_num),menu,menu);
		fprintf(fp, "<img src=/images/detail.gif>"\
					"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
		fprintf(fp, "<div id=div1>");
		fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_nasid_byvlan.cgi?UN=%s&DELRULE=%s&INDEX=%d target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" , locate ,search(portal_public,"delete"));
		fprintf(fp, "</div>"\
		  			"</div>"\
		   			"</div>");
	   	fprintf(fp,	"</td>");
		fprintf(fp,	"</tr>");
		
		memset( buf, 0, 256);
		cl = !cl;
		nas_num ++ ;
		
		
	}

	fclose( fd );			
	fprintf(fp,	"</table>");
	return 0;	
}


