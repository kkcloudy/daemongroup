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
* wp_add_multi_portal.c
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
#include <string.h>
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

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};


#ifndef ADD_MULTI_PORTAL  
#define ADD_MULTI_PORTAL

#define SUBMIT_NAME 	"submit_add_multi_portal"

#define MULTI_PORTAL_CONF_PATH "/opt/services/conf/multiportal_conf.conf"

#define MAX_PORTAL_WEBURL_LEN  128
#define MAX_PORTAL_ACNAME_LEN 32
#define MAX_PORTAL_DOMAIN_LEN   64
#define MAX_PORTAL_URL_SUFFIX_LEN 64

#endif
#define APMAC_TO_URL 0
#define ACIP_TO_URL 1
#define NASID_TO_URL 2
#define USERMAC_TO_URL 3
#define WLANAPMAC_TO_URL 4
#define WLANUSERMAC_TO_URL 5
int eag_portal_policy_get_num(int policy_id);
int port_input_is_legal(const char *str);
int weburl_input_is_legal(const char *str);
int acname_input_is_legal(const char *str);
int domain_input_is_legal(const char *str);
int url_suffix_add_input_is_legal(const char *src,int srclen,char *dst,int dstsize);
static int deskey_input_is_legal(const char *str);
static int url_suffix_is_legal(const char *str);
static int ifch_func(const char *inputstr);


static int get_portal_param_by_portal_url
(
char *portal_url, 
char *url_prefix,
int url_prefix_len,
char *ip_domain,
int *ip_domain_len,
int *port, 
char * web_page,
int web_page_len
);	


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
	int p_id;
	
} STPageInfo;



