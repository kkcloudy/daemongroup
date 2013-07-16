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
* wp_black_list_domain.c
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
#include <ctype.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"

#include "ws_init_dbus.h"

#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_dcli_vrrp.h"
#include "ws_eag_conf.h"
#include "drp/drp_def.h"
#include "drp/drp_interface.h"
#include "ws_dbus_list_interface.h"

#ifndef  BLACK_LIST_DOMAIN
#define  BLACK_LIST_DOMAIN

#define SUBMIT_NAME "submit_black_list_domain"



#define CP_BLACK_BLACK_LIST_FILE "/opt/services/option/portal_option"

//#define SCRIPT_PATH	"sudo /usr/bin/"

#define ADD_BLACK_LIST_DOMAIN_SH	"sudo /usr/bin/cp_add_black_list_domain.sh %d %s"

#define DEL_BLACK_LIST_DOMAIN_SH "sudo /usr/bin/cp_del_black_list_domain.sh %d %s"

#define CP_BLACK_LIST_FLAG_DOMAIN 4

#define MAX_ID_NUM_DOMAIN		17

#define MAX_BLACK_LIST_NUM_DOMAIN	512



#endif


typedef struct {
	STPortalContainer *pstradiusContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;			  /*存储解密后的当前登陆用户名*/
	int iUserGroup; //为0时表示是管理员。
	FILE *fp;
	int plotid;
	
} STPageInfo;

static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};

#if 0
int Check_domain_Name(char * domain);
#endif

static int
is_domain(const char *str)
{
	int i = 0;
	for (i = 0; str[i] != 0; i++) {
		if (0 == isalnum(str[i])
		&& '.' != str[i]
		&& '-' != str[i]) {
			return 0;
		}
	}
	if (str[0] == '-' || str[i-1] == '-') {
		return 0;
	}
	return 1;
}


/***************************************************************
申明回调函数
****************************************************************/
static int s_black_list_domain_prefix_of_page( STPageInfo *pstPageInfo );
static int s_black_list_domain_content_of_page( STPageInfo *pstPageInfo );


static int black_list_domain_run_script(const char *domain, int plotid, char add_or_del);
/***************************************************************
*USEAGE:	 主函数
*Param:
*Return:
*
*Auther:WangKe
*Date:2009-8-11
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	int ret = 0;
	STPageInfo stPageInfo;

	DcliWInit();
	ccgi_dbus_init();  
	
	
//初始化常用公共变量
	memset( &stPageInfo, 0, sizeof(stPageInfo ) );
	ret = init_portal_container(&(stPageInfo.pstradiusContainer));
	stPageInfo.lpublic = stPageInfo.pstradiusContainer->lpublic;
	stPageInfo.lauth = stPageInfo.pstradiusContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	//stPageInfo.username_encry = dcryption(stPageInfo.encry);
	if ( WS_ERR_PORTAL_ILLEGAL_USER == ret )
	{
		ShowErrorPage(search(stPageInfo.lpublic, "ill_user"));	  /*用户非法*/
		release_portal_container(&(stPageInfo.pstradiusContainer));
		return 0;
	}
	//stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );
	stPageInfo.iUserGroup = stPageInfo.pstradiusContainer->iUserGroup;
//stPageInfo.pstModuleContainer = MC_create_module_container();
	if ( NULL == stPageInfo.pstradiusContainer )
	{
		release_portal_container(&(stPageInfo.pstradiusContainer));
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




		char del_opt[10]={0};
		cgiFormStringNoNewlines("del_opt",del_opt,sizeof(del_opt));
		if(!strcmp(del_opt,"delete"))
		{
			int ret = 0;
			char domain [128]={0};
			
			
			cgiFormStringNoNewlines("domain",domain,sizeof(domain));
			
			//fprintf(stderr,"insid=%d$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",insid);
			//fprintf(stderr,"domain=%s$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",domain);
			
						
			RULE_TYPE type = RULE_DOMAIN;
			
			//fprintf(stderr,"delete is ok-------------------------------------\n");
			ret = eag_conf_captive_list(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									type,
									"",
									"",
									domain,
									"",
									CP_DEL_LIST,
									CP_BLACK_LIST);
			fprintf(stderr,"ret=%d$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",ret);
			
	
		}



//处理表单




	MC_setActiveLabel( stPageInfo.pstradiusContainer->pstModuleContainer, WP_EAG_BLACKLIST);
	
	MC_setPrefixOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_black_list_domain_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_black_list_domain_content_of_page, &stPageInfo );
	
	
	MC_setOutPutFileHandle( stPageInfo.pstradiusContainer->pstModuleContainer, cgiOut );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_METHOD, "post" );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic, "img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );
	
	
	MC_writeHtml( stPageInfo.pstradiusContainer->pstModuleContainer );

	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstradiusContainer));
	
	
	return 0;
}


