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
* wp_edit_nasid.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*add by liuyu  2011-5-8
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
#include "ws_dbus_list_interface.h"

#ifndef ADD_NASID  
#define ADD_NASID

#define SUBMIT_NAME 	"submit_add_nasid"
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

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};


/***************************************************************
申明回调函数
****************************************************************/
static int s_editnasid_prefix_of_page( STPageInfo *pstPageInfo );
static int s_editnasid_content_of_page( STPageInfo *pstPageInfo );

int cgiMain()
{
	int ret = 0;
    system("echo \"1\" > /opt/services/status/nasidvlan_status.status");
	STPageInfo stPageInfo;
	char url[256]="";
	
	DcliWInit();
	ccgi_dbus_init();	

//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret = init_portal_container(&(stPageInfo.pstPortalContainer));
	stPageInfo.lpublic=stPageInfo.pstPortalContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
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
	
//初始化完毕

	
//处理表单


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_NASID);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editnasid_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editnasid_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"edit_nasid"),5);
	sprintf(url,"%s?UN=%s","wp_addnasid.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,5);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_nasid_byvlan.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}

int Input_Nas_ID_Check(char * NasID)
{
	if( NULL == NasID )
	{
		return INPUT_NULL;
	}
	int i = 0;
	char * src = NasID ;
	while( src[i] != '\0')
		{
			if( (src[i] >= 32 && src[i] <= 126) 	
			  )
			{
				//fprintf(stderr,"boby check ok!\n");
				i++;
				continue;
			}
			else
			{
				return INPUT_CHAR_ERROR;
			}
		
		}

	return INPUT_OK;
}

static int s_editnasid_prefix_of_page( STPageInfo *pstPageInfo )
{
	FILE * pp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	
	char tpz[64],tpz_new[64], plot_id[30],nas_type[MAX_INDEX_LEN], start_point[MAX_INDEX_LEN], end_point[MAX_INDEX_LEN], nasid[MAX_INDEX_LEN],syntaxis_point[MAX_INDEX_LEN];
	int  check_ret2 = 0, check_ret3 = 0, check_ret4 = 0, check_ret5 = 0;
	
	//////////memset////////////////
	memset( tpz,0,sizeof(tpz));
	memset( tpz_new,0,sizeof(tpz_new));
	memset( plot_id,0,30);
	memset( nas_type, 0, MAX_INDEX_LEN );
	memset( start_point, 0, MAX_INDEX_LEN );
	memset( end_point, 0, MAX_INDEX_LEN );
	memset( nasid, 0, MAX_INDEX_LEN );
	memset( syntaxis_point, 0, MAX_INDEX_LEN );

	if( (cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess) && (pstPageInfo->iUserGroup == 0) )
	{
		/*用于检查eag实例是否开启
		    添加动态配置后不做此检查
		if(check_all_eag_services() == 1)
			{
				ShowAlert( search(portal_auth, "diss_all_eag_fir") );
				return 0;
			}
		*/
		cgiFormStringNoNewlines("attz_tmp",tpz, sizeof(tpz));
		cgiFormStringNoNewlines("plotid",plot_id, 30 );
		cgiFormStringNoNewlines("nas_type",nas_type, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("start_point",start_point, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("end_point",end_point, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("nasid",nasid, MAX_INDEX_LEN );
		cgiFormStringNoNewlines("syntaxis_point",syntaxis_point, MAX_INDEX_LEN );

		//fprintf(stderr,"test_ATTZ=%s,plot_id=%s,nas_type=%s,start_point=%s,end_point=%s,nasid=%s,syntaxis_point=%s\n",tpz,plot_id,nas_type,start_point,end_point,nasid,syntaxis_point);
		
		//////////check input valid///////////////////
		if( !strcmp(nas_type, "vlan") )
		{
			check_ret2 = check_input_digit_valid( start_point, 1, 4094 );
			check_ret3 = check_input_digit_valid( end_point, 1, 4094 );
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}
		else if( !strcmp(nas_type, "subintf") )
		{
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}
		else if( !strcmp(nas_type, "wlan") )
		{
			check_ret2 = check_input_digit_valid( start_point, 1, 128 );
			check_ret3 = check_input_digit_valid( end_point, 1, 128 );
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}
		else if( !strcmp(nas_type, "wtp") )
		{
			check_ret2 = check_input_digit_valid( start_point, 1, 4094 );
			check_ret3 = check_input_digit_valid( end_point, 1, 4094 );
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);			
		}
		else if( !strcmp(nas_type, "ipAddress") )
		{
			check_ret2 = Input_IP_address_Check(start_point);
			check_ret3 = Input_IP_address_Check(end_point);
			check_ret4 = Input_Nas_ID_Check(nasid);
			check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		}

		//fprintf(stderr,"test_check_ret2=%d,check_ret3=%d,check_ret4=%d,check_ret5=%d\n",check_ret2,check_ret3,check_ret4,check_ret5);
		if( check_ret2 == 0 && check_ret3 == 0 && check_ret4 == 0 && check_ret5 == 0 )
		{			
			FILE * fp = NULL;
		char tmpz[20];
		int attr_flag = -1, def_flag = -1;
		memset(tmpz,0,20);
		sprintf(tmpz,"%s%s",MTN_N,plot_id);

		def_flag = if_eag_arrt_default(MULTI_NAS_F, tmpz, ATT_Z, tpz);
		if(1 == def_flag )
		{
			memset( tpz, 0, sizeof(tpz));
			strcpy(tpz, "default");
		}
		mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NTYPE, nas_type);
	    mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NSTART, start_point);
        mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NEND, end_point);
        mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NNASID, nasid);
        mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, tpz, NCOVER, syntaxis_point);

		//修改完值后重新设置attr属性(default 属性不做修改)
		sprintf(tpz_new,"%s_%s_%s",nas_type,start_point,end_point);	
		if(1 != def_flag)
			attr_flag = mod_eag_node_arrt(MULTI_NAS_F, tmpz, ATT_Z, tpz, tpz_new);
		//fprintf(stderr,"test_tpz=%s tpz_new=%s,attr_flag=%d\n",tpz,tpz_new,attr_flag);
		
		write_status_file( MULTI_NAS_STATUS, "start" );

			/*eag 动态配置加载start 
			add by liuyu 2011-5-10*/
			
			int dbus_ret = -1;
			dbus_nas_conf nas_conf;
			memset(&nas_conf, 0, sizeof(nas_conf));
			if(1 == def_flag )
				nas_conf.n_default = FLAG_DEFAULT;
			else
				nas_conf.n_default = FLAG_NOT_DEFAULT;
			if(NULL!=plot_id)
				nas_conf.n_plotid = atoi(plot_id);
			nas_conf.n_flag = FLAG_EDIT;
			strncpy(nas_conf.n_attz, tpz, sizeof(nas_conf.n_attz)-1);//传入旧的attz给eag匹配
			strncpy(nas_conf.n_nasid, nasid, sizeof(nas_conf.n_nasid)-1);
			strncpy(nas_conf.n_syntaxis, syntaxis_point, sizeof(nas_conf.n_syntaxis)-1);
			strncpy(nas_conf.n_nastype, nas_type, sizeof(nas_conf.n_nastype)-1);
			strncpy(nas_conf.n_start, start_point, sizeof(nas_conf.n_start)-1);
			strncpy(nas_conf.n_end, end_point, sizeof(nas_conf.n_end)-1);
			//fprintf(stderr,"eag_auto_conf_eag_conf:n_default=%d,n_nasid=%s,syntaxis=%s,n_plotid=%d,n_nastype=%s,n_start=%s,n_end=%s\n",\
			//				nas_conf.n_default,nas_conf.n_nasid,nas_conf.n_syntaxis,nas_conf.n_plotid,nas_conf.n_nastype,nas_conf.n_start,nas_conf.n_end);
			
			ccgi_dbus_init();
			if(NULL != ccgi_dbus_connection)
			{
				dbus_ret = ccgi_dbus_eag_conf_nas(ccgi_dbus_connection, nas_conf);
				if(0 == dbus_ret)
					ShowAlertTwoMsg(search(portal_auth, "edit_nasid_suc"), search(portal_auth, "dbus_eag_conf_succ"));
				else if(ERR_DBUS_REPLY_NULL == dbus_ret)
				{
					//eag is stop,don't need for auto-conf loading
					ShowAlert(search(portal_auth, "edit_nasid_suc"));
				}
				else
				{
					ShowAlertTwoMsg(search(portal_auth, "edit_nasid_suc"), search(portal_auth, "dbus_eag_conf_err"));
				}
			}
			else
			{
				if(check_all_eag_services() == 0)
				{
					//eag is stop,don't need for auto-conf loading
					ShowAlert(search(portal_auth, "edit_nasid_suc"));
				}
				else
					ShowAlertTwoMsg(search(portal_auth, "edit_nasid_suc"), search(portal_auth, "dbus_eag_conf_err"));
			}
			/*eag 动态配置加载end*/			
		}
		else
		{
			ShowAlert(search(portal_public, "input_illegal"));
		}
				
		fprintf( pp, "<script type='text/javascript'>\n" );
		fprintf( pp, "window.location.href='wp_nasid_byvlan.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plot_id);
		fprintf( pp, "</script>\n" );
	}
	return 0;		
}