/***************************************************************
申明回调函数
****************************************************************/
static int s_addMulitPortal_prefix_of_page( STPageInfo *pstPageInfo );
static int s_addMulitPortal_content_of_page( STPageInfo *pstPageInfo );


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
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret = init_portal_container(&(stPageInfo.pstPortalContainer));
	stPageInfo.lpublic=stPageInfo.pstPortalContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");

	DcliWInit();
	ccgi_dbus_init();  
	
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


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_PORTAL);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addMulitPortal_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addMulitPortal_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"add_multi_portal"),6);
	sprintf(url,"%s?UN=%s","wp_add_multi_portal.cgi",stPageInfo.encry);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,6);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_multi_portal.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_addMulitPortal_prefix_of_page( STPageInfo *pstPageInfo )
{
	//FILE * fp = pstPageInfo->fp;
	FILE *pp = pstPageInfo->fp;
	
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	char key_word[128] = {0};
	char secstr[20] = {0};
	char urlstrl[256] = {0};
	char portstr[10] = {0};
	char domain[64] = {0};/*char domain[20] = {0};*/
	int port_int = 0;
	int ret = 0;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	char essidtmp[128] = {0};
	char acname[128] = {0};
	char apmac[10] = {0};
	char acip[10] = {0};
	char nasid[10] = {0};
	char usermac[10] = {0};
	char wlanapmac[10] = {0};
	char wlanusermac[10] = {0};
	char wlandeskey[10] = {0};
	char urlsuffix[64] = {0};
	char firsturl[10];
	char wlanparameter[10];
	char deskey[10];
	char wisprlogin[10];
	int flag[10] = {0};
	int mac_server_ip = 0;
	int mac_server_port = 0;

	if( (cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess) && (pstPageInfo->iUserGroup == 0))
	{
		
		cgiFormStringNoNewlines( "key_word", key_word, sizeof(key_word) );
		cgiFormStringNoNewlines( "key_type", secstr, sizeof(secstr) );
		cgiFormStringNoNewlines( "url", urlstrl, sizeof(urlstrl) );
		cgiFormStringNoNewlines( "noteport", portstr, sizeof(portstr) );
		cgiFormStringNoNewlines( "domain", domain, sizeof(domain) );
		cgiFormStringNoNewlines( "acname", acname, sizeof(acname) );
		cgiFormStringNoNewlines( "apmac", apmac, sizeof(apmac) );
		cgiFormStringNoNewlines( "acip", acip, sizeof(acip) );
		cgiFormStringNoNewlines( "nasid", nasid, sizeof(nasid) );
		cgiFormStringNoNewlines( "usermac", usermac, sizeof(usermac) );
		cgiFormStringNoNewlines( "wlanapmac", wlanapmac, sizeof(wlanapmac) );
		cgiFormStringNoNewlines( "wlanusermac", wlanusermac, sizeof(wlanusermac) );
		cgiFormStringNoNewlines( "wlandeskey", wlandeskey, sizeof(wlandeskey) );
		cgiFormStringNoNewlines( "urlsuffix", urlsuffix, sizeof(urlsuffix) );
		cgiFormStringNoNewlines( "firsturl",firsturl,sizeof(firsturl));
		cgiFormStringNoNewlines( "wlanparameter",wlanparameter,sizeof(wlanparameter));
		cgiFormStringNoNewlines( "deskey",deskey,sizeof(deskey));
		cgiFormStringNoNewlines( "wisprlogin", wisprlogin, sizeof(wisprlogin) );


		port_int = atoi(portstr);
		if(port_int < 1 || port_int > 65535)
		{
			ShowAlert( search(portal_public, "hs_param_must") );
			return 0;
		}
		if ((strlen(urlstrl)< 1)) {
						ShowAlert( search(portal_public, "hs_param_must") );
						return 0;
		}
		
		if (strncmp(secstr, "wlanid", strlen(secstr)) == 0) 
		{
			key_type = PORTAL_KEYTYPE_WLANID;
			keyid = atoi(key_word);
			if (keyid == 0 || keyid > 128)
			{
				//vty_out(vty, "%% wlan id is out of range 1~128\n");
				ShowAlert( search(portal_public, "wlan_range") );
				return 0;
			}
		}
		else if (strncmp(secstr, "vlanid", strlen(secstr)) == 0) 
		{
			key_type = PORTAL_KEYTYPE_VLANID;
			keyid = atoi(key_word);
			if(keyid == 0 || keyid > 4096)
			{
				//vty_out(vty, "%% vlan id is out of range 1~4096\n");
				ShowAlert( search(portal_public, "vlanid_range") );
				
				return 0;
			}		
		}
		else if (strncmp(secstr, "wtpid", strlen(secstr)) == 0)
		{
			keyid = atoi(key_word);
			key_type = PORTAL_KEYTYPE_WTPID;
			if(keyid == 0 || keyid > 2048)
			{
				//vty_out(vty, "%% wtp id is out of range 1~2048\n");
				ShowAlert( search(portal_public, "wtpid_range") );
				
				return 0;
			}	
		}
		else if (strncmp(secstr, "interface", strlen(secstr)) == 0) 
		{
			key_type = PORTAL_KEYTYPE_INTF;
			if ((strlen(key_word) > 63)||(strlen(key_word) < 1)) {
				ShowAlert( search(portal_public, "interface_range") );
				return 0;
			}
			keystr = (char *)key_word;
		}
		else if (strncmp(secstr, "essid", strlen(secstr)) == 0) 
		{
			key_type = PORTAL_KEYTYPE_ESSID;
			if ((strlen(key_word) > 31)||(strlen(key_word) < 1)) {
				ShowAlert( search(portal_public, "essid_range") );
				return 0;
			}
			
			strncpy(essidtmp, key_word, 31);
			keystr = (char *)essidtmp;
		}
		else 
		{
			//vty_out(vty, "%% unknown index type %s\n", secstr);
			ShowAlert( search(portal_public, "hs_param_must") );
			return 0;
		}

		
		#if 0 
		/////////////处理数据///////////////////
		if(strcmp(plot_id,"")==0)
		  cgiFormStringNoNewlines("plotid",plot_id, sizeof(plot_id));

		policy_id = atoi(plot_id);
		if (eag_portal_policy_get_num(policy_id) >= MAX_MULTIPORTAL_OPTION_NUM)
		{
			ShowAlert( search(portal_auth, "portal_num_out_of_range") );
			return 0;
		}
		//check length of SSID
		if(strlen(key_word) > 32)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_too_long") );
			return 0;
		}
		if(strlen(key_word) == 0)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_null") );
			return 0;
		}
		int i = 0;
		while( key_word[i] != '\0')
		{
			if( (key_word[i] >= '0' && key_word[i] <= '9') ||	(key_word[i] >= 'a' && key_word[i] <= 'z') ||
				(key_word[i] >= 'A' && key_word[i] <= 'Z') || key_word[i] == '-' || key_word[i] == '_' ||
				key_word[i] == '.' )
			{
				i++;
			}else
			{
				ShowAlert( search(portal_auth, "multi_portal_name_error") );
				return 0;
			}
		}


		
		/* add by chensheng on 2010-3-18 */
		if (acname != NULL && !acname_input_is_legal(acname)){
			ShowAlert(search(portal_auth, "multi_portal_acname_error"));
			return 0;
		}
		if (domain != NULL && !domain_input_is_legal(domain)){
			ShowAlert(search(portal_auth, "multi_portal_domain_error"));
			return 0;
		}
		if (noteport != NULL && strcmp(noteport, "") != 0 && !port_input_is_legal(noteport)){
			ShowAlert(search(portal_auth, "multi_portal_notice_port_error"));
			return 0;
		}

		if (mpport != NULL && strcmp(mpport, "") != 0 && !port_input_is_legal(mpport)){
			ShowAlert(search(portal_auth, "multi_portal_port_error"));
			return 0;
		}

		if (suffix_temp != NULL && 
			!url_suffix_add_input_is_legal(suffix_temp,strlen(suffix_temp),
								url_suffix,sizeof(url_suffix)))
		{
			ShowAlert(search(portal_auth, "multi_portal_url_suffix_error"));
			return 0;
		}
		#endif
		
		ret = eag_add_portal_server(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,					
									key_type,
									keyid,
									keystr,
									urlstrl, 
									port_int,
									domain,
									mac_server_ip,
									mac_server_port);

		if (EAG_ERR_PORTAL_ADD_SRV_MAX_NUM == ret) {
			ShowAlert(search(portal_auth, "portal_num_out_of_range"));
			return 0;
		}

		if(0 == strcmp(apmac, "checked"))
		{
			flag[APMAC_TO_URL] = 1;

		} else {
			flag[APMAC_TO_URL] = 0;
		}


		ret = eag_set_portal_server_apmac_to_url(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid, 
									keystr, 
									flag[APMAC_TO_URL]);
		if(0 == strcmp(acip, "checked"))
		{
			flag[ACIP_TO_URL] = 1;	
		} else {
			flag[ACIP_TO_URL] = 0;
		}
		ret = eag_set_portal_server_acip_to_url(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									key_type,
									keyid, 
									keystr,
									flag[ACIP_TO_URL]);
		
		
		if(0 == strcmp(nasid, "checked"))
		{
			flag[NASID_TO_URL] = 1;
		} else {
			flag[NASID_TO_URL] = 0;
		}
			ret = eag_set_portal_server_nasid_to_url(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									key_type, 
									keyid, 
									keystr,
									flag[NASID_TO_URL]);
		
		if(0 == strcmp(usermac, "checked"))
		{
			flag[USERMAC_TO_URL] = 1;
		} else {
			flag[USERMAC_TO_URL] = 0;
		}
			ret = eag_set_portal_server_usermac_to_url(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									key_type, 
									keyid, 
									keystr,
									flag[USERMAC_TO_URL]);
		
		if(0 == strcmp(wlanapmac, "checked"))
		{
			flag[WLANAPMAC_TO_URL] = 1;	
		} else {
			flag[WLANAPMAC_TO_URL] = 0;
		}
			ret = eag_set_portal_server_wlanapmac(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid, 
									keystr, 
									flag[WLANAPMAC_TO_URL]);
		
		
		if(0 != strcmp(acname,""))
		{
			ret = eag_set_portal_server_acname(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									key_type,
									keyid,
									keystr,
									acname);
		}
		if (0 != strcmp(wlandeskey, "") && 0 != strcmp(wlanusermac, "checked")){
				ShowAlert(search(portal_auth, "multi_portal_wlandeskey_not_need"));
				goto jump;
			} else if (0 == strcmp(wlandeskey, "") && 0 == strcmp(wlanusermac, "checked")) {
				ShowAlert(search(portal_auth, "multi_portal_wlandeskey_no_config"));
				goto jump;
			} else if (!deskey_input_is_legal(wlandeskey)) {
				ShowAlert(search(portal_auth, "multi_portal_wlandeskey_error"));
				goto jump;
			}
		if( 0 == strcmp(wlanusermac, "checked"))
		{
			flag[WLANUSERMAC_TO_URL] = 1;
		} else {
			flag[WLANUSERMAC_TO_URL] = 0;
		}
		ret = eag_set_portal_server_wlanusermac(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid,
									keystr, 
									flag[WLANUSERMAC_TO_URL], 
									wlandeskey);
		if((0 != strcmp(urlsuffix, ""))&&(url_suffix_is_legal(urlsuffix)))
		{
			ret = eag_set_portal_server_url_suffix(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid, 
									keystr, 
									urlsuffix);
		} else if (0 == strcmp(urlsuffix, ""))
		{
			ret = eag_set_portal_server_url_suffix(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,  
									key_type, 
									keyid, 
									keystr, 
									urlsuffix);

		
		}
		else {
			ShowAlert(search(portal_auth, "invalid_urlsuffix"));
			goto jump;

		}
		if ( 0 == strcmp(firsturl, "checked")) {
			ret = eag_set_portal_server_wlanuserfirsturl(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid,
									keystr, 
									1);
		} else {
			ret = eag_set_portal_server_wlanuserfirsturl(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid,
									keystr, 
									0);
		}

		if (0 != strcmp(deskey, "") && 0 != strcmp(wlanparameter, "checked")){
			ShowAlert(search(portal_auth, "multi_portal_deskey_not_need"));
			goto jump;
		} else if (0 == strcmp(deskey, "") && 0 == strcmp(wlanparameter, "checked")) {
			ShowAlert(search(portal_auth, "multi_portal_deskey_no_config"));
			goto jump;
		} else if (!deskey_input_is_legal(deskey)) {
			ShowAlert(search(portal_auth, "multi_portal_deskey_error"));
			goto jump;
		}
		if ( 0 == strcmp(wlanparameter, "checked")) {
			ret = eag_set_portal_server_wlanparameter(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid,
									keystr, 
									1,
									deskey);
		} else {
			ret = eag_set_portal_server_wlanparameter(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid,
									keystr, 
									0,
									deskey);
		}

		
		if ( 0 != strcmp(wisprlogin, "disable")) {
			ret = eag_set_portal_server_wisprlogin(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid,
									keystr, 
									1,
									wisprlogin);
		} else {
			ret = eag_set_portal_server_wisprlogin(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid,
									keystr, 
									0,
									wisprlogin);
		}
