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
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"

#include "ws_eag_conf.h"

#ifndef ADD_MULTI_PORTAL  
#define ADD_MULTI_PORTAL

#define SUBMIT_NAME 	"submit_add_multi_portal"

#define MULTI_PORTAL_CONF_PATH "/opt/services/conf/multiportal_conf.conf"

#define MAX_PORTAL_WEBURL_LEN  128
#define MAX_PORTAL_ACNAME_LEN 32
#define MAX_PORTAL_DOMAIN_LEN   10

#endif

int port_input_is_legal(const char *str);
int weburl_input_is_legal(const char *str);
int acname_input_is_legal(const char *str);
int domain_input_is_legal(const char *str);
static int get_portal_param_by_portal_url
(
char *portal_url, 
char *url_prefix,
int url_prefix_len,
int *ip_addr, 
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
//初始化完毕

	
//处理表单


	

	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, 5 );

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
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_addMulitPortal_prefix_of_page( STPageInfo *pstPageInfo )
{
	//FILE * fp = pstPageInfo->fp;
	FILE *pp = pstPageInfo->fp;
	
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	
	char plot_id[30],portal_server_portal[10],noteport[10],key_word[256], ipaddr[256],acname[256],domain[256];
	char advertise_url[256];
	char type[64];
	
	//////////memset////////////////
//	memset( ssid, 0, sizeof(ssid));
//	memset( ipaddr, 0, sizeof(ipaddr));
//	memset( port, 0, sizeof(port));
//	memset( web_page, 0, sizeof(web_page));

	memset( plot_id , '\0', sizeof(plot_id));
	memset( key_word, '\0', sizeof(key_word));
	memset( ipaddr, '\0', sizeof(ipaddr));
	memset( acname, '\0', sizeof(acname));
	memset( portal_server_portal, '\0', sizeof(portal_server_portal));
	memset( noteport, '\0', sizeof(noteport));
	memset( domain, '\0', sizeof(domain));
	memset( type, '\0', sizeof(type));  
	memset( advertise_url, '\0', sizeof(advertise_url));  

	//web_page[0]='\0';
	

	if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		cgiFormStringNoNewlines("plot_id",plot_id, sizeof(plot_id));
		cgiFormStringNoNewlines("key_word",key_word, sizeof(key_word));
		cgiFormStringNoNewlines("ipaddr",ipaddr, sizeof(ipaddr));
		cgiFormStringNoNewlines("acname",acname, sizeof(acname));
		cgiFormStringNoNewlines("portal_server_portal",portal_server_portal, sizeof(portal_server_portal));
		cgiFormStringNoNewlines("noteport",noteport, sizeof(noteport));
		cgiFormStringNoNewlines("domain",domain, sizeof(domain));
		cgiFormStringNoNewlines("type",type, sizeof(type));
		cgiFormStringNoNewlines("advertise_url",advertise_url, sizeof(advertise_url));
		
		
		/////////////处理数据///////////////////
		#if 0
		FILE * fp = NULL;
		char content[1024];
		memset( content, 0, sizeof(content));
		sprintf( content, "%s=%s=%s=%s=%s\n",ssid,ipaddr,port,web_page,domain);

		#endif
		
		if(strcmp(plot_id,"")==0)
		  cgiFormStringNoNewlines("plotid",plot_id, sizeof(plot_id));

		//check length of SSID
		if(strlen(key_word) > 32)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_too_long") );
			return 0;
		}
		if(strlen(key_word) == 0)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_too_long") );
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
		
		/* check portal url --add by wk*/
		int ret = -1;
		char temp_buff[1024];
		int temp_int;
		ret = get_portal_param_by_portal_url(	ipaddr,
													temp_buff,
													sizeof(temp_buff),
													&temp_int,
													&temp_int,
													temp_buff,
													sizeof(temp_buff));
		if (1 != ret)
		{
			ShowAlert(search(portal_auth, "multi_portal_weburl_error"));
			return 0;
		}
		//check port
		/*
		char *port_temp;
		char *p;
		char port_str[10];
		int port_num;
		
		port_temp = strdup(port);
		//debug_printf( stderr, "is_legal_port  begin port = %s\n", port );
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
		*/



	
		//fprintf(stderr,"get submit,content=%s",content);
		//fprintf(stderr,"content=%s",content);
		
        #if 1 
		char *tmpz=(char *)malloc(40);
		memset(tmpz,0,40);
		sprintf(tmpz,"%s%s",MTP_N,plot_id);
		add_eag_node_attr(MULTI_PORTAL_F, tmpz, ATT_Z, key_word);
        mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, P_KEYWORD, key_word);
        mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, PIP, ipaddr);
		mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, PPORT, "0");
        mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, PWEB, "");
		mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, PACN, acname);
		mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, PJOM, portal_server_portal);
		if(strcmp(noteport,"")!=0)
        mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, PNPORT, noteport);		
		mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, PDOMAIN, domain);
		mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, P_TYPE, type);
		mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, key_word, P_ADVERTISE_URL, advertise_url);
		

		free(tmpz);
        #endif
		
		#if 0
		if( NULL != (fp = fopen(MULTI_PORTAL_CONF_PATH,"a+")) )
			{
				fwrite(content, strlen(content), 1, fp);
				//fflush(fp);
				fclose(fp);
				ShowAlert( search(portal_auth, "add_multi_portal_suc") );
				//ShowAlert( search(portal_public, "input_illegal") );

			}
		#endif
		fprintf( pp, "<script type='text/javascript'>\n" );
		fprintf( pp, "window.location.href='wp_multi_portal.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plot_id);
		fprintf( pp, "</script>\n" );

		
		//fprintf(stderr,"111");
	}
	return 0;		
}

