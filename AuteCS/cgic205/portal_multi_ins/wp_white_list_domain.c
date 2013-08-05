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
#include <ctype.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"
//#include "ws_domain_conf.h"

#include "ws_init_dbus.h"

#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_dcli_vrrp.h"
#include "ws_eag_conf.h"
#include "drp/drp_def.h"
#include "drp/drp_interface.h"
#include "ws_dbus_list_interface.h"

#ifndef  WHITE_LIST_DOMAIN
#define  WHITE_LIST_DOMAIN

#define SUBMIT_NAME "submit_white_list_domain"

//#define WHITE_LIST_DOMAIN_STATUS_FILE_PATH	"/opt/services/status/whiteListDomain_status.status"

//#define WHITE_LIST_DOMAIN_CONF_FILE_PATH	"/opt/services/conf/whiteListDomain_conf.conf"

#define CP_WHITE_BLACK_LIST_FILE "/opt/services/option/portal_option"

//#define SCRIPT_PATH	"sudo /usr/bin/"

#define ADD_WHITE_LIST_DOMAIN_SH	"sudo /usr/bin/cp_add_white_list_domain.sh %d %s  >/dev/null 2>&1"
#define ADD_WHITE_LIST_DOMAIN_IP_SH	   "sudo /usr/bin/cp_add_white_list_domain_ip.sh %d %s  %s>/dev/null 2>&1"
#define DEL_WHITE_LIST_DOMAIN_SH "sudo /usr/bin/cp_del_white_list_domain.sh %d %s >/dev/null 2>&1"

#define CP_WHITE_LIST_FLAG_DOMAIN 16

#define MAX_ID_NUM_DOMAIN		17

#define MAX_WHITE_LIST_NUM_DOMAIN	128

#define XML_DOMAIN_D                        "/opt/www/htdocs/dns_cache.xml"
#define XML_DOMAIN_PATH	   	      "/opt/services/option/domain_option"
#define  DOMAIN_NAME_IP_TEMP_PATH    "/opt/services/option/white_list_domain_temp"
#define PORTAL_LIST_PATH	"/opt/services/option/portal_option"


//#define MULTI_STATUS_FILE_PATH "/opt/services/conf/multi_radius.conf"

//#define MULTI_radius_CONF_PATH "/opt/services/conf/multi_radius.conf"

//#define MAX_INDEX_LEN 32

#endif

//#define SUBMIT_NAME "submit_nasid_by_vlan"
static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};



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
static int s_white_list_domain_prefix_of_page( STPageInfo *pstPageInfo );
static int s_white_list_domain_content_of_page( STPageInfo *pstPageInfo );


static int white_list_domain_run_script(const char *domain, int plotid, char add_or_del);
static int check_white_list_domain(int cp_id, const char *domain);
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
	memset( &stPageInfo, 0, sizeof(STPageInfo) );

	ret = init_portal_container(&(stPageInfo.pstradiusContainer));
	stPageInfo.lpublic = stPageInfo.pstradiusContainer->lpublic;
	stPageInfo.lauth = stPageInfo.pstradiusContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	char portal_id_buf[BUF_LEN];
	
	
	memset( portal_id_buf, 0, sizeof(portal_id_buf) );
	
	
	memset(plotid,0,sizeof(plotid));
	cgiFormStringNoNewlines("plotid", plotid, sizeof(plotid)); 
	fprintf(stderr,"-------domain-----------plotid=%s",plotid);
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
//初始化完毕


