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
* wp_wtpwlan_map_vlan.c
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
#include "ws_init_dbus.h"

#include "ws_eag_conf.h"
#include "ws_eag_auto_conf.h"
#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"


#ifndef VLAN_MAPING
#define VLAN_MAPING

#define SUBMIT_NAME "submit_map_vlan"

#define MAX_ID_LEN	10
#define MAX_PAGE_NUM  10

#endif

typedef struct{
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;

	int cur_page_num;
	int all_page_num;
} STPageInfo;

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};


/***************************************************************
申明回调函数
****************************************************************/
static int s_wtpwlanmapvlan_prefix_of_page( STPageInfo *pstPageInfo );
static int s_wtpwlanmapvlan_content_of_page( STPageInfo *pstPageInfo );

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
	int ret;		

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

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_NASPORTID);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_wtpwlanmapvlan_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_wtpwlanmapvlan_content_of_page, &stPageInfo );

	
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


static int s_wtpwlanmapvlan_prefix_of_page( STPageInfo *pstPageInfo )
{
	struct list * portal_auth = pstPageInfo->lauth;
	char del_rule[10] = "";
	int ret = -1;
	char wlansid[10] = {0};
	char wlaneid[10] = {0};
	char wtpsid[10] = {0};
	char wtpeid[10] = {0};
	char vlansid[10] = {0};
	char vlaneid[10] = {0};
	unsigned long wlanid_begin = 0;
	unsigned long wlanid_end = 0;
	unsigned long wtpid_begin = 0;
	unsigned long wtpid_end = 0;
	unsigned long vlanid_begin = 0;
	unsigned long vlanid_end = 0;
	char types[10] = {0};

	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0;
	char nasp[10] = {0};

	FILE * fp = pstPageInfo->fp;
	
	fprintf(fp, "<style type=text/css>"\
	 	 		"#div1{ width:58px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  			"#div2{ width:56px; height:15px; padding-left:3px; padding-top:3px}"\
	  			"#link{ text-decoration:none; font-size: 12px}</style>" );
	
	fprintf(fp,	"<script type=\"text/javascript\">\n"\
				"function popMenu(objId)\n"\
				"{\n"\
			   		"var obj = document.getElementById(objId);\n"\
			   		"if (obj.style.display == 'none')\n"\
			   		"{\n"\
				 		"obj.style.display = 'block';\n"\
			   		"}\n"\
			   		"else\n"\
			   		"{\n"\
				 		"obj.style.display = 'none';\n"\
			   		"}\n"\
		   		"}\n"\
		   		"</script>\n");
	
	cgiFormStringNoNewlines( "DELRULE", del_rule, sizeof(del_rule) );

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_wtpwlan_map_vlan.cgi?UN=%s';\n", pstPageInfo->encry );
		fprintf( fp, "</script>\n" );
	}

	if( !strcmp(del_rule, "delete") )
	{
		cgiFormStringNoNewlines( "TYPE", types, sizeof(types) );
		cgiFormStringNoNewlines( "WLANSID", wlansid, sizeof(wlansid) );
		cgiFormStringNoNewlines( "WLANEID", wlaneid, sizeof(wlaneid) );
		cgiFormStringNoNewlines( "WTPSID", wtpsid, sizeof(wtpsid) );
		cgiFormStringNoNewlines( "WTPEID", wtpeid, sizeof(wtpeid) );
		cgiFormStringNoNewlines( "VLANSID", vlansid, sizeof(vlansid) );
		cgiFormStringNoNewlines( "VLANEID", vlaneid, sizeof(vlaneid) );
		cgiFormStringNoNewlines( "NASP", nasp, sizeof(nasp) );
		
		wlanid_begin = strtoul(wlansid, NULL, 10);
		wlanid_end = strtoul(wlaneid, NULL, 10);
		wtpid_begin = strtoul(wtpsid, NULL, 10);
		wtpid_end = strtoul(wtpeid, NULL, 10);
		vlanid_begin = strtoul(vlansid, NULL, 10);
		vlanid_end = strtoul(vlaneid, NULL, 10);
		nasportid = strtoul(nasp, NULL, 10);
		
		if(0 == strcmp(types,"1"))
		{
			wlanid.begin = wlanid_begin;
			wlanid.end = wlanid_end;
			wtpid.begin = wtpid_begin;
			wtpid.end= wtpid_end;
			ret = eag_del_nasportid(ccgi_connection, parameter.local_id, parameter.instance_id, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_WLAN_WTP, nasportid);
		}
		else if(0 == strcmp(types,"2"))
		{
			vlanid.begin = vlanid_begin;
			vlanid.end = vlanid_end;
			ret = eag_del_nasportid(ccgi_connection, parameter.local_id, parameter.instance_id, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_VLAN, nasportid);
		}
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_wtpwlan_map_vlan.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry ,plotid);
		fprintf( fp, "</script>\n" );
	
	}

	return 0;
}

