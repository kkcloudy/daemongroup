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
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"

#define SUBMIT_NAME "submit_nasid_by_vlan"

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
	int ret1 = 0;
	STPageInfo stPageInfo;
	char *tmp=(char *)malloc(64);
	int ret;

	DcliWInit();
	ccgi_dbus_init();
	
//初始化常用公共变量
	memset(&stPageInfo, 0,sizeof(STPageInfo) );
	ret1 = init_portal_container(&(stPageInfo.pstPortalContainer));
	//strcpy(stPageInfo.encry,stPageInfo.pstPortalContainer->encry);
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
//初始化完毕

	
//处理表单


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_NASID);

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

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_nasidbyvlan_prefix_of_page( STPageInfo *pstPageInfo )
{
	char del_rule[10] = "";
	char nodez[32] = {0};
	int nastype = 0;
	unsigned long key_type = 0;
	unsigned long keywd_1 = 0;
	unsigned long keywd_2 = 0;
	char *tmp_ip = NULL;
	char *ip = NULL;
	char ip_1[24];
	char ip_2[24];
	int plot_id = 0;
	int hs_flag = 0;
	char attz[128] = {0};//edit by wk
	struct nasid_map_t nasidmap;
	char *tmp = NULL;
	char *tmp_1 = NULL;
	int ret = 0;

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
	if( !strcmp(del_rule, "delete")  && (pstPageInfo->iUserGroup == 0))
	{
		/*
		if(check_all_eag_services() == 1)
			{
				ShowAlert( search(pstPageInfo->lauth, "diss_all_eag_fir") );
				return 0;
			}
		*/
		cgiFormStringNoNewlines( "NODEZ", nodez, sizeof(nodez) );
		cgiFormStringNoNewlines( "ATTZ", attz, sizeof(attz) );
	    nastype = strtoul(nodez ,NULL, 10);	
		
		memset (&nasidmap, 0, sizeof(struct nasid_map_t));

		switch(nastype)
		{						
			case NASID_KEYTYPE_WLANID:
				tmp=(char *)attz;
				nasidmap.key_type = NASID_KEYTYPE_WLANID;
				if (NULL != (tmp_1 = strchr(tmp, '-'))) 
				{
					keywd_1 = strtoul(tmp,NULL,10);
					tmp=tmp_1+1;
					keywd_2 = strtoul(tmp,NULL,10);
				} 
				else 
				{
					keywd_1 = strtoul(tmp,NULL,10);
					keywd_2 = keywd_1;
				}
				nasidmap.key.wlanidrange.id_begin = keywd_1;
				nasidmap.key.wlanidrange.id_end = keywd_2;
				break;
			case NASID_KEYTYPE_VLANID:
				tmp=(char *)attz;
				nasidmap.key_type = NASID_KEYTYPE_VLANID;
				if (NULL != (tmp_1 = strchr(tmp, '-'))) 
				{
					keywd_1 = strtoul(tmp,NULL,10);
					tmp=tmp_1+1;
					keywd_2 = strtoul(tmp,NULL,10);
				} 
				else 
				{
					keywd_1 = strtoul(tmp,NULL,10);
					keywd_2 = keywd_1;
				}
				nasidmap.key.vlanidrange.id_begin = keywd_1;
				nasidmap.key.vlanidrange.id_end = keywd_2;
				break;
			case NASID_KEYTYPE_WTPID:
				tmp=(char *)attz;
				nasidmap.key_type = NASID_KEYTYPE_WTPID;
				if (NULL != (tmp_1 = strchr(tmp, '-'))) 
				{
					keywd_1 = strtoul(tmp,NULL,10);
					tmp=tmp_1+1;
					keywd_2 = strtoul(tmp,NULL,10);
				} 
				else 
				{
					keywd_1 = strtoul(tmp,NULL,10);
					keywd_2 = keywd_1;
				}
				nasidmap.key.wtpidrange.id_begin = keywd_1;
				nasidmap.key.wtpidrange.id_end = keywd_2;
				break;
			case NASID_KEYTYPE_IPRANGE:
			{
				memset(ip_1, 0, sizeof(ip_1));
				memset(ip_2, 0, sizeof(ip_2));
				nasidmap.key_type = NASID_KEYTYPE_IPRANGE;
				tmp_ip = (char *)attz;

				if (NULL == (ip = strchr(tmp_ip, '-'))) 
				{
					strncpy(ip_1, tmp_ip, sizeof(ip_1) - 1);
					ccgi_inet_atoi(ip_1, &nasidmap.key.iprange.ip_begin);
					nasidmap.key.iprange.ip_end = nasidmap.key.iprange.ip_begin;
				}
				else 
				{
					strncpy(ip_1, tmp_ip, ip-tmp_ip);
					tmp_ip = ip + 1;
					strncpy(ip_2, tmp_ip, 24-1);
					ccgi_inet_atoi(ip_1, &nasidmap.key.iprange.ip_begin);
					ccgi_inet_atoi(ip_2, &nasidmap.key.iprange.ip_end);
				}	
			}
			break;
			case NASID_KEYTYPE_INTF:
			nasidmap.key_type = NASID_KEYTYPE_INTF;
			strncpy(nasidmap.key.intf,attz,MAX_NASID_KEY_BUFF_LEN-1);
			break;
		}
		
		ret = eag_del_nasid(ccgi_connection, parameter.local_id, parameter.instance_id, &nasidmap);

		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_nasid_byvlan.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry ,plotid);
		fprintf( fp, "</script>\n" );

		
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

	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
				   "<tr>");
	if(pstPageInfo->iUserGroup == 0)
		{
			fprintf(fp, "<td><a id=link href=wp_addnasid.cgi?UN=%s&plotid=%s>%s</a></td>", pstPageInfo->encry,plotid,search(portal_auth,"add_nasid") );
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
   	"var url = 'wp_nasid_byvlan.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );

	fprintf(fp,	"<table border=0 width=573 cellspacing=0 cellpadding=0>"\
				"<tr height=30 align=left bgcolor=#eaeff9>");
	fprintf(fp, "<th width=120>Type</th>");
	fprintf(fp,	"<th width=120>%s</th>", "nasid" );
	fprintf(fp,	"<th width=120>%s</th>", search(portal_auth,"syntaxis") );
	fprintf(fp, "<th width=50>&nbsp;</th>"\
				"</tr>");
	
    int locate = 0;
	int ret = 0;
	struct api_nasid_conf nasidconf;
	memset(&nasidconf, 0, sizeof(nasidconf));
	char nasvalue[128] = {0};

	ret = eag_get_nasid(ccgi_connection, parameter.local_id, parameter.instance_id, &nasidconf);
	for (i = 0; i < nasidconf.current_num; i++) 
	{
			locate ++;
			memset(menu,0,21);
			strcpy(menu,"menulist");
			sprintf(i_char,"%d",nas_num+1);
			strcat(menu,i_char);

			memset(nasvalue,0,sizeof(nasvalue));
			fprintf(fp,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
			switch(nasidconf.nasid_map[i].key_type)
			{						
				case NASID_KEYTYPE_WLANID:
					if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2) {
						fprintf(fp, "<td>%d.wlanid %lu </td>", i + 1, nasidconf.nasid_map[i].keywd_1);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%lu",nasidconf.nasid_map[i].keywd_1);
					} else {
						fprintf(fp, "<td>%d.wlanid %lu-%lu </td>", i + 1, 
							nasidconf.nasid_map[i].keywd_1, nasidconf.nasid_map[i].keywd_2);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%lu-%lu",
							nasidconf.nasid_map[i].keywd_1, nasidconf.nasid_map[i].keywd_2);	
					}
					break;
				case NASID_KEYTYPE_VLANID:
					if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2) {
						fprintf(fp, "<td>%d.vlanid %lu </td>", i + 1, nasidconf.nasid_map[i].keywd_1);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%lu",nasidconf.nasid_map[i].keywd_1);
					} else {
						fprintf(fp, "<td>%d.vlanid %lu-%lu </td>", i + 1, 
							nasidconf.nasid_map[i].keywd_1, nasidconf.nasid_map[i].keywd_2);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%lu-%lu",
							nasidconf.nasid_map[i].keywd_1, nasidconf.nasid_map[i].keywd_2);
					}
					break;
				case NASID_KEYTYPE_WTPID:
					if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2) {
						fprintf(fp, "<td>%d.wtpid %lu </td>", i + 1, nasidconf.nasid_map[i].keywd_1);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%lu",nasidconf.nasid_map[i].keywd_1);
					} else {
						fprintf(fp, "<td>%d.wtpid %lu-%lu </td>", i + 1, 
							nasidconf.nasid_map[i].keywd_1, nasidconf.nasid_map[i].keywd_2);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%lu-%lu",
							nasidconf.nasid_map[i].keywd_1, nasidconf.nasid_map[i].keywd_2);
					}
					break;
				case NASID_KEYTYPE_IPRANGE:
				{
					char ip_1[24] = {0};
					char ip_2[24] = {0};
					if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2)
					{
						ccgi_ip2str(nasidconf.nasid_map[i].keywd_1, ip_1, sizeof(ip_1));
						fprintf(fp,"<td>%d.iprange %s </td>",i+1,ip_1);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%s",ip_1);
					}
					else 
					{
						ccgi_ip2str(nasidconf.nasid_map[i].keywd_1, ip_1, sizeof(ip_1));
						ccgi_ip2str(nasidconf.nasid_map[i].keywd_2, ip_2, sizeof(ip_2));
						fprintf(fp,"<td>%d.iprange %s-%s </td>", i + 1, ip_1, ip_2);
						snprintf(nasvalue,sizeof(nasvalue)-1,"%s-%s",ip_1, ip_2);
					}
				}
				break;
				case NASID_KEYTYPE_INTF:
				fprintf(fp, "<td>%d.interface %s </td>", i + 1, nasidconf.nasid_map[i].keystr);
				snprintf(nasvalue,sizeof(nasvalue)-1,"%s",nasidconf.nasid_map[i].keystr);
				break;
			}
			fprintf(fp,	"<td>%s</td>", nasidconf.nasid_map[i].nasid);
			fprintf(fp,	"<td>%lu</td>", nasidconf.nasid_map[i].conid);
			fprintf(fp, "<td>");
			if(pstPageInfo->iUserGroup == 0)
			{
				fprintf(fp, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(NASID_MAX_NUM-nas_num),menu,menu);
				fprintf(fp, "<img src=/images/detail.gif>"\
				"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
				fprintf(fp, "<div id=div1>");
				fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_nasid_byvlan.cgi?UN=%s&DELRULE=%s&plotid=%s&NODEZ=%d&ATTZ=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , "delete" , plotid,nasidconf.nasid_map[i].key_type,nasvalue,search(portal_public,"delete"));
				//fprintf(fp, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_edit_nasid.cgi?UN=%s&N_TYPE=%s&N_START=%s&N_END=%s&N_NASID=%s&N_SYNTAXIS=%s&PLOTIDZ=%s&NODEZ=%d&ATTZ=%s target=mainFrame>%s</a></div>",pstPageInfo->encry ,"","","", "","",plot_id,"","",search(portal_public,"configure"));
				fprintf(fp, "</div>"\
				"</div>"\
				"</div>");
			}
			
			fprintf(fp,	"</td>");
			fprintf(fp,	"</tr>");

		    memset( buf, 0, 256);
			nas_num++ ;
			cl = !cl;
		}

	fprintf(fp,	"</table>");
	fprintf(fp,"<input type=hidden name=plotid value=%s",plotid);
	fprintf(fp,"<input type=hidden name=UN value=%s",pstPageInfo->encry);
	return 0;	
}


