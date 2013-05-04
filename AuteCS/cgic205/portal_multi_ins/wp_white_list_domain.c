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
* wp_white_list_domain.c
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

#ifndef  WHITE_LIST_DOMAIN
#define  WHITE_LIST_DOMAIN

#define SUBMIT_NAME "submit_white_list_domain"

//#define WHITE_LIST_DOMAIN_STATUS_FILE_PATH	"/opt/services/status/whiteListDomain_status.status"

//#define WHITE_LIST_DOMAIN_CONF_FILE_PATH	"/opt/services/conf/whiteListDomain_conf.conf"

#define CP_WHITE_BLACK_LIST_FILE "/opt/services/option/portal_option"

//#define SCRIPT_PATH	"sudo /usr/bin/"

#define ADD_WHITE_LIST_DOMAIN_SH	"sudo /usr/bin/cp_add_white_list_domain.sh %d %s"

#define DEL_WHITE_LIST_DOMAIN_SH "sudo /usr/bin/cp_del_white_list_domain.sh %d %s"

#define CP_WHITE_LIST_FLAG_DOMAIN 3

#define MAX_ID_NUM_DOMAIN		8

#define MAX_WHITE_LIST_NUM_DOMAIN	512



//#define MULTI_STATUS_FILE_PATH "/opt/services/conf/multi_radius.conf"

//#define MULTI_radius_CONF_PATH "/opt/services/conf/multi_radius.conf"

//#define MAX_INDEX_LEN 32

#endif

//#define SUBMIT_NAME "submit_nasid_by_vlan"



typedef struct {
	STPortalContainer *pstradiusContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;			  /*存储解密后的当前登陆用户名*/
	int iUserGroup; //为0时表示是管理员。
	FILE *fp;
	int portal_id;
	
} STPageInfo;


/***************************************************************
申明回调函数
****************************************************************/
static int s_white_list_domain_prefix_of_page( STPageInfo *pstPageInfo );
static int s_white_list_domain_content_of_page( STPageInfo *pstPageInfo );


static int white_list_domain_run_script(const char *domain, int portal_id, char add_or_del);
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

	STPageInfo stPageInfo;
	
//初始化常用公共变量
	memset( &stPageInfo, 0, sizeof(STPageInfo) );
	
	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	char portal_id_buf[BUF_LEN];
	
	
	memset( portal_id_buf, 0, sizeof(portal_id_buf) );
	
	
	if (cgiFormStringNoNewlines("portal_id", portal_id_buf, sizeof(portal_id_buf)) == cgiFormNotFound)
	{
		stPageInfo.portal_id = 0;
	} else
	{
		sscanf(portal_id_buf, "%d", &(stPageInfo.portal_id));
	}
	
	stPageInfo.username_encry = dcryption(stPageInfo.encry);
	if ( NULL == stPageInfo.username_encry )
	{
		ShowErrorPage(search(stPageInfo.lpublic, "ill_user"));	  /*用户非法*/
		return 0;
	}
	stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );
	
//stPageInfo.pstModuleContainer = MC_create_module_container();
	init_portal_container(&(stPageInfo.pstradiusContainer));
	if ( NULL == stPageInfo.pstradiusContainer )
	{
		return 0;
	}
	stPageInfo.lpublic = stPageInfo.pstradiusContainer->lpublic;//get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lauth = stPageInfo.pstradiusContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");
	
	stPageInfo.fp = cgiOut;
//初始化完毕


//处理表单




	MC_setActiveLabel( stPageInfo.pstradiusContainer->pstModuleContainer, 1 );
	
	MC_setPrefixOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_white_list_domain_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstradiusContainer->pstModuleContainer, (MC_CALLBACK)s_white_list_domain_content_of_page, &stPageInfo );
	
	
	MC_setOutPutFileHandle( stPageInfo.pstradiusContainer->pstModuleContainer, cgiOut );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, FORM_METHOD, "post" );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic, "img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstradiusContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );
	
	
	MC_writeHtml( stPageInfo.pstradiusContainer->pstModuleContainer );
	
	release_portal_container(&(stPageInfo.pstradiusContainer));
	
	
	return 0;
}


