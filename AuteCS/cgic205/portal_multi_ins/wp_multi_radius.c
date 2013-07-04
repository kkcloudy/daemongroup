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
* wp_multi_radius.c
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
#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"


#ifndef MULTI_RADIUS
#define MULTI_RADIUS

#define SUBMIT_NAME "submit_multi_radius"

#define MULTI_RADIUS_STATUS_FILE_PATH	"/opt/services/status/multiradius_status.status"

#endif

typedef struct{
	STPortalContainer *pstradiusContainer;
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

#define MAX_RADIUS_DOMAIN_LEN 64

#define CHARACTER_NUM	8
char *escape_character[] = {"%", "&", "+", "#", " ", "/", "?", "="};
char *encoder_character[] = {"%25", "%26", "%2B", "%23", "%20", "%2F", "%3F", "%3D"};


/***************************************************************
申明回调函数
****************************************************************/
static int s_multiRadius_prefix_of_page( STPageInfo *pstPageInfo );
static int s_multiRadius_content_of_page( STPageInfo *pstPageInfo );

static char *replace(char *source,char *sub,char *rep);

static void url_encoder(char *source,char *result,char **escape_character,char **encoder_character,int character_num);


/***************************************************************
*USEAGE:	主函数
*Param:		
*Return:	
*			
*Auther:WangKe
*Date:2009-8-3
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	int ret1 = 0;
	STPageInfo stPageInfo;
	int ret;		

	DcliWInit();
	ccgi_dbus_init();
	
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret1 = init_portal_container(&(stPageInfo.pstradiusContainer));
	stPageInfo.lpublic = stPageInfo.pstradiusContainer->lpublic;
	stPageInfo.lauth=stPageInfo.pstradiusContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	//stPageInfo.username_encry=dcryption(stPageInfo.encry);
    if( WS_ERR_PORTAL_ILLEGAL_USER == ret1 )
    {
	    ShowErrorPage(search(stPageInfo.lpublic,"ill_user")); 	  /*用户非法*/
		release_portal_container(&(stPageInfo.pstradiusContainer));
		return 0;
	}
	//stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );
	stPageInfo.iUserGroup = stPageInfo.pstradiusContainer->iUserGroup;

	//stPageInfo.pstModuleContainer = MC_create_module_container();
	if( NULL == stPageInfo.pstradiusContainer )
	{
		release_portal_container(&(stPageInfo.pstradiusContainer));
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


	

	MC_setActiveLabel( stPageInfo.pstradiusContainer->pstModuleContainer, WP_EAG_RADIUS);

	MC_setPrefixOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_multiRadius_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_multiRadius_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstradiusContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_METHOD, "post" );

	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstradiusContainer->pstModuleContainer );

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstradiusContainer));
	
	
	return 0;
}


static int s_multiRadius_prefix_of_page( STPageInfo *pstPageInfo )
{
	char del_rule[10] = "";
	char nodez[MAX_RADIUS_DOMAIN_LEN] = {0};
	int ret = 0;

	FILE * fp = pstPageInfo->fp;

	//if file not exist,creat it and write "start" in it
	char buf_start[]="start\n";
	
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
		fprintf( fp, "window.location.href='wp_multi_radius.cgi?UN=%s';\n", pstPageInfo->encry );
		fprintf( fp, "</script>\n" );
	}
	cgiFormStringNoNewlines( "DELRULE", del_rule, 10 );
	if( !strcmp(del_rule, "delete") && (pstPageInfo->iUserGroup == 0))
	{
		cgiFormStringNoNewlines( "NODEZ", nodez, sizeof(nodez) );

		ret = eag_del_radius(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									nodez );

		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_radius.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry ,plotid);
		fprintf( fp, "</script>\n" );
	}

	return 0;
}

