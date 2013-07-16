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
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_log_conf.h"

#define MINS_MAX_CRON 59
#define MINS_MIN_CRON 10
#define HOURS_MAX_CRON 23
#define HOURS_MIN_CRON 1
#define DAYS_MAX_CRON 31
#define DAYS_MIN_CRON 1

int ShowExportConfPage(struct list *lpublic, struct list *lsystem,struct list *lcontrol);     /*m代表加密后的字符串*/
int if_ip_null(char *keyname,char *zipz);/*0:succ,1:fail*/
int count_progress(char *proname);
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
		int j,cl=1,nodenum=0,limit=0,if_ntp=0;
		char log_encry[BUF_LEN]; 
		int ret=-1,flag1=-1,flag2=-1,flag3=-1;
		char *sntpip=(char *)malloc(32);
		memset(sntpip,0,32);
		char *smask=(char *)malloc(32);
		memset(smask,0,32);
        char *cntpip=(char *)malloc(32);
		memset(cntpip,0,32);
		char cperz[10];
		char tyz[10];
		char pertemp[20];

		char cmd[128] = {0};
		int status = -1;
		char ntpsource[32] = {0};
		char ntpv[10] = {0};
	 
		if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
		{
			memset(encry,0,BUF_LEN);
			cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
			str=dcryption(encry);
			if(str==NULL)
			{
				ShowErrorPage(search(lpublic,"ill_user"));		  
				return 0;
			}
			strcpy(addn,str);
			memset(log_encry,0,BUF_LEN);				
		}
	    else
		{
			cgiFormStringNoNewlines("encry_import",log_encry,BUF_LEN);
			str=dcryption(log_encry);
			if(str==NULL)
			{
				ShowErrorPage(search(lpublic,"ill_user")); 	
			}
			strcpy(addn,str);
			memset(log_encry,0,BUF_LEN);                  

		}
		cgiFormStringNoNewlines("encry_import",log_encry,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		/***********************2008.5.26*********************/
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

		memset(tyz,0,10);
		memset(pertemp,0,20);
		cgiFormStringNoNewlines("ID", pertemp, 20); 
		cgiFormStringNoNewlines("TY", tyz, 10); 
        

		

		int flagz=0;
		if(strcmp(tyz,"1")==0)
		{
		    flagz=0;
			find_second_xmlnode(NTP_XML_FPATH, NTP_SERVZ, NTP_TIMEF, pertemp, &flagz);
			del_second_xmlnode(NTP_XML_FPATH, NTP_SERVZ, flagz);
			
		}
		if(strcmp(tyz,"2")==0)
		{
		    flagz=0;
			find_second_xmlnode(NTP_XML_FPATH, NTP_CLIZ, NTP_TIMEF, pertemp, &flagz);
			del_second_xmlnode(NTP_XML_FPATH, NTP_CLIZ, flagz);
			
		}		
	    if(cgiFormSubmitClicked("ntpstart") == cgiFormSuccess)
        {
			save_ntp_conf();
			reset_sigmask();
        	ret=start_ntp();  

			if(ret==0)
    		{				
        		ShowAlert(search(lpublic,"oper_succ"));	
    		}
			
    	    else
    	    {			
        		ShowAlert(search(lpublic,"oper_fail"));
    	    }
	
        }
		if(cgiFormSubmitClicked("ntpstop") == cgiFormSuccess)
        {
			save_ntp_conf();
			reset_sigmask();
        	ret=stop_ntp();

    		if(ret==0)
    		{	
		        ShowAlert(search(lpublic,"oper_succ"));	
    		}
			
    	    else
    	    {
    		    ShowAlert(search(lpublic,"oper_fail")); 		
    	    }
        }
		if(cgiFormSubmitClicked("ntptime") == cgiFormSuccess)
        {
			memset(ntpv,0,sizeof(ntpv));
			cgiFormStringNoNewlines("ntpver", ntpv, sizeof(ntpv)); 
			if_ntp=count_progress("/usr/sbin/ntpd");
			get_second_xmlnode(NTP_XML_FPATH, NTP_CLIZ, NTP_CIPZ,  &ntpsource, 1);
			if (0 != strcmp(ntpsource,""))
			{
	            if (if_ntp == 0)
	        	{
					memset(cmd,0,sizeof(cmd));
					if (0 == strcmp(ntpv,"3"))
					{
						sprintf(cmd,"sudo /usr/sbin/ntpdate -o 3 %s",ntpsource);
					}
					else
					{
						sprintf(cmd,"sudo /usr/sbin/ntpdate %s",ntpsource);
					}
					reset_sigmask();
					status = system(cmd);
					ret = WEXITSTATUS(status);
		    		if (ret == 0)
					{
			    		ShowAlert(search(lpublic,"oper_succ"));
					}
		    	    else
			    	{
			    		ShowAlert(search(lpublic,"oper_fail"));
			    	}
	        	}
				else
				{
					ShowAlert(search(lpublic,"ntp_timestop"));
				}
			}
			else
			{
				ShowAlert(search(lpublic,"ntp_noserver"));
			}
        }
		if(cgiFormSubmitClicked("serveradd") == cgiFormSuccess)
		{

            flag1=if_ip_null("vlan_ip", sntpip);
			flag2=if_ip_null("mask_ip", smask);
			
			if((flag1==0)&&(flag2==0))
			{
				add_ntp_server(NTP_XML_FPATH, sntpip, smask);
			}
			else
			{
			    if(flag1!=0)
			 		ShowAlert(search(lpublic,"ip_not_null"));
				if(flag2!=0)
					ShowAlert(search(lpublic,"mask_not_null"));
			}
		}
		if(cgiFormSubmitClicked("clientadd") == cgiFormSuccess)
		{

			flag3=if_ip_null("serv_ip", cntpip);
			memset(cperz,0,10);
			cgiFormStringNoNewlines("ifper", cperz, 10); 
			if(flag3==0)
			{
				add_ntp_client(NTP_XML_FPATH,cntpip, cperz);
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
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=log_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	
		if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
		{
       		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		}
     	else
     	{
     		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",log_encry,search(lpublic,"img_cancel"));
		}
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
						if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
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

							//新增时间条目
							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
							fprintf(cgiOut,"</tr>");
							
						}
						else if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)			
						{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=25>");
							fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));;
							fprintf(cgiOut,"</tr>");					  						
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"ntp_s"));  /*突出显示*/
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),"PPPOE");
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_pppoe_snp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),"PPPOE SNP");
							fprintf(cgiOut,"</tr>");
							
							//新增时间条目
							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
							fprintf(cgiOut,"</tr>");

						}
					}
					else
					{

						if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
						{					 
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>");							

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");
						}
						else if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)			
						{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
							fprintf(cgiOut,"</tr>");		

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
							fprintf(cgiOut,"</tr>");
						}
					}

                   /*点击延长*/
                    count_xml_node(NTP_XML_FPATH, &nodenum);
				    if(nodenum>0)
						limit=nodenum+3;

					
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

						if_ntp=count_progress("/usr/sbin/ntpd");
						fprintf(cgiOut,"<tr>");
						fprintf(cgiOut,"<td width=140>%s</td>",search(lpublic,"ntp_s"));
						fprintf(cgiOut,"<td><font color=blue>%s</font></td>\n",(if_ntp?search(lcontrol,"start"):search(lcontrol,"stop")));
						fprintf(cgiOut,"<td colspan=2></td>\n");
						fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr style=padding-top:15px>");
						fprintf(cgiOut,"<td width=140>");
						fprintf(cgiOut,"<input type=submit  name=ntpstart value=\"%s\">",search(lpublic,"ntp_start"));
						fprintf(cgiOut,"</td><td>\n");
						fprintf(cgiOut,"<input type=submit  name=ntpstop value=\"%s\">",search(lpublic,"ntp_stop"));
						fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"<td colspan=2><input type=submit  name=ntptime value=\"%s\" width=140></td>\n",search(lpublic,"ntp_time"));
						fprintf(cgiOut,"</tr>");
						
						fprintf(cgiOut,"<tr style=padding-top:15px><td colspan=4>%s</td></tr>",search(lpublic,"ntp_per"));

						fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
						"</tr>" );
						 
						fprintf(cgiOut,"<tr>");
						fprintf(cgiOut,"<td width=140>%s IP/MASK:</td>",search(lpublic,"ntp_vlan"));
						fprintf(cgiOut,"<td width=180>");
						fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
						fprintf(cgiOut,"<input type=text	name=vlan_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
						fprintf(cgiOut,"<input type=text	name=vlan_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text	name=vlan_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text	name=vlan_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
						fprintf(cgiOut,"</div></td>");	
						fprintf(cgiOut,"<td>\n");
						fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
						fprintf(cgiOut,"<input type=text	name=mask_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
						fprintf(cgiOut,"<input type=text	name=mask_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text	name=mask_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text	name=mask_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
						fprintf(cgiOut,"</div></td>");	
						fprintf(cgiOut,"<td><input type=submit name=serveradd value=\"%s\"></td>\n",search(lpublic,"ntp_add"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=15><td colspan=4>");
						fprintf(cgiOut,"<table>"\
							"<tr bgcolor=#eaeff9 style=font-size:14px align=left>\n");
						fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","INDEX");
						fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","IP");
						fprintf(cgiOut,"<th width=175 style=font-size:12px>%s</th>","MASK");
						fprintf(cgiOut,"<th width=175 style=font-size:12px></th>");
						fprintf(cgiOut,"</tr></table>\n");
						fprintf(cgiOut,"</td></tr>");
						struct serverz_st servst ,*sq;
						memset(&servst,0,sizeof(servst));
						int servnum=0,count=0;
						read_ntp_server(NTP_XML_FPATH, &servst, &servnum);
						sq=servst.next;
						while(sq!=NULL)
						{
						    count++;
							fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
							fprintf(cgiOut,"<td>%d</td>",count);
							fprintf(cgiOut,"<td>%s</td>\n",sq->servipz);
							fprintf(cgiOut,"<td>%s</td>\n",sq->maskz);
							if(strcmp(encry,"")==0)
								strcpy(encry,log_encry);
							fprintf(cgiOut,"<td><a href=wp_ntp.cgi?UN=%s&ID=%s&TY=1 target=mainFrame><font color=black>%s</font></a></td>",encry,sq->timeflag,search(lpublic,"delete"));
							fprintf(cgiOut,"</tr>\n");
                            sq=sq->next;
							cl=!cl;
						}

						if(servnum>0)
							Free_read_ntp_server(&servst);						
						
						fprintf(cgiOut,"<tr style=padding-top:20px><td colspan=4>%s</td></tr>",search(lpublic,"ntp_sets"));

						 //分割线
						fprintf(cgiOut, "<tr><td colspan=4><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
						"</tr>" );


						fprintf(cgiOut,"<tr>");
						fprintf(cgiOut,"<td width=140>IP:</td>");
						fprintf(cgiOut,"<td width=140>\n");
						fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
						fprintf(cgiOut,"<input type=text	name=serv_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
						fprintf(cgiOut,"<input type=text	name=serv_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text	name=serv_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text	name=serv_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
						fprintf(cgiOut,"</div></td>");	
						fprintf(cgiOut,"<td><input type=checkbox name=ifper value=\"prefer\">%s</td>\n",search(lpublic,"ntp_pri"));
						fprintf(cgiOut,"<td><input type=submit name=clientadd value=\"%s\"></td>\n",search(lpublic,"ntp_add"));
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
							fprintf(cgiOut,"<td>%d</td>",count);
							fprintf(cgiOut,"<td>%s</td>\n",cq->clitipz);
							fprintf(cgiOut,"<td>%s</td>\n",(strncmp(cq->ifper,"prefer",6) == 0)?search(lpublic,"ntp_pri"):"");
							if(strcmp(encry,"")==0)
								strcpy(encry,log_encry);
							fprintf(cgiOut,"<td><a href=wp_ntp.cgi?UN=%s&ID=%s&TY=2 target=mainFrame><font color=black>%s</font></a></td>",encry,cq->timeflag,search(lpublic,"delete"));
							fprintf(cgiOut,"</tr>\n");
                            cq=cq->next;
							cl=!cl;
						}
						if(clinum>0)
						{
							Free_read_ntp_client(&clitst);
						}
						fprintf(cgiOut,"<tr style='padding-top:5px'><td colspan=5>%s%s</td></tr>",search(lpublic,"ntp_time"),search(lpublic,"l_item"));
						 //分割线
						 fprintf(cgiOut, "<tr><td colspan=4><hr width=100%% size=1 color=#fff align=center noshade /></td></tr>" );
						 fprintf(cgiOut,"<tr>\n");
						 memset(ntpv,0,sizeof(ntpv));
						 get_first_xmlnode(NTP_XML_FPATH, "ntpv", &ntpv);
						 if (0 == strcmp(ntpv,"3"))
					 	{
							 fprintf(cgiOut,"<td colspan=2 id=td2><input type=radio name=ntpver value=\"3\" checked>NTPv3</td>\n");
							 fprintf(cgiOut,"<td colspan=2 id=td2><input type=radio name=ntpver value=\"4\">NTPv4</td>\n");
					 	}
						 else
					 	{
							 fprintf(cgiOut,"<td colspan=2 id=td2><input type=radio name=ntpver value=\"3\">NTPv3</td>\n");
							 fprintf(cgiOut,"<td colspan=2 id=td2><input type=radio name=ntpver value=\"4\" checked>NTPv4</td>\n");
					 	}
						 fprintf(cgiOut,"</tr>\n");
						fprintf(cgiOut,"<tr style='padding-top:10px'><td colspan=4>%s</td></tr>\n",search(lpublic,"ntp_cron"));
						 //分割线
						fprintf(cgiOut, "<tr><td colspan=4><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
						"</tr>" );

						char cront[20] = {0};
						char crontime[10] = {0};
						char cronradio[10] = {0};
						get_first_xmlnode(NTP_XML_FPATH, "cront", &cront);
						sscanf(cront,"%[^^]%s",crontime,cronradio);

						fprintf(cgiOut,"<tr>\n");
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
						fprintf(cgiOut,"<td><input type=text name=cront onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
								onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
								ondragenter=\"return  false;\"\
								style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" maxLength=4 value=\"%s\"/></td>",crontime);
						fprintf(cgiOut,"</tr>\n");
						
						///////////////////////////////

						fprintf(cgiOut, "<tr><td colspan=4><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
						"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
						"</tr>" );

						char newc1[20] = {0};
						get_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, &newc1);
						fprintf(cgiOut,"<tr>");
						fprintf(cgiOut,"<td>%s</td>\n","IPV4");
						if (0 == strcmp(newc1,"ignore"))
						{
							fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"1\">%s</td>\n",search(lpublic,"ntp_listen"));
							fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"2\" checked>%s</td>\n",search(lpublic,"ntp_ignore"));
						}
						else
						{
							fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"1\" checked>%s</td>\n",search(lpublic,"ntp_listen"));
							fprintf(cgiOut,"<td><input type=radio name=ipv4 value=\"2\">%s</td>\n",search(lpublic,"ntp_ignore"));
						}
						fprintf(cgiOut,"<td></td>\n");
						fprintf(cgiOut,"</tr>\n");
						
						char newc2[20] = {0};
						get_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, &newc2);
						fprintf(cgiOut,"<tr>");
						fprintf(cgiOut,"<td>%s</td>\n","IPV6");
						if (0 == strcmp(newc2,"listen"))
						{
							fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"1\" checked>%s</td>\n",search(lpublic,"ntp_listen"));
							fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"2\">%s</td>\n",search(lpublic,"ntp_ignore"));
						}
						else
						{
							fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"1\">%s</td>\n",search(lpublic,"ntp_listen"));
							fprintf(cgiOut,"<td><input type=radio name=ipv6 value=\"2\" checked>%s</td>\n",search(lpublic,"ntp_ignore"));
						}
						fprintf(cgiOut,"<td></td>\n");
						fprintf(cgiOut,"</tr>\n");

						//////////////////////////////

						if(cgiFormSubmitClicked("log_config") != cgiFormSuccess)
						{
							fprintf(cgiOut,"<tr><td colspan=4><input type=hidden name=encry_import value=%s></td></tr>",encry);
						}
						else if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)
						{
							fprintf(cgiOut,"<tr><td colspan=4><input type=hidden name=encry_import value=%s></td></tr>",log_encry);
						}	 
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
free(cntpip);
free(smask);
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
int count_progress(char *proname)
{
	char cmd[128] = {0};
	char buff[128] = {0};
	FILE *fp;
	int pronum=0;
	sprintf(cmd,"ps -ef|grep %s|grep -v grep|wc -l",proname);
	fp=popen(cmd,"r");
	if(fp== NULL)
	{
		return 0;
	}
	fgets( buff, sizeof(buff), fp);	
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
	if (0 == strcmp(ipv4,"1"))
	{
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, "listen");
	}
	else
	{
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, "ignore");
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
		mod_first_xmlnode(NTP_XML_FPATH, "ntpv","3");
	}
	else
	{
		mod_first_xmlnode(NTP_XML_FPATH, "ntpv","4");
	}
	if (0 != strcmp(cront,""))
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
				system("sudo cronntp.sh > /dev/null");
			}
			else
			{
				ShowAlert(search(lpublic,"input_overflow"));
			}
		}
	}   
}