//处理表单
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
			ret = eag_conf_captive_list(ccgi_connection, parameter.local_id, parameter.instance_id,
					type, "", "", domain, "", CP_DEL_LIST, CP_WHITE_LIST);
			//fprintf(stderr,"ret=%d$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",ret);
			
	
		}



	MC_setActiveLabel( stPageInfo.pstradiusContainer->pstModuleContainer, WP_EAG_WHITELIST);
	
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

	free_instance_parameter_list(&paraHead1);
	
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

	
 	fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );
	
	int i = 0;
	domain_pt domain_conf;
	domain_ct domain_ctr;
	#if 0  
	
	fprintf(fp, "<script type=\"text/javascript\">\n"\
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
									"}\n"\
									"</script>\n");
	#endif
									  
	char domain[256] = {0};
	char tmp_domain[256] = {0};
	int ret = 1;
	RULE_TYPE type = RULE_DOMAIN;

	if ( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess )
	{
		
		cgiFormStringNoNewlines("domain_input", tmp_domain, sizeof(tmp_domain));
		

		if (is_domain(tmp_domain) == 0)
		{
			ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));
			return 0;
		}
		#if 0
		if (!strcmp(tmp_domain, ""))
		{
			ShowAlert( search(pstPageInfo->lpublic, "input_illegal"));
			return 0;
		}
		#endif
		/////////////////////////////
		memset(&domain_conf,0,sizeof(domain_conf));
		strncpy((domain_conf.domain_name),tmp_domain,sizeof(domain_conf.domain_name)-1);
		memset(&domain_ctr,0,sizeof(domain_ctr));
		ret = conf_drp_get_domain_ip(ccgi_connection, &domain_conf, &domain_ctr);
		if (0 == domain_ctr.num)
		{
		
			ShowAlert( search( pstPageInfo->lauth, "whitelist_add_err" ) );
			fprintf( fp, "<script type='text/javascript'>\n" );
			fprintf( fp, "window.location.href='wp_white_list_domain.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
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
		/////////////////////////////
		ret = eag_conf_captive_list(ccgi_connection, parameter.local_id, parameter.instance_id,
				type, "", "", domain, "", CP_ADD_LIST, CP_WHITE_LIST);
		if (ret != 0)
		{			
			if(ret == 3)
			{
				ShowAlert( search( pstPageInfo->lauth, "whitelist_add_err_no_dns" ) );
			}
			else
			{
				ShowAlert( search( pstPageInfo->lauth, "whitelist_add_err" ) );
			}
			
		}
		
		/*
		fprintf( fp, "<script type='text/javascript'>\n" );
		fprintf( fp, "window.location.href='wp_white_list_domain.cgi?UN=%s&plotid=%d';\n", pstPageInfo->encry,pstPageInfo->plotid);
		fprintf( fp, "</script>\n" );*/
		
	}
	#if 0
	cgiFormStringNoNewlines( "DELRULE", del_rule, 10 );
	if ( !strcmp(del_rule, "delete") )
	{
		//fprintf(stderr,"delete\n");
		cgiFormStringNoNewlines( "domain_name", domain_name, sizeof(domain_name) );
		if(white_list_domain_run_script(domain_name, pstPageInfo->plotid, 'd') != 0)
		{
			ShowAlert( search( pstPageInfo->lauth, "whitelist_add_err" ) );
		}
	}
	#endif
	return 0;
}

static int s_white_list_domain_content_of_page( STPageInfo *pstPageInfo )
{
	int i;
	instance_parameter *pq = NULL;
	char temp[10] = { 0 };
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
										"									 window.location.href='wp_white_list_domain.cgi?UN=%s&plotid='+obj.value;\n"\
										"								 }\n", pstPageInfo->encry );
	fprintf( cgiOut, " 					 </script>\n" );
	fprintf(cgiOut, "						 </td>\n"\
									"					 </tr>\n");
									
	fprintf( cgiOut, "					<tr>\n"\
										"						<td>Mode:</td>\n"\
										"						<td>\n"\
										"						<select name='plotid' style='width:100%%;height:auto' onchange='on_mode_change(this);'>\n"\
										"							<option value=0 >Ip</option>"\
										"							<option value=1 selected>Domain</option>"\
										"						</select>\n");
	fprintf( cgiOut, "						<script type=text/javascript>\n"\
										"							function on_mode_change(obj)\n"
										"							{\n"\
										"								window.location.href='wp_white_list.cgi?UN=%s&plotid=%s';\n", pstPageInfo->encry,plotid);
	fprintf( cgiOut, "							}\n");
	fprintf( cgiOut, "					</script>\n"
										"					</td>\n"\
										"				</tr>\n" );
										
										