jump:		
		fprintf( pp, "<script type='text/javascript'>\n" );
		fprintf( pp, "window.location.href='wp_multi_portal.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
		fprintf( pp, "</script>\n" );

		
		//fprintf(stderr,"111");
	}
	return 0;		
}

static int s_addMulitPortal_content_of_page( STPageInfo *pstPageInfo )
{
  fprintf( cgiOut, "<style type=text/css>.a3{width:30px;border:0px; text-align:center}</style>" );
  fprintf( cgiOut, "<script language=javascript src=/ip.js></script>\n" );
  fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );
    
  fprintf(cgiOut,"<script language=javascript src=/ip.js>\n"\
  	"</script>\n"\
  	"<script type=\"text/javascript\">\n"\
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
		   "}");
  fprintf(cgiOut,"function mysubmit(){\n"\
  			"var username=document.all.a_name.value;\n"\
 			"var pass1=document.all.a_pass1.value;\n"\
 			"var pass2=document.all.a_pass2.value;\n"\
 			"if(username==\"\")\n"\
 				"{\n"\
					"alert(\"user name is empty!\");\n"\
					"window.event.returnValue = false;\n"\
					"return false;\n"\
  					"}\n"\
  			"if(pass1==\"\")"\
 			"{\n"\
					"alert(\"password is empty!\");\n"\
					"window.event.returnValue = false;\n"\
					"return false;\n"\
  			"}\n"\
 			"if(pass1!=pass2)"\
  			"{\n"\
  				"alert(\"%s\");\n"\
  				"window.event.returnValue = false;\n"\
  				"return false;\n"\
  				"}\n"\
  		  "var value1 = document.all.ip1.value;\n"\
		  "var value2 = document.all.ip2.value;\n"\
		  "var value3 = document.all.ip3.value;\n"\
		  "var value4 = document.all.ip4.value;\n"\
		  "if(value1==\"\" ||value2==\"\" ||value3==\"\" ||value4==\"\"){\n"\
		  "alert(\"%s\");\n"\
		  "window.event.returnValue = false;\n"\
		  "return false;\n"\
		  "}\n"\
		  "if( (document.all.portal_port.value-0).toString() == 'NaN' || document.all.portal_port.value-0 > 65535 || document.all.portal_port.value-0 < 0 )\n"\
		  "{\n"\
		  "		alert(\"Port num is error!\");\n"\
	 			 "window.event.returnValue = false;\n"\
			"		return false;\n"\
			"}\n"\
		  "}","pass_incon","ip_error");

  fprintf(cgiOut,"</script>\n");
	FILE * fp = pstPageInfo->fp;
	//struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	struct list * portal_public = pstPageInfo->lpublic;

	int i = 0;
	

	fprintf(fp,	"<table border=0 width=600 cellspacing=0 cellpadding=0>");
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td>");
	fprintf(fp,"<select name=plotid onchange=plotid_change(this)>\n");
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
	fprintf(fp,"</select>");
	fprintf(fp,"</td>"\
				"</tr>\n");

	fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_add_multi_portal.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);	
    fprintf(fp,"</script>\n" );

	//key word type;
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=100>%s</td>","Key Word Type");
	fprintf(fp,"<td>");
	fprintf(fp,"<select name=key_type>\n");
	fprintf(fp,"<option value='%s'>%s</option>","essid","Essid");
	fprintf(fp,"<option value='%s'>%s</option>","vlanid","Vlanid");
	fprintf(fp,"<option value='%s'>%s</option>","interface","Interface");	
	fprintf(fp,"<option value='%s'>%s</option>","wlanid","Wlanid");	
	fprintf(fp,"<option value='%s'>%s</option>","wtpid","Wtpid");	
	fprintf(fp,"</select>");
	fprintf(fp,"</td></tr>\n");	

	fprintf(fp,"<tr height=30 align=left>");
	#if 0 
	if(cflag==0)
	{
		cq=c_head.next;
		while(cq !=NULL)
		{
			if ('\0' != cq->p_type[0])
			{
				fprintf(fp,	"<td width=100>%s</td>",cq->p_type);				
			}else
			{
				fprintf(fp,	"<td width=100>KeyWord</td>");
			}
			break;
			cq=cq->next;
		}
	}
	#endif
	fprintf(fp,	"<td width=100>KeyWord</td>");
	fprintf(fp,"<td><input type=text name=key_word size=15 style=\"width:200px;\"></td>");
	
	fprintf(fp,"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>URL</td>"\
				"<td><input type=text name=url size=15 style=\"width:400px;\"></td>"\
				"</tr>");
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>Domain</td>"\
				"<td><input type=text name=domain size=15 style=\"width:200px;\"></td>"\
				"</tr>");		
	
	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"error_offline_port"));
	fprintf(fp, "<td><input type=text name=noteport id=noteport  maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\" style=\"width:200px;\"><font color=red>(1-65535)</font></td>\n");
	fprintf(fp,"</tr>\n");
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>%s</td>"\
				"<td><input type=text name=wlandeskey size=15 style=\"width:200px;\"></td>"\
				"</tr>", search(portal_auth,"deskey_for_wlanusermac"));
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>%s</td>"\
				"<td><input type=text name=deskey size=15 style=\"width:200px;\"></td>"\
				"</tr>", search(portal_auth,"deskey_for_wlanparameter"));

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"HS_ACID"));
	fprintf(fp, "<td><input type=text name=acname id=acname value=\"\" style=\"width:200px;\"></td>\n");
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"add_url_suffix"));
	fprintf(fp, "<td><input type=text name=urlsuffix id=urlsuffix value=\"\" style=\"width:200px;\"><font color=red>(%s)</font></td>\n",search(portal_auth,"warn_suffix") );
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>"\
				 "<td width=115>%s</td>"\
				 "<td><input type=checkbox name=acip value=checked size=15></td>"\
				 "</tr>",search(portal_auth,"add_acip_to_url"));
	fprintf(fp,"<tr height=30 align=left>"\
				 "<td width=115>%s</td>"\
				 "<td><input type=checkbox name=nasid value=checked size=15></td>"\
				 "</tr>",search(portal_auth,"add_nasid_to_url"));

     
	fprintf(fp,"<tr height=30 align=left>"\
				 "<td width=115>%s</td>"\
				 "<td><input type=checkbox name=apmac value=checked size=15></td>"\
				 "</tr>",search(portal_auth,"add_apmac_to_url"));
	fprintf(fp,"<tr height=30 align=left>"\
				 "<td width=115>%s</td>"\
				 "<td><input type=checkbox name=usermac value=checked size=15></td>"\
				 "</tr>",search(portal_auth,"add_usermac_to_url"));
	fprintf(fp,"<tr height=30 align=left>"\
				 "<td width=115>%s</td>"\
				 "<td><input type=checkbox name=wlanapmac value=checked size=15></td>"\
				 "</tr>",search(portal_auth,"add_wlanapmac"));
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=115>%s</td>"\
				"<td><input type=checkbox name=wlanusermac value=checked size=15></td>"\
				"</tr>",search(portal_auth, "add_wlanusermac_to_url"));	
	fprintf(fp,"<tr height=30 align=left>"\
				 "<td width=115>%s</td>"\
				 "<td><input type=checkbox name=firsturl value=checked size=15></td>"\
				 "</tr>",search(portal_auth,"add_firsturl_to_url"));
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=115>%s</td>"\
				"<td><input type=checkbox name=wlanparameter value=checked size=15></td>"\
				"</tr>",search(portal_auth, "add_wlanparameter_to_url"));	
	fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"wisprlogin"));
		fprintf(fp,"<td><select name=wisprlogin>");
		fprintf(fp,"<option value='%s'>%s</option>","disable", "disable");
		fprintf(fp,"<option value='%s'>%s</option>","http","http");
		fprintf(fp,"<option value='%s'>%s</option>","https","https");
		fprintf(fp,"</select></td>");
		fprintf(fp,"</tr>");
	
	fprintf(fp,	"</table>");
	fprintf(fp,"<input type=hidden name=UN value=\"%s\">",pstPageInfo->encry);				
	fprintf(fp,"<input type=hidden name=plotid value=\"%s\">", plotid);				
	return 0;
}

