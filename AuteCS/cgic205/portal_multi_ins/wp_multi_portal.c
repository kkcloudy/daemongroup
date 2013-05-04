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

#include "ws_eag_conf.h"


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

	char *tmp=(char *)malloc(64);
	int ret;
	if(access(MULTI_PORTAL_F,0)!=0)
	{
		create_eag_xml(MULTI_PORTAL_F);
		write_status_file( MULTI_PORTAL_STATUS, "start" );
	}
	else
	{
	  ret=if_xml_file_z(MULTI_PORTAL_F);
	  if(ret!=0)
	  {
		   memset(tmp,0,64);
		   sprintf(tmp,"sudo rm  %s > /dev/null",MULTI_PORTAL_F);
		   system(tmp);
		   create_eag_xml(MULTI_PORTAL_F);
		   write_status_file( MULTI_PORTAL_STATUS, "start" );

	  }
	}
    free(tmp);

	
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
	char nodez[32];
	memset(nodez,0,32);
	char attz[32];
	memset(attz,0,32);
	struct st_portalz c_head,*cq;
	int cnum,cflag=-1;
	
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
		cgiFormStringNoNewlines( "PLOTIDZ", index, 32 );
		cgiFormStringNoNewlines( "NODEZ", nodez, 32 );
		cgiFormStringNoNewlines( "ATTZ", attz, 32 );
		cnum = -1;
		
		read_portal_xml(MULTI_PORTAL_F, &c_head, &cnum, nodez);
					
		if( /* 1 == cnum ||*/strcmp(attz,MTD_N)!=0)/*add this to allow delet default when it is last*/
		{
			delete_eag_onelevel(MULTI_PORTAL_F, nodez, ATT_Z, attz);
		}
		else
		{		    
			ShowAlert("默认的不能删除");
		}
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_portal.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry ,index);
		fprintf( fp, "</script>\n" );

	}   
	return 0;
}