static int s_addMulitPortal_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	//struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	char *urlnode=(char *)malloc(20);
	memset(urlnode,0,20);
	cgiFormStringNoNewlines( "plotid", urlnode, 20 );
	char *tempz=(char *)malloc(20);
	memset(tempz,0,20);
    sprintf(tempz,"%s%s",MTP_N,urlnode);

    int flag=1;
	flag=if_design_node(MULTI_PORTAL_F, tempz, ATT_Z, MTD_N);
    free(tempz);

	struct st_portalz c_head,*cq;
	int cnum,cflag=-1;
	char nodename[20] = "";
	memset(nodename, 0, sizeof(nodename));
	if(strcmp(urlnode,"") != 0)
	{
		sprintf(nodename,"%s%s",MTP_N,urlnode);
		cflag = read_portal_xml(MULTI_PORTAL_F, &c_head, &cnum, nodename);
	}
	else
	{
		cflag = -1;
	}
	fprintf(fp,	"<table border=0 width=600 cellspacing=0 cellpadding=0>");
	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td>");
	fprintf(fp,"<select name=plot_id disabled>\n");
	if(strcmp(urlnode,"")==0)
	{
		fprintf(fp,"<option value='%s'>1</option>",PLOTID_ONE);
		fprintf(fp,"<option value='%s'>2</option>",PLOTID_TWO);
		fprintf(fp,"<option value='%s'>3</option>",PLOTID_THREE);
		fprintf(fp,"<option value='%s'>4</option>",PLOTID_FOUR);
		fprintf(fp,"<option value='%s'>5</option>",PLOTID_FIVE);
	}
	else
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

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>Protocol</td>");
	fprintf(fp, "<td><input type=\"radio\" name=\"portal_server_portal\" value=\"JSON\" onclick=\"document.getElementById('noteport').disabled=true;\">JSON\n");
	fprintf(fp, "<input type=\"radio\" name=\"portal_server_portal\" value=\"MOBILE\" onclick=\"document.getElementById('noteport').disabled=false;\" checked>MOBILE</td>\n");
	fprintf(fp,"</tr>\n");

	//key word type;
	if(flag==-1)
	{
		fprintf(fp,"<tr height=30 align=left>");
		fprintf(fp,"<td width=100>%s</td>","Key Word Type");
		fprintf(fp,"<td>");
		fprintf(fp,"<select name=type>\n");
		fprintf(fp,"<option value='%s'>%s</option>","Essid","Essid");
		fprintf(fp,"<option value='%s'>%s</option>","Vlanid","Vlanid");
		fprintf(fp,"<option value='%s'>%s</option>","Interface","Interface");	
		fprintf(fp,"<option value='%s'>%s</option>","Wlanid","Wlanid");	
		fprintf(fp,"<option value='%s'>%s</option>","Wtpid","Wtpid");	
		fprintf(fp,"</select>");
		fprintf(fp,"</td></tr>\n");
	}
	else
	{
		//fprintf(fp,"<select type=hidden name=multi_portal_type disabled>\n");
		//fprintf(fp,"<option value='%s' selected>%s</option>","other","other");
		fprintf(fp,"<tr><td colspan=2><input type=hidden name=type value=\"%s\"></td></tr>\n","(as-default)");	
	}

	fprintf(fp,"<tr height=30 align=left>");
	
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
	
	if (0 != cflag || NULL == cq)
	{
		fprintf(fp,"<td width=100>Key Word</td>");	
	}
	
	if(flag==-1)
	fprintf(fp,"<td><input type=text name=key_word size=15 value=\"%s\" readonly style=\"width:200px;\"></td>",MTD_N);
	else
	fprintf(fp,"<td><input type=text name=key_word size=15 style=\"width:200px;\"></td>");
	
	fprintf(fp,"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>URL</td>"\
				"<td><input type=text name=ipaddr size=15 style=\"width:400px;\"></td>"\
				"</tr>");
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>ACname</td>"\
				"<td><input type=text name=acname size=15 style=\"width:200px;\"></td>"\
				"</tr>");	
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>Domain</td>"\
				"<td><input type=text name=domain size=15 style=\"width:200px;\"></td>"\
				"</tr>");		
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>Advertise URL</td>"\
				"<td><input type=text name=advertise_url size=15 style=\"width:200px;\"></td>"\
				"</tr>");	
	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"error_offline_port"));
	fprintf(fp, "<td><input type=text name=noteport id=noteport value=\"\" style=\"width:200px;\"></td>\n");
	fprintf(fp,"</tr>\n");

	fprintf(fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",urlnode);				
	fprintf(fp,	"</table>");

	free(urlnode);

	
	return 0;
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

static int get_portal_param_by_portal_url
(
char *portal_url, 
char *url_prefix,
int url_prefix_len,
int *ip_addr, 
int *port, 
char * web_page,
int web_page_len
)
{
	if (	NULL == portal_url	|| NULL == ip_addr 
		||	NULL == port 		|| NULL == web_page
		||	NULL == url_prefix	|| 0 >= web_page_len
		||	0 >= url_prefix_len)
	{
		//log_err("param err!\n");
		return -1;
	}
	
	unsigned int ip_part_value = -1;
	int ip_part_num = -1;
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