// domain
	fprintf( cgiOut, "					 <tr>\n"\
										"						 <td>Domain:</td>\n"\
										"						 <td><input type=text name=domain_input value='' maxLength=256/></td>\n"\
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




	int ret=-1; 
	int hansitype;
	int insid;

	RULE_TYPE type;
	struct bw_rules white;
	char id[4]={0};
	memset(&white,0,sizeof(white));
	
	ret = eag_show_white_list(ccgi_connection, parameter.local_id, parameter.instance_id, &white); 
	if(ret == 0)
	{
		int num; 
		num = white.curr_num;
 
		if(num >0 )
		{

			char domain[CP_MAX_BW_DOMAIN_NAME_LEN] = {0};
			for(i = 0 ; i < num ; i++)
			{   
			    type = white.rule[i].type;
				memset(domain,0,sizeof(domain));
				if(type == RULE_DOMAIN)
				{
	  				strncpy(domain,white.rule[i].key.domain.name,sizeof(domain)-1);
	  				fprintf(cgiOut,"					<tr height=25>\n");
	  				fprintf(cgiOut,"						<td style=font-size:12px align=left>%s</td>\n",domain);
	  				fprintf(cgiOut,"						<td>\n" );
	  				fprintf(cgiOut, "						<script type=text/javascript>\n" );
	  				fprintf(cgiOut , "							var popMenu%d = new popMenu('popMenu%d','%d');\n", i, i,num-i);
	  				fprintf(cgiOut, "							popMenu%d.addItem( new popMenuItem( '%s', 'wp_white_list_domain.cgi?del_opt=%s&UN=%s&plotid=%s&domain=%s' ) );\n",
	  									i,"delete","delete", pstPageInfo->encry, plotid ,domain ); 
	  				fprintf(cgiOut, "							popMenu%d.show();\n", i );
	  				fprintf(cgiOut, "						</script>\n" );
	  				fprintf(cgiOut, "						</td>\n");
	  				fprintf(cgiOut,"					</tr>\n");
				}


			}
				
		}
	}	
	
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
#if 0
static int white_list_domain_run_script(const char *domain, int plotid, char add_or_del)
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
	int domain_num=0, free_domain_flag=0, ret_delete_node=-2;
	int number_of_whitelist_domain=0;

	if(access(XML_DOMAIN_PATH ,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo cp %s	%s",XML_DOMAIN_D,XML_DOMAIN_PATH);
		system(cmd); 			
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 666 %s",XML_DOMAIN_PATH);
		system(cmd);
	}
	
	if (add_or_del == 'a')
	{
		//snprintf( cmd, sizeof(cmd) - 1, ADD_WHITE_LIST_DOMAIN_SH , plotid, domain);
		if (check_white_list_domain(plotid, domain)){
			/*the same domain has already existed in white list domain*/
			return   1;
		}
		/* find the domainname<-->ip in XML file*/
		struct domain_st domain_head,*domain_cq=NULL;	
		check_and_get_domainname_xml(XML_DOMAIN_PATH,&domain_head,"domain",domain,&domain_num);
		//vty_out(vty,"the domain num is %d for the domain name %s\n",domain_num,argv[1]);
		if( 0 == domain_num)
		{	
			/*a new domain name,can't find in the dns cache xml file*/
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd, sizeof(cmd)-1, ADD_WHITE_LIST_DOMAIN_SH, plotid, domain);
			if(strchr(cmd,';') == NULL)
			{
				status = system(cmd);
				ret = WEXITSTATUS(status);
				if (0 == ret)
				{
					//vty_out(vty, "add white list succeeded!\n");
					add_domain_second_node_attr(XML_DOMAIN_PATH,"domain","attribute",domain);
					add_domain_ip_into_cache_xml(domain);
					return ret;
				}
			}
		}
		else
		{            	
		/*find the domain name in XML DNS cache file,don't parse the domain name*/
				//vty_out(vty,"don't parse the domain name\n");
				domain_cq=domain_head.next;
            			while( domain_cq != NULL)
            	                    {
					//vty_out(vty," white domain name =%s ,ip=%s\n",domain_cq->domain_name,domain_cq->domain_ip);
					//vty_out(vty , "%s\t",domain_cq->domain_ip);
					memset(cmd,0,sizeof(cmd));
					snprintf(cmd, sizeof(cmd)-1, ADD_WHITE_LIST_DOMAIN_IP_SH, plotid, domain,domain_cq->domain_ip);
					if(strchr(cmd,';') == NULL)
					{
						status = system(cmd);
						ret = WEXITSTATUS(status);
#if 0
						if( 0 != ret)
						{
							//vty_out(vty, "add white list failed!\n");
							free_read_domain_xml(&domain_head,&free_domain_flag);
							return ret;
						}
						else 
						{
							domain_cq=domain_cq->next;
						}
#endif
						if( 0 == ret || 4 == ret )
						{
							domain_cq=domain_cq->next;
						}
						else 
						{
							//vty_out(vty, "add white list failed!\n");
							free_read_domain_xml(&domain_head,&free_domain_flag);
							//vty_out(vty,"bbb don't parse the domain name\n");
							return	 2;
						}
					}
            	       		 }	
				free_read_domain_xml(&domain_head,&free_domain_flag);
				return   0;    /*add white list successful,return 0 */
		}
		
	}
	else if (add_or_del == 'd')
	{
		//snprintf( cmd, sizeof(cmd) - 1, DEL_WHITE_LIST_DOMAIN_SH , plotid, domain);
		if (!check_white_list_domain(plotid, domain)){
			/*white list domain is not exist in this captive portal*/
			return 1;
		}
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd, sizeof(cmd)-1, DEL_WHITE_LIST_DOMAIN_SH, plotid, domain);
		if(strchr(cmd,';') == NULL)
		{
			status = system(cmd);
			ret = WEXITSTATUS(status);
			ret_delete_node=delete_domain_second_xmlnode(XML_DOMAIN_PATH,"domain","attribute",domain);
			if (0 == ret && 0 == ret_delete_node)
			{
				//del white list succeeded!
				return 0;
			}
			else 
			{
				//del white list failed!
				return 1;
			}
		}
	}
	//status = system(cmd);
	//ret = WEXITSTATUS(status);
	
	//return ret;//脚本中应该返回0表示这个ip添加成功，否则添加失败。*/
}

#endif
static int check_white_list_domain(int cp_id, const char *domain)
{
	FILE *fp = NULL;
	char line[256], *p = NULL, *token = NULL;
	
	memset(line, 0, sizeof(line));
		
	if (cp_id < 0 || cp_id > 7)
		return 0;
		
	if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL)
		return 0;
	
	while (fgets(line, sizeof(line)-1, fp) != NULL){
		for (p = line; *p; p++)
			if ('\r' == *p || '\n' == *p){
				*p = '\0';
				break;
			}
		if ( (token = strtok(line, " \t")) == NULL || atoi(token) != CP_WHITE_LIST_FLAG_DOMAIN)
			continue;
		if ( (token = strtok(NULL, " \t")) == NULL || atoi(token) != cp_id)
			continue;
		if ( (token = strtok(NULL, " \t")) != NULL && strcmp(token, domain) == 0)
			return 1;	
	}
	
	fclose(fp);
		
	return 0;
}
