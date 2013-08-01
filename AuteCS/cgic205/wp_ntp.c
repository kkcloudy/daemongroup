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
* wp_ntp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for system Network Time Protocol
*
*
*******************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ac_manage_def.h"
#include "ws_public.h"
#include "ws_dbus_list.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"
#include "ac_manage_extend_interface.h"
#include "ac_manage_ntpsyslog_interface.h"
#include "ws_log_conf.h"

#define MINS_MAX_CRON 59
#define MINS_MIN_CRON 10
#define HOURS_MAX_CRON 23
#define HOURS_MIN_CRON 1
#define DAYS_MAX_CRON 31
#define DAYS_MIN_CRON 1

int ShowExportConfPage(struct list *lpublic, struct list *lsystem,struct list *lcontrol);     /*m代表加密后的字符串*/
int if_ip_null(char *keyname,char *zipz);/*0:succ,1:fail*/
int count_progress();
void config_iptype();
void config_crontime(struct list *lpublic);

int cgiMain()
{
	struct list *lpublic;	
	struct list *lsystem;	  
	struct list *lcontrol;
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
    if_ntp_exist();
	ShowExportConfPage(lpublic,lsystem,lcontrol);
  	release(lpublic);  
  	release(lsystem);
  	release(lcontrol);

	return 0;
}



