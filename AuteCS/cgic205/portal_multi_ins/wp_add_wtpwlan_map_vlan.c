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

#include "ws_eag_conf.h"

#ifndef ADD_VLAN_MAPING_MULTI_RADIUS
#define ADD_VLAN_MAPING_MULTI_RADIUS
#define SUBMIT_NAME "submit_add_map_vlan"
#define MAX_ID_LEN	10
#define MAX_PAGE_NUM  10
#endif

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

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 9 );

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
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}

static int s_wtpwlanmapvlan_prefix_of_page( STPageInfo *pstPageInfo )
{
	char buf[]="start\n";
	char del_rule[10] = "";
	char index[32] = "";
	FILE * fp = pstPageInfo->fp;
	char cur_page_num_str[10]="";
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	int wlanid_begin, wlanid_end, wtpid_begin, wtpid_end, vlanid;
	char wlanid_begin_str[MAX_ID_LEN],wlanid_end_str[MAX_ID_LEN];
	char wtpid_begin_str[MAX_ID_LEN], wtpid_end_str[MAX_ID_LEN];
	char vlanid_str[MAX_ID_LEN];
	char wwvkey[50];
	char plot_id[10];
    memset(wlanid_begin_str,0,sizeof(wlanid_begin_str));
    memset(wlanid_end_str,0,sizeof(wlanid_end_str));
    memset(wtpid_begin_str,0,sizeof(wtpid_begin_str));
    memset(wtpid_end_str,0,sizeof(wtpid_end_str));
    memset(vlanid_str,0,sizeof(vlanid_str));
    memset(wwvkey,0,sizeof(wwvkey));
	memset(plot_id,0,sizeof(plot_id));

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		cgiFormStringNoNewlines("wlanid_begin",wlanid_begin_str, sizeof(wlanid_begin_str));
		cgiFormStringNoNewlines("wlanid_end",wlanid_end_str, sizeof(wlanid_end_str));
		cgiFormStringNoNewlines("wtpid_begin",wtpid_begin_str, sizeof(wtpid_begin_str));
		cgiFormStringNoNewlines("wtpid_end",wtpid_end_str, sizeof(wtpid_end_str));
		cgiFormStringNoNewlines("vlanid",vlanid_str, sizeof(vlanid_str));
		cgiFormStringNoNewlines("plot_id",plot_id, sizeof(plot_id));

        wlanid_begin=strtoul(wlanid_begin_str,0,10);
        wlanid_end=strtoul(wlanid_end_str,0,10);
		wtpid_begin=strtoul(wtpid_begin_str,0,10);
        wtpid_end=strtoul(wtpid_end_str,0,10);
        vlanid=strtoul(vlanid_str,0,10);

		if(!number_input_is_legal(wlanid_begin_str) || !number_input_is_legal(wlanid_end_str)
			|| !number_input_is_legal(wtpid_begin_str) || !number_input_is_legal(wtpid_end_str)
			|| !number_input_is_legal(vlanid_str))
		{
			//err input
			fprintf(fp,"<script type=text/javascript>\n"\
						"alert('%s');\n"\
						"</script>\n",search(portal_public,"input_error"));
		}
		else if( wlanid_begin <= 0 || wlanid_end < wlanid_begin || 
			wtpid_begin <=0 || wtpid_end < wtpid_begin ||
			vlanid <=0 || vlanid >4096 )
		{
			//err input
			fprintf(fp,"<script type=text/javascript>\n"\
						"alert('%s');\n"\
						"</script>\n",search(portal_public,"input_error"));
		}
		else
		{
		    sprintf(wwvkey,"%s.%s.%s.%s",wlanid_begin_str,wlanid_end_str,wtpid_begin_str,wtpid_end_str);
			char *tmpz=(char *)malloc(20);
			memset(tmpz,0,20);
			sprintf(tmpz,"%s%s",MTW_N,plot_id);	

			add_eag_node_attr(MULTI_WWV_F, tmpz, ATT_Z, wwvkey);
			mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WLANSIDZ, wlanid_begin_str);
			mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WLANEIDZ, wlanid_end_str);
			mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WTPSIDZ, wtpid_begin_str);
			mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WTPEIDZ, wtpid_end_str);
			mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, VLANIDZ, vlanid_str);
			free(tmpz);
			fprintf( fp, "<script type=text/javascript>\n"\
						"alert('%s');\n"\
		                "window.location.href='wp_wtpwlan_map_vlan.cgi?UN=%s&plotid=%s';\n"\
						"</script>\n",search(portal_auth,"add_vlan_map_success"), pstPageInfo->encry,plot_id);
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
	
    char *urlnode=(char *)malloc(20);
	memset(urlnode,0,20);
	cgiFormStringNoNewlines( "plotid", urlnode, 20 );
	
	fprintf(fp,"<table border=0 width=500 cellspacing=0 cellpadding=0>\n");
	
	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wlan_id_begin"));
	fprintf(fp,"<td width=250><input type=text name=wlanid_begin size=15></td>"\
				"</tr>\n");
	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wlan_id_end"));
	fprintf(fp,"<td width=250><input type=text name=wlanid_end size=15></td>"\
				"</tr>\n");
	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wtp_id_begin"));
	fprintf(fp,"<td width=250><input type=text name=wtpid_begin size=15></td>"\
				"</tr>\n");
	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"wtp_id_end"));
	fprintf(fp,"<td width=250><input type=text name=wtpid_end size=15></td>"\
				"</tr>\n");
	fprintf(fp,"<tr height=30 align=left><td width=250>%s</td>",search(portal_auth,"vlan_id"));
	fprintf(fp,"<td width=250><input type=text name=vlanid size=15></td>"\
				"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td colspan=2><input type=hidden name=plot_id value=\"%s\"></td>"\
				"</tr>\n",urlnode);
	fprintf(fp,	"</table>");
    free(urlnode);
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