static int s_white_list_domain_prefix_of_page( STPageInfo *pstPageInfo )
{
	char del_rule[10] = "";
	char index[32] = "";
	FILE * fp = pstPageInfo->fp;
	
	/*
	//if file not exist,creat it and write "start" in it
	FILE * p_file_status =NULL ;
	char buf_start[]="start\n";
	if( (p_file_status = fopen(WHITE_LIST_DOMAIN_STATUS_FILE_PATH,"w+")) != NULL )
	{
	
	fwrite(buf_start,strlen(buf_start),1,p_file_status);
	//fprintf( stderr, "write status\n");
	fclose(p_file_status);
	}*/
	
	
	
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
									
	FILE * p_file = NULL;
	char portal_id_buf[64];
	char domain_name[512];
	
	memset(domain_name, 0, sizeof(domain_name));
	memset(portal_id_buf, 0, sizeof(portal_id_buf));
	if ( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		//fprintf(stderr,"submit\n");
		cgiFormStringNoNewlines("domain_input", domain_name, sizeof(domain_name));
		cgiFormStringNoNewlines("portal_id_hide", portal_id_buf, sizeof(portal_id_buf));
		
		pstPageInfo->portal_id = atoi(portal_id_buf);
		
		if (!strcmp(domain_name, ""))
		{
			ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));
			return 0;
		}
		/////////////处理数据///////////////////
		
		//char content[2048];
		//memset( content, 0, sizeof(content));
		//sprintf( content, "%d;%s\n", pstPageInfo->portal_id, domain_name);
		int ret_add = white_list_domain_run_script(domain_name, pstPageInfo->portal_id, 'a');
		if (ret_add != 0)
		{			
			if(ret_add == 3)
			{
				ShowAlert( search( pstPageInfo->lauth, "whitelist_add_err_no_dns" ) );
			}
			else
			{
				ShowAlert( search( pstPageInfo->lauth, "whitelist_add_err" ) );
			}
			/*
			//fprintf(stderr,"get submit,content=%s",content);
			//fprintf(stderr,"content=%s",content);
			if( NULL != (p_file = fopen(WHITE_LIST_DOMAIN_CONF_FILE_PATH,"a+")) )
			{
			fwrite(content, strlen(content), 1, p_file);
			//fflush(fp);
			fclose(p_file);
			ShowAlert( "Add successful." );
			//ShowAlert( search(radius_public, "input_illegal") );
			}*/
		}
		
		/*
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_white_list_domain.cgi?UN=%s&portal_id=%d';\n", pstPageInfo->encry,pstPageInfo->portal_id);
		fprintf( fp, "</script>\n" );*/
		
	}
	cgiFormStringNoNewlines( "DELRULE", del_rule, 10 );
	if ( !strcmp(del_rule, "delete") )
	{
		//fprintf(stderr,"delete\n");
		cgiFormStringNoNewlines( "domain_name", domain_name, sizeof(domain_name) );
		if(white_list_domain_run_script(domain_name, pstPageInfo->portal_id, 'd') != 0)
		{
			ShowAlert( search( pstPageInfo->lauth, "whitelist_add_err" ) );
		}
	}
	return 0;
}

static int s_white_list_domain_content_of_page( STPageInfo *pstPageInfo )
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
									"						 <select name='portal_id' style='width:100%%;height:auto' onchange='on_id_change(this);'>\n");
	for (i = 0;i < MAX_ID_NUM_DOMAIN;i++)
	{
		if ( pstPageInfo->portal_id == i )
		{
			fprintf(cgiOut, "						<option value='%d' selected>%d</option>", i, i);
		}
		else
		{
			fprintf(cgiOut, "						<option value='%d'>%d</option>", i, i);
		}
	}
	fprintf(cgiOut, "						 </select>\n" );
	fprintf( cgiOut, "			 			<script type=text/javascript>\n"\
										"							 function on_id_change(obj)\n"
										"								 {\n"\
										"									 window.location.href='wp_white_list_domain.cgi?UN=%s&portal_id='+obj.value;\n"\
										"								 }\n", pstPageInfo->encry );
	fprintf( cgiOut, " 					 </script>\n" );
	fprintf(cgiOut, "						 </td>\n"\
									"					 </tr>\n");
									
	fprintf( cgiOut, "					<tr>\n"\
										"						<td>Mode:</td>\n"\
										"						<td>\n"\
										"						<select name='portal_id' style='width:100%%;height:auto' onchange='on_mode_change(this);'>\n"\
										"							<option value=0 >Ip</option>"\
										"							<option value=1 selected>Domain</option>"\
										"						</select>\n");
	fprintf( cgiOut, "						<script type=text/javascript>\n"\
										"							function on_mode_change(obj)\n"
										"							{\n"\
										"								window.location.href='wp_white_list.cgi?UN=%s&portal_id='+%d;\n", pstPageInfo->encry, pstPageInfo->portal_id);
	fprintf( cgiOut, "							}\n");
	fprintf( cgiOut, "					</script>\n"
										"					</td>\n"\
										"				</tr>\n" );
										
										
// domain
	fprintf( cgiOut, "					 <tr>\n"\
										"						 <td>Domain:</td>\n"\
										"						 <td><input type=text name=domain_input value='' /></td>\n"\
										"					 </tr>\n" );

	//note
	fprintf( cgiOut, "					 <tr>\n"\
						"						 <td colspan=2>(%s)</td>\n"\										
						"					 </tr>\n" ,search( pstPageInfo->lauth, "whitelist_must_set_dns_first" ));
										
										
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
	
	
//---------------------

	char menu[21] = "";
	char i_char[10] = "";
	
