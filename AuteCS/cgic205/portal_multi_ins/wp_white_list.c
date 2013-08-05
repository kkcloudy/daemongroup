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
* wp_white_list.c
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
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include <dbus/dbus.h>
#include <dbus/npd/npd_dbus_def.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include "ws_public.h"



#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"


#include "ws_user_manage.h"
#include "user_manage.h"

#include "ws_conf_engine.h"

#include "ws_init_dbus.h"
#include "ws_eag_auto_conf.h"

#include "drp/drp_def.h"
#include "drp/drp_interface.h"

#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_dcli_vrrp.h"
#include "ws_eag_conf.h"
#include "ws_dbus_list_interface.h"


#define _DEBUG	0

#if _DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif


#define SCRIPT_PATH			"sudo /usr/bin/"
#define ADD_WHITE_LIST_CMD_SINGIP	SCRIPT_PATH"cp_add_white_list.sh %s %s %s > /dev/null 2>&1"
#define DEL_WHITE_LIST_CMD_SINGIP	SCRIPT_PATH"cp_del_white_list.sh %s %s %s > /dev/null 2>&1"

#define ADD_WHITE_LIST_CMD	SCRIPT_PATH"cp_add_white_list.sh %s %s-%s %s > /dev/null 2>&1"
#define DEL_WHITE_LIST_CMD	SCRIPT_PATH"cp_del_white_list.sh %s %s-%s %s > /dev/null 2>&1"
#define GET_WHITE_LIST_CMD	SCRIPT_PATH"cp_get_white_list.sh %s 2>/dev/null"

#define PORTAL_CONF_PATH	"/opt/services/conf/portal_conf.conf"
#define PORTAL_LIST_PATH	"/opt/services/option/portal_option"
#define GET_ID_LIST_CMD		"sudo cat "PORTAL_CONF_PATH" | awk '{print $1}' | sort"
#define CHECK_ID_INUSE_CMD	"sudo grep \"^%s\" "PORTAL_CONF_PATH" >/dev/null 2>&1"




#define DEL_ENTER_CHAR(line)	{for(;strlen(line)>0&&(line[strlen(line)-1]==0x0d||line[strlen(line)-1]==0x0a);line[strlen(line)-1]='\0');}
#define IS_LEGAL_ID(id)		(id[0]>='0'&&id[0]<='7')




#define MAX_CMD_LEN			256
#define IP_ADDR_LEN			24
#define MAX_PORT_LEN		32
#define MAX_WHITE_LIST_NUM	50
#define MAX_ID_NUM			17

//定义错误
#define ERR_DEFAULT			-1
#define ERR_NOT_LEGAL_ID	(ERR_DEFAULT-1)
#define ERR_IPADDR_FORMAT	(ERR_DEFAULT-2)
#define ERR_DO_COMMAND		(ERR_DEFAULT-3)
#define ERR_PORTAL_ID		(ERR_DEFAULT-4)
#define ERR_IP_ADDR			(ERR_DEFAULT-5)
#define ERR_PORTAL_ID_NOUSE	(ERR_DEFAULT-6)
#define ERR_IP_NOT_IN_RANGE	(ERR_DEFAULT-7)
#define ERR_IP_HAS_IN_LIST	(ERR_DEFAULT-8)
#define ERR_IP_NOT_IN_LIST	(ERR_DEFAULT-9)
#define ERR_WHITE_LIST_MAX	(ERR_DEFAULT-10)
#define ERR_PORT_FORMAT		(ERR_DEFAULT-11)

//shell 返回的错误
#define ERR_OUT_OF_RANGE	1

//定义用到的结构体
typedef struct{
	char ipaddr_begin[IP_ADDR_LEN];
	char ipaddr_end[IP_ADDR_LEN];//can be empty
	char port[MAX_PORT_LEN+1];
}white_list_item_t;

typedef struct{
	char id[4];
	int num;
	white_list_item_t ipaddr_list[MAX_WHITE_LIST_NUM];
}white_list_t;






//定义全局变量


static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};



//定义静态函数
static int is_legal_ip( char *ipaddr )
{
	unsigned int ip0=0,ip1=0,ip2=0,ip3=0;	
	int iRet;
	
	iRet = sscanf( ipaddr,"%u.%u.%u.%u", &ip0, &ip1, &ip2, &ip3 );
	
	if( 4 == iRet )
	{
		if( (ip0 > 0 && ip0 < 255) || (ip1 >= 0 && ip1 < 255) || (ip2 >= 0 && ip2 < 255) || (ip3 >= 0 && ip3 < 255) )
		{
			debug_printf( stderr, "is_legal_ip not legal ip\n" );
			return 1;
		}
	}
	
	debug_printf( stderr, "is_legal_ip return 0\n" );
	return 0;	//1表示是正确的ipaddr格式
}