static int url_suffix_is_legal(const char *str)
{
	if(strlen(str) > 63){
		return 0;
	}
	if(NULL == strchr(str, '=') ){
		return 0;
	}
	if(NULL != strchr(str, ' ')){
		return 0;
	}
	
	return 1;

}

static int
deskey_input_is_legal(const char *str)
{
	if (strlen(str) > 8) {
		return 0;
	}
	if (0 == strlen(str)) {
		return 1;
	}
#if 0	
	while ('\0' != *str) {
		if (*str < '0' || *str > '9') {
			return 0;
		}
		str++;
	}
#endif
	return 1;
}

int port_input_is_legal(const char *str)
{
	const char *p = NULL;
	int port;
	
	if (NULL == str || '\0' == str[0] || '0' == str[0])
		return 0;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;
		
	port = atoi(str);
	if (port < 1 || port > 65535)
		return 0;
	
	return 1;
}

int weburl_input_is_legal(const char *str)
{
	if (NULL == str)
		return 1;

	if (strlen(str) > MAX_PORTAL_WEBURL_LEN -1)
		return 0;
	
	return 1;
}

int acname_input_is_legal(const char *str)
{
	if (NULL == str)
		return 1;

	if (strlen(str) > MAX_PORTAL_ACNAME_LEN -1)
		return 0;
	
	return 1;
}