int ShowExportConfPage(struct list *lpublic, struct list *lsystem,struct list *lcontrol)
{ 
	 
		char *encry=(char *)malloc(BUF_LEN);				
		char *str;
		char addn[N]="";
		char state[10] = {0};
		char slotid[5]={0};
		char del_ty[10]={0};
		int s_id=0,nodenum=0,limit=0;
		int j,if_ntp=0,cl=1,ret=-1,flag1=-1,flag2=-1,flag3=-1,flagz = 0;
		DBusConnection *connection;
		instance_parameter *paraHead2 = NULL;
		instance_parameter *p_q = NULL;
		int p_masterid = 0;
		int master_slot_id=0;
		char *sntpip=(char *)malloc(32);
		char *smask=(char *)malloc(32);
        char *cntpip=(char *)malloc(32);
		char * deleteOP=(char *)malloc(10);
		char ntpv[10] = {0};
		char inter_name[20]={0};
		char vename[20]={0};
		char cperz[10];
		memset(smask,0,32);
		memset(sntpip,0,32);
		memset(cntpip,0,32);
		memset(cperz,0,10);
		memset(deleteOP,0,10);		
	 
	 	ccgi_dbus_init();
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user"));		  
			return 0;
		}
		strcpy(addn,str);
		
		list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
		p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
		master_slot_id = get_product_info(SEM_ACTIVE_MASTER_SLOT_ID_PATH);	
		
		/***********************2013.7.29*********************/
		cgiHeaderContentType("text/html");
		fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
		fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
		fprintf(cgiOut,"<title>%s</title>",search(lpublic,"ntp_s"));
		fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
			"<style type=text/css>"\
			".a3{width:30;border:0; text-align:center}"\
			"</style>"\
			"</head>"\
			"<script language=javascript src=/ip.js>"\
			"</script>");
	   fprintf(cgiOut,"<script language=javascript>\n");
	   fprintf(cgiOut, "function getradio(rname)\n"\
			"{\n"\
				"var v1=document.getElementsByName(rname);\n"\
				"var i;"\
				"if(v1!=null)"\
				"{"\
					"for(i=0;i<v1.length;i++)"\
					"{"\
						"if(v1[i].checked)"\
						"{"\
							"return v1[i].value;"\
						"}"\
					"}"\
				"}"\
				"else"\
				"{"\
					"return null;"\
				"}"\
			"}\n");
	   fprintf(cgiOut, "function mysubmit()\n"\
			"{\n"\
				"var v1 = getradio(\"ipv4\");\n"\
				"var v2 = getradio(\"ipv6\");\n"\
				"if((v1 == '2')&&(v2 == '2'))\n"\
				"{\n"\
					"alert('radio select cannot be all ignore');"\
					"return false;\n"\
				"}\n"\
			"}\n");
			fprintf(cgiOut,"</script>"\
			"<body>");

		cgiFormStringNoNewlines("DELETE",deleteOP,10);
		cgiFormStringNoNewlines("TY",del_ty,10);
	    if(cgiFormSubmitClicked("ntpstart") == cgiFormSuccess)
        {
			memset(state,0,10);
        	ret=start_ntp();
			memset(state,0,sizeof(state));
			strncpy(state,"enable",sizeof(state)-1);

			if(ret==0)
    		{				
				mod_first_xmlnode(NTP_XML_FPATH, NODE_LSTATUS, state);
				for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
				{
					ac_manage_set_ntpstatus_rule(p_q->connection, state);
					if(p_masterid == p_q->parameter.slot_id)
					{
						set_master_default_server_func();
					}
					else
					{
						ac_manage_inside_ntp_rule(p_q->connection);
					}
				}
        		ShowAlert(search(lpublic,"oper_succ"));	
    		}
    	    else
    	    {			
        		ShowAlert(search(lpublic,"oper_fail"));
    	    }
	
        }
		if(cgiFormSubmitClicked("ntpstop") == cgiFormSuccess)
        {
        	ret=stop_ntp();
			memset(state,0,sizeof(state));
			strncpy(state,"disable",sizeof(state)-1);

    		if(ret==0)
    		{	
        		ShowAlert(search(lpublic,"oper_succ"));	
				mod_first_xmlnode(NTP_XML_FPATH, NODE_LSTATUS, state);
				for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
				{
					ac_manage_set_ntpstatus_rule(p_q->connection, state);
					if(p_masterid == p_q->parameter.slot_id)
					{
						set_master_default_server_func();
					}
					else
					{
						ac_manage_inside_ntp_rule(p_q->connection);
					}
				}				
    		}
    	    else
    	    {
    		    ShowAlert(search(lpublic,"oper_fail")); 		
    	    }
        }
		if(cgiFormSubmitClicked("resetntp") == cgiFormSuccess)
        {
        	ret=restart_ntp();
			memset(state,0,sizeof(state));
			strncpy(state,"enable",sizeof(state)-1);

    		if(ret==0)
    		{	
        		ShowAlert(search(lpublic,"oper_succ"));	
				mod_first_xmlnode(NTP_XML_FPATH, NODE_LSTATUS, state);
				for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
				{
					ac_manage_set_ntpstatus_rule(p_q->connection, state);
					if(p_masterid == p_q->parameter.slot_id)
					{
						set_master_default_server_func();
					}
					else
					{
						ac_manage_inside_ntp_rule(p_q->connection);
					}
				}				
    		}
    	    else
    	    {
    		    ShowAlert(search(lpublic,"oper_fail")); 		
    	    }
        }
		if(strcmp(deleteOP,"delete")==0)
		{
			if(strcmp(del_ty,"client")==0)
			{
				memset(slotid,0,5);
				memset(sntpip,0,32);
				memset(smask,0,32);
				cgiFormStringNoNewlines("del_slot",slotid,5);
				cgiFormStringNoNewlines("ip_cli",sntpip,32);
				cgiFormStringNoNewlines("mask_cli",smask,32);
				s_id=strtoul(slotid,0,10);
				if((strcmp(slotid,"")==0)||(s_id == master_slot_id))
				{
					flagz = 0;
					find_second_xmlnode(NTP_XML_FPATH,	NTP_SERVZ, NTP_SIPZ,sntpip,&flagz);
					if(0 != flagz)
					{
						ret = del_second_xmlnode(NTP_XML_FPATH,  NTP_SERVZ , flagz);
						if(ret != 0)
						{
							ShowAlert(search(lpublic,"oper_fail")); 
						}
					}
				}
				else
				{
					struct serverz_st rule;
					memset(&rule,0,sizeof(rule));
					
					strcpy(rule.servipz,sntpip);
					strcpy(rule.maskz,smask);
					for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
					{
						if(s_id == p_q->parameter.slot_id)
						{
							connection = p_q->connection;
						}
					}
					ret = ac_manage_add_ntpclient_rule(connection, &rule,OPT_DEL);
					if(ret != 0)
					{
						ShowAlert(search(lpublic,"oper_fail")); 
					}
				}
			}
			else if(strcmp(del_ty,"server")==0)
			{	
				memset(cntpip,0,32);
				cgiFormStringNoNewlines("IP_ser",cntpip,32);
				flagz = 0;
				find_second_xmlnode(NTP_XML_FPATH, NTP_CLIZ, NTP_CIPZ,cntpip,&flagz);
				ret = del_second_xmlnode(NTP_XML_FPATH, NTP_CLIZ , flagz);
				if(ret != 0)
				{
					ShowAlert(search(lpublic,"oper_fail")); 
				}
			}
			else if(strcmp(del_ty,"upserver")==0)
			{	
				memset(cntpip,0,32);
				memset(cperz,0,10);
				memset(slotid,0,5);
				cgiFormStringNoNewlines("up_per", cperz, 10); 
				cgiFormStringNoNewlines("ipup_cli",cntpip,32);
				cgiFormStringNoNewlines("delup_slot", slotid, 10); 
				s_id=strtoul(slotid,0,10);
				connection = NULL;
				struct clientz_st rule_array;
				memset(&rule_array,0,sizeof(rule_array));
				strcpy(rule_array.clitipz,cntpip);
				strcpy(rule_array.ifper,cperz);
				strncpy(rule_array.slotid,slotid,sizeof(rule_array.slotid)-1);
				for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
				{
					if(s_id == p_q->parameter.slot_id)
					{
						connection = p_q->connection;
					}
				}
				ret = ac_manage_add_ntpserver_rule(connection, &rule_array, OPT_DEL);
				if(0 != ret)
				{
					ShowAlert(search(lpublic,"oper_fail")); 
				}
			}
		}
		if(cgiFormSubmitClicked("clientadd") == cgiFormSuccess)
		{
			
			memset(slotid,0,5);
			memset(sntpip,0,32);
			memset(smask,0,32);
			cgiFormStringNoNewlines("insid",slotid,5);
            flag1=if_ip_null("vlan_ip", sntpip);
			flag2=if_ip_null("mask_ip", smask);
			s_id=strtoul(slotid,0,10);
			if((flag1==0)&&(flag2==0))
			{
				if((strcmp(slotid,"")!=0)&&((s_id != master_slot_id)))
				{
					struct serverz_st rule;
					memset(&rule,0,sizeof(rule));
					
					strcpy(rule.servipz,sntpip);
					strcpy(rule.maskz,smask);
					for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
					{
						if(s_id == p_q->parameter.slot_id)
						{
							connection = p_q->connection;
						}
					}
					ret = ac_manage_add_ntpclient_rule(connection, &rule,OPT_ADD);
					if(0 != ret)
					{
						ShowAlert(search(lpublic,"oper_fail")); 
					}
				}
				else
				{
					if(ntp_serverip_duplication(sntpip))
					{
						if(0 == strncmp(sntpip,"169.254.1.0",11))
						{
							ShowAlert(search(lpublic,"ip_dupli")); 
						}
						else
						{
							ShowAlert(search(lpublic,"ip_ntp_exit")); 
						}
					}
					else
					{
						ret = add_ntp_server(NTP_XML_FPATH, sntpip, smask);
						save_ntp_conf ();		
						if(0 != ret)
						{
							ShowAlert(search(lpublic,"oper_fail")); 
						}
					}
				}
			}
			else
			{
			    if(flag1!=0)
			 		ShowAlert(search(lpublic,"ip_not_null"));
				if(flag2!=0)
					ShowAlert(search(lpublic,"mask_not_null"));
			}
		}
		if(cgiFormSubmitClicked("serveradd") == cgiFormSuccess)
		{
			memset(cntpip,0,32);
			memset(cperz,0,10);
			flag3=if_ip_null("serv_ip", cntpip);
			cgiFormStringNoNewlines("ifper", cperz, 10); 
			if(flag3==0)
			{
				if(ntp_clientip_duplication(cntpip))
				{
					ShowAlert(search(lpublic,"ip_ntp_exit")); 
				}
				else
				{
					ret = add_ntp_client(NTP_XML_FPATH,cntpip, cperz);
					if(0 != ret)
					{
						ShowAlert(search(lpublic,"oper_fail")); 
					}
					save_ntp_conf ();					
				}
			}
			else
			{
			 	ShowAlert(search(lpublic,"ip_not_null"));
			}		
		}
		if(cgiFormSubmitClicked("upserveradd") == cgiFormSuccess)
		{
			int if_ve=0;
			memset(cntpip,0,sizeof(cntpip));
			memset(cperz,0,sizeof(cperz));
			memset(inter_name,0,sizeof(inter_name));
			memset(slotid,0,sizeof(slotid));
			flag3=if_ip_null("upserv_ip", cntpip);
			cgiFormStringNoNewlines("upifper", cperz, 10); 
			cgiFormStringNoNewlines("inter_na", inter_name, 20); 
			cgiFormStringNoNewlines("upinsid",slotid,5);
			s_id=strtoul(slotid,0,10);
			if(flag3==0)
			{	
				connection = NULL;
				struct clientz_st rule_array;
				memset(&rule_array,0,sizeof(rule_array));
				strcpy(rule_array.clitipz,cntpip);
				strcpy(rule_array.ifper,cperz);
				strncpy(rule_array.slotid,slotid,sizeof(rule_array.slotid)-1);
				if_ve = ve_interface_parse(inter_name, &vename, sizeof(vename));
				if(0 != if_ve)
				{
					memset(vename,0,sizeof(vename));
					strncpy(vename,inter_name,sizeof(vename)-1);
				}
				for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
				{
					if(s_id == p_q->parameter.slot_id)
					{
						connection = p_q->connection;
					}
				}
				ret = ac_manage_add_ntpserver_rule(connection, &rule_array, OPT_ADD);
				ac_manage_config_ntp_pfm_requestpkts(ccgi_dbus_connection, vename, cntpip,OPT_ADD);	
				if(0 != ret)
				{
					ShowAlert(search(lpublic,"oper_fail")); 
				}
			}
			else
			{
			 	ShowAlert(search(lpublic,"ip_not_null"));
			}		
		}
		if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)
		{
			config_iptype();
			config_crontime(lpublic);
		}
		if(cgiFormSubmitClicked("cleanntp") == cgiFormSuccess)
		{
			for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
			{
					connection = NULL;
					connection = p_q->connection;
					ac_manage_clean_ntp_rule(connection);					
			}
		}

	  fprintf(cgiOut,"<form method=post onsubmit=\"return mysubmit()\">"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  
	  fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
		fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  
			fprintf(cgiOut,"<tr><td width=62 align=center><input id=but type=submit name=log_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	       	fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		fprintf(cgiOut,"</tr>"\
		"</table>"); 
	 
	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>");
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>");
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 border=0 cellspacing=0 cellpadding=0>"); 	

			  fprintf(cgiOut,"<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>"); 
                     /*管理员*/
                     if(checkuser_group(addn)==0)
					{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
							fprintf(cgiOut,"</tr>");						
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"ntp_s"));  /*突出显示*/
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE");
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_pppoe_snp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE SNP");
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
							fprintf(cgiOut,"</tr>");
					}
					else
					{

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>");							

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");
					}
                    count_xml_node(NTP_XML_FPATH, &nodenum);
					limit=nodenum+8;				
					
					fprintf(stderr,"-----------------------------nodenum=%d\n", nodenum);
					fprintf(stderr,"-----------------------------limit=%d\n", limit);
					for(j=0;j<limit;j++)
					{
					    fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
					}
                 
				       fprintf(cgiOut,"</table>");
				        fprintf(cgiOut,"</td>"\
				        "<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");
						fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");
						/*******************设置ntp服务状态*******************/
						fprintf(cgiOut,"<tr height=20><td></td></tr>");
						fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
						  "<td width=690>"\
							"<fieldset align=left>"\
							  "<legend><font color=Navy>%s</font></legend>",search(lpublic,"ntp_s"));
							  fprintf(cgiOut,"<table width=680 border=0 cellspacing=0 cellpadding=0>");
							  if_ntp=count_progress();
							  fprintf(stderr,"--------------------------if_ntp=%d\n",if_ntp);
							  fprintf(cgiOut,"<tr style=padding-top:15px>");
							  fprintf(cgiOut,"<td width=140>");
							  fprintf(cgiOut,"<input type=submit  name=ntpstart value=\"%s\">",search(lpublic,"ntp_start"));
							  fprintf(cgiOut,"</td><td>\n");
							  fprintf(cgiOut,"<input type=submit  name=ntpstop value=\"%s\">",search(lpublic,"ntp_stop"));
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td><input type=submit name=resetntp value=\"%s\" width=140></td>\n",search(lpublic,"ntp_reboot"));
							  fprintf(cgiOut,"</tr>");
							  
							  fprintf(cgiOut,"<tr height=10><td></td></tr>");
							fprintf(cgiOut,"<tr>");
							fprintf(cgiOut,"<td width=140>%s:</td>",search(lpublic,"ntp_s"));
							fprintf(cgiOut,"<td><font color=blue>%s</font></td>\n",((if_ntp!=0)?search(lcontrol,"start"):search(lcontrol,"stop")));
							fprintf(cgiOut,"<td colspan=2></td>\n");
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"</table>");
							fprintf(cgiOut,"</fieldset>");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"</tr>");
						/************************ntp 其他设置***********************/	
						fprintf(cgiOut,"<tr height=20><td></td></tr>");
						fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
						  "<td width=690>"\
							"<fieldset align=left>"\
							  "<legend><font color=Navy>%s</font></legend>",search(lpublic,"other_ntp_set"));
							  fprintf(cgiOut,"<table width=680 border=0 cellspacing=0 cellpadding=0>");


							//ntp time
							char cront[20] = {0};
							char crontime[10] = {0};
							char cronradio[10] = {0};
							get_first_xmlnode(NTP_XML_FPATH, "cront", cront);
							sscanf(cront,"%[^^]%s",crontime,cronradio);
							fprintf(cgiOut,"<tr>\n");
							fprintf(cgiOut,"<td width=80>%s:</td>",search(lpublic,"ntp_cron"));
							if (NULL != strstr(cronradio,"mins"))
							{
								fprintf(cgiOut,"<td><input type=radio name=cronr value=\"1\" checked><font color=red>(%d-%d)%s</font></td>\n",MINS_MIN_CRON,MINS_MAX_CRON,search(lcontrol,"time_min"));
							}
							else
							{
								fprintf(cgiOut,"<td><input type=radio name=cronr value=\"1\"><font color=red>(%d-%d)%s</font></td>\n",MINS_MIN_CRON,MINS_MAX_CRON,search(lcontrol,"time_min"));
							}
							if (NULL != strstr(cronradio,"hours"))
							{
								fprintf(cgiOut,"<td><input type=radio name=cronr value=\"2\" checked><font color=red>(%d-%d)%s</font></td>\n",HOURS_MIN_CRON,HOURS_MAX_CRON,search(lcontrol,"hour"));
							}
							else
							{
								fprintf(cgiOut,"<td><input type=radio name=cronr value=\"2\"><font color=red>(%d-%d)%s</font></td>\n",HOURS_MIN_CRON,HOURS_MAX_CRON,search(lcontrol,"hour"));
							}
							if (NULL != strstr(cronradio,"days"))
							{
								fprintf(cgiOut,"<td><input type=radio name=cronr value=\"3\" checked><font color=red>(%d-%d)%s</font></td>\n",DAYS_MIN_CRON,DAYS_MAX_CRON,search(lcontrol,"day"));
							}
							else
							{
								fprintf(cgiOut,"<td><input type=radio name=cronr value=\"3\"><font color=red>(%d-%d)%s</font></td>\n",DAYS_MIN_CRON,DAYS_MAX_CRON,search(lcontrol,"day"));
							}
							fprintf(cgiOut,"<td><input type=text name=cront onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\""\
									"onpaste=\"var s=clipboardData.getData('text');	 if(!/\\D/.test(s)) value=s.replace(/^0*/,'');	 return   false;\" "\
									"ondragenter=\"return  false;\" "\
									"style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" maxLength=2  size=10 value=\"%s\"/></td>",crontime);
							fprintf(cgiOut,"</tr>\n");
							//ntp type
							memset(ntpv,0,sizeof(ntpv));
							get_first_xmlnode(NTP_XML_FPATH, "ntpv", ntpv);
							fprintf(cgiOut,"<tr>\n");
							fprintf(cgiOut,"<td width=80>NTP %s:</td>",search(lpublic,"type"));
							 if (0 == strcmp(ntpv,"3"))
							{
								 fprintf(cgiOut,"<td id=td2><input type=radio name=ntpver value=\"3\" checked>NTPv3</td>\n");
								 fprintf(cgiOut,"<td id=td2><input type=radio name=ntpver value=\"4\">NTPv4</td>\n");
							}
							 else if(0 == strcmp(ntpv,"4"))
							{
								 fprintf(cgiOut,"<td id=td2><input type=radio name=ntpver value=\"3\">NTPv3</td>\n");
								 fprintf(cgiOut,"<td id=td2><input type=radio name=ntpver value=\"4\" checked>NTPv4</td>\n");
							}
							 else
							 {
								 fprintf(cgiOut,"<td id=td2><input type=radio name=ntpver value=\"3\">NTPv3</td>\n");
								 fprintf(cgiOut,"<td id=td2><input type=radio name=ntpver value=\"4\">NTPv4</td>\n");
							 }
							 fprintf(cgiOut,"</tr>\n");

							//IP TYPE
							char newc1[20] = {0};
							get_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, &newc1);
							fprintf(cgiOut,"<tr>");
							fprintf(cgiOut,"<td width=80>%s:</td>\n","IPV4");
							if (0 == strcmp(newc1,"ignore"))
							{
								fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"1\">%s</td>\n",search(lpublic,"ntp_listen"));
								fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"2\" checked>%s</td>\n",search(lpublic,"ntp_ignore"));
							}
							else if(0 == strcmp(newc1,"listen"))
							{
								fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"1\" checked>%s</td>\n",search(lpublic,"ntp_listen"));
								fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"2\">%s</td>\n",search(lpublic,"ntp_ignore"));
							}
							else
							{
								fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"1\">%s</td>\n",search(lpublic,"ntp_listen"));
								fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"2\">%s</td>\n",search(lpublic,"ntp_ignore"));
							}
							fprintf(cgiOut,"<td></td>\n");
							fprintf(cgiOut,"</tr>\n");
							
							char newc2[20] = {0};
							get_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, &newc2);
							fprintf(cgiOut,"<tr>");
							fprintf(cgiOut,"<td width=80>%s:</td>\n","IPV6");
							if (0 == strcmp(newc2,"listen"))
							{
								fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"1\" checked>%s</td>\n",search(lpublic,"ntp_listen"));
								fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"2\">%s</td>\n",search(lpublic,"ntp_ignore"));
							}
							else if(0 == strcmp(newc2,"ignore"))
							{
								fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"1\">%s</td>\n",search(lpublic,"ntp_listen"));
								fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"2\" checked>%s</td>\n",search(lpublic,"ntp_ignore"));
							}
							else
							{
								fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"1\">%s</td>\n",search(lpublic,"ntp_listen"));
								fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"2\">%s</td>\n",search(lpublic,"ntp_ignore"));
							}
							fprintf(cgiOut,"<td></td>\n");
							fprintf(cgiOut,"</tr>\n");



							fprintf(cgiOut,"</table>");
							fprintf(cgiOut,"</fieldset>");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"</tr>");
							/************************客户端设定***********************/
							fprintf(cgiOut,"<tr height=20><td></td></tr>");
							fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
							  "<td width=690>"\
								"<fieldset align=left>"\
								  "<legend><font color=Navy>%s</font></legend>",search(lpublic,"ntp_ser"));
								  fprintf(cgiOut,"<table width=680 border=0 cellspacing=0 cellpadding=0>");
							
								fprintf(cgiOut,"<tr>");
								fprintf(cgiOut,"<td width=130>%s IP/MASK:</td>",search(lpublic,"ntp_vlan"));
								fprintf(cgiOut,"<td width=150>");
								fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
								fprintf(cgiOut,"<input type=text	name=vlan_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
								fprintf(cgiOut,"<input type=text	name=vlan_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								fprintf(cgiOut,"<input type=text	name=vlan_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								fprintf(cgiOut,"<input type=text	name=vlan_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
								fprintf(cgiOut,"</div></td>");	
								fprintf(cgiOut,"<td>\n");
								fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
								fprintf(cgiOut,"<input type=text	name=mask_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
								fprintf(cgiOut,"<input type=text	name=mask_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								fprintf(cgiOut,"<input type=text	name=mask_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								fprintf(cgiOut,"<input type=text	name=mask_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
								fprintf(cgiOut,"</div></td>");
								fprintf(cgiOut,"<td valign=top>SLOT ID:");
								fprintf( cgiOut, "<select name=insid>");
								for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
								{
									fprintf(cgiOut,"<option value=\"%d\">%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
								}
								fprintf( cgiOut, "</select>\n");	
								fprintf(cgiOut,"</td>");
								fprintf(cgiOut,"<td><input type=submit name=clientadd value=\"%s\"></td>\n",search(lpublic,"ntp_add"));
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr height=15><td colspan=5>");
								fprintf(cgiOut,"<table>"\
									"<tr bgcolor=#eaeff9 style=font-size:14px align=left>\n");
								fprintf(cgiOut,"<th width=120 style=font-size:12px>%s</th>","INDEX");
								fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","IP");
								fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","MASK");
								fprintf(cgiOut,"<th width=100 style=font-size:12px>%s</th>","SLOT ID");
								fprintf(cgiOut,"<th width=130 style=font-size:12px></th>");
								fprintf(cgiOut,"</tr>");
								struct serverz_st servst ,*sq;
								int servnum=0,count=0;
								for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
								{
									int rret=0;
									connection=	NULL;
									connection=p_q->connection;
									memset(&servst,0,sizeof(servst));
									rret=ac_manage_show_ntpclient_rule(connection,&servst,&servnum);
									fprintf(stderr,"rret=%d\n",rret);
									sq=servst.next;
									while(sq!=NULL)
									{
										if(0 != strncmp(sq->timeflag,"def",3))
										{
											count++;
											fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
											fprintf(cgiOut,"<td width=120 align=left>%d</td>",count);
											fprintf(cgiOut,"<td width=175 align=left>%s</td>\n",sq->servipz);
											fprintf(cgiOut,"<td width=175 align=left>%s</td>\n",sq->maskz);
											fprintf(cgiOut,"<td width=100 align=left>%d</td>\n",p_q->parameter.slot_id);
											fprintf(cgiOut,"<td width=130 align=left><a href=wp_ntp.cgi?UN=%s&DELETE=%s&TY=%s&del_slot=%d&ip_cli=%s&mask_cli=%s target=mainFrame><font color=black>%s</font></a></td>",encry,"delete","client",p_q->parameter.slot_id,sq->servipz,sq->maskz,search(lpublic,"delete"));
											fprintf(cgiOut,"</tr>\n");
											cl=!cl;
										}
										sq=sq->next;
									}
									if(servnum > 0)
										Free_read_ntp_server(&servst);
								}
								fprintf(cgiOut,"</table>");
								fprintf(cgiOut,"</td></tr>");
								fprintf(cgiOut,"</table>");
								fprintf(cgiOut,"</fieldset>");
								fprintf(cgiOut,"</td>");
								fprintf(cgiOut,"</tr>");


						/*********************服务器设定**********************/
						fprintf(cgiOut,"<tr height=20><td></td></tr>");
						fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
						  "<td width=690>"\
							"<fieldset align=left>"\
							  "<legend><font color=Navy>%s</font></legend>",search(lpublic,"ntp_server"));
							  fprintf(cgiOut,"<table width=680 border=0 cellspacing=0 cellpadding=0>");
							  fprintf(cgiOut,"<tr>");
							  fprintf(cgiOut,"<td width=120>IP:</td>");
							  fprintf(cgiOut,"<td width=150>\n");
							  fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text	  name=serv_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
							  fprintf(cgiOut,"<input type=text	  name=serv_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text	  name=serv_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text	  name=serv_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div></td>");  
							  fprintf(cgiOut,"<td width=150><input type=checkbox name=ifper value=\"prefer\">%s</td>\n",search(lpublic,"ntp_pri"));
							  fprintf(cgiOut,"<td><input type=submit name=serveradd value=\"%s\"></td>\n",search(lpublic,"ntp_add"));
							  fprintf(cgiOut,"</tr>");
							  
							  fprintf(cgiOut,"<tr height=15><td colspan=4>");
							  fprintf(cgiOut,"<table>"\
								  "<tr bgcolor=#eaeff9 style=font-size:14px align=left>\n");
							  fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","INDEX");
							  fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","IP");
							  fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","TYPE");
							  fprintf(cgiOut,"<th width=175 style=font-size:12px></th>");
							  fprintf(cgiOut,"</tr></table>\n");
							  fprintf(cgiOut,"</td></tr>");
							  
							  struct clientz_st clitst ,*cq;
							  memset(&clitst,0,sizeof(clitst));
							  int clinum=0;
							  read_ntp_client(NTP_XML_FPATH, &clitst, &clinum);
							  cq=clitst.next;
							  count=0;
							  while(cq!=NULL)
							  {
								  count++;
								  fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
								  fprintf(cgiOut,"<td width=175>%d</td>",count);
								  fprintf(cgiOut,"<td width=175>%s</td>\n",cq->clitipz);
								  fprintf(cgiOut,"<td width=175>%s</td>\n",(strncmp(cq->ifper,"prefer",6) == 0)?search(lpublic,"ntp_pri"):"");
								  fprintf(cgiOut,"<td width=175><a href=wp_ntp.cgi?UN=%s&DELETE=%s&IP_ser=%s&TY=%s target=mainFrame><font color=black>%s</font></a></td>",encry,"delete",cq->clitipz,"server",search(lpublic,"delete"));
								  fprintf(cgiOut,"</tr>\n");
								  cq=cq->next;
								  cl=!cl;
							  }
							  if(clinum>0)
							  {
								  Free_read_ntp_client(&clitst);
							  }

							fprintf(cgiOut,"</table>");
							fprintf(cgiOut,"</fieldset>");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"</tr>");

						/*********************上层服务器设定**********************/
						fprintf(cgiOut,"<tr height=20><td></td></tr>");
						fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
						  "<td width=690>"\
							"<fieldset align=left>"\
							  "<legend><font color=Navy>%s</font></legend>",search(lpublic,"ntp_sets"));
							  fprintf(cgiOut,"<table width=680 border=0 cellspacing=0 cellpadding=0>");
							  fprintf(cgiOut,"<tr>");
							  fprintf(cgiOut,"<td width=120>IP:</td>");
							  fprintf(cgiOut,"<td width=150>\n");
							  fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text	  name=upserv_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
							  fprintf(cgiOut,"<input type=text	  name=upserv_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text	  name=upserv_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text	  name=upserv_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div></td>");  
							  fprintf(cgiOut,"<td width=80><input type=checkbox name=upifper value=\"prefer\">%s</td>\n",search(lpublic,"ntp_pri"));


							  fprintf(cgiOut,"<td width=150>%s:<select name=inter_na>",search(lpublic,"inter"));
							  infi	interf;
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
							  free_inf(&interf);
							  fprintf(cgiOut,"</select></td>");

							  
//							  fprintf(cgiOut,"<td width=150>%s:<input type=text name=inter_na size=15></td>",search(lpublic,"inter"));
							  fprintf(cgiOut,"<td  width=120>SLOT ID:");
							  fprintf( cgiOut, "<select name=upinsid>");
							  for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
							  {
							  	if(p_q->parameter.slot_id != master_slot_id)
								  	fprintf(cgiOut,"<option value=\"%d\">%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
							  }
							  fprintf( cgiOut, "</select>\n");	  
							  fprintf(cgiOut,"</td>");
							  fprintf(cgiOut,"<td><input type=submit name=upserveradd value=\"%s\"></td>\n",search(lpublic,"ntp_add"));
							  fprintf(cgiOut,"</tr>");
							  
							  fprintf(cgiOut,"<tr height=15><td colspan=6>");
							  fprintf(cgiOut,"<table>"\
								  "<tr bgcolor=#eaeff9 style=font-size:14px align=left>\n");
							  fprintf(cgiOut,"<th width=120 style=font-size:12px>%s</th>","INDEX");
							  fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","IP");
							  fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","TYPE");
							  fprintf(cgiOut,"<th width=100 style=font-size:12px>%s</th>","SLOT ID");
							  fprintf(cgiOut,"<th style=font-size:12px width=130></th>");
							  fprintf(cgiOut,"</tr>");
							  
							  struct clientz_st upclitst ,*uq;
							  servnum=0;
							  count=0;
							  for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
							  {
							  	if(p_q->parameter.slot_id == master_slot_id)
							  	{
									continue;
								}
								  int rret=0;
								  connection= NULL;
								  connection=p_q->connection;
								  memset(&upclitst,0,sizeof(upclitst));
								  rret=ac_manage_show_ntpupserver_rule(connection,&upclitst,&servnum);
								  fprintf(stderr,"rret=%d\n",rret);
								  uq=upclitst.next;
								  while(uq!=NULL)
								  {
									  count++;
									  fprintf(cgiOut,"<tr bgcolor=%s  align=left>",setclour(cl));
									  fprintf(cgiOut,"<td width=120 >%d</td>",count);
									  fprintf(cgiOut,"<td width=175 >%s</td>\n",uq->clitipz);
									  fprintf(cgiOut,"<td width=175>%s</td>\n",(strncmp(uq->ifper,"prefer",6) == 0)?search(lpublic,"ntp_pri"):"");
									  fprintf(cgiOut,"<td width=100 >%d</td>\n",p_q->parameter.slot_id);
									  fprintf(cgiOut,"<td width=130 ><a href=wp_ntp.cgi?UN=%s&DELETE=%s&TY=%s&delup_slot=%d&ipup_cli=%s&up_per=%s target=mainFrame><font color=black>%s</font></a></td>",encry,"delete","upserver",p_q->parameter.slot_id,uq->clitipz,uq->ifper,search(lpublic,"delete"));
									  fprintf(cgiOut,"</tr>\n");
									  cl=!cl;
									  uq=uq->next;
								  }
								  if(servnum > 0)
									  Free_read_upper_ntp(&upclitst);
							  }

							fprintf(cgiOut,"</table>\n");
							fprintf(cgiOut,"</td></tr>");
							fprintf(cgiOut,"</table>");
							fprintf(cgiOut,"</fieldset>");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"</tr>");

							
							/*******************清除ntp配置*******************/
							fprintf(cgiOut,"<tr height=20><td></td></tr>");
							fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
							  "<td width=690>"\
								"<fieldset align=left>"\
								  "<legend><font color=Navy>%s</font></legend>",search(lpublic,"rm_conf"));
								  fprintf(cgiOut,"<table width=680 border=0 cellspacing=0 cellpadding=0>");

								  fprintf(cgiOut,"<tr>");
								  fprintf(cgiOut,"<td width=200><input type=radio name=clean_ntp value=\"1\" checked>%s</td>\n",search(lpublic,"clean_all_ntp"));
								  fprintf(cgiOut,"<td width=200><input type=radio name=clean_ntp value=\"2\">%s</td>\n",search(lpublic,"clean_slot_ntp"));
								  fprintf(cgiOut,"<td  width=120>SLOT ID:");
								  fprintf( cgiOut, "<select name=cleaninsid>");
								  for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
								  {
										fprintf(cgiOut,"<option value=\"%d\">%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
								  }
								  fprintf( cgiOut, "</select>\n");	  
								  fprintf(cgiOut,"</td>");
								  fprintf(cgiOut,"<td><input type=submit name=cleanntp value=\"%s\"></td>\n",search(lpublic,"remove"));
								  fprintf(cgiOut,"</tr>");
								  							
								fprintf(cgiOut,"</table>");
								fprintf(cgiOut,"</fieldset>");
								fprintf(cgiOut,"</td>");
								fprintf(cgiOut,"</tr>");

						/************************************************/
   fprintf(cgiOut,"</table>");
			  fprintf(cgiOut,"</td>"\
			  "</tr>"\
			  "<tr height=4 valign=top>"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
			  "</tr>"\
			"</table>");
		  fprintf(cgiOut,"</td>"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
		"</tr>"\
	  "</table></td>"); 
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
	  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
	"</tr>"\
	"</table>");
	fprintf(cgiOut,"</div>"\
	"</form>"\
	"</body>"\
	"</html>");
free(encry); 
free(sntpip); 
free(smask); 
free(cntpip); 
free(deleteOP); 
return 0;
}  
int if_ip_null(char *keyname,char *zipz)/*0:succ,1:fail*/
{
	char keyip[20];
	char ipz[4];
	char ipall[32];
	memset(ipall,0,32);
	int i=0,j=0,flag=0;
	for(i=1;i<5;i++)
	{
	    j++;
		memset(keyip,0,20);
		memset(ipz,0,4);
		sprintf(keyip,"%s%d",keyname,i);
		cgiFormStringNoNewlines(keyip, ipz, 4); 
		strcat(ipall,ipz);
		if(j != 4)
			strcat(ipall,".");
		
		if(strcmp(ipz,"")==0)
		{
			flag=1;
			break;
		}
	
	}
	if(flag==0)
		strcpy(zipz,ipall);
	
	return flag;
}
int count_progress()
{
	char buff[10] = { 0 };


	FILE *fp = NULL;
	int pronum=0;
	fp=popen("sudo ps -ef|grep ntpd |grep -v grep|wc -l","r");
	if(fp == NULL)
	{
		return 0;
	}
	fprintf(stderr,"-----------------------------buff=%s\n",buff);
	fgets( buff, sizeof(buff), fp);	
	fprintf(stderr,"-----------------------------buff=%s\n", buff);
	pronum=strtoul(buff,0,10);
	pclose(fp);

	return pronum;
}
void config_iptype()
{
	char ipv4[10] = {0};
	char ipv6[10] = {0};
	cgiFormStringNoNewlines("ipv4", ipv4, 10); 
	cgiFormStringNoNewlines("ipv6", ipv6, 10); 
	if (0 == strcmp(ipv4,"2"))
	{
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, "ignore");
	}
	else
	{
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, "listen");
	}
	if (0 == strcmp(ipv6,"1"))
	{
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, "listen");
	}
	else
	{
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, "ignore");
	}
}
void config_crontime(struct list *lpublic)
{
	char cront[20] = {0};
	char cronradio[10] = {0};
	char cront_str[20] = {0};
	char ntpv[10] = {0};
	int crontnum = 0;	
	
	cgiFormStringNoNewlines("cront", cront, sizeof(cront));
	cgiFormStringNoNewlines("cronr", cronradio, sizeof(cronradio));
	cgiFormStringNoNewlines("ntpver", ntpv, sizeof(ntpv));
	if (0 == strcmp(ntpv,"3"))
	{
		mod_first_xmlnode(NTP_XML_FPATH, "ntpv", "3");
	}
	else
	{
		mod_first_xmlnode(NTP_XML_FPATH, "ntpv", "4");
	}
	if ((0 != strcmp(cront,"")) && (0 != strcmp(cronradio,"")))
	{
		crontnum = atoi(cront);
		//mins
		if (0 == strcmp(cronradio,"1"))
		{
			if ((crontnum <= MINS_MAX_CRON) && (crontnum >= MINS_MIN_CRON))
			{
				memset(cront_str,0,sizeof(cront_str));
				snprintf(cront_str,sizeof(cront_str),"%d^%s",crontnum,"mins");
				mod_first_xmlnode(NTP_XML_FPATH, "cront",cront_str);
				save_ntp_conf ();
				system("sudo cronntp.sh > /dev/null");
			}
			else
			{
				ShowAlert(search(lpublic,"input_overflow"));
			}
		}
		//hours
		if (0 == strcmp(cronradio,"2"))
		{
			if ((crontnum <= HOURS_MAX_CRON) && (crontnum >= HOURS_MIN_CRON))
			{
				memset(cront_str,0,sizeof(cront_str));
				snprintf(cront_str,sizeof(cront_str),"%d^%s",crontnum,"hours");
				mod_first_xmlnode(NTP_XML_FPATH, "cront",cront_str);
				save_ntp_conf ();
				system("sudo cronntp.sh > /dev/null");
			}
			else
			{
				ShowAlert(search(lpublic,"input_overflow"));
			}
		}
		//days
		if (0 == strcmp(cronradio,"3"))
		{
			if ((crontnum <= DAYS_MAX_CRON) && (crontnum >= DAYS_MIN_CRON))
			{
				memset(cront_str,0,sizeof(cront_str));
				snprintf(cront_str,sizeof(cront_str),"%d^%s",crontnum,"days");
				mod_first_xmlnode(NTP_XML_FPATH, "cront",cront_str);
				save_ntp_conf ();
				system("sudo cronntp.sh > /dev/null");
			}
			else
			{
				ShowAlert(search(lpublic,"input_overflow"));
			}
		}
	}   
}
