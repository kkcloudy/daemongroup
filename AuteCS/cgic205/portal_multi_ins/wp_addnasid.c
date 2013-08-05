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
* wp_addnasid.c
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
#include "ws_init_dbus.h"
#include "ws_eag_auto_conf.h"

#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_dcli_vrrp.h"
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


int eag_nas_policy_get_num(int policy_id);
static int interface_input_is_legal(const char  *intf_str);


/***************************************************************
申明回调函数
****************************************************************/
static int s_addnasid_prefix_of_page( STPageInfo *pstPageInfo );
static int s_addnasid_content_of_page( STPageInfo *pstPageInfo );


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
    system("echo \"1\" > /opt/services/status/nasidvlan_status.status");
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

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addnasid_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_addnasid_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"add_nasid"),5);
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

static int s_addnasid_prefix_of_page( STPageInfo *pstPageInfo )
{
	FILE * pp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	
	char plot_id[30],nas_type[MAX_INDEX_LEN], value_point[128], nasid[MAX_INDEX_LEN],syntaxis_point[MAX_INDEX_LEN];
	int  check_ret2 = 0, check_ret3 = 0, check_ret4 = 0, check_ret5 = 0;
	int ret = 0;
	char *tmp = NULL;
	char *tmp_1 = NULL;
	//////////memset////////////////
	memset( plot_id,0,sizeof(plot_id));
	memset( nas_type, 0, sizeof(nas_type) );
	memset( value_point, 0, sizeof(value_point) );
	memset( nasid, 0, sizeof(nasid) );
	memset( syntaxis_point, 0, sizeof(syntaxis_point) );

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
		cgiFormStringNoNewlines("nas_type",nas_type, sizeof(nas_type) );
		cgiFormStringNoNewlines("value_point",value_point, sizeof(value_point) );
		cgiFormStringNoNewlines("nasid",nasid, sizeof(nasid) );
		cgiFormStringNoNewlines("syntaxis_point",syntaxis_point, sizeof(syntaxis_point) );

		struct nasid_map_t nasidmap;
		memset(&nasidmap, 0, sizeof(struct nasid_map_t));
		unsigned long key_type = 0;
		unsigned long keywd_1 = 0;
		unsigned long keywd_2 = 0;
		char *tmp_ip = NULL;
		char *ip = NULL;
		char ip_1[24] = {0};
		char ip_2[24] = {0};

		if (0 == strncmp(nas_type, "wlanid", strlen(nas_type))) 
		{
			tmp=(char *)value_point;
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
            		if (keywd_1 == 0 || keywd_2 > 128)
                              {
                                        ShowAlert(search(portal_public, "input_overflow"));
                                        return 0;
                              }
			nasidmap.key.wlanidrange.id_begin = keywd_1;
			nasidmap.key.wlanidrange.id_end = keywd_2;
		}
		else if (0 == strncmp(nas_type, "vlanid", strlen(nas_type))) 
		{
			tmp=(char *)value_point;
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
            		if(keywd_1 == 0 || keywd_2 > 4096)
                        	{
                                        ShowAlert(search(portal_public, "input_overflow"));
                                        return 0;
                              }
			nasidmap.key.vlanidrange.id_begin = keywd_1;
			nasidmap.key.vlanidrange.id_end = keywd_2;

			////////////////////////////////////////
		}
		else if (0 == strncmp(nas_type, "wtpid", strlen(nas_type)))
		{
			tmp=(char *)value_point;
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
            		if(keywd_1 == 0 || keywd_2 > 2048)
                        	{
                                        ShowAlert(search(portal_public, "input_overflow"));
                                        return 0;
                              }
			nasidmap.key.wtpidrange.id_begin = keywd_1;
			nasidmap.key.wtpidrange.id_end = keywd_2;
		}
		else if (0==strncmp(nas_type, "iprange", strlen(nas_type)))
		{
			memset(ip_1, 0, sizeof(ip_1));
			memset(ip_2, 0, sizeof(ip_2));
			nasidmap.key_type = NASID_KEYTYPE_IPRANGE;
			tmp_ip = (char *)value_point;
			if (NULL == (ip = strchr(tmp_ip, '-'))) 
			{
				strncpy(ip_1, tmp_ip, sizeof(ip_1) - 1);
				if (ccgi_eag_check_ip_format(ip_1) != 0) 
				{
					ShowAlert(search(portal_public, "input_illegal"));
					return 0;  
				}
				ccgi_inet_atoi(ip_1, &nasidmap.key.iprange.ip_begin);
				nasidmap.key.iprange.ip_end = nasidmap.key.iprange.ip_begin;
			}
			else 
			{
				strncpy(ip_1, tmp_ip, ip - tmp_ip);
				if (ccgi_eag_check_ip_format(ip_1) != 0) 
				{
					ShowAlert(search(portal_public, "input_illegal"));
					return 0;  
				}
				tmp_ip = ip + 1;
				strncpy(ip_2, tmp_ip, sizeof(ip_1) - 1);
				if (ccgi_eag_check_ip_format(ip_2) != 0)
				{
					ShowAlert(search(portal_public, "input_illegal"));
					return 0;  
				}
				ccgi_inet_atoi(ip_1, &(nasidmap.key.iprange.ip_begin));
				ccgi_inet_atoi(ip_2, &(nasidmap.key.iprange.ip_end));
			}
		}
		else if (0 == strncmp(nas_type, "interface", strlen(nas_type)))
		{
			if (!interface_input_is_legal(value_point)) {
                			ShowAlert(search(portal_public, "input_illegal"));
                                	return 0;
                              }
			nasidmap.key_type = NASID_KEYTYPE_INTF;
			strncpy(nasidmap.key.intf,(char *)value_point,MAX_NASID_KEY_BUFF_LEN-1);
		}
		
		strncpy(nasidmap.nasid, nasid, MAX_NASID_LEN-1);
		nasidmap.conid = strtoul(syntaxis_point, NULL, 10);

		check_ret4 = Input_Nas_ID_Check(nasid);
		check_ret5 = check_input_digit_valid(syntaxis_point, 0, 99);
		if((check_ret4 !=0) || (check_ret5 != 0)){
			ShowAlert(search(portal_public, "input_illegal"));
			return 0;
			}			
		ret = eag_add_nasid(ccgi_connection, parameter.local_id, parameter.instance_id, &nasidmap);
		if(EAG_RETURN_OK != ret){
               switch(ret){
			   	case EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE:
					   ShowAlert(search(portal_auth, "error_range"));
                        break;
				case EAG_ERR_INPUT_PARAM_ERR:
					    ShowAlert(search(portal_auth, "error_param"));
                        break;
				case EAG_ERR_CONFIG_ITEM_PARAM_CONFLICT:
					    ShowAlert(search(portal_auth, "error_conflict"));
                        break;
				default:ShowAlert(search(portal_auth, "whitelist_add_err"));
				        break;
			   }
		}
		else if((EAG_RETURN_OK == ret)){
		ShowAlert(search(portal_auth, "add_nasid_suc"));			
		}
		fprintf( pp, "<script type='text/javascript'>\n" );
		fprintf( pp, "window.location.href='wp_nasid_byvlan.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
		fprintf( pp, "</script>\n" );
	}
	return 0;		
}

