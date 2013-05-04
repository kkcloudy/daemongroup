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
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"

#include "ws_eag_conf.h"
#ifndef EDIT_MULTI_PORTAL
#define EDIT_MULTI_PORTAL

#define SUBMIT_NAME 	"submit_edit_multi_portal"

#define MULTI_PORTAL_CONF_FILE_PATH	"/opt/services/conf/multiportal_conf.conf"

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
static int s_editMulitPortal_prefix_of_page( STPageInfo *pstPageInfo );
static int s_editMulitPortal_content_of_page( STPageInfo *pstPageInfo );
static int edit_num;

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

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editMulitPortal_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_editMulitPortal_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	LB_changelabelName_Byindex(stPageInfo.pstPortalContainer->pstModuleContainer,search(stPageInfo.lauth,"multi_portal_management"),6);
	sprintf(url,"%s?UN=%s","wp_multi_portal.cgi",stPageInfo.encry);
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


static int s_editMulitPortal_prefix_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;

	
	char plot_id[30],portal_server_portal[10],noteport[10],key_word[256], ipaddr[256], acname[256],domain[256];
	char type[32],advertise_url[256];

	
	memset( plot_id , '\0', sizeof(plot_id));
	memset( key_word, '\0', sizeof(key_word));
	memset( ipaddr, '\0', sizeof(ipaddr));
	memset( acname, '\0', sizeof(acname));
	memset( portal_server_portal, '\0', sizeof(portal_server_portal));
	memset( noteport, '\0', sizeof(noteport));
	memset( domain, '\0', sizeof(domain));
	memset( type, '\0', sizeof(type));
	memset( advertise_url, '\0', sizeof(advertise_url));

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

		if(strcmp(plot_id,"")==0)
		  cgiFormStringNoNewlines("plotid",plot_id, sizeof(plot_id));

		//check length of SSID
		if(strlen(key_word) > 32)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_too_long") );
			goto return_line;
		}
		if(strlen(key_word) == 0)
		{
			ShowAlert( search(portal_auth, "multi_portal_name_too_long") );
			goto return_line;
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
				goto return_line;
			}
		}

		
		if (acname != NULL && !acname_input_is_legal(acname)){
			ShowAlert(search(portal_auth, "multi_portal_acname_error"));
			goto return_line;
		}
		if (domain != NULL && !domain_input_is_legal(domain)){
			ShowAlert(search(portal_auth, "multi_portal_domain_error"));
			goto return_line;
		}
		if (noteport != NULL && strcmp(noteport, "") != 0 && !port_input_is_legal(noteport)){
			ShowAlert(search(portal_auth, "multi_portal_port_error"));
			goto return_line;
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
			goto return_line;
		}
		
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
		
		return_line:
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_multi_portal.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plot_id);
		fprintf( fp, "</script>\n" );		
	}
	return 0;		
}