static int is_legal_port( char *port )
{
	char *port_temp;
	char *p;
	char port_str[10];
	int port_num;
	
	port_temp = strdup(port);
	debug_printf( stderr, "is_legal_port  begin port = %s\n", port );

	p = port_temp;//just allow number and ',' ---add by wk
	while(*p != '\0')
	{
		if( (*p < '0' || *p > '9') && *p != ',')
		{
			free( port_temp );
			return 0;	
		}
		p++;
	}
	
	p = strtok( port_temp, "," );
	while( NULL != p )
	{
		debug_printf( stderr, "is_legal_port  p = %s\n", p );
		strcpy( port_str, p );
		port_num = atoi(port_str);
		if( port_num <= 0 || port_num > 65535 )
		{
			free( port_temp );
			return 0;	
		}
		p = strtok( NULL, "," );
	}
	debug_printf( stderr, "is_legal_port  end port = %s\n", port );
	free( port_temp );
	return 1;
}

/*convert the ip address to unsigned long int data*/
static unsigned long  ip2ulong(char *str)
{
	char *separate=".";
	char *token = NULL;
	unsigned long ip_long[4]; 
	unsigned long ip = 0;
	int i = 1;
	
	token=strtok(str,separate);
	if(NULL != token){
	    ip_long[0] = strtoul(token,NULL,10);
	}
	while((token!=NULL)&&(i<4))
	{
		token=strtok(NULL,separate);
		if(NULL != token){
		    ip_long[i] = strtoul(token,NULL,10);
		}
		i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	return ip;
}


/******************************************************************
上面的都是页面调用是需要用到的api，下面才是和页面相关的东西
******************************************************************/
#define SUBMIT_NAME		"submit_add_white_list"


/***************************************************************
定义页面要用到的结构体
****************************************************************/
#define MAX_URL_LEN         256


typedef struct{
	char id[4];
	white_list_item_t ipaddr_add;
	white_list_item_t ipaddr_del;
}STUserInput;

typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;/*解析control.txt文件的链表头*/
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	STUserInput stUserInput;
	white_list_t st_white_list;
	
	int formProcessError;
} STPageInfo;

/***************************************************************
申明回调函数
****************************************************************/
static int s_white_prefix_of_page( STPageInfo *pstPageInfo );
static int s_white_content_of_page( STPageInfo *pstPageInfo );

