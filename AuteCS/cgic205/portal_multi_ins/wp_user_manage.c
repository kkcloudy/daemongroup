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
* wp_user_manage.c
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

#include<sys/socket.h>
#include <sys/un.h>

#include<netinet/in.h>
#include<arpa/inet.h>

#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"
#include "ws_user_manage.h"

#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_eag_conf.h"
#include "ws_dbus_list_interface.h"

/***************************************************************
定义页面要用到的结构体
****************************************************************/
//#define MAX_URL_LEN         256

typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
		
	int formProcessError;
} STPageInfo;

//STOnlineUserInfo search_user;


#define SUBMIT_NAME_USER_MANAGE	"user_mng_submit"
#define DATA_NUM_EVERYPAGE 10


char  search_content[30];	
int typeChoice = 0;

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};	

static int s_usr_mng_prefix_of_page( STPageInfo *pstPageInfo );
static int s_usr_mng_content_of_page( STPageInfo *pstPageInfo );

int cgiMain()
{
	int ret = 0;
	STPageInfo stPageInfo;
	//portal_lcon = get_chain_head("../../htdocs/text/control.txt");
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret = init_portal_container(&(stPageInfo.pstPortalContainer));
	stPageInfo.lpublic = stPageInfo.pstPortalContainer->lpublic;
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);

	DcliWInit();
	ccgi_dbus_init();  
	
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
	fprintf(stderr, "--------------------------------------plotid=%s\n", plotid);
