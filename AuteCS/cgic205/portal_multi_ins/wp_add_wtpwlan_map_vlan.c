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
* wp_add_wtpwlan_map_vlan.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
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

#ifndef ADD_VLAN_MAPING_MULTI_RADIUS
#define ADD_VLAN_MAPING_MULTI_RADIUS
#define SUBMIT_NAME "submit_add_map_vlan"
#define MAX_ID_LEN	10
#define MAX_PAGE_NUM  10
#endif
#define MAX_MAPED_NASPORTID 	99999999

int number_input_is_legal(const char *str);

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

int eag_vlanmap_policy_get_num(int policy_id);

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

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_NASPORTID);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_wtpwlanmapvlan_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_wtpwlanmapvlan_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );


	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"add_vlan_maping"),10);
	sprintf(url,"%s?UN=%s","wp_wtpwlan_map_vlan.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,7);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	snprintf( url, sizeof(url), "wp_wtpwlan_map_vlan.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}

static int s_wtpwlanmapvlan_prefix_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	int wlanid_begin, wlanid_end, wtpid_begin, wtpid_end,vlanid_begin,vlanid_end;
	char wlanid_begin_str[MAX_ID_LEN] = {0};
	char wlanid_end_str[MAX_ID_LEN] = {0};
	char wtpid_begin_str[MAX_ID_LEN] = {0};
	char wtpid_end_str[MAX_ID_LEN] = {0};
	char vlanid_begin_str[MAX_ID_LEN] = {0};
	char vlanid_end_str[MAX_ID_LEN] = {0};
	char nasp[10] = {0};
	int ret = 0;
	char types[10] = {0};
	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0;
	

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		cgiFormStringNoNewlines("wlanid_begin",wlanid_begin_str, sizeof(wlanid_begin_str));
		cgiFormStringNoNewlines("wlanid_end",wlanid_end_str, sizeof(wlanid_end_str));
		cgiFormStringNoNewlines("wtpid_begin",wtpid_begin_str, sizeof(wtpid_begin_str));
		cgiFormStringNoNewlines("wtpid_end",wtpid_end_str, sizeof(wtpid_end_str));
		cgiFormStringNoNewlines("vlansid",vlanid_begin_str, sizeof(vlanid_begin_str));
		cgiFormStringNoNewlines("vlaneid",vlanid_end_str, sizeof(vlanid_end_str));
		cgiFormStringNoNewlines("nasp",nasp, sizeof(nasp));
		cgiFormStringNoNewlines("types",types, sizeof(types));

		wlanid_begin = strtoul(wlanid_begin_str, NULL, 10);
		wlanid_end = strtoul(wlanid_end_str, NULL, 10);
		wtpid_begin = strtoul(wtpid_begin_str, NULL, 10);
		wtpid_end = strtoul(wtpid_end_str, NULL, 10);	
		vlanid_begin = strtoul(vlanid_begin_str, NULL, 10);	
		vlanid_end = strtoul(vlanid_end_str, NULL, 10);	
		nasportid = strtoul(nasp, NULL, 10);


 		if( (nasportid <= 0) ||(nasportid > MAX_MAPED_NASPORTID) )
		{
			//err input
			fprintf(fp,"<script type=text/javascript>\n"\
						"alert('%s');\n"\
						"</script>\n",search(portal_public,"input_error"));
		}
		
		if(0 == strcmp(types,"wlan"))
		{
			if(!number_input_is_legal(wlanid_begin_str) || !number_input_is_legal(wlanid_end_str)
				|| !number_input_is_legal(wtpid_begin_str) || !number_input_is_legal(wtpid_end_str)
				|| !number_input_is_legal(nasp))
			{
				//err input
				fprintf(fp,"<script type=text/javascript>\n"\
							"alert('%s');\n"\
							"</script>\n",search(portal_public,"input_error"));
			}
			else if( (wlanid_begin <= 0) ||(wlanid_begin > MAX_WLANID_INPUT)|| (wlanid_end < wlanid_begin) || 
				(wlanid_end <= 0) ||(wlanid_end > MAX_WLANID_INPUT)||(wtpid_end <= 0) || (wtpid_end > MAX_WTPID_INPUT)||
				(wtpid_begin <= 0) || (wtpid_begin > MAX_WTPID_INPUT)||(wtpid_end < wtpid_begin) )
			{
				//err input
				fprintf(fp,"<script type=text/javascript>\n"\
							"alert('%s');\n"\
							"</script>\n",search(portal_public,"input_error"));
			}
			else
			{
				wlanid.begin = wlanid_begin;
				wlanid.end = wlanid_end;
				wtpid.begin = wtpid_begin;
				wtpid.end = wtpid_end;		
				
				ret  = eag_add_nasportid(ccgi_connection, parameter.local_id, parameter.instance_id,
					wlanid, wtpid, vlanid, 
					NASPORTID_KEYTYPE_WLAN_WTP, nasportid);

				fprintf( fp, "<script type='text/javascript'>\n" );
				fprintf( fp, "window.location.href='wp_wtpwlan_map_vlan.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
				fprintf( fp, "</script>\n" );		
				
			}
		}
		else if(0 == strcmp(types,"vlan"))
		{
			if(!number_input_is_legal(vlanid_begin_str) || !number_input_is_legal(vlanid_end_str)|| !number_input_is_legal(nasp))
			{
				//err input
				fprintf(fp,"<script type=text/javascript>\n"\
							"alert('%s');\n"\
							"</script>\n",search(portal_public,"input_error"));
			}
			else if( (vlanid_begin <= 0) ||(vlanid_begin > MAX_VLANID_INPUT)
				||(vlanid_end <= 0) ||(vlanid_end> MAX_VLANID_INPUT)
				|| (vlanid_end < vlanid_begin) )
			{
				//err input
				fprintf(fp,"<script type=text/javascript>\n"\
							"alert('%s');\n"\
							"</script>\n",search(portal_public,"input_error"));
			}
			else
			{
				vlanid.begin = vlanid_begin;
				vlanid.end = vlanid_end;
				
				ret  = eag_add_nasportid(ccgi_connection, parameter.local_id, parameter.instance_id, 
					wlanid, wtpid, vlanid, 
					NASPORTID_KEYTYPE_VLAN, nasportid);

				fprintf( fp, "<script type='text/javascript'>\n" );
				fprintf( fp, "window.location.href='wp_wtpwlan_map_vlan.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
				fprintf( fp, "</script>\n" );		
				
			}
		}
		 

	}
	return 0;
}