static int s_black_list_domain_prefix_of_page( STPageInfo *pstPageInfo )
{
	//char del_rule[10] = "";
	//char index[32] = "";
	char domain[256] = {0};
	char tmp_domain[256] = {0};
	int ret = -1;
	RULE_TYPE type = RULE_DOMAIN;
	FILE *fp = pstPageInfo->fp;
	int i = 0;
	domain_pt domain_conf;
	domain_ct domain_ctr;
	
	fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );
	#if 0
	fprintf(fp, "<script type=\"text/javascript\">"\
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
	#endif
	
	memset(tmp_domain,0,sizeof(tmp_domain));
			
	if ( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		//fprintf(stderr,"submit\n");
		cgiFormStringNoNewlines("domain_input", tmp_domain, sizeof(tmp_domain));
		
		if (is_domain(tmp_domain) == 0)
		{
			ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));
			return 0;
		}

		memset(&domain_conf,0,sizeof(domain_conf));
		strncpy((domain_conf.domain_name),tmp_domain,sizeof(domain_conf.domain_name)-1);
		memset(&domain_ctr,0,sizeof(domain_ctr));
		ccgi_dbus_init(); 
		ret = conf_drp_get_domain_ip(ccgi_dbus_connection,	&domain_conf, &domain_ctr);
		if (0 == domain_ctr.num)
		{
			ShowAlert( search( pstPageInfo->lauth, "blacklist_add_err" ) );
			fprintf( fp, "<script type='text/javascript'>\n" );
			fprintf( fp, "window.location.href='wp_black_list_domain.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry, plotid);
			fprintf( fp, "</script>\n" );
			return 0;
		}

		if (0 == ret) {
			int nbyte = 0;
			for (i = 0; i < domain_ctr.num; i++){
				nbyte = snprintf(domain, sizeof(domain) -1, "%s", (char *)tmp_domain);
				for (i = 0; i<domain_ctr.num; i++){
					nbyte += snprintf(domain+nbyte,sizeof(domain)-nbyte -1,";%lu",domain_ctr.domain_ip[i].ipaddr);
				}
			}
		}
		fprintf(stderr,"#cgic# ------------------------------------- \n");
		fprintf(stderr,"#cgic# ------------------ ret is: %d\n",ret);
		fprintf(stderr,"#cgic# ------------------ domain_ctr.num is: %d\n",domain_ctr.num);
		

		ret = eag_conf_captive_list(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									type,
									"",
									"",
									domain,
									"",
									CP_ADD_LIST,
									CP_BLACK_LIST);
		

	}
	#if 0
	cgiFormStringNoNewlines( "DELRULE", del_rule, 10 );
	if ( !strcmp(del_rule, "delete") )
	{
		//fprintf(stderr,"delete\n");
		cgiFormStringNoNewlines( "domain_name", domain_name, sizeof(domain_name) );
		if (black_list_domain_run_script(domain_name, pstPageInfo->plotid, 'd') != 0)
		{
			ShowAlert( search( pstPageInfo->lauth, "blacklist_add_err" ) );
		}		
	}
	#endif
	return 0;
}