//初始化完毕

	
//处理表单
//	stPageInfo.formProcessError = getUserInput( &(stPageInfo.stUserInput) );
	
	char  url[256];
	memset( url, 0, 256 );

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_USERLIST);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_usr_mng_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_usr_mng_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ACTION, "wp_user_manage.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME_USER_MANAGE );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, "submit_user_manage" );
	
	snprintf( url, sizeof(url), "wp_authentication.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}




static int s_usr_mng_prefix_of_page( STPageInfo *pstPageInfo )
{

	FILE *fp = pstPageInfo->fp;
	
	fprintf(fp, "<style type=text/css>"\
  				"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    			"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
   				"#link{ text-decoration:none; font-size: 12px}"\
			    ".usrlis {overflow-x:hidden;	overflow:auto; width: 790; height: 400px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
   				"</style>");
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
	struct list * portal_lcon = pstPageInfo->pstPortalContainer->lcon;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_lauth = pstPageInfo->lauth;
	
	char downflag[20] = {0};
	char indexstr[10] = {0};
	int index_int = 0;
	int ret = 0;
	
	//////////强制下线///////////////////////
	cgiFormStringNoNewlines( "DOWNRULE", downflag, sizeof(downflag) );
	if( cgiFormSubmitClicked("submit_user_manage") == cgiFormSuccess )
	{
		fprintf( fp,	"<script type='text/javascript'>");
		fprintf( fp,	"window.location.href = 'wp_authentication.cgi?UN=%s';\n", pstPageInfo->encry);
		fprintf( fp,	"</script>");
	}
	
	if( !strcmp(downflag, "delete") )
	{
		cgiFormStringNoNewlines( "INDEX", indexstr, sizeof(indexstr) );
		index_int  = atoi(indexstr);

		ret = eag_kick_user_by_index(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									index_int);
		ShowAlert( search(portal_public, "usr_force_logout") );
	    
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
   		fprintf( cgiOut, "window.location.href='wp_user_manage.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
		fprintf( cgiOut, "</script>\n" );
	}
	
	return 0;	
}

static int s_usr_mng_content_of_page( STPageInfo *pstPageInfo )
{
	int i;
	FILE *fp = pstPageInfo->fp;
	struct list * portal_lpublic	= pstPageInfo->lpublic;
	struct list * portal_lauth 		= pstPageInfo->lauth;
	struct list * portal_lcon 		= pstPageInfo->pstPortalContainer->lcon;
	char menu[21] = {0};
	char i_char[10] = {0};
	int  hour = 0;
	int minute = 0;
	int second = 0;
	char ipstr[32] = "";
	char macstr[36] = "";
	char ap_macstr[36] = "";
	char timestr[32] = "";
	char types[10] = {0};
	instance_parameter *pq = NULL;
	char temp[10] = { 0 };
	 
	unsigned char  macChar[20];
	memset( macChar, 0, 20 );
	
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>" );
	fprintf(fp, "<tr align=left> " );
	fprintf( fp,"</tr><tr hight=10><td colspan=4></td></tr>" );
	
	fprintf( fp,"<tr>"\
  				"<td id=sec1 colspan=4 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(portal_lauth,"auth_user_info"));
  	fprintf(fp, "</tr>");
	
	fprintf(fp,	"<tr style=padding-top:5px><td>");
	fprintf(fp,	"<table border=0 cellspacing=0 cellpadding=0>"\
				"<tr align=left>" );

	fprintf(fp, "<td colspan=4>ID:&nbsp&nbsp<select name='plotid' onchange='plotid_change(this);'>" );
	for (pq=paraHead1;(NULL != pq);pq=pq->next)
	{
		memset(temp,0,sizeof(temp));
		snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
		
		if (strcmp(plotid, temp) == 0)
			fprintf(cgiOut,"<option value='%s' selected>%s</option>\n",temp,temp);
		else	       
			fprintf(cgiOut,"<option value='%s'>%s</option>\n",temp,temp);
	}
	fprintf( fp,"</select>&nbsp&nbsp</td>\n" );
	
	typeChoice = 0;
	memset(types,0,sizeof(types));
	cgiFormStringNoNewlines("TYPE", types, sizeof(types));
	typeChoice = atoi(types);
	
	fprintf(fp, "<td><select name=\"search_type\">");
	
	fprintf(fp, "<option value=0 %s>%s",(typeChoice==0)?"selected=selected":"",search(portal_lauth,"user_index"));
	fprintf(fp,	"<option value=1 %s>%s",(typeChoice==1)?"selected=selected":"",search(portal_lauth,"user_name"));
	fprintf(fp,	"<option value=2 %s >IP %s",(typeChoice==2)?"selected=selected":"",search(portal_lpublic,"addr"));
	fprintf(fp,	"<option value=3 %s>MAC %s",(typeChoice==3)?"selected=selected":"",search(portal_lpublic,"addr"));

	
	fprintf(fp,	"</select></td>"\

				"<td style=padding-left:2px><input type=text name=search_content size=22 value=%s ></td>",search_content);
	fprintf(fp,	"<td style=padding-left:5px><input type=submit name=search_but style=background-image:url(/images/SubBackGif.gif) value=%s></td>",search(portal_lpublic,"search"));
	fprintf(fp,	"<td style=padding-left:2px></td>");
	fprintf(fp,	"</tr>"\
				"</table>");
	fprintf(fp,	"</td></tr>");

	fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_user_manage.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );
	

	fprintf(fp,	"<tr style=padding-top:15px><td>");
	fprintf(fp, "<div class=usrlis><table border=0 width=750 cellspacing=0 cellpadding=0>");
	
	fprintf(fp, "<tr align=left>"\
	"<th width=30><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),"Index");
	fprintf(fp, "<th width=70><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"user_name"));
	fprintf(fp, "<th width=80><font id=%s>IP </font><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lpublic,"menu_thead"),search(portal_lpublic,"addr"));
	fprintf(fp, "<th width=130><font id=%s>MAC </font><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lpublic,"menu_thead"),search(portal_lpublic,"addr"));
	fprintf(fp, "<th width=100><font id=%s>%s</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"on_time"));
	fprintf(fp, "<th width=80><font id=%s>%s(KBytes)</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"up_octets"));
	fprintf(fp, "<th width=80><font id=%s>%s(KBytes)</font></th>",search(portal_lpublic,"menu_thead"),search(portal_lauth,"down_octets"));
	fprintf(fp, "<th width=130><font id=%s>WTP MAC</font></th>",search(portal_lpublic,"menu_thead"));
	fprintf(fp, "<th width=30><font id=%s>VLANID</font></th>",search(portal_lpublic,"menu_thead"));
	fprintf(fp, "<th width=50>&nbsp;</th>");
	fprintf(fp,	"</tr>");
	struct in_addr addrtemp;
 ///////////////////////////////
 	int ret = 0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	eag_userdb_init(&userdb);
	uint32_t  userip = 0;
	struct in_addr addr = {0};
	char usermac[6] = {0};
	int index = 0;
	int check_ret = 0;
	if( cgiFormSubmitClicked("search_but") == cgiFormSuccess )
	{
		typeChoice = 0;
		memset(types,0,sizeof(types));
		cgiFormStringNoNewlines("search_type", types, sizeof(types));
		memset(search_content, 0, 30);
		cgiFormStringNoNewlines("search_content", search_content, 30);
		typeChoice = atoi(types);
		
		if( !strcmp(search_content, "") )
		{
			fprintf( cgiOut, "<script type='text/javascript'>\n" );
	   		fprintf( cgiOut, "window.location.href='wp_user_manage.cgi?UN=%s&plotid=%s&TYPE=%d';\n", pstPageInfo->encry,plotid,typeChoice);
			fprintf( cgiOut, "</script>\n" );
			return 0;
		}
		if( typeChoice == 2 )
		{
			check_ret = Input_IP_address_Check(search_content);
		}
		else if( typeChoice == 3 )
		{
			check_ret = Input_MAC_address_Check(search_content);
		}
		else if( typeChoice == 1 )
		{
			check_ret = Input_User_Name_Check(search_content);
		}
		else if( typeChoice == 0 )
		{
			check_ret = Input_User_Index_Check(search_content);
		}
		else
		{
			ShowAlert("Not pass search type!\n");
		}
		if( check_ret != 0 )
		{
			ShowAlert( search(portal_lauth, "input_char_error") );
			fprintf( cgiOut, "<script type='text/javascript'>\n" );
	   		fprintf( cgiOut, "window.location.href='wp_user_manage.cgi?UN=%s&plotid=%s&TYPE=%d';\n", pstPageInfo->encry,plotid,typeChoice);
			fprintf( cgiOut, "</script>\n" );
			return 0;
		}
		////////start search///////////////////////
		switch(typeChoice)
		{
			case 0:
			index = atoi(search_content);
			if(index>=1 && index <=10240)
			{
				ret = eag_show_user_by_index(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									&userdb,
									index);
			}
				break;
			case 1:
			ret = eag_show_user_by_username(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									&userdb,
									search_content);
				break;
			case 2:
			inet_aton(search_content, &addr);
			userip = ntohl(addr.s_addr);
			ret = eag_show_user_by_userip(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									&userdb,
									userip);
				break;
			case 3:
			ccgi_str2mac(search_content, usermac);
			ret = eag_show_user_by_usermac(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									&userdb,
									usermac);
				break;
		}

		
	}
	else
	{
		ret = eag_show_user_all(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									&userdb);
		
		
	}
	if (0 == ret)
	{
		i = 0;
		list_for_each_entry(user, &(userdb.head), node)
		{
			i++;
			memset(menu,0,21);
			strcpy(menu,"menulist");
			sprintf(i_char,"%d",i);
			strcat(menu,i_char);
			
			memset(ipstr,0,sizeof(ipstr));
			memset(macstr,0,sizeof(macstr));
			memset(ap_macstr,0,sizeof(ap_macstr));
			ccgi_ip2str(user->user_ip, ipstr, sizeof(ipstr));
			sprintf( macstr, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", user->usermac[0],user->usermac[1],user->usermac[2],user->usermac[3],user->usermac[4],user->usermac[5] );
			sprintf( ap_macstr, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", user->apmac[0],user->apmac[1],user->apmac[2],user->apmac[3],user->apmac[4],user->apmac[5] );
			//ccgi_mac2str(user->usermac, macstr, sizeof(macstr), ':');
			//ccgi_mac2str(user->apmac, ap_macstr, sizeof(ap_macstr), ':');
			hour = user->session_time/3600;
			minute = (user->session_time%3600)/60;
			second = user->session_time%60;
			
			memset(timestr,0,sizeof(timestr));
			snprintf(timestr, sizeof(timestr), "%u:%02u:%02u",hour, minute, second);
			fprintf(fp,	"<tr align=left bgcolor=%s height=30>",setclour((i+1)%2) );
			fprintf(fp, "<td>%d</td>",i);
			fprintf(fp, "<td>%s</td>",user->username);
			fprintf(fp, "<td>%s</td>",ipstr);
			fprintf(fp, "<td>%s</td>",macstr);
			fprintf(fp, "<td>%s</td>",timestr);
			fprintf(fp, "<td>%llu</td>",(user->output_octets/1024));
			fprintf(fp, "<td>%llu</td>",(user->input_octets/1024));
			fprintf(fp, "<td>%s</td>",ap_macstr);
			fprintf(fp, "<td>%lu</td>",user->vlanid);
			fprintf(fp,"<td>");
	       	fprintf(fp,	"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(10240-i),menu,menu);
			fprintf(fp,	"<img src=/images/detail.gif>"\
						"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
			fprintf(fp,	"<div id=div1>");
			fprintf(fp,	"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_user_manage.cgi?UN=%s&DOWNRULE=delete&INDEX=%d&plotid=%s&TYPE=%d target=mainFrame>%s</a></div>", pstPageInfo->encry , i ,plotid,typeChoice,search(portal_lauth,"force_user_down"));
		 	fprintf(fp, "</div>"\
			  			"</div>"\
			   			"</div>");
			fprintf(fp,"</td>");
			fprintf(fp, "</tr>");
		}
	}
	fprintf(fp, "</table></div>");
	fprintf(fp, "</td></tr>");
	
	fprintf(fp, "<tr style=padding-top:30px><td>");
	fprintf(fp, "<input type=hidden name=UN value=%s>", pstPageInfo->encry );
	fprintf(fp, "<input type=hidden name=plotid value=%s>", plotid );
	fprintf(fp, "<input type=hidden name=typeChoice value=%d>", typeChoice );
	fprintf(fp,	"</td></tr></table>");

	return 0;	
}