int domain_input_is_legal(const char *str)
{
	if (NULL == str)
		return 1;

	if (strlen(str) > MAX_PORTAL_DOMAIN_LEN -1)
		return 0;
	
	return 1;
}

int url_suffix_add_input_is_legal(const char *src,int srclen,char *dst,int dstsize)
{	
	if (NULL == src ||0==srclen)
		return 1;
	if (0 != strncmp(src,"&",strlen("&"))) {
		snprintf(dst, dstsize-1, "&%s",src);
	} else 
		snprintf(dst, dstsize-1, "%s",src);

	if (strlen(dst) > MAX_PORTAL_URL_SUFFIX_LEN -1) {
		return 0;
	}
	return 1;
}

static int get_portal_param_by_portal_url
(
char *portal_url, 
char *url_prefix,
int url_prefix_len,
char  *ip_addr_domain, 
int * ip_addr_domain_len,
int *port, 
char * web_page,
int web_page_len
)
	{
		if (	NULL == portal_url	|| NULL == ip_addr_domain 
			||	NULL == port		|| NULL == web_page
			||	NULL == url_prefix	|| 0 >= web_page_len
			||	0 >= url_prefix_len)

	{
		//log_err("param err!\n");
		return -1;
	}
	
//	unsigned int ip_part_value = -1;
//	int ip_part_num = -1;
	int i = 0;
	
	char *p_url = NULL;	
	p_url = portal_url;
	
	if ( 0 == strncmp(p_url,"http://",strlen("http://")))
	{
		strncpy(url_prefix,"http://", url_prefix_len);
		p_url += strlen("http://");
	}
	else if( 0 == strncmp(p_url,"https://",strlen("https://")))
	{
		strncpy(url_prefix,"https://", url_prefix_len);
		p_url += strlen("https://");
	}
	else
	{
		//log_err("url err! url must start as http:// or https:// \n");
		return -1;
	}

#if  0
	/* get ip */
	ip_part_value = -1;
	ip_part_num = 1;
	*ip_addr = 0;
	
	for(i=0; i<17; i++)
	{
		if('\0' == *p_url)
		{
			//log_err("url ip err!\n");
			return -1;
		}
		else if ('/' == *p_url || ':' == *p_url)
		{
			if (0 <= ip_part_value && 255 >= ip_part_value && 4 == ip_part_num)
			{
				/* success */
				*ip_addr += ip_part_value;
				break;
			}
			else
			{
				//log_err("url ip err!\n");
				return -1;
			}
		}
		else if ('.' == *p_url)
		{
			if (		((1 == ip_part_num && 0 < ip_part_value) || (1 < ip_part_num && 0 <= ip_part_value))
					&&	255 >= ip_part_value
					&&	4 > ip_part_num)
			{
				/* legal */
				*ip_addr += ip_part_value << ((4-ip_part_num)*8);
				ip_part_value = -1;
				ip_part_num++;
			}
			else
			{
				//log_err("url ip err!\n");
				return -1;
			}
		}
		else if ('0' <= *p_url || '9' >= *p_url)
		{
			if (-1 == ip_part_value)
			{
				ip_part_value = 0;
			}
			else
			{
				ip_part_value *= 10;
			}
			
			ip_part_value += (*p_url - '0');
		}
		else
		{
			//log_err("url ip err!\n");
			return -1;
		}
		p_url++;
	}
#endif	
		char   url_temp1[256]={0};
		char   url_temp2[256]={0};
		char * temp_url1=NULL;
		temp_url1=p_url;
		int  url_temp_len=0;
		strncpy(url_temp1,p_url,sizeof(url_temp1)-1);
		strncpy(url_temp2,p_url,sizeof(url_temp2)-1);

		for(i=0;i<strlen(url_temp2);i++)
		{

			 if( ':' ==url_temp2[i] ||'/'==url_temp2[i])
			{
				//log_dbg("bbbb p_url=%s,	url_temp2=%s \n",p_url,url_temp2);
				break;
			}
			else	if (('0' <= url_temp2[i] && url_temp2[i] <= '9' ) ||( 'a'<=url_temp2[i] && url_temp2[i] <='z') || ( 'A'<=url_temp2[i] && url_temp2[i] <= 'Z') || '.' == url_temp2[i] )
			{
				p_url++;
				//log_dbg("p_url=%p\n",p_url);
				url_temp_len++;
			}
			else 
			{
				//log_dbg("input parameter  error !!");
				return  -1;
			}	

		}
		//log_dbg("p_url=%s,  url_temp_len=%d\n",p_url,url_temp_len);
		strncpy(url_temp1,temp_url1,url_temp_len);
		strncpy(ip_addr_domain,temp_url1,url_temp_len);
		*ip_addr_domain_len=url_temp_len;
		url_temp1[url_temp_len]='\0';
		//log_dbg("the seperate url=%s,ip_addr_domain_len=%d\n",url_temp1,*ip_addr_domain_len);
		
	/* get port */
	*port = 0;
	if (':' == *p_url)
	{
		p_url++;
		while (NULL != p_url && '0' <= *p_url && '9' >= *p_url)
		{
			*port *= 10;
			*port += (*p_url - '0');			
			p_url++;
		}

		if (65535 < *port || 0 >= *port)
		{
				//log_err("url port err!\n");
				return -1;
		}		
	}
	else if ('/' == *p_url)
	{
		*port = 80;		
	}
	else
	{
		//log_err("url port err!\n");
		return -1;
	}
	
	/* get web page */
	if ('/' != *p_url)	
	{
		//log_err("web page err!\n");
		return -1;
	}
	else
	{		
		strncpy(web_page,p_url,web_page_len);
	}
	
	return 1;
}

int eag_portal_policy_get_num(int policy_id)
{
	char tempz[20] = "";
	int flag = -1;
	struct st_portalz chead = {0};
	int num = 0;
	
	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTP_N, policy_id);
	memset(&chead, 0, sizeof(chead));

	if (access(MULTI_PORTAL_F, 0) != 0)
	{
		return num;
	}
	flag = read_portal_xml(MULTI_PORTAL_F, &chead, &num, tempz);
	if(0 == flag && num > 0)
	{
		Free_portal_info(&chead);
	}

	return num;
}