/***************************************************************
*USEAGE:	得到已经使用的id列表
*Param:		p_used_id -> 存放id列表的结构体
*Return:	0 -> success
*			!= 0 -> failure。
*Auther:shao jun wu
*Date:2008-12-30 14:12:46
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	int ret = 0;
	STPageInfo stPageInfo;
	char del_opt[10]={0};

	DcliWInit();
	ccgi_dbus_init();
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret = init_portal_container(&(stPageInfo.pstPortalContainer));
	stPageInfo.lpublic=stPageInfo.pstPortalContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	
	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	memset(plotid,0,sizeof(plotid));
	cgiFormStringNoNewlines("plotid", plotid, sizeof(plotid)); 
	
	fprintf(stderr,"-------ip-----------plotid=%s",plotid);
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
//初始化完毕

	cgiFormStringNoNewlines("del_opt",del_opt,sizeof(del_opt));
	if(!strcmp(del_opt,"delete"))
	{
		int ret = 0;
		char insid_str[4]={0};
		char ipbegin[32]={0};
		char ipend[32]={0};
		char iprange[64]={0};
		char ipport[16]={0};
		char ipintf[16]={0};
		
		cgiFormStringNoNewlines("ip_begin",ipbegin,sizeof(ipbegin));
		cgiFormStringNoNewlines("ip_end",ipend,sizeof(ipend));
		cgiFormStringNoNewlines("ip_port",ipport,sizeof(ipport));
		cgiFormStringNoNewlines("ip_intf",ipintf,sizeof(ipintf));

		
		if(0 == strcmp(ipend,""))
		{
			sprintf(iprange,"%s",ipbegin);
		}
		else
		{
			sprintf(iprange,"%s-%s",ipbegin,ipend);
		}
		
		RULE_TYPE type = RULE_IPADDR;
		

		ret = eag_conf_captive_list(ccgi_connection, parameter.local_id,
						parameter.instance_id, type, iprange, ipport, "", ipintf, CP_DEL_LIST, CP_WHITE_LIST);

		

	}

	
 	
	

//当前页面是static arp的页面，所以设置活动label为2
	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_WHITELIST);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_white_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_white_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ACTION, "wp_white_list.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	release(stPageInfo.lauth);
	release(stPageInfo.lpublic);
	
	free_instance_parameter_list(&paraHead1);
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_white_prefix_of_page( STPageInfo *pstPageInfo )
{
   char *error_message=NULL;

	if( NULL == pstPageInfo )
	{
		return -1;
	}
 	
 	fprintf( cgiOut, "<style type=text/css>.a3{width:30px;border:0px; text-align:center}</style>" );
 	fprintf( cgiOut, "<script language=javascript src=/ip.js></script>\n" );
 	fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );



	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess)
	{
		int ret = -1;
		int ret_value = 0;

		char begin_ip1[4]={0};
		char begin_ip2[4]={0};
		char begin_ip3[4]={0};
		char begin_ip4[4]={0};
		char begin_ip[16]={0};

		char end_ip1[4]={0};
		char end_ip2[4]={0};
		char end_ip3[4]={0};
		char end_ip4[4]={0};
		char end_ip[16]={0};

		char ip_range[64];

		char ip_port[8]={0};
		char ip_intf[16]={0};
		int endip_flag = 0;

		RULE_TYPE type = RULE_IPADDR;

		cgiFormStringNoNewlines("begin_ip1",begin_ip1,sizeof(begin_ip1));
		cgiFormStringNoNewlines("begin_ip2",begin_ip2,sizeof(begin_ip2));
		cgiFormStringNoNewlines("begin_ip3",begin_ip3,sizeof(begin_ip3));
		cgiFormStringNoNewlines("begin_ip4",begin_ip4,sizeof(begin_ip4));

		//fprintf(stderr,"%s,%s,%s,%s@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n",begin_ip1,begin_ip2,begin_ip3,begin_ip4);

		cgiFormStringNoNewlines("end_ip1",end_ip1,sizeof(end_ip1));
		cgiFormStringNoNewlines("end_ip2",end_ip2,sizeof(end_ip2));
		cgiFormStringNoNewlines("end_ip3",end_ip3,sizeof(end_ip3));
		cgiFormStringNoNewlines("end_ip4",end_ip4,sizeof(end_ip4));

		cgiFormStringNoNewlines("port",ip_port,sizeof(ip_port));

		cgiFormStringNoNewlines("intf",ip_intf,sizeof(ip_intf));
		if( strlen(begin_ip1) == 0 || strlen(begin_ip2) == 0 || strlen(begin_ip3) == 0 || strlen(begin_ip4) == 0  )
		{
			ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));				  
			return 0;
		}	
		sprintf(begin_ip,"%s.%s.%s.%s",begin_ip1,begin_ip2,begin_ip3,begin_ip4);

		if( strlen(end_ip1) == 0 && strlen(end_ip2) == 0 && strlen(end_ip3) == 0 && strlen(end_ip4) == 0  )
		{
			endip_flag = 1;
		}	
		else if( strlen(end_ip1) == 0 || strlen(end_ip2) == 0 || strlen(end_ip3) == 0 || strlen(end_ip4) == 0 )
		{
			ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));				  
			return 0;
		}
		else
		{
			sprintf(end_ip,"%s.%s.%s.%s",end_ip1,end_ip2,end_ip3,end_ip4);
		}

		if(0 == endip_flag)
		{
			sprintf(ip_range,"%s-%s",begin_ip,end_ip);
		}
		else
		{
			sprintf(ip_range,"%s",begin_ip);
		}

			
		if( strlen(ip_port) > 0 && 0 != strcmp(ip_port, "all") && ! is_legal_port(ip_port) )
		{
			ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));
			return 0;
		}
		else if( strlen(ip_port)==0 )
		{
			strcpy( ip_port, "all" );
		}

		ret =  eag_conf_captive_list(ccgi_connection, parameter.local_id, parameter.instance_id,
					type, ip_range, ip_port, "", ip_intf, CP_ADD_LIST, CP_WHITE_LIST);



	}
	return 0;		
}

static int s_white_content_of_page( STPageInfo *pstPageInfo )
{
	int i;
	static instance_parameter *pq = NULL;
	char temp[10] = {0};
	
	if( NULL == pstPageInfo )
	{
		return -1;
	}
	
	fprintf(cgiOut,"	<table>\n");
	fprintf(cgiOut,"		<tr>\n"\
					"			<td colspan='3'>\n"\
					"				<table>\n"\
					"					<tr>\n"\
					"						<td>ID:</td>\n"\
					"						<td>\n"\
					"						<select name='plotid' style='width:100%%;height:auto' onchange='on_id_change(this);'>\n");



//pstPageInfo->stUserInput.id[0] == i+'0' 
	for (pq=paraHead1;(NULL != pq);pq=pq->next)
	{
		memset(temp,0,sizeof(temp));
		snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
		
		if (strcmp(plotid, temp) == 0)
			fprintf(cgiOut,"<option value='%s' selected>%s</option>\n",temp,temp);
		else	       
			fprintf(cgiOut,"<option value='%s'>%s</option>\n",temp,temp);
	}	
	fprintf(cgiOut,"						</select>\n" );
	fprintf( cgiOut,"			<script type=text/javascript>\n"\
					"			function on_id_change(obj)\n"
					"			{\n"\
					"				window.location.href='wp_white_list.cgi?UN=%s&plotid='+obj.value;\n"\
					"			}\n",pstPageInfo->encry );
	fprintf( cgiOut, "			</script>\n" );
	fprintf(cgiOut,"						</td>\n"\
					"					</tr>\n");
					
	fprintf( cgiOut,"<tr>\n"\
					"<td>Mode:</td>\n"\
					"<td>\n"\
					"<select name='plotid' style='width:100%%;height:auto' onchange='on_mode_change(this);'>\n"\
					"<option value=0 selected>Ip</option>"\
					"<option value=1 >Domain</option>"\
					"</select>\n");
	fprintf( cgiOut,"<script type=text/javascript>\n"\
					"function on_mode_change(obj)\n"
					"{\n"\
					"window.location.href='wp_white_list_domain.cgi?UN=%s&plotid=%s';\n",pstPageInfo->encry,plotid);
	fprintf( cgiOut,"}\n");
	fprintf( cgiOut, "</script>\n  </td>\n	</tr>\n" );


	fprintf( cgiOut, "					<tr>\n"\
					"						<td>IP Begin:</td>\n"\
					"						<td>\n" );
	fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:147px;font-size:9pt'>");
	if(pstPageInfo->iUserGroup == 0)
		{
			fprintf( cgiOut, "<input type=text name='begin_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
			fprintf( cgiOut, "<input type=text name='begin_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
			fprintf( cgiOut, "<input type=text name='begin_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
			fprintf( cgiOut, "<input type=text name='begin_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",search(pstPageInfo->lpublic,"ip_error"));
		}
	else
		{
			fprintf( cgiOut, "<input type=text name='begin_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>.",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"" );
			fprintf( cgiOut, "<input type=text name='begin_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>.",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"" );
			fprintf( cgiOut, "<input type=text name='begin_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>.",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"" );
			fprintf( cgiOut, "<input type=text name='begin_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"" );
	
		}
	fprintf( cgiOut, "</div>\n" );		
					
	fprintf( cgiOut,"				</td>\n"\
					"						<td></td>\n"\
					"					</tr>\n" );
	//ip end
	fprintf( cgiOut,"					<tr>\n"\
					"						<td>IP End:</td>\n"\
					"						<td>\n" );
	fprintf( cgiOut, "<div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:147px;font-size:9pt'>");
	if(pstPageInfo->iUserGroup == 0)
		{
			fprintf( cgiOut, "<input type=text name='end_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
			fprintf( cgiOut, "<input type=text name='end_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
			fprintf( cgiOut, "<input type=text name='end_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
			fprintf( cgiOut, "<input type=text name='end_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",search(pstPageInfo->lpublic,"ip_error"));
		}
	else
		{
			fprintf( cgiOut, "<input type=text name='end_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>.",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"");
			fprintf( cgiOut, "<input type=text name='end_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>.",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"");
			fprintf( cgiOut, "<input type=text name='end_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>.",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"");
			fprintf( cgiOut, "<input type=text name='end_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() %s/>",search(pstPageInfo->lpublic,"ip_error"),(pstPageInfo->iUserGroup)?"disabled":"");
			
		}
	fprintf( cgiOut, "</div>\n" );		
	fprintf( cgiOut,"				</td>\n"\
					"						<td></td>\n"\
					"					</tr>\n" );	
	// port
	if(pstPageInfo->iUserGroup == 0)
		{
			fprintf( cgiOut,"					<tr>\n"\
							"						<td>Port:</td>\n"\
							"						<td><input type=text name=port value='' /></td>\n"\
							"					</tr>\n" );
		}
	else
		{
			fprintf( cgiOut,"					<tr>\n"\
							"						<td>Port:</td>\n"\
							"						<td><input type=text name=port value='' %s/></td>\n"\
							"					</tr>\n" ,(pstPageInfo->iUserGroup)?"disabled":"");
		}

		 fprintf(  cgiOut,"                         <tr>\n"\
						"                           <td>Intf:</td>\n");
		 fprintf(cgiOut,"<td><select name=intf>");
        fprintf(cgiOut,"<option value=''></option>");		
		infi  interf;
		interface_list_ioctl (0,&interf);
		char dupinf[20] = {0};
		infi * q ;
		q = interf.next;
		while(q)
		{
			    memset(dupinf,0,sizeof(dupinf));
				if(NULL != q->next)
				{
					strcpy(dupinf,q->next->if_name);
				}
				if( !strcmp(q->if_name,"lo") )
				{
					q = q->next;
					continue;
				}
				if( !strcmp(q->if_name,dupinf) )
				{
					q = q->next;
					continue;
				}
		        fprintf(cgiOut,"<option value=%s>%s</option>",q->if_name,q->if_name);		
				q = q->next;
			}
		 fprintf(cgiOut,"</select></td>\n"
						"						 </tr>\n");

	
	
	fprintf( cgiOut,"				</table>\n"\
					"			</td>\n"\
				  	"		</tr>\n");
	fprintf(cgiOut,"		<tr>\n");
	fprintf(cgiOut,"			<td>\n"\
						"				<table frame=below rules=rows border=1 style='border-collapse:collapse;align:left;overflow:visible'>\n" );
#if 1
	fprintf( cgiOut, "					<tr height=30 bgcolor=#eaeff9 id=td1 align=left>\n");
	fprintf(cgiOut,"						<td width=150 style=font-size:12px><font id=%s>IP</font></th>\n",search(pstPageInfo->lpublic,"menu_thead"));
	fprintf(cgiOut,"						<td width=20></th>\n");
	fprintf(cgiOut,"					</tr>\n");
#endif


	int ret=-1; 

	RULE_TYPE type;
	struct bw_rules white;
	memset(&white,0,sizeof(white));
	
	ret = eag_show_white_list(ccgi_connection, parameter.local_id,
					parameter.instance_id, &white); 
	if(ret == 0)
	{
		int num; 
		num = white.curr_num;
		if(num >0 )
		{
			char ipbegin[32];
			char ipend[32];
			char ports[CP_MAX_PORTS_BUFF_LEN]={0};
			char intf[MAX_IF_NAME_LEN]={0};
			

			for(i = 0 ; i < num ; i++)
			{   
			    type = white.rule[i].type;
				if(type == RULE_IPADDR)
				{
	  				ccgi_ip2str(white.rule[i].key.ip.ipbegin,ipbegin,sizeof(ipbegin));
	  				ccgi_ip2str(white.rule[i].key.ip.ipend,ipend,sizeof(ipend));
	  				strcpy(ports,white.rule[i].key.ip.ports);
	  				strcpy(intf,white.rule[i].intf);
	  				//fprintf(stderr,"intf=%s+++++++++++++++++++++&&&&&&&&&&&&&&&&&&&&&&&&&&&&&",intf);
	  				fprintf(cgiOut,"					<tr height=25>\n");
	  				fprintf(cgiOut,"						<td style=font-size:12px align=left>%s%s%s%s%s%s%s</td>\n",ipbegin,"-",ipend,":",ports,"   ",intf);
	  				fprintf(cgiOut,"						<td>\n" );
	  				fprintf( cgiOut, "						<script type=text/javascript>\n" );
	  				fprintf( cgiOut, "							var popMenu%d = new popMenu('popMenu%d','%d');\n", i, i,num-i);
	  				fprintf( cgiOut, "							popMenu%d.addItem( new popMenuItem( '%s', 'wp_white_list.cgi?del_opt=%s&UN=%s&plotid=%s&ip_begin=%s&ip_end=%s&ip_port=%s&ip_intf=%s' ) );\n",
	  									i,"delete","delete", pstPageInfo->encry, plotid ,ipbegin,ipend,ports,intf ); 
	  				fprintf( cgiOut, "							popMenu%d.show();\n", i );
	  				fprintf( cgiOut, "						</script>\n" );
	  				fprintf( cgiOut, "						</td>\n");
	  				fprintf(cgiOut,"					</tr>\n");
				}


			}
				
		}
	}

		fprintf(cgiOut,"				</table>"\
						"			</td>");
		fprintf(cgiOut,"			<td width='50'>&nbsp;</td>\n");

		fprintf( cgiOut, "<td></td>" );
					
		fprintf(cgiOut,"		</tr>\n"\
						"	</table>\n"); 
	fprintf(cgiOut,"<input type=hidden name=UN value=%s>",pstPageInfo->encry);
	return 0;
}
