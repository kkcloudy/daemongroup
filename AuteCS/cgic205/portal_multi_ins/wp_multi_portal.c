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
#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"

#ifndef MULTI_PORTAL  
#define MULTI_PORTAL

#define SUBMIT_NAME "submit_multi_portal"

//#define MAX_INDEX_LEN 32

#endif


//#define SUBMIT_NAME "submit_nasid_by_vlan"

#define MAX_POR_LINE 70
#define MAX_DOM_LINE 32
typedef struct{
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	int plot_id;
	FILE *fp;
	
} STPageInfo;

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};


#define CHARACTER_NUM	8
char *escape_character[] = {"%", "&", "+", "#", " ", "/", "?", "="};
char *encoder_character[] = {"%25", "%26", "%2B", "%23", "%20", "%2F", "%3F", "%3D"};

/***************************************************************
申明回调函数
****************************************************************/
static int s_multiPortal_prefix_of_page( STPageInfo *pstPageInfo );
static int s_multiPortal_content_of_page( STPageInfo *pstPageInfo );

static char *replace(char *source,char *sub,char *rep);

static void url_encoder(char *source,char *result,char **escape_character,char **encoder_character,int character_num);



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
	int ret1 = 0;
	STPageInfo stPageInfo;

	DcliWInit();
	ccgi_dbus_init();  
	
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret1 = init_portal_container(&(stPageInfo.pstPortalContainer));
	stPageInfo.lpublic = stPageInfo.pstPortalContainer->lpublic;
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	//stPageInfo.username_encry=dcryption(stPageInfo.encry);
    if( WS_ERR_PORTAL_ILLEGAL_USER == ret1 )
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


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_PORTAL);

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
	
	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_multiPortal_prefix_of_page( STPageInfo *pstPageInfo )
{
	char del_rule[10] = "";
	char nodez[32];
	memset(nodez,0,32);
	char attz[64];
	memset(attz,0,sizeof(attz));
	int ret = 0;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	
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
		fprintf( fp, "window.location.href='wp_multi_portal.cgi?UN=%s';\n", pstPageInfo->encry );
		fprintf( fp, "</script>\n" );
	}
	cgiFormStringNoNewlines( "DELRULE", del_rule, 10 );
	if( !strcmp(del_rule, "delete") && (pstPageInfo->iUserGroup == 0))
	{
		cgiFormStringNoNewlines( "NODEZ", nodez, sizeof(nodez) );
		cgiFormStringNoNewlines( "ATTZ", attz, sizeof(attz));
		key_type = strtol(nodez,NULL,10);
		
		if (key_type == PORTAL_KEYTYPE_WLANID) 
		{
			keyid = atoi(attz);
			if (keyid == 0 || keyid > 128)
			{
				return 0;
			}
		}
		else if (key_type == PORTAL_KEYTYPE_VLANID) 
		{
			keyid = atoi(attz);
			if(keyid == 0 || keyid > 4096)
			{
				return 0;
			}		
		}
		else if (key_type == PORTAL_KEYTYPE_WTPID)
		{
			keyid = atoi(attz);
		}
		else if (key_type == PORTAL_KEYTYPE_INTF)
		{
			keystr = (char *)attz;
		}
		else if (key_type == PORTAL_KEYTYPE_ESSID)
		{
			keystr = (char *)attz;
		}


		
		ret = eag_del_portal_server(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 	
									key_type,
									keyid,
									keystr);
		
							
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_portal.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry ,plotid);
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

	char menu[21]="";
	char i_char[10]="";

	/////////////读取数据/////////////////////////
	int ret = 0;
	struct portal_conf portalconf;
	char keyvalue[128] = {0};
	memset( &portalconf, 0, sizeof(struct portal_conf) );
	
	ret = eag_get_portal_conf(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									&portalconf );

	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
					"<tr>");
	if(pstPageInfo->iUserGroup == 0)
		{
			fprintf(fp,"<td><a id=link href=wp_add_multi_portal.cgi?UN=%s&plotid=%s>%s</a></td>", pstPageInfo->encry,plotid,search(portal_auth,"add_multi_portal") );
		}
	fprintf(fp, "</tr>");
	fprintf(fp,"<tr height=7><td></td></tr>");
	fprintf(fp,"<tr>\n");
	fprintf(fp,"<td>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td><select name=plotid onchange=plotid_change(this)>");
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
   	"var url = 'wp_multi_portal.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );

	fprintf(fp,	"<table border=0 width=650 cellspacing=0 cellpadding=0>");
	fprintf(fp, "<tr height=30 align=left bgcolor=#eaeff9>");
	fprintf(fp,	"<th width=100>KeyWord</th>");
	fprintf(fp, "<th width=50>Value</th>");
	fprintf(fp,	"<th width=300>URL</th>"\
			"<th width=50>Domain</th>"\
			"<th width=100>%s</th>"\
			"<th width=13>&nbsp;</th>"\
			"</tr>",search(portal_auth,"HS_ACID"));
	for( i=0; i<portalconf.current_num; i++ )
	{		
			memset(menu,0,21);
			strcpy(menu,"menulist");
			sprintf(i_char,"%d",i+1);
			strcat(menu,i_char);

			memset(keyvalue,0,sizeof(keyvalue));
			
			fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
			switch(portalconf.portal_srv[i].key_type) 
			{
				case PORTAL_KEYTYPE_ESSID:
					fprintf(fp, "<td align=left>Essid</td>");
					fprintf(fp, "<td>%s</td>", portalconf.portal_srv[i].key.essid);
					
					//replace_url
					if(replace(portalconf.portal_srv[i].key.essid," ","%20") != NULL)
					{
						strncpy(keyvalue,replace(portalconf.portal_srv[i].key.essid," ","%20"),sizeof(keyvalue)-1);
					}
					else
					{
						strncpy(keyvalue,portalconf.portal_srv[i].key.essid,sizeof(keyvalue)-1);
					}					
					break;
				case PORTAL_KEYTYPE_WLANID:
					fprintf(fp, "<td align=left>Wlanid</td>");
					fprintf(fp, "<td>%u</td>", portalconf.portal_srv[i].key.wlanid);
					snprintf(keyvalue,sizeof(keyvalue)-1,"%u",portalconf.portal_srv[i].key.wlanid);
					break;
				case PORTAL_KEYTYPE_VLANID:
					fprintf(fp, "<td align=left>Vlanid</td>");
					fprintf(fp, "<td>%u</td>", portalconf.portal_srv[i].key.vlanid);
					snprintf(keyvalue,sizeof(keyvalue)-1,"%u",portalconf.portal_srv[i].key.vlanid);
					break;
				case PORTAL_KEYTYPE_WTPID:
					fprintf(fp, "<td align=left>Wtpid</td>");
					fprintf(fp, "<td>%u</td>", portalconf.portal_srv[i].key.wtpid);
					snprintf(keyvalue,sizeof(keyvalue)-1,"%u",portalconf.portal_srv[i].key.wtpid);
					break;
				case PORTAL_KEYTYPE_INTF:
					fprintf(fp, "<td align=left>Intf</td>");
					fprintf(fp, "<td>%s</td>", portalconf.portal_srv[i].key.intf);
					strncpy(keyvalue,portalconf.portal_srv[i].key.intf,sizeof(keyvalue)-1);
					break;					
				default:
					fprintf(fp, "<td align=left colspan=2></td>");
					break;
			}				
			fprintf(fp, "<td>%s</td>", portalconf.portal_srv[i].portal_url);				
			//fprintf(fp, "portal ntfport			:%u\n", portalconf.portal_srv[i].ntf_port);
			if(0 != strcmp(portalconf.portal_srv[i].domain, ""))
			{
				fprintf(fp, "<td>%s</td>", portalconf.portal_srv[i].domain);
			}
			else
			{
				fprintf(fp, "<td>&nbsp;</td>");
			}
			if(0 != strcmp(portalconf.portal_srv[i].acname, ""))
			{
				fprintf(fp, "<td>%s</td>", portalconf.portal_srv[i].acname);
			}
			else
			{
				fprintf(fp, "<td>&nbsp;</td>");
			}
			fprintf(fp,"<td>");
			fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(512-i),menu,menu);
			fprintf(fp, "<img src=/images/detail.gif>"\
			"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
			fprintf(fp, "<div id=div1>");
			
			char encoder_character_result[256] = { 0 };
			url_encoder(keyvalue, encoder_character_result, escape_character, encoder_character, CHARACTER_NUM);

			fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_multi_portal.cgi?UN=%s&DELRULE=%s&plotid=%s&NODEZ=%d&ATTZ=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" , plotid,portalconf.portal_srv[i].key_type,encoder_character_result,search(portal_public,"delete"));
			fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_edit_multi_portal.cgi?UN=%s&EDITCONF=%s&plotid=%s&NODEZ=%d&ATTZ=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , "edit" ,plotid,portalconf.portal_srv[i].key_type,encoder_character_result,search(portal_public,"configure"));
			fprintf(fp, "</div>"\
			"</div>"\
			"</div>");
			fprintf(fp,"</td>");
			fprintf(fp,"</tr>");
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