static int s_wtpwlanmapvlan_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	int i, cl = 0;
	char menu[21]="";
	
	char types[10] = {0};
	cgiFormStringNoNewlines( "TYPE", types, sizeof(types) );
	
	
	fprintf(fp,"<table border=0 width=500 cellspacing=0 cellpadding=0>\n");

	fprintf(fp,"<tr height=30 align=left>\n");
	fprintf(fp,"<td width=250>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td width=250><select name=port_id onchange=plotid_change(this)>");
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
	fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_add_wtpwlan_map_vlan.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );	
	
	fprintf(fp,"<tr height=30 align=left>\n");
	fprintf(fp,"<td width=250>%s</td>","types");
	fprintf(fp,"<td width=250><select name=types onchange='sel_change(this);'>");
	if(0 == strcmp(types,"wlan"))
	{
		fprintf(fp,"<option value=wlan selected=selected>Wlan/Wtp</option>");
	}
	else
	{
		fprintf(fp,"<option value=wlan>Wlan/Wtp</option>");
	}
	if(0 == strcmp(types,"vlan"))
	{
		fprintf(fp,"<option value=vlan selected=selected>Vlan</option>");
	}
	else
	{
		fprintf(fp,"<option value=vlan>Vlan</option>");
	}
	fprintf(fp,"</select></td>");
	fprintf(fp, "</tr>");

	fprintf(fp,"<script type=text/javascript>\n");
	fprintf(fp,"function sel_change( obj )\n"\
	"{\n"\
	"var selstr = obj.options[obj.selectedIndex].value;\n"\
	"var url = 'wp_add_wtpwlan_map_vlan.cgi?UN=%s&plotid=%s&TYPE='+selstr;\n"\
	"window.location.href = url;\n"\
	"}\n", pstPageInfo->encry,plotid);
	fprintf(fp,"</script>\n" );

	if(0 == strcmp(types,"vlan")) 
	{
		fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>","vlan begin");
		fprintf(fp,"<td width=250><input type=text name=vlansid size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1-%d)</font></td>",MAX_VLANID_INPUT);
					fprintf(fp,"</tr>\n");
		fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>","vlan end");
		fprintf(fp,"<td width=250><input type=text name=vlaneid size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1-%d)</font></td>",MAX_VLANID_INPUT);
					fprintf(fp,"</tr>\n");		
	}
	else
	{
		fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wlan_id_begin"));
		fprintf(fp,"<td width=250><input type=text name=wlanid_begin size=15 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1-%d)</font></td>",MAX_WLANID_INPUT);
					fprintf(fp,"</tr>\n");
		fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wlan_id_end"));
		fprintf(fp,"<td width=250><input type=text name=wlanid_end size=15 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1-%d)</font></td>",MAX_WLANID_INPUT);
					fprintf(fp,"</tr>\n");
		fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wtp_id_begin"));
		fprintf(fp,"<td width=250><input type=text name=wtpid_begin size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1-%d)</font></td>",MAX_WTPID_INPUT);
					fprintf(fp,"</tr>\n");
		fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wtp_id_end"));
		fprintf(fp,"<td width=250><input type=text name=wtpid_end size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1-%d)</font></td>",MAX_WTPID_INPUT);
					fprintf(fp,"</tr>\n");
	}

	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>","Nasportid");
	fprintf(fp,"<td width=250><input type=text name=nasp size=15 maxLength=8 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1-%d)</font></td>",MAX_MAPED_NASPORTID);
				fprintf(fp,"</tr>\n");

	fprintf(fp,	"</table>");
	fprintf(fp,"<input type=hidden name=UN value=\"%s\">",pstPageInfo->encry);
	fprintf(fp,"<input type=hidden name=plotid value=\"%s\">",plotid);
	return 0;	
}

int number_input_is_legal(const char *str)
{
	const char *p = NULL;
	
	if (NULL == str || '\0' == str[0])
		return 0;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;
	if (strlen(str) > 1 && '0' == str[0])
		return 0;
	if (strlen(str) > MAX_ID_LEN-1)
		return 0;
	return 1;
}

int eag_vlanmap_policy_get_num(int policy_id)
{
	char tempz[20] = "";
	int flag = -1;
	struct st_wwvz chead = {0};
	int num = 0;
	
	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTW_N, policy_id);
	memset(&chead, 0, sizeof(chead));

	if (access(MULTI_WWV_F, 0) != 0)
	{
		return num;
	}
	flag = read_wwvz_xml(MULTI_WWV_F, &chead, &num, tempz);
	if(0 == flag && num > 0)
	{
		Free_wwvz_info(&chead);
	}

	return num;
}