/////////////读取数据/////////////////////////
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	
//char portal_id[32];
	char domain_name[512];
	
	int i_portal_id;
	
	int cl = 0;
	
	FILE * fd = NULL ;
	
//fprintf(cgiOut,	"<tr height=30 align=left bgcolor=#eaeff9>");

//fprintf(cgiOut, "<th width=200>%s</th>", search(pstPageInfo->lauth,"domain_name") );

//fprintf(cgiOut, "<th width=13>&nbsp;</th>"\
//		"</tr>");


	if ( NULL != (fd = fopen(CP_WHITE_BLACK_LIST_FILE, "r")))
	{
	
		//fprintf( stderr, "file here\n" );
		// int locate = 0;
		int index = 0;
		int flag;
		
		while ( (fgets( buf, sizeof(buf), fd )) != NULL )
		{
			if( !strcmp(buf,"") )	
			{			
				continue;//空行跳过 	
			}
			
			//memset(portal_id, 0, sizeof(portal_id));
			memset(domain_name, 0, sizeof(domain_name));
			
			// fprintf(stderr,"sccanf:::::::::::::::\n");
			sscanf(buf, "%d %d %[^\n]\n", &flag, &i_portal_id, domain_name);
			//fprintf(stderr,"buf=%s",buf);
			// fprintf(stderr,"i_portal_id=%d~~~~~~~domain_name=%s\n",i_portal_id,domain_name);
			if ( CP_WHITE_LIST_FLAG_DOMAIN != flag )
			{
				continue;
			}
			if (i_portal_id != pstPageInfo->portal_id)
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
			fprintf(cgiOut, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">", (MAX_WHITE_LIST_NUM_DOMAIN - index), menu, menu);
			fprintf(cgiOut, "<img src=/images/detail.gif>"\
											"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">", menu);
			fprintf(cgiOut, "<div id=div1>");
			fprintf(cgiOut, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\">"\
											"<a id=link href=wp_white_list_domain.cgi?UN=%s&portal_id=%d&DELRULE=%s&domain_name=%s target=mainFrame>%s</a></div>", pstPageInfo->encry , pstPageInfo->portal_id, "delete" , domain_name , search(pstPageInfo->lpublic, "delete"));
			fprintf(cgiOut, "</div>"\
											"</div>"\
											"</div>");
			fprintf(cgiOut,	"</td>");
			/*
			
			fprintf(cgiOut,"						<td>\n" );
			fprintf( cgiOut, "						<script type=text/javascript>\n" );
			fprintf( cgiOut, "							var popMenu%d = new popMenu('popMenu%d','%d');\n", i, i, pstPageInfo->st_white_list.num-i );
			fprintf( cgiOut, "							popMenu%d.addItem( new popMenuItem( '%s', 'wp_white_list.cgi?UN=%s&portal_id=%s&del_white_list=%s&ip_end=%s&port=%s' ) );\n",
			i,"delete", pstPageInfo->encry, pstPageInfo->st_white_list.id,pstPageInfo->st_white_list.ipaddr_list[i].ipaddr_begin,
			pstPageInfo->st_white_list.ipaddr_list[i].ipaddr_end, pstPageInfo->st_white_list.ipaddr_list[i].port );
			fprintf( cgiOut, "							popMenu%d.show();\n", i );
			fprintf( cgiOut, "						</script>\n" );
			fprintf( cgiOut, "						</td>\n");*/
			
			fprintf(cgiOut,	"</tr>");
			
			memset(buf, 0, sizeof(buf));
			cl = !cl;
			
		}
		fclose(fd);
	}
	fprintf(cgiOut,	"				</table>"\
									"			</td>");
	fprintf(cgiOut,	"			<td width='50'>&nbsp;</td>\n");
	
	fprintf( cgiOut,	"			<td></td>" );
	
	fprintf(cgiOut,	"		</tr>\n"\
									"	</table>\n");
	fprintf(cgiOut, "<input type=hidden name=portal_id_hide value=%d>", pstPageInfo->portal_id);
	
	return 0;
}
static int white_list_domain_run_script(const char *domain, int portal_id, char add_or_del)
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
		snprintf( cmd, sizeof(cmd) - 1, ADD_WHITE_LIST_DOMAIN_SH , portal_id, domain);
	} else if (add_or_del == 'd')
	{
		snprintf( cmd, sizeof(cmd) - 1, DEL_WHITE_LIST_DOMAIN_SH , portal_id, domain);
	}
	status = system(cmd);
	ret = WEXITSTATUS(status);
	
	if ( 0 == ret )
	{
		//add_addr_to_list( p_list, ipaddr_begin, ipaddr_end, port );
	}
	return ret;//脚本中应该返回0表示这个ip添加成功，否则添加失败。*/
}