static int s_editnasid_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	char urlnode[10];
	char attz[128];
	char nas_type[32];
	char nas_start[32];
	char nas_end[32];
	char nas_id[32];
	char nas_syntaxis[32];	
	char tempz[20]={0};
	int flag=1;
	
	memset(urlnode,0,sizeof(urlnode));
	memset(attz,0,sizeof(attz));
	memset(nas_type,0,sizeof(nas_type));
	memset(nas_start,0,sizeof(nas_start));
	memset(nas_end,0,sizeof(nas_end));
	memset(nas_id,0,sizeof(nas_id));
	memset(nas_syntaxis,0,sizeof(nas_syntaxis));
	
	cgiFormStringNoNewlines( "ATTZ", attz, sizeof(attz) );
	cgiFormStringNoNewlines( "PLOTIDZ", urlnode, sizeof(urlnode));
	cgiFormStringNoNewlines( "N_TYPE", nas_type, sizeof(nas_type) );
	cgiFormStringNoNewlines( "N_START", nas_start, sizeof(nas_start) );
	cgiFormStringNoNewlines( "N_END", nas_end, sizeof(nas_end) );
	cgiFormStringNoNewlines( "N_NASID", nas_id, sizeof(nas_id) );
	cgiFormStringNoNewlines( "N_SYNTAXIS", nas_syntaxis, sizeof(nas_syntaxis) );
	
	fprintf(fp,	"<table border=0 width=280 cellspacing=0 cellpadding=0>");
	
	sprintf(tempz,"%s%s",MTN_N,urlnode);

	//fprintf(stderr,"test_flag=%d,tempz=%s\n",flag,tempz);
	
	fprintf(fp, "<tr height=30 align=left>");
    fprintf(fp,"</tr>");
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td width=250>");
	fprintf(fp,"<select name=plot_id disabled>\n");
	if(strcmp(urlnode,"")!=0)
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
	fprintf(fp,"</select>");
	fprintf(fp,"</td>"\
				"</tr>\n");	
	
	fprintf(fp, "<tr height=30 align=left>"\
				"<td width=100>Type:</td>"\
				"<td width=180><select name=nas_type onchange=\"nas_type_change()\">");	
	if(strcmp(nas_type,"vlan")==0)		
		fprintf(fp,"<option value=vlan selected>VLAN</option>");
	else
		fprintf(fp,"<option value=vlan>VLAN</option>");	
	if(strcmp(nas_type,"subintf")==0)	
		fprintf(fp,"<option value=subintf selected>SUBINTF</option>");
	else
		fprintf(fp,"<option value=subintf>SUBINTF</option>");
	if(strcmp(nas_type,"wlan")==0)
		fprintf(fp,"<option value=wlan selected>WLAN</option>");
	else
		fprintf(fp,"<option value=wlan>WLAN</option>");
	if(strcmp(nas_type,"wtp")==0)
		fprintf(fp,"<option value=wtp selected>WTP</option>");
	else
		fprintf(fp,"<option value=wtp>WTP</option>");
	if(strcmp(nas_type,"ipAddress")==0)
		fprintf(fp,"<option value=ipAddress selected>ipAddress</option>");
	else
		fprintf(fp,"<option value=ipAddress>ipAddress</option>");
	
	fprintf(fp,"</select>"\
				"</td>");
	
	fprintf(fp,	"</tr>"\

				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>",search(portal_auth,"start"));
	fprintf(fp,	"<td width=180><input type=text name=start_point value=\"%s\" size=15></td>",nas_start);
	
	fprintf(fp, "</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>",search(portal_auth,"end"));
	fprintf(fp,	"<td width=180><input type=text name=end_point value=\"%s\" size=15></td>",nas_end);

	
	fprintf(fp, "</tr>"\

				"<tr height=30 align=left>"\
				"<td width=100>Nasid:</td>"\
				"<td width=180><input type=text name=nasid value=\"%s\" size=15></td>"\
				"</tr>"\
				
				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>",nas_id,search(portal_auth,"syntaxis"));
	fprintf(fp,	"<td width=180><input type=text value=\"%s\" name=syntaxis_point size=15></td>"\
				"</tr>",nas_syntaxis);				
	fprintf(fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",urlnode);	
	fprintf(fp,"<tr><td colspan=2><input type=hidden name=attz_tmp value=\"%s\"></td></tr>\n",attz);
	fprintf(fp,	"</table>");

	return 0;	
}


/*************************************
检查数字的输入合法性
*************************************/
int check_input_digit_valid(char * vlanid, int min, int max)
{
	if( vlanid == NULL || 0 == strcmp(vlanid, "") 
		||(0 == strncmp(vlanid,"0",strlen("0"))&&strlen(vlanid)>1))
	{
		return -1;
	}
	
	char * src = vlanid;
	int i = 0;
	while(src[i] != '\0')
	{
		if( isdigit(src[i]) )
			i++;
		else
			return -2;
	}
	
	char * endptr = NULL;
	int i_vlan = strtoul( vlanid, &endptr, 0 );

	if( i_vlan < min || i_vlan > max )
	{
		return -3;
	}
	
	return 0;
}

/*************************************
检查vlan id的输入合法性
*************************************/
int check_nasid_valid(char * vlanid)
{
	if( vlanid == NULL )
	{
		return -1;
	}
	
	char * src = vlanid;
	int i = 0;
	while(src[i] != '\0')
	{
		if( isdigit(src[i]) )
			i++;
		else
			return -2;
	}
	
	char * endptr = NULL;
	int i_vlan = strtoul( vlanid, &endptr, 0 );
	
	if( i_vlan < 0 || i_vlan > 512 )
	{
		return -3;
	}
	
	return 0;
}





