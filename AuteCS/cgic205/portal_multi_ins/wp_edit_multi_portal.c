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
* wp_edit_multi_portal.c
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

#define SUBMIT_NAME 	"submit_edit_multi_portal"
#define APMAC_TO_URL 0
#define ACIP_TO_URL 1
#define NASID_TO_URL 2
#define USERMAC_TO_URL 3
#define WLANAPMAC_TO_URL 4
#define WLANUSERMAC_TO_URL 5

char *portal_key_type[] = {   /*security type*/
	"Essid",
	"Wlanid",
	"Vlanid",
	"Wtpid",
	"Interface"
};

char *retstr[] = {
	"",
	"checked"
};

//#define HHH(n) ((n)?"checked":"")

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

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};


/***************************************************************
申明回调函数
****************************************************************/
static int s_editMulitPortal_prefix_of_page( STPageInfo *pstPageInfo );
static int s_editMulitPortal_content_of_page( STPageInfo *pstPageInfo );
static int deskey_input_is_legal(const char *str);
static int url_suffix_is_legal(const char *str);


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

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editMulitPortal_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editMulitPortal_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"edit_multi_portal"),WP_EAG_PORTAL);
	sprintf(url,"%s?UN=%s&plotid=%s","wp_edit_multi_portal.cgi",stPageInfo.encry,plotid);
	LB_changelabelUrl_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,url,WP_EAG_PORTAL);

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );


	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	snprintf( url, sizeof(url), "wp_multi_portal.cgi?UN=%s&plotid=%s", stPageInfo.encry, plotid);
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_editMulitPortal_prefix_of_page( STPageInfo *pstPageInfo )
{
	//FILE * fp = pstPageInfo->fp;
	FILE *pp = pstPageInfo->fp;
	
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	char key_word[128] = {0};
	char secstr[20] = {0};
	char urlstrl[128] = {0};
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
		
		
		key_type = strtol(secstr,NULL,10);

		if (PORTAL_KEYTYPE_WLANID == key_type) 
		{
			keyid = atoi(key_word);
			if (keyid == 0 || keyid > 128)
			{
				//vty_out(vty, "%% wlan id is out of range 1~128\n");
				ShowAlert( search(portal_public, "wlan_range") );
				return 0;
			}
		}
		else if (PORTAL_KEYTYPE_VLANID == key_type) 
		{
			keyid = atoi(key_word);
			if(keyid == 0 || keyid > 4096)
			{
				//vty_out(vty, "%% vlan id is out of range 1~4096\n");
				ShowAlert( search(portal_public, "vlanid_range") );
				
				return 0;
			}		
		}
		else if (PORTAL_KEYTYPE_WTPID == key_type)
		{
			keyid = atoi(key_word);
			if(keyid == 0 || keyid > 2048)
			{
				//vty_out(vty, "%% wtp id is out of range 1~2048\n");
				ShowAlert( search(portal_public, "wtpid_range") );
				
				return 0;
			}	
		}
		else if (PORTAL_KEYTYPE_INTF == key_type) 
		{
		if ((strlen(key_word) > 63)||(strlen(key_word) < 1)) {
				ShowAlert( search(portal_public, "interface_range") );
				return 0;
			}
			keystr = (char *)key_word;
		}
		else if (PORTAL_KEYTYPE_ESSID == key_type) 
		{
			if ((strlen(key_word) > 31)||(strlen(key_word) < 1)) {
							ShowAlert( search(portal_public, "essid_range") );
							return 0;
						}
			strncpy(essidtmp,key_word,31);
			keystr = (char *)essidtmp;
		}
		else 
		{
			//vty_out(vty, "%% unknown index type %s\n", secstr);
			ShowAlert( search(portal_public, "hs_param_must") );
			return 0;
		}


		ret = eag_modify_portal_server(ccgi_connection, 
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


		if(0 == strcmp(apmac, "checked"))
		{
			flag[APMAC_TO_URL] = 1;

		} 
		else 
		{
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
		if( (0 != strcmp(urlsuffix, ""))&&(url_suffix_is_legal(urlsuffix)))
		{
			ret = eag_set_portal_server_url_suffix(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									key_type, 
									keyid, 
									keystr, 
									urlsuffix);
		} else if (0 == strcmp(urlsuffix, "")){
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
		fprintf( pp, "window.location.href='wp_multi_portal.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry, plotid);
		fprintf( pp, "</script>\n" );
	}
	return 0;		
}

static int s_editMulitPortal_content_of_page( STPageInfo *pstPageInfo )
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

	char nodez[32] = { 0 };
	int key_type = 0;	
	cgiFormStringNoNewlines( "NODEZ", nodez, sizeof(nodez) );
	key_type = strtol(nodez,NULL,10);
	char attz[64] = { 0 };
	cgiFormStringNoNewlines( "ATTZ", attz, sizeof(attz));
	char *keystr = "";
	unsigned long keyid = 0;
	if (key_type == PORTAL_KEYTYPE_ESSID)
	{
		keystr = (char *)attz;
	}
	else if (key_type == PORTAL_KEYTYPE_WLANID) 
	{
		keyid = atoi(attz);
		if (keyid == 0 || keyid > 128)
		{
			ShowAlert( search(portal_public, "hs_param") );
			return 0;
		}
	}
	else if (key_type == PORTAL_KEYTYPE_VLANID) 
	{
		keyid = atoi(attz);
		if(keyid == 0 || keyid > 4096)
		{
			ShowAlert( search(portal_public, "hs_param") );
			return 0;
		}		
	}
	else if (key_type == PORTAL_KEYTYPE_WTPID)
	{
		keyid = atoi(attz);
		if (keyid == 0 || keyid > 2048)
		{
			ShowAlert( search(portal_public, "hs_param") );
			return 0;
		}
	}
	else if (key_type == PORTAL_KEYTYPE_INTF)
	{
		keystr = (char *)attz;
	}

	int i = 0;

	int ret = EAG_ERR_UNKNOWN;
	struct portal_conf portalconf;
	int pose_flag = -1;
	ret = eag_get_portal_conf(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									&portalconf);
	if(EAG_RETURN_OK == ret)
	{
		for( i=0; i<portalconf.current_num; i++ )
		{
			if(portalconf.portal_srv[i].key_type == key_type)
			{
				switch(portalconf.portal_srv[i].key_type)
				{
					case PORTAL_KEYTYPE_ESSID:
						if(strcmp(portalconf.portal_srv[i].key.essid, keystr) == 0)
							pose_flag = i;		
						break;
					case PORTAL_KEYTYPE_WLANID:
						if(portalconf.portal_srv[i].key.wlanid == keyid)
							pose_flag = i;
						break;
					case PORTAL_KEYTYPE_VLANID:
						if(portalconf.portal_srv[i].key.vlanid == keyid)
							pose_flag = i;
						break;
					case PORTAL_KEYTYPE_WTPID:
						if(portalconf.portal_srv[i].key.wtpid == keyid)
							pose_flag = i;
						break;
					case PORTAL_KEYTYPE_INTF:
						if(strcmp(portalconf.portal_srv[i].key.intf, keystr) == 0)
							pose_flag = i;	
						break;
				}
				if((pose_flag >= 0)&&(pose_flag < portalconf.current_num))
				{
					break;
				}
			}
		}
	}
	if((pose_flag < 0)||(pose_flag >= portalconf.current_num))/*没有匹配的portal server*/
		return 0;

	fprintf(fp,	"<table border=0 width=600 cellspacing=0 cellpadding=0>");
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td>%s</td>",plotid);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_edit_multi_portal.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);	
    fprintf(fp,"</script>\n" );

	//key word type;
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=100>%s</td>","Key Word Type");
	fprintf(fp,"<td>%s</td>",portal_key_type[key_type]);
	fprintf(fp,"</tr>\n");	
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,	"<td width=100>KeyWord</td>");
	if((PORTAL_KEYTYPE_ESSID == key_type)||(PORTAL_KEYTYPE_INTF == key_type))
		fprintf(fp,"<td>%s</td>",keystr);
	else
		fprintf(fp,"<td>%d</td>",keyid);
	
	fprintf(fp,"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>URL</td>"\
				"<td><input type=text name=url size=15 style=\"width:400px;\" value=\"%s\"></td>",portalconf.portal_srv[pose_flag].portal_url);
				fprintf(fp,"</tr>");
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>Domain</td>"\
				"<td><input type=text name=domain size=15 style=\"width:200px;\" value=\"%s\"></td>",portalconf.portal_srv[pose_flag].domain);
				fprintf(fp,"</tr>");		
	
	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"error_offline_port"));
	fprintf(fp, "<td><input type=text name=noteport id=noteport  maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"%d\" style=\"width:200px;\"><font color=red>(1-65535)</font></td>\n",portalconf.portal_srv[pose_flag].ntf_port);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"deskey_for_wlanusermac"));
	fprintf(fp, "<td><input type=text name=wlandeskey size=15 value=\"%s\" style=\"width:200px;\"></td>\n",portalconf.portal_srv[pose_flag].wlanusermac_deskey);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"deskey_for_wlanparameter"));
	fprintf(fp, "<td><input type=text name=deskey size=15 value=\"%s\" style=\"width:200px;\"></td>\n",portalconf.portal_srv[pose_flag].deskey);
	fprintf(fp,"</tr>\n");


	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"HS_ACID"));
	fprintf(fp, "<td><input type=text name=acname id=acname value=\"%s\" style=\"width:200px;\"></td>\n",portalconf.portal_srv[pose_flag].acname);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"add_url_suffix"));
	fprintf(fp, "<td><input type=text name=urlsuffix id=urlsuffix value=\"%s\" style=\"width:200px;\"><font color=red>(%s)</font></td>\n",portalconf.portal_srv[pose_flag].url_suffix, search(portal_auth,"warn_suffix"));
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_acip_to_url"));
	fprintf(fp, "<td><input type=checkbox name=acip size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].acip_to_url]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_nasid_to_url"));
	fprintf(fp, "<td><input type=checkbox name=nasid size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].nasid_to_url]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_apmac_to_url"));
	fprintf(fp, "<td><input type=checkbox name=apmac size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].apmac_to_url]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_usermac_to_url"));
	fprintf(fp, "<td><input type=checkbox name=usermac size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].usermac_to_url]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_wlanapmac"));
	fprintf(fp, "<td><input type=checkbox name=wlanapmac size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].wlanapmac]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_wlanusermac_to_url"));
	fprintf(fp, "<td><input type=checkbox name=wlanusermac size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].wlanusermac]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_firsturl_to_url"));
	fprintf(fp, "<td><input type=checkbox name=firsturl size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].wlanuserfirsturl]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=115>%s</td>",search(portal_auth,"add_wlanparameter_to_url"));
	fprintf(fp, "<td><input type=checkbox name=wlanparameter size=15 value=checked %s></td>\n",retstr[portalconf.portal_srv[pose_flag].wlanparameter]);
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr>");
	fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"wisprlogin"));
	fprintf(fp,"<td><select name=wisprlogin>");
	fprintf(fp,"<option value='%s' %s>%s</option>","disable", (WISPR_URL_NO == portalconf.portal_srv[pose_flag].wisprlogin)?"selected":"", "disable");
	fprintf(fp,"<option value='%s' %s>%s</option>","http", (WISPR_URL_HTTP == portalconf.portal_srv[pose_flag].wisprlogin)?"selected":"", "http");
	fprintf(fp,"<option value='%s' %s>%s</option>","https", (WISPR_URL_HTTPS == portalconf.portal_srv[pose_flag].wisprlogin)?"selected":"", "https");
	fprintf(fp,"</select></td>");
	fprintf(fp,"</tr>");

	fprintf(fp,	"</table>");
	fprintf(fp,"<input type=hidden name=UN value=\"%s\">",pstPageInfo->encry);				
	fprintf(fp,"<input type=hidden name=plotid value=\"%s\">",plotid);
	fprintf(fp,"<input type=hidden name=key_type value=\"%d\">",key_type);
	if((PORTAL_KEYTYPE_ESSID == key_type)||(PORTAL_KEYTYPE_INTF == key_type))
		fprintf(fp,"<input type=hidden name=key_word value=\"%s\">",keystr);
	else
		fprintf(fp,"<input type=hidden name=key_word value=\"%d\">",keyid);
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