static int s_editMulitPortal_content_of_page( STPageInfo *pstPageInfo )
{
	FILE * fp = pstPageInfo->fp;
	struct list * portal_public = pstPageInfo->lpublic;
	struct list * portal_auth = pstPageInfo->lauth;
	int freeflag=-1;

	struct st_portalz cq;
	memset(&cq,0,sizeof(cq));
		
	int i,j;
	
	char edit_rule[10] = "";
	char index[32] = "";
	char nodez[32];
	memset(nodez,0,32);
	char attz[32];
	memset(attz,0,32);
	char plotidz[32];
	memset(plotidz,0,32);
 	
	int i_index ;
	
	cgiFormStringNoNewlines( "EDITCONF", edit_rule, 10 );
	if( !strcmp(edit_rule, "edit") )
	{
		cgiFormStringNoNewlines( "INDEX", index, 32 );
		cgiFormStringNoNewlines( "NODEZ", nodez, 32 );
		cgiFormStringNoNewlines( "ATTZ", attz, 32 );
		cgiFormStringNoNewlines( "PLOTIDZ", plotidz, 32 );


		char * ptr ;
		i_index = strtoul( index, &ptr, 0 );
		FILE * pfile= NULL;
		int   revnum=0, location=-1;
		
		int temp=-1;
		int del_loc = 0;
		char * revinfo[512];
		char buf[2048];

		freeflag=get_portal_struct(MULTI_PORTAL_F, nodez, ATT_Z, attz, &cq);
		fprintf( stderr, "cq.pdomain=%s\n",cq.pdomain);
		fprintf( stderr, "nodez=%s\n",nodez);
		fprintf( stderr, "attz=%s,cq.p_keyword=%s\n",attz,cq.p_keyword);
	}
	
	fprintf(fp,"<table border=0 width=600 cellspacing=0 cellpadding=0>\n");

	fprintf(fp,"<tr height=30 align=left>");
	fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"plot_idz"));
	fprintf(fp,"<td>");
	fprintf(fp,"<select name=plot_id disabled>\n");

	if(strcmp(plotidz,PLOTID_ZEAO)==0)
	fprintf(fp,"<option value='%s' selected>%s</option>",PLOTID_ZEAO,MTD_N);
	else
	fprintf(fp,"<option value='%s'>%s</option>",PLOTID_ZEAO,MTD_N);

	if(strcmp(plotidz,PLOTID_ONE)==0)
	fprintf(fp,"<option value='%s' selected>1</option>",PLOTID_ONE);
	else
	fprintf(fp,"<option value='%s'>1</option>",PLOTID_ONE);

	if(strcmp(plotidz,PLOTID_TWO)==0)
	fprintf(fp,"<option value='%s' selected>2</option>",PLOTID_TWO);
	else
	fprintf(fp,"<option value='%s'>2</option>",PLOTID_TWO);

	if(strcmp(plotidz,PLOTID_THREE)==0)
	fprintf(fp,"<option value='%s' selected>3</option>",PLOTID_THREE);
	else
	fprintf(fp,"<option value='%s'>3</option>",PLOTID_THREE);

	if(strcmp(plotidz,PLOTID_FOUR)==0)
	fprintf(fp,"<option value='%s' selected>4</option>",PLOTID_FOUR);
	else
	fprintf(fp,"<option value='%s'>4</option>",PLOTID_FOUR);

	if(strcmp(plotidz,PLOTID_FIVE)==0)
	fprintf(fp,"<option value='%s' selected>5</option>",PLOTID_FIVE);
	else
	fprintf(fp,"<option value='%s'>5</option>",PLOTID_FIVE);
	
	fprintf(fp,"</select>");
	fprintf(fp,"</td>"\
				"</tr>\n");	

	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>Protocol</td>");
	if( strcmp(cq.pjom,"JSON") == NULL)
	{
		fprintf(fp, "<td><input type=\"radio\" name=\"portal_server_portal\" value=\"JSON\" onclick=\"document.getElementById('noteport').disabled=true;\" checked>JSON\n");
		fprintf(fp, "<input type=\"radio\" name=\"portal_server_portal\" value=\"MOBILE\" onclick=\"document.getElementById('noteport').disabled=false; \" >MOBILE</td>\n");
	}else
	{
		fprintf(fp, "<td><input type=\"radio\" name=\"portal_server_portal\" value=\"JSON\" onclick=\"document.getElementById('noteport').disabled=true;\" >JSON\n");
		fprintf(fp, "<input type=\"radio\" name=\"portal_server_portal\" value=\"MOBILE\" onclick=\"document.getElementById('noteport').disabled=false;\" checked>MOBILE</td>\n");
	}
	fprintf(fp,"</tr>\n");

	if(0 == strcmp(cq.p_keyword,MTD_N))
	{
		fprintf(fp,"<tr height=30 align=left>");
		fprintf(fp,"<td width=100>%s</td>","Key Word Type");
		fprintf(fp,"<td>");
		fprintf(fp,"<select name=type>\n");

		char *essid_selected = "";
		char *vlanid_selected = "";
		char *wlanid_selected = "";
		char *interface_selected = "";
		char *wtpid_selected = "";
		
		if (0 == strcmp(cq.p_type,"Essid"))
		{
			essid_selected = "selected";
		}
		else if (0 == strcmp(cq.p_type,"Vlanid"))
		{
			vlanid_selected = "selected";
		}
		else if (0 == strcmp(cq.p_type,"Interface"))
		{
			interface_selected = "selected";
		}
		else if (0 == strcmp(cq.p_type,"Wlanid"))
		{
			wlanid_selected = "selected";
		}
		else if (0 == strcmp(cq.p_type,"Wtpid"))
		{
			wtpid_selected = "selected";
		}

		fprintf(fp,"<option value='%s' %s>%s</option>","Essid", essid_selected, "Essid");
		fprintf(fp,"<option value='%s' %s>%s</option>","Vlanid", vlanid_selected, "Vlanid");
		fprintf(fp,"<option value='%s' %s>%s</option>","Interface", interface_selected, "Interface");
		fprintf(fp,"<option value='%s' %s>%s</option>","Wlanid", wlanid_selected, "Wlanid");
		fprintf(fp,"<option value='%s' %s>%s</option>","Wtpid", wtpid_selected, "Wtpid");
	
		fprintf(fp,"</select>");
		fprintf(fp,"</td></tr>\n");
	}
	else
	{
		//fprintf(fp,"<select type=hidden name=multi_portal_type disabled>\n");
		//fprintf(fp,"<option value='%s' selected>%s</option>","other","other");
		fprintf(fp,"<tr><td colspan=2><input type=hidden name=type value=\"%s\"></td></tr>\n","(as-default)");	
	}


	
	struct st_portalz c_head,*p_cq;
	int cnum,cflag=-1;
	char nodename[20] = "";
	memset(nodename, 0, sizeof(nodename));
	if(strcmp(plotidz,"") != 0)
	{
		sprintf(nodename,"%s%s",MTP_N,plotidz);
		cflag = read_portal_xml(MULTI_PORTAL_F, &c_head, &cnum, nodename);
	}
	else
	{
		cflag = -1;
	}
	
	fprintf(fp,"<tr height=30 align=left>");
	if(cflag==0)
	{
		p_cq=c_head.next;
		while(p_cq !=NULL)
		{
			if ('\0' != p_cq->p_type[0])
			{
				fprintf(fp,	"<td width=100>%s</td>",p_cq->p_type);				
			}else
			{
				fprintf(fp,	"<td width=100>KeyWord</td>");
			}
			break;
			p_cq=p_cq->next;
		}
	}
	
	if (0 != cflag || NULL == p_cq)
	{
		fprintf(fp,"<td width=100>Key Word</td>");	
	}

	
	fprintf(fp,"<td width=180><input type=text name=key_word value=\"%s\" size=15 readonly style=\"width:200px;\"></td>",cq.p_keyword);
		
	fprintf(fp,"</tr>"\
				"<tr height=30 align=left>"\
				"<td width=100>URL</td>"\
				"<td><input type=text name=ipaddr value=\"%s\" size=15 style=\"width:400px;\"></td>"\
				"</tr>",cq.pip);
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>ACname</td>"\
				"<td><input type=text name=acname value=\"%s\" size=15 style=\"width:200px;\"></td>"\
				"</tr>",cq.pacname);
	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>Domain</td>"\
				"<td><input type=text name=domain value=\"%s\" size=15 style=\"width:200px;\"></td>"\
				"</tr>",cq.pdomain);

	fprintf(fp,"<tr height=30 align=left>"\
				"<td width=100>Advertise URL</td>"\
				"<td><input type=text name=advertise_url value=\"%s\" size=15 style=\"width:200px;\"></td>"\
				"</tr>",cq.advertise_url);
	
	fprintf(fp,"<tr height=30 align=left>");
    fprintf(fp,"<td width=100>%s</td>",search(portal_auth,"error_offline_port"));
	if( strcmp(cq.pjom,"JSON") == NULL)
	{
		fprintf(fp, "<td><input type=text name=noteport id=noteport disabled style=\"width:200px;\"></td>\n");
	}
	else
	{
		fprintf(fp, "<td><input type=text name=noteport id=noteport value=\"%s\" style=\"width:200px;\"></td>\n",cq.pnport);
	}
	fprintf(fp,"</tr>\n");
	fprintf(fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",plotidz);				
	fprintf(fp,	"</table>");

	fprintf(fp,"<input type=hidden name=i_index value=%d>",i_index);	
	fprintf(fp,"<input type=hidden name=NODEZ value=%s>",nodez);	
	fprintf(fp,"<input type=hidden name=ATTZ value=%s>",attz);	
	fprintf(fp,"<input type=hidden name=PLOTIDZ value=%s>",plotidz);	

	if(freeflag==0)
		Free_get_portal_struct(&cq);
	
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