static int s_multiRadius_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * radius_public = pstPageInfo->lpublic;
	struct list * radius_auth = pstPageInfo->lauth;
	int i, j,cl = 0;

	char menu[21]="";
	char i_char[10]="";
	/////////////读取数据/////////////////////////

	fprintf(fp,	"<table border=0  cellspacing=0 cellpadding=0>"\
					"<tr>");
	if(pstPageInfo->iUserGroup == 0)
		{
			fprintf(fp,"<td><a id=link href=wp_add_multi_radius.cgi?UN=%s&plotid=%s>%s</a></td>", pstPageInfo->encry,plotid,search(radius_auth,"add_multi_radius") );
		}
	fprintf(fp,"</tr>");
	fprintf(fp,"<tr height=7><td></td></tr>");
	fprintf(fp,"<tr>\n");
	fprintf(fp,"<td>%s</td>",search(radius_auth,"plot_idz"));
	fprintf(fp,"<td><select name=port_id onchange=plotid_change(this)>");
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
	
	fprintf(fp,"</select></td>");
	fprintf(fp, "</tr>");
	fprintf(fp,"<tr height=7><td></td></tr>");
	fprintf(fp,"</table>");

    fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_multi_radius.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );
                                                      
	
	fprintf(fp,	"<table border=0 width=700 cellspacing=0 cellpadding=0>"\
				"<tr height=30 align=left bgcolor=#eaeff9>");
	fprintf(fp, "<th width=400>%s</th>", search(radius_auth,"domain_name") );	
	fprintf(fp, "<th width=400>%s</th>", search(radius_auth,"radius_server_ip") );	
	fprintf(fp, "<th width=250>%s</th>", search(radius_auth,"radius_server_port") );	
	fprintf(fp, "<th width=400>%s</th>", search(radius_auth,"charging_server_ip") );
	fprintf(fp, "<th width=250>%s</th>", search(radius_auth,"charging_server_port") );

	fprintf(fp, "<th width=13>&nbsp;</th>"\
				"</tr>");
	int ret = 0;
	char *domain=NULL;
	struct radius_conf radiusconf;
	memset( &radiusconf, 0, sizeof(struct radius_conf) );
	ret = eag_get_radius_conf(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									domain,
									&radiusconf );

	for( i=0; i<radiusconf.current_num; i++ )
	{
			char ipstr[32] = {0};
			memset(menu,0,21);
			strcpy(menu,"menulist");

			char temp[5];
			memset(&temp,0,5);
			sprintf(temp,"%d",i);
			strcat(menu,temp);

			fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );

			fprintf(fp,	"<td>%s</td>", radiusconf.radius_srv[i].domain );
			ccgi_ip2str( radiusconf.radius_srv[i].auth_ip, ipstr,sizeof(ipstr));
			fprintf(fp,	"<td>%s</td>", ipstr);
			fprintf(fp,	"<td>%u</td>", radiusconf.radius_srv[i].auth_port);
			ccgi_ip2str( radiusconf.radius_srv[i].acct_ip, ipstr,sizeof(ipstr));			
			fprintf(fp,	"<td>%s</td>", ipstr);
			fprintf(fp,	"<td>%u</td>", radiusconf.radius_srv[i].acct_port);

			fprintf(fp, "<td>");
			if(pstPageInfo->iUserGroup == 0)
			{
				fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(512-i),menu,menu);
				fprintf(fp, "<img src=/images/detail.gif>"\
				"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
				fprintf(fp, "<div id=div1>");

				char encoder_character_result[256] = { 0 };
				url_encoder(radiusconf.radius_srv[i].domain, encoder_character_result, escape_character, encoder_character, CHARACTER_NUM);

				fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_multi_radius.cgi?UN=%s&DELRULE=%s&plotid=%s&NODEZ=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" ,plotid,encoder_character_result,search(radius_public,"delete"));
				fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_edit_multi_radius.cgi?UN=%s&plotid=%s&NODEZ=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , plotid,encoder_character_result,search(radius_public,"configure"));
				fprintf(fp, "</div>"\
				"</div>"\
				"</div>");
			}
			fprintf(fp,	"</td>");
			fprintf(fp,	"</tr>");

			cl = !cl;
		}
	
		fprintf(fp,	"</table>");
		fprintf(fp,"<input type=hidden name=UN value=%s>",pstPageInfo->encry);
		return 0;	
}

char *replace(char *source,char *sub,char *rep)
{
	char *result;
	char *pc1,*pc2,*pc3;
	int isource,isub,irep;
	isub=strlen(sub);
	irep=strlen(rep);
	isource=strlen(source);
	if(NULL == *sub)
	return strdup(source);
	result=(char*)malloc(( (irep > isub) ? (float)strlen(source) / isub* irep+ 1:isource ) * sizeof(char));
	pc1=result;
	while(*source!=NULL)
	{
		pc2=source;
		pc3=sub;
		while(*pc2==*pc3&&*pc3!=NULL&&*pc2!=NULL)
		pc2++,pc3++;
		if(NULL==*pc3)
		{
			pc3=rep;
			while(*pc3!=NULL)
			*pc1++=*pc3++;
			pc2--;
			source=pc2;
		}
		else
		*pc1++ = *source;
		source++;
	}
	*pc1=NULL;
	return result;
}

void url_encoder(char *source,char *result,char **escape_character,char **encoder_character,int character_num)
{
	char temp[256] = { 0 };
	char temp_result[256] = { 0 };
	int i = 0;
	strncpy(temp, source, sizeof(temp)-1);

	for(i=0; i<character_num; i++)
	{
		memset(temp_result, 0, sizeof(temp_result));
		if(replace(temp, escape_character[i], encoder_character[i]) != NULL)
		{
 			strncpy(temp_result, replace(temp, escape_character[i], encoder_character[i]), sizeof(temp_result)-1);
 		}
 		else
 		{
 			strncpy(temp_result, temp, sizeof(temp_result)-1);
 		}
		memset(temp, 0, sizeof(temp));
		strncpy(temp, temp_result, sizeof(temp)-1);
	}
	strcpy(result, temp_result);
}