static int s_black_list_domain_content_of_page( STPageInfo *pstPageInfo )
{
	int i;
	FILE * fp = pstPageInfo->fp;
	
	if ( NULL == pstPageInfo )
	{
		return -1;
	}
	
	fprintf(cgiOut, "	 <table>\n");
	fprintf(cgiOut, "		 <tr>\n"\
									"			 <td colspan='3'>\n"\
									"				 <table>\n"\
									"					 <tr>\n"\
									"						 <td>ID:</td>\n"\
									"						 <td>\n"\
									"						 <select name='plotid' style='width:100%%;height:auto' onchange='on_id_change(this);'>\n");
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
	fprintf(cgiOut, "						 </select>\n" );
	fprintf( cgiOut, "			 			<script type=text/javascript>\n"\
										"							 function on_id_change(obj)\n"
										"								 {\n"\
										"									 window.location.href='wp_black_list_domain.cgi?UN=%s&plotid='+obj.value;\n"\
										"								 }\n", pstPageInfo->encry );
	fprintf( cgiOut, " 					 </script>\n" );
	fprintf(cgiOut, "						 </td>\n"\
									"					 </tr>\n");
									
	fprintf( cgiOut, "					<tr>\n"\
										"						<td>Mode:</td>\n"\
										"						<td>\n"\
										"						<select name='mode' style='width:100%%;height:auto' onchange='on_mode_change(this);'>\n"\
										"							<option value=0 >Ip</option>"\
										"							<option value=1 selected>Domain</option>"\
										"						</select>\n");
	fprintf( cgiOut, "						<script type=text/javascript>\n"\
										"							function on_mode_change(obj)\n"
										"							{\n"\
										"								window.location.href='wp_black_list.cgi?UN=%s&plotid='+%d;\n", pstPageInfo->encry, pstPageInfo->plotid);
	fprintf( cgiOut, "							}\n");
	fprintf( cgiOut, "					</script>\n"
										"					</td>\n"\
										"				</tr>\n" );
										
										
// domain
	fprintf( cgiOut, "					 <tr>\n"\
										"						 <td>Domain:</td>\n"\
										"						 <td><input type=text name=domain_input value='' /></td>\n"\
										"					 </tr>\n" );
										
										
	fprintf( cgiOut, "				</table>\n"\
										"			 </td>\n"\
										"		 </tr>\n");
										
	/*fprintf( cgiOut, "</table>" );*/
	
	
	
	
	fprintf(cgiOut, "		 <tr>\n");
	fprintf(cgiOut, "			 <td>\n"\
									"				 <table frame=below rules=rows border=1 style='border-collapse:collapse;align:left;overflow:visible'>\n" );
#if 1
	fprintf( cgiOut, " 				 <tr height=30 bgcolor=#eaeff9 id=td1 align=left>\n");
	fprintf(cgiOut, "						 <td width=240 style=font-size:12px><font>Domain</font></td>\n");
	fprintf(cgiOut, "						 <td width=30></td>\n");
	fprintf(cgiOut, "					 </tr>\n");
	
	
#endif


    int ret =-1;

	RULE_TYPE type;
	struct bw_rules black;
	memset(&black,0,sizeof(black));
	
	//fprintf(stderr,"show insid=%d...............................+++++++++++\n",insid);
	//fprintf(stderr,"hansitype=%d........................++++++++++++++\n",hansitype);
	ret = eag_show_black_list(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									&black); 
	//fprintf(stderr,"ret=%d...............................+++++++++++\n",ret);
	if(ret == 0)
	{
		int num; 
		num = black.curr_num;
		
		//fprintf(stderr,"num=%d\n+++++++++++++++++",num);
 
		if(num >0 )
		{
			//char ipbegin[32];
			//char ipend[32];
			//char ports[CP_MAX_PORTS_BUFF_LEN]={0};
			//char intf[MAX_IF_NAME_LEN]={0};

			char domain[CP_MAX_BW_DOMAIN_NAME_LEN];
			

			for(i = 0 ; i < num ; i++)
			{   
			    type = black.rule[i].type;
				if(type == RULE_DOMAIN)
				{
	  				//ip2str(white.rule[i].key.ip.ipbegin,ipbegin,sizeof(ipbegin));
	  				//ip2str(white.rule[i].key.ip.ipend,ipend,sizeof(ipend));
	  				//strcpy(ports,white.rule[i].key.ip.ports);
	  				//strcpy(intf,white.rule[i].intf);
	  				//fprintf(stderr,"intf=%s+++++++++++++++++++++&&&&&&&&&&&&&&&&&&&&&&&&&&&&&",intf);
	  				strcpy(domain,black.rule[i].key.domain.name);
	  				fprintf(cgiOut,"					<tr height=25>\n");
	  				fprintf(cgiOut,"						<td style=font-size:12px align=left>%s</td>\n",domain);
	  				fprintf(cgiOut,"						<td>\n" );
	  				fprintf(cgiOut, "						<script type=text/javascript>\n" );
	  				fprintf(cgiOut , "							var popMenu%d = new popMenu('popMenu%d','%d');\n", i, i,num-i);
	  				fprintf(cgiOut, "							popMenu%d.addItem( new popMenuItem( '%s', 'wp_black_list_domain.cgi?del_opt=%s&UN=%s&plotid=%s&domain=%s' ) );\n",
	  									i,"delete","delete", pstPageInfo->encry, plotid ,domain ); 
	  				fprintf(cgiOut, "							popMenu%d.show();\n", i );
	  				fprintf(cgiOut, "						</script>\n" );
	  				fprintf(cgiOut, "						</td>\n");
	  				fprintf(cgiOut,"					</tr>\n");
				}


			}
				
		}
	}	




	
#if 0
 

	char menu[21] = "";
	char i_char[10] = "";
	
/////////////读取数据/////////////////////////
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	
//char plotid[32];
	char domain_name[512];
	
	int i_portal_id;
	
	int cl = 0;
	
	FILE * fd = NULL ;
	
//fprintf(cgiOut,	"<tr height=30 align=left bgcolor=#eaeff9>");

//fprintf(cgiOut, "<th width=200>%s</th>", search(pstPageInfo->lauth,"domain_name") );

//fprintf(cgiOut, "<th width=13>&nbsp;</th>"\
//		"</tr>");


	if ( NULL != (fd = fopen(CP_BLACK_BLACK_LIST_FILE, "r")))
	{
	
		//fprintf( stderr, "file here\n" );
		// int locate = 0;
		int index = 0;
		int flag = -1;
		
		while ( (fgets( buf, sizeof(buf), fd )) != NULL )
		{
			if( !strcmp(buf,"") )	
			{			
				continue;//空行跳过 	
			}
			
			//memset(plotid, 0, sizeof(plotid));
			memset(domain_name, 0, sizeof(domain_name));
			
			// fprintf(stderr,"sccanf:::::::::::::::\n");
			sscanf(buf, "%d %d %[^\n]\n", &flag, &i_portal_id, domain_name);
			//fprintf(stderr,"buf=%s",buf);
			// fprintf(stderr,"i_portal_id=%d~~~~~~~domain_name=%s\n",i_portal_id,domain_name);
			if ( CP_BLACK_LIST_FLAG_DOMAIN != flag )
			{
				continue;
			}
			if (i_portal_id != pstPageInfo->plotid)
			{
				continue;
			}
			index++;
			
			memset(menu, 0, 21);
			strcpy(menu, "menulist");
			//sprintf(i_char,"%d",nas_num+1);
			//strcat(menu,i_char);
			
			char temp[32];
			memset(&temp, 0, 32);
			sprintf(temp, "%d", index);
			strcat(menu, temp);
			
			fprintf(cgiOut,	"<tr height=30 align=left bgcolor=%s>", setclour(cl) );
			//fprintf(fp,	"<td>%d</td>", locate);
			
			fprintf(cgiOut,	"<td>%s</td>", domain_name );
			
			//fprintf(fp,	"<td>%s</td>", );
			fprintf(cgiOut, "<td>");
			fprintf(cgiOut, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">", (MAX_BLACK_LIST_NUM_DOMAIN - index), menu, menu);
			fprintf(cgiOut, "<img src=/images/detail.gif>"\
											"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">", menu);
			fprintf(cgiOut, "<div id=div1>");
			fprintf(cgiOut, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\">"\
											"<a id=link href=wp_black_list_domain.cgi?UN=%s&plotid=%d&DELRULE=%s&domain_name=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , pstPageInfo->plotid, "delete" , domain_name , search(pstPageInfo->lpublic, "delete"));
			fprintf(cgiOut, "</div>"\
											"</div>"\
											"</div>");
			fprintf(cgiOut,	"</td>");

			
			fprintf(cgiOut,	"</tr>");
			
			memset(buf, 0, sizeof(buf));
			cl = !cl;
			
		}
		fclose(fd);
	}
	#endif
	fprintf(cgiOut,	"				</table>"\
									"			</td>");
	fprintf(cgiOut,	"			<td width='50'>&nbsp;</td>\n");
	
	fprintf( cgiOut,	"			<td></td>" );
	
	fprintf(cgiOut,	"		</tr>\n"\
									"	</table>\n");
	fprintf(cgiOut, "<input type=hidden name=plotid value=%s>", plotid);
	fprintf(cgiOut, "<input type=hidden name=UN value=%s>", pstPageInfo->encry);
	return 0;
}
static int black_list_domain_run_script(const char *domain, int plotid, char add_or_del)
{

	if (NULL == domain)
	{
		return 0;
	}
	
	if (add_or_del != 'a' && add_or_del != 'd')
	{
		return 0;
	}
	char cmd[256] = "";
	int status, ret;
	
	
	if (add_or_del == 'a')
	{
		snprintf( cmd, sizeof(cmd) - 1, ADD_BLACK_LIST_DOMAIN_SH , plotid, domain);
	} else if (add_or_del == 'd')
	{
		snprintf( cmd, sizeof(cmd) - 1, DEL_BLACK_LIST_DOMAIN_SH , plotid, domain);
	}
	if (strstr(cmd,";"))
	{
		return -1;
	}
	status = system(cmd);
	ret = WEXITSTATUS(status);
	
	if ( 0 == ret )
	{
		//add_addr_to_list( p_list, ipaddr_begin, ipaddr_end, port );
	}
	return ret;//脚本中应该返回0表示这个ip添加成功，否则添加失败。*/
}

#if 0
int Check_domain_Name(char * domain)
{
	if( NULL == domain )
	{
		return -1;
	}
	if( strcmp(domain, "") == 0)
	{
		return -1;
	}
	int i = 0;
	char * src = domain ;

	while( src[i] != '\0')
	{
		if( (src[i] >= '0' && src[i] <= '9') 	|| 
			(src[i] >= 'a' && src[i] <= 'z') 	||
			(src[i] >= 'A' && src[i] <= 'Z') 	||
			 src[i] == '.' 	|| src[i] == '_' || src[i] == '-' 
		  )
		{
			i++;
			continue;
		}
		else
		{
			return -1;
		}
	
	}

	return 0;
}
#endif