static int s_multiPortal_content_of_page( STPageInfo *pstPageInfo )
{
	
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	int i, cl = 0;

	char *urlnode=(char *)malloc(20);
	memset(urlnode,0,20);
	cgiFormStringNoNewlines( "plotid", urlnode, 20 );

	char addpid[10];
	memset(addpid,0,10);
	char plotidz[10];
    memset(plotidz,0,10);

	char menu[21]="";
	char i_char[10]="";

	/////////////读取数据/////////////////////////
	char buf[1024];
	int por_num = 0;
	char a1[256],a2[256],a3[256],a4[256],a5[256];//add domain value --by wk
	
	FILE * fd = NULL ;
	

    if(strcmp(urlnode,"")==0)
		strcpy(addpid,PLOTID_ONE);
	else
		strcpy(addpid,urlnode);
	
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
					"<tr>"\
						"<td><a id=link href=wp_add_multi_portal.cgi?UN=%s&plotid=%s>%s</a></td>", pstPageInfo->encry,addpid,search(portal_auth,"add_multi_portal") );
	fprintf(fp, "</tr>");
	fprintf(fp,"<tr height=7><td></td></tr>");
	fprintf(fp,"<tr>\n");
	fprintf(fp,"<td>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td><select name=port_id onchange=plotid_change(this)>");
	if(strcmp(urlnode,"")==0)
	{
		fprintf(fp,"<option value='%s'>1</option>",PLOTID_ONE);
		fprintf(fp,"<option value='%s'>2</option>",PLOTID_TWO);
		fprintf(fp,"<option value='%s'>3</option>",PLOTID_THREE);
		fprintf(fp,"<option value='%s'>4</option>",PLOTID_FOUR);
		fprintf(fp,"<option value='%s'>5</option>",PLOTID_FIVE);
	}
	else
	{

		if(strcmp(urlnode,PLOTID_ONE)==0)
			fprintf(fp,"<option value='%s' selected>1</option>",PLOTID_ONE);
		else
			fprintf(fp,"<option value='%s'>1</option>",PLOTID_ONE);

		if(strcmp(urlnode,PLOTID_TWO)==0)
			fprintf(fp,"<option value='%s' selected>2</option>",PLOTID_TWO);
		else
			fprintf(fp,"<option value='%s'>2</option>",PLOTID_TWO);

		if(strcmp(urlnode,PLOTID_THREE)==0)
			fprintf(fp,"<option value='%s' selected>3</option>",PLOTID_THREE);
		else
			fprintf(fp,"<option value='%s'>3</option>",PLOTID_THREE);

		if(strcmp(urlnode,PLOTID_FOUR)==0)
			fprintf(fp,"<option value='%s' selected>4</option>",PLOTID_FOUR);
		else
			fprintf(fp,"<option value='%s'>4</option>",PLOTID_FOUR);

		if(strcmp(urlnode,PLOTID_FIVE)==0)
			fprintf(fp,"<option value='%s' selected>5</option>",PLOTID_FIVE);
		else
			fprintf(fp,"<option value='%s'>5</option>",PLOTID_FIVE);
	}
	fprintf(fp,"</select></td>");
	fprintf(fp, "</tr>");
	fprintf(fp,"<tr height=7><td></td></tr>");
	fprintf(fp,"</table>");

	fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_multi_portal.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );

	fprintf(fp,	"<table border=0 width=493 cellspacing=0 cellpadding=0>");


    struct st_portalz c_head,*cq,*c1q;
	int cnum,cflag=-1;
	char *tempz=(char *)malloc(20);
	memset(tempz,0,20);
	if(strcmp(urlnode,"")!=0)
	{
		memset(tempz,0,20);
		sprintf(tempz,"%s%s",MTP_N,urlnode);
	}
	else
	{
		memset(tempz,0,20);
		sprintf(tempz,"%s%s",MTP_N,PLOTID_ONE);
	}
	cflag=read_portal_xml(MULTI_PORTAL_F, &c_head, &cnum, tempz);
	
	int locate = 0;
    memset(buf, 0, sizeof(buf));

	fprintf(fp,	"<tr height=30 align=left bgcolor=#eaeff9>");
		
	if(cflag==0)
	{
		c1q=c_head.next;
		while(c1q !=NULL)
		{
			if (0 == strcmp(c1q->p_keyword,MTD_N))
			{
				if ('\0' != c1q->p_type[0])
				{
					fprintf(fp,	"<th width=100>%s</th>",c1q->p_type);
				}
				else
				{
					fprintf(fp,	"<th width=100>KeyWord</th>");
				}					
				break;
			}
			c1q=c1q->next;
		}		
	}
	
	if (0 != cflag || NULL == c1q)
	{
		fprintf(fp,	"<th width=100>KeyWord</th>");
	}
	
	fprintf(fp,	"<th width=350>URL</th>"\
			"<th width=150>Domain</th>"\
			"<th width=13>&nbsp;</th>"\
			"</tr>");

	
    if(cflag==0)
	{
		cq=c_head.next;
		while(cq !=NULL)
		{
            locate ++;

			memset(menu,0,21);
			strcpy(menu,"menulist");

			char temp[5];
			memset(&temp,0,5);
			sprintf(temp,"%d",locate);
			strcat(menu,temp);


			memset(plotidz,0,10);
			if(strcmp(urlnode,"")==0)
			strcpy(plotidz,PLOTID_ONE);
			else
			strcpy(plotidz,urlnode);

			fprintf(stderr,"zym keyword is : %s\n",cq->p_keyword);

			fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
			fprintf(fp,	"<td>%s</td>", cq->p_keyword);
			fprintf(fp,	"<td>%s</td>", cq->pip);
			fprintf(fp,	"<td>%s</td>", cq->pdomain);
			//fprintf(fp,	"<td>%s</td>", );
			fprintf(fp, "<td>");
			fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(512-locate),menu,menu);
			fprintf(fp, "<img src=/images/detail.gif>"\
			"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
			fprintf(fp, "<div id=div1>");
			fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_multi_portal.cgi?UN=%s&DELRULE=%s&PLOTIDZ=%s&NODEZ=%s&ATTZ=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" , plotidz,tempz,cq->p_keyword,search(portal_public,"delete"));
			fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_edit_multi_portal.cgi?UN=%s&EDITCONF=%s&PLOTIDZ=%s&NODEZ=%s&ATTZ=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , "edit" ,plotidz,tempz,cq->p_keyword,search(portal_public,"configure"));
			fprintf(fp, "</div>"\
			"</div>"\
			"</div>");
			fprintf(fp,	"</td>");
			fprintf(fp,	"</tr>");

			cq=cq->next;
			cl = !cl;
			por_num++ ;

		}			
	}

	if((cflag==0 )&& (cnum > 0))
		Free_portal_info(&c_head);	
	
	free(tempz);
	free(urlnode);
	fprintf(fp,	"</table>");
	return 0;	
}