static int s_addnasid_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	int i = 0;

	fprintf(fp,	"<table border=0 width=280 cellspacing=0 cellpadding=0>");

	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=250>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td width=250>");
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
   	"var url = 'wp_addnasid.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );


	fprintf(fp, "<tr height=30 align=left>"\
				"<td width=100>Type:</td>"\
				"<td width=180><select name=nas_type>"\
				"<option value=vlanid>vlanid</option>"\
				"<option value=interface>interface</option>"\
				"<option value=wlanid>wlanid</option>"\
				"<option value=wtpid>wtpid</option>"\
				"<option value=iprange>iprange</option>"\
				"</select>"\
				"</td>");
	fprintf(fp,	"</tr>"\

				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>","value");

	fprintf(fp,	"<td width=180><input type=text name=value_point size=15></td>");
	
	fprintf(fp, "</tr>"\

				"<tr height=30 align=left>"\
				"<td width=100>Nasid:</td>"\
				"<td width=180><input type=text name=nasid size=15></td>"\
				"</tr>"\
				
				"<tr height=30 align=left>"\
				"<td width=100>%s: </td>",search(portal_auth,"syntaxis"));
	fprintf(fp,	"<td width=180><input type=text name=syntaxis_point size=15></td>"\
				"</tr>");
				
	fprintf(fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",plotid);				
	fprintf(fp,	"</table>");
	fprintf(fp,"<input type=hidden name=UN value=\"%s\">",pstPageInfo->encry);				
	
	return 0;	
}


/*************************************
检查shuzi的输入合法性
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

int eag_nas_policy_get_num(int policy_id)
{
	char tempz[20] = "";
	int flag = -1;
	struct st_nasz chead = {0};
	int num = 0;
	
	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTN_N, policy_id);
	memset(&chead, 0, sizeof(chead));

	if (access(MULTI_NAS_F, 0) != 0)
	{
		return num;
	}
	flag = read_nas_xml(MULTI_NAS_F, &chead, &num, tempz);
	if(0 == flag && num > 0)
	{
		Free_nas_info(&chead);
	}

	return num;
}

static int interface_input_is_legal(const char  *intf_str)
{
	const char *p = NULL;

	if (NULL == intf_str || '\0' == intf_str[0] 
        		|| strlen(intf_str) > MAX_NASID_KEY_BUFF_LEN - 1)
          {
          	return 0;
          }

	for (p = intf_str; NULL != *p; p++) {
		if ( (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') \
                       || (*p == '-') || (*p == '.') || (*p >= 'A' && *p <= 'Z') ) {
            		continue;
		}
        		else {
			return 0;
            	}
    	}

    	return 1;
}

