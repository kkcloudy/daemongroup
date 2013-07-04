/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_wtpcon.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_sysinfo.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_boot.h"
#include "ws_init_dbus.h"


int ShowSystemconPage(char *m,struct list *lpublic,struct list *lsystem); 
void config_system(struct list *lpublic,struct list *lsystem);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char *str = NULL; 
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsystem = NULL;     /*解析system.txt文件的链表头*/  

  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsystem=get_chain_head("../htdocs/text/system.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));

  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	;
  }
  else
  {    
	cgiFormStringNoNewlines("encry_consys",encry,BUF_LEN);
  }
  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
  else
	ShowSystemconPage(encry,lpublic,lsystem);
  
  release(lpublic);  
  release(lsystem);
  destroy_ccgi_dbus();
  return 0;
}

int ShowSystemconPage(char *m,struct list *lpublic,struct list *lsystem)
{  
  char IsSubmit[5] = { 0 };
  int i = 0;
  int ret = 0;
  char hostname[128] = { 0 };
  int ssh_state = 0;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SystemConfig</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>");
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("syscon_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
		config_system(lpublic,lsystem);
  }  
  fprintf(cgiOut,"<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_config"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=syscon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
   fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
			
				
      fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
      "<tr>"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"\
            "<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>"\
            "<tr>"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>"\
                   "<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");	              
				  fprintf(cgiOut,"<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=26>"\
				  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"sys_config"));/*突出显示*/
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"import_config"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"export_config"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_up"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
				  fprintf(cgiOut,"</tr>"\
			  	  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"log_info"));
				  fprintf(cgiOut,"</tr>"\
			  	  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"l_user"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=26>"\
				  "<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"systime"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),"PPPOE");
				  fprintf(cgiOut,"</tr>");
				  //新增时间条目
				  fprintf(cgiOut,"<tr height=26>"\
				  "<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"web_service"));
				  fprintf(cgiOut,"</tr>");
				  for(i=0;i<1;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">"\
				"<table width=720 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
					"<td width=60>%s:</td>",search(lsystem,"hostname"));
					memset(hostname, 0, sizeof(hostname));
					ret = get_hostname(hostname);
					if(1 == ret)
					{
						fprintf(cgiOut,"<td width=150><input name=hostname size=21 maxLength=64 onkeypress=\"return event.keyCode!=32\" value=\"%s\"></td>",hostname);
					}
					else
					{
						fprintf(cgiOut,"<td width=150><input name=hostname size=21 maxLength=64 onkeypress=\"return event.keyCode!=32\"></td>");
					}
				 	fprintf(cgiOut,"<td width=510 align=left><font color=red>(%s)</font></td>",search(lsystem,"hostname_range"));
				fprintf(cgiOut,"</tr>"
				"<tr height=30>"\
					"<td>SSH%s:</td>",search(lpublic,"service"));
            		fprintf(cgiOut,"<td colspan=2><select name=ssh_service id=ssh_service style=width:130px>");
					ret = 0;
					ret = show_ssh_func_cmd(&ssh_state);
					if((1 == ret)&&(1 == ssh_state))
					{
                        fprintf(cgiOut,"<option value=enable selected=selected>enable</option>"\
                        "<option value=disable>disable</option>");
					}
					else
					{
                        fprintf(cgiOut,"<option value=enable>enable</option>"\
                        "<option value=disable selected=selected>disable</option>");
					}
                 	fprintf(cgiOut,"</td>"\
				"</tr>"\
				"<tr height=30>"\
					"<td>%s:</td>",search(lsystem,"dns"));
					fprintf(cgiOut,"<td colspan=2>"\
						 "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
						 "<input type=text name='dns_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						 fprintf(cgiOut,"<input type=text name='dns_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						 fprintf(cgiOut,"<input type=text name='dns_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						 fprintf(cgiOut,"<input type=text name='dns_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
						 fprintf(cgiOut,"</div>"\
					"</td>"\
				"</tr>"
				"<tr valign=top style=\"padding-top:10px\">"\
					"<td colspan=3>"\
						"<fieldset align=left>"\
						  "<legend><font color=Navy>%s</font></legend>",search(lsystem,"dns_cache"));
						  fprintf(cgiOut,"<table width=200 border=0 cellspacing=0 cellpadding=0>"\
						  	"<tr height=30>"\
					   		  "<td width=50>%s:</td>",search(lpublic,"l_name"));
							  fprintf(cgiOut,"<td width=150><input name=domain size=21 maxLength=32 onkeypress=\"return event.keyCode!=32\"></td>"\
							"</tr>"\
						    "<tr height=30>"\
					   		  "<td>IP:</td>"\
							  "<td>"\
							     "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
								 "<input type=text name='cache_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								 fprintf(cgiOut,"<input type=text name='cache_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								 fprintf(cgiOut,"<input type=text name='cache_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								 fprintf(cgiOut,"<input type=text name='cache_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
								 fprintf(cgiOut,"</div>"\
							  "</td>"\
							"</tr>"\
							"<tr height=30>"\
					   		  "<td>%s:</td>",search(lpublic,"index"));
							  fprintf(cgiOut,"<td><input name=index size=21 maxLength=32 onkeypress=\"return event.keyCode!=32\"></td>"\
							"</tr>"\
						  "</table>"\
						"</fieldset>"
					"</td>"\
				"</tr>"
				"<tr valign=top style=\"padding-top:10px\">"\
					"<td colspan=3>"\
						"<fieldset align=left>"\
						  "<legend><font color=Navy>%s</font></legend>",search(lsystem,"console_user"));
						  fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
						    "<tr height=30>"\
					   		  "<td width=50>%s:</td>",search(lpublic,"usr_name"));
							  fprintf(cgiOut,"<td width=150><input name=console_username size=21 maxLength=32 onkeypress=\"return event.keyCode!=32\"></td>"\
							  "<td align=left width=500><font color=red>(%s)</font></td>",search(lsystem,"userna_dep"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=30>"\
					   		  "<td>%s:</td>",search(lsystem,"password"));
							  fprintf(cgiOut,"<td><input type=password name=console_userpwd size=23 maxLength=32 onkeypress=\"return event.keyCode!=32\"></td>"\
							  "<td align=left><font color=red>(%s)</font></td>",search(lsystem,"console_userpwd_range"));
							fprintf(cgiOut,"</tr>"\
						  "</table>"\
						"</fieldset>"
					"</td>"\
				"</tr>"
				"<tr>"\
				  "<td><input type=hidden name=encry_consys value=%s></td>",m);
				  fprintf(cgiOut,"<td colspan=2><input type=hidden name=SubmitFlag value=%d></td>",1);
				fprintf(cgiOut,"</tr>"\
			    "</table>"\
              "</td>"\
            "</tr>"\
            "<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
      "</tr>"\
    "</table></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");
		
return 0;
}


void config_system(struct list *lpublic,struct list *lsystem)
{
	int flag = 1;
	int ret = 0;
	char hostname[128] = { 0 };
	char ssh_service[1] = { 0 };
	char dns_ip[20] = { 0 };
	char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
	char domain[64] = { 0 };
	char dns_cache_ip[20] = { 0 };
	char index[10] = { 0 };
	char command[255] = { 0 };
	int status = 0;
	char console_username[64] = { 0 };
	char console_userpwd[64] = { 0 };

	/************************set hostname*************************/
	memset(hostname,0,sizeof(hostname));
	cgiFormStringNoNewlines("hostname",hostname,128);
	if(strcmp(hostname,"")!=0)
	{
		ret=set_hostname(hostname);/*返回0表示失败，返回1表示成功*/
								   /*返回-1表示hostname is too long,should be 1 to 64*/
								   /*返回-2表示hostname should be start with a letter*/
								   /*返回-3表示hostname should be use letters,numbers,"-","."*/
		switch(ret)
		{
			case 0:flag=0;
				   ShowAlert(search(lsystem,"set_hostname_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
					ShowAlert(search(lpublic,"input_too_long"));
					break;
			case -2:flag=0;
				    ShowAlert(search(lsystem,"hostname_start_with_letter"));
				    break;
			case -3:flag=0;
				    ShowAlert(search(lsystem,"hostname_use_letter"));
				    break;
		}
	}
	
	/************************set ssh service*************************/
	memset(ssh_service,0,sizeof(ssh_service));
	cgiFormStringNoNewlines("ssh_service",ssh_service,10);
	if(strcmp(ssh_service,"")!=0)
	{
		if(0 == strcmp(ssh_service,"enable"))
		{
			ret=ssh_up_func_cmd(NULL);/*返回0表示失败，返回1表示成功*/
									  /*返回-1表示only active master can enable ssh*/
									  /*返回-2表示system error，返回-3表示Port number is incorrect*/
									  /*返回-4表示Port is already in use，返回-5表示some thing is Wrong*/
			switch(ret)
			{
				case 0:flag=0;
					   ShowAlert(search(lsystem,"set_ssh_service_fail"));
					   break;
				case 1:break;
				case -1:flag=0;
						ShowAlert(search(lsystem,"active_master_set_ssh"));
						break;
				case -2:flag=0;
						ShowAlert(search(lpublic,"sys_err"));
						break;
				case -3:flag=0;
						ShowAlert(search(lpublic,"port_is_incorrect"));
						break;
				case -4:flag=0;
						ShowAlert(search(lpublic,"port_is_use"));
						break;
				case -5:flag=0;
						ShowAlert(search(lpublic,"error"));
						break;
			}
		}
		else
		{
			ret=ssh_down_func_cmd();  /*返回0表示失败，返回1表示成功*/
									  /*返回-1表示only active master can disable ssh*/
									  /*返回-2表示SSH can not be shut down because someone has logged into the system using it. If you want, please use the 'kick user' command to kick the user off first.*/
									  /*返回-3表示some thing is Wrong*/
			switch(ret)
			{
				case 0:flag=0;
					   ShowAlert(search(lsystem,"set_ssh_service_fail"));
					   break;
				case 1:break;
				case -1:flag=0;
						ShowAlert(search(lsystem,"active_master_set_ssh"));
						break;
				case -2:flag=0;
						ShowAlert(search(lsystem,"ssh_not_shut_down"));
						break;
				case -3:flag=0;
						ShowAlert(search(lpublic,"error"));
						break;
			}
		}	
	}
	
	/************************set ip dns func cmd*************************/
	memset(dns_ip,0,sizeof(dns_ip));                                 
    memset(ip1,0,sizeof(ip1));
    cgiFormStringNoNewlines("dns_ip1",ip1,4);	
    strncat(dns_ip,ip1,sizeof(dns_ip)-strlen(dns_ip)-1);
    strncat(dns_ip,".",sizeof(dns_ip)-strlen(dns_ip)-1);
    memset(ip2,0,sizeof(ip2));
    cgiFormStringNoNewlines("dns_ip2",ip2,4); 
    strncat(dns_ip,ip2,sizeof(dns_ip)-strlen(dns_ip)-1);	
    strncat(dns_ip,".",sizeof(dns_ip)-strlen(dns_ip)-1);
    memset(ip3,0,sizeof(ip3));
    cgiFormStringNoNewlines("dns_ip3",ip3,4); 
    strncat(dns_ip,ip3,sizeof(dns_ip)-strlen(dns_ip)-1);	
    strncat(dns_ip,".",sizeof(dns_ip)-strlen(dns_ip)-1);
    memset(ip4,0,sizeof(ip4));
    cgiFormStringNoNewlines("dns_ip4",ip4,4);
    strncat(dns_ip,ip4,sizeof(dns_ip)-strlen(dns_ip)-1);

	if(strcmp(dns_ip,"...")!=0)
	{
		ret=set_ip_dns_func_cmd(dns_ip);/*返回0表示失败，返回1表示成功*/
										/*返回-1表示Can't get system dns seting*/
										/*返回-2表示The system has 3 dns,can't set again*/
										/*返回-3表示The dns server is exist,can't set again*/
										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   ShowAlert(search(lsystem,"set_dns_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
					ShowAlert(search(lsystem,"cant_get_dns_seting"));
					break;
			case -2:flag=0;
				    ShowAlert(search(lsystem,"has_3_dns"));
				    break;
			case -3:flag=0;
				    ShowAlert(search(lsystem,"dns_server_exist"));
				    break;
		}
	}

	/************************add dns cache domain ip cmd*************************/
	memset(domain,0,sizeof(domain));
	cgiFormStringNoNewlines("domain",domain,64);
	
	memset(dns_cache_ip,0,sizeof(dns_cache_ip));                                 
    memset(ip1,0,sizeof(ip1));
    cgiFormStringNoNewlines("cache_ip1",ip1,4);	
    strncat(dns_cache_ip,ip1,sizeof(dns_cache_ip)-strlen(dns_cache_ip)-1);
    strncat(dns_cache_ip,".",sizeof(dns_cache_ip)-strlen(dns_cache_ip)-1);
    memset(ip2,0,sizeof(ip2));
    cgiFormStringNoNewlines("cache_ip2",ip2,4); 
    strncat(dns_cache_ip,ip2,sizeof(dns_cache_ip)-strlen(dns_cache_ip)-1);	
    strncat(dns_cache_ip,".",sizeof(dns_cache_ip)-strlen(dns_cache_ip)-1);
    memset(ip3,0,sizeof(ip3));
    cgiFormStringNoNewlines("cache_ip3",ip3,4); 
    strncat(dns_cache_ip,ip3,sizeof(dns_cache_ip)-strlen(dns_cache_ip)-1);	
    strncat(dns_cache_ip,".",sizeof(dns_cache_ip)-strlen(dns_cache_ip)-1);
    memset(ip4,0,sizeof(ip4));
    cgiFormStringNoNewlines("cache_ip4",ip4,4);
    strncat(dns_cache_ip,ip4,sizeof(dns_cache_ip)-strlen(dns_cache_ip)-1);

	memset(index,0,sizeof(index));
	cgiFormStringNoNewlines("index",index,10);

	if((strcmp(domain,"")!=0)&&(strcmp(dns_cache_ip,"")!=0)&&(strcmp(index,"")!=0))
	{
		#if 0
		ret=add_dns_cache_domain_ip_cmd(domain,dns_cache_ip,index);  /*返回0表示失败，返回1表示成功*/
																	 /*返回-1表示Bad parameter，返回-2表示error*/
		switch(ret)
		{
			case 0:flag=0;
				   ShowAlert(search(lsystem,"set_dns_cache_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
				    ShowAlert(search(lpublic,"Vrrp_Bad_Parameter_Input"));
				    break;
			case -2:flag=0;
					ShowAlert(search(lpublic,"error"));
					break;
		}
		#endif

		memset(command,0,sizeof(command));
	    snprintf(command, sizeof(command)-1, "add_dns_cache_domain_ip.sh %s %s %s", domain, dns_cache_ip, index);

	    status = system(command); 	 
	    if(0 != status)    /*command fail*/
	    {
	    	flag=0;
			ShowAlert(search(lsystem,"set_dns_cache_fail"));
	    }
	}

	
	/************************set system consolepwd func cmd1*************************/
	memset(console_username,0,sizeof(console_username));
	cgiFormStringNoNewlines("console_username",console_username,64);
	memset(console_userpwd,0,sizeof(console_userpwd));
	cgiFormStringNoNewlines("console_userpwd",console_userpwd,64);
	if((strcmp(console_username,"")!=0)&&(strcmp(console_userpwd,"")!=0))
	{
		ret=set_system_consolepwd_func1(console_username,console_userpwd);  /*返回0表示失败，返回1表示成功*/
																				/*返回-1表示user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'*/
																			    /*返回-2表示user name length should be >=4 & <=32*/
																				/*返回-3表示user name first char  should be 'A'-'Z' or 'a'-'z'*/
																				/*返回-4表示user password is a palindrome*/	
																				/*返回-5表示user password is too simple*/
																				/*返回-6表示user password should be not same as username*/
																				/*返回-7表示user password too short or too long*/
																				/*返回-8表示user password length should be >= 4 && <=32*/
		switch(ret)
		{
			case 0:flag=0;
				   ShowAlert(search(lsystem,"set_console_user_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
				    ShowAlert(search(lsystem,"username_use_letter"));
				    break;
			case -7:
			case -8:
			case -2:flag=0;
					ShowAlert(search(lpublic,"input_too_long"));
					break;
			case -3:flag=0;
				    ShowAlert(search(lsystem,"username_start_with_letter"));
				    break;
			case -4:flag=0;
				    ShowAlert(search(lpublic,"pwd_is_palindrome"));
				    break;
			case -5:flag=0;
				    ShowAlert(search(lpublic,"pwd_too_simple"));
				    break;
			case -6:flag=0;
				    ShowAlert(search(lpublic,"pwd_same_as_username"));
				    break;
		}
	}
	
	if(1 == flag)
	{
		ShowAlert(search(lpublic,"oper_succ"));
	}
}