static int s_wtpwlanmapvlan_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	int i, cl = 0;
	int ret = 0;
	char menu[21]="";
	
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
					"<tr>");
	if(pstPageInfo->iUserGroup == 0)
		fprintf(fp,	"<td><a id=link href=wp_add_wtpwlan_map_vlan.cgi?UN=%s&plotid=%s>%s</a></td>", pstPageInfo->encry,plotid,search(portal_auth,"add_vlan_maping") );
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
	fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_wtpwlan_map_vlan.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );
	

	fprintf(fp,"</table>");	
	fprintf(fp,	"<table border=0 width=700 cellspacing=0 cellpadding=0>");
	
	struct nasportid_conf nasportid;
	memset(&nasportid, 0, sizeof(nasportid));
	ret = eag_get_nasportid(ccgi_connection, parameter.local_id, parameter.instance_id, &nasportid);

	
	fprintf(fp, "<tr height=30 align=left bgcolor=#eaeff9>");
	fprintf(fp, "<th width=100>%s</th>", "Index" );
	fprintf(fp, "<th width=100>%s</th>", search(portal_auth,"wlan_id_begin") );	
	fprintf(fp, "<th width=100>%s</th>", search(portal_auth,"wlan_id_end") );	
	fprintf(fp, "<th width=100>%s</th>", search(portal_auth,"wtp_id_begin") );	
	fprintf(fp, "<th width=100>%s</th>", search(portal_auth,"wtp_id_end") );
	fprintf(fp, "<th width=100>%s</th>", "nasportid" );
	fprintf(fp, "<th width=100>&nbsp;</th>"\
				"</tr>");
	for (i = 0; i < nasportid.current_num; i++)
	{
			memset(menu,0,21);
			strcpy(menu,"menulist");

			char temp[5];
			memset(&temp,0,5);
			sprintf(temp,"%d",i);
			strcat(menu,temp);

			if(NASPORTID_KEYTYPE_WLAN_WTP == nasportid.nasportid_map[i].key_type)
			{
				fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
				fprintf(fp, "<td>%d.wlanid</td>",i+1);
				fprintf(fp,	"<td>%u</td>", nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin);
				fprintf(fp,	"<td>%u</td>", nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end);
				fprintf(fp,	"<td>%u</td>", nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin);
				fprintf(fp,	"<td>%u</td>", nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end);
				fprintf(fp,	"<td>%u</td>", nasportid.nasportid_map[i].nasportid);
				fprintf(fp, "<td>");
				if(pstPageInfo->iUserGroup == 0)
					{
						fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(MAX_WWV_OPTION_NUM-i),menu,menu);
						fprintf(fp, "<img src=/images/detail.gif>"\
						"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
						fprintf(fp, "<div id=div1>");
						fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpwlan_map_vlan.cgi?UN=%s&DELRULE=%s&plotid=%s&WLANSID=%u&WLANEID=%u&WTPSID=%u&WTPEID=%u&TYPE=1&NASP=%u target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" ,plotid,nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin,nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end,nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin,nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end,nasportid.nasportid_map[i].nasportid,search(portal_public,"delete"));
						/*fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_edit_wtpwlan_map_vlan.cgi?UN=%s&PLOTIDZ=%s&NODEZ=%s&WLANSID=%s&WLANEID=%s&WTPSID=%s&WTPEID=%s&VLANID=%s&ATTZ=%s target=mainFrame>%s</a></div>", \
								pstPageInfo->encry ,plotid,tempz,cq->wlansidz,cq->wlaneidz,cq->wtpsidz,cq->wtpeidz,cq->vlanidz,wwzkey,search(portal_public,"config"));*/
						fprintf(fp, "</div>"\
						"</div>"\
						"</div>");
					}
				fprintf(fp,	"</td>");
				fprintf(fp,	"</tr>");
				cl = !cl;
			}
	}
	fprintf(fp, "<tr height=30 align=left bgcolor=#eaeff9>");
	fprintf(fp, "<th width=100>%s</th>", "Index" );
	fprintf(fp, "<th width=200 colspan=2>%s</th>", "vlan begin" );	
	fprintf(fp, "<th width=200 colspan=2>%s</th>", "vlan end" );
	fprintf(fp, "<th width=100>%s</th>", "nasportid" );
	fprintf(fp, "<th width=100>&nbsp;</th>"\
				"</tr>");
	for (i = 0; i < nasportid.current_num; i++)
	{
			memset(menu,0,21);
			strcpy(menu,"menulist");

			char temp[5];
			memset(&temp,0,5);
			sprintf(temp,"%d",i);
			strcat(menu,temp);

			if(NASPORTID_KEYTYPE_VLAN == nasportid.nasportid_map[i].key_type)
			{
				fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
				fprintf(fp, "<td>%d.vlanid</td>",i+1);
				fprintf(fp,	"<td colspan=2>%u</td>", nasportid.nasportid_map[i].key.vlan.vlanid_begin);
				fprintf(fp,	"<td colspan=2>%u</td>", nasportid.nasportid_map[i].key.vlan.vlanid_end);
				fprintf(fp,	"<td>%u</td>", nasportid.nasportid_map[i].nasportid);
				fprintf(fp, "<td>");
				if(pstPageInfo->iUserGroup == 0)
					{
						fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(MAX_WWV_OPTION_NUM-i),menu,menu);
						fprintf(fp, "<img src=/images/detail.gif>"\
						"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
						fprintf(fp, "<div id=div1>");
						fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpwlan_map_vlan.cgi?UN=%s&DELRULE=%s&plotid=%s&VLANSID=%u&VLANEID=%u&TYPE=2&NASP=%u target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" ,plotid,nasportid.nasportid_map[i].key.vlan.vlanid_begin,nasportid.nasportid_map[i].key.vlan.vlanid_end,nasportid.nasportid_map[i].nasportid,search(portal_public,"delete"));
						/*fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_edit_wtpwlan_map_vlan.cgi?UN=%s&PLOTIDZ=%s&NODEZ=%s&WLANSID=%s&WLANEID=%s&WTPSID=%s&WTPEID=%s&VLANID=%s&ATTZ=%s target=mainFrame>%s</a></div>", \
								pstPageInfo->encry ,plotid,tempz,cq->wlansidz,cq->wlaneidz,cq->wtpsidz,cq->wtpeidz,cq->vlanidz,wwzkey,search(portal_public,"config"));*/
						fprintf(fp, "</div>"\
						"</div>"\
						"</div>");
					}
				fprintf(fp,	"</td>");
				fprintf(fp,	"</tr>");
				cl = !cl;
			}
	}
	fprintf(fp,	"</table>");
	fprintf(fp,"<input type=hidden name=plotid value=%s>",plotid);
	fprintf(fp,"<input type=hidden name=UN value=%s>",pstPageInfo->encry);
	return 0;	
}
