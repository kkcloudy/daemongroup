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
* wp_pppoe_snp.c
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
#include "ws_init_dbus.h"
#include "ws_dcli_wlans.h"

int ShowPppoeSnpPage(char *m,struct list *lpublic,struct list *lsystem);  /*m代表加密字符串，n代表wlan id，t代表wlan name ，r代表security id，intf代表interafce，Hessid代表Hideessid，s代表service state*/
void ConPppoeSnp(struct list *lpublic,struct list *lsystem);

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
    cgiFormStringNoNewlines("encry_conpppoesnp",encry,BUF_LEN);
  }  
    
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
	ShowPppoeSnpPage(encry,lpublic,lsystem);
  
  release(lpublic);  
  release(lsystem);
  destroy_ccgi_dbus();
  return 0;
}

int ShowPppoeSnpPage(char *m,struct list *lpublic,struct list *lsystem)
{    
  char IsSubmit[5] = { 0 };
  int i = 0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>PPPOE SNP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");	  

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);  
  if((cgiFormSubmitClicked("pppoesnp_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {  	
	ConPppoeSnp(lpublic,lsystem);
  }  
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>PPPOE SNP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
	   
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
            "<td width=62 align=center><input id=but type=submit name=pppoesnp_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
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
				  "<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
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
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=26>"\
                  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),"PPPOE SNP");   /*突出显示*/
				  fprintf(cgiOut,"</tr>");
				  //新增时间条目
				  fprintf(cgiOut,"<tr height=26>"\
				  "<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"web_service"));
				  fprintf(cgiOut,"</tr>");
				  
                  for(i=0;i<0;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">"\
				"<table width=450 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
					"<td>PPPEO SNP%s:</td>",search(lpublic,"service"));
            		fprintf(cgiOut,"<td colspan=2><select name=pppoe_snp_service id=pppoe_snp_service style=width:130px>"\
						"<option value=></option>"\
                        "<option value=enable>enable</option>"\
                        "<option value=disable>disable</option>");
                 	fprintf(cgiOut,"</td>"\
				"</tr>"\
				"<tr height=30>"\
					"<td width=120>%sPPPEO SNP%s:</td>",search(lpublic,"inter"),search(lpublic,"service"));
					fprintf(cgiOut,"<td width=140><input name=if_name maxLength=16 onkeypress=\"return event.keyCode!=32\"></td>"
            		"<td width=190><select name=if_pppoe_snp_service id=if_pppoe_snp_service style=width:130px>"\
						"<option value=></option>"\
                        "<option value=enable>enable</option>"\
                        "<option value=disable>disable</option>");
                 	fprintf(cgiOut,"</td>"\
				"</tr>"\
				"<tr>"\
				  "<td><input type=hidden name=encry_conpppoesnp value=%s></td>",m);
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

void ConPppoeSnp(struct list *lpublic,struct list *lsystem)
{
	int ret = 0,flag = 1;
	char pppoe_snp_service[10] = { 0 };
	char if_name[20] = { 0 };
	char if_pppoe_snp_service[10] = { 0 };
	
    /*************config pppoe snp server cmd******************/	
	memset(pppoe_snp_service,0,sizeof(pppoe_snp_service));
	cgiFormStringNoNewlines("pppoe_snp_service",pppoe_snp_service,10); 
	if(strcmp(pppoe_snp_service,""))
	{
 		ret=config_pppoe_snp_server_cmd(pppoe_snp_service); /*返回0表示失败，返回1表示成功*/
															/*返回-1表示bad command parameter*/
															/*返回-2表示error*/
		switch(ret)
		{
		  case 0:ShowAlert(search(lsystem,"set_pppoe_snp_service_fail"));
				 flag=0;
				 break;
		  case 1:break;
		  case -1:ShowAlert(search(lpublic,"input_para_illegal"));
				  flag=0;
				  break;
		  case -2:ShowAlert(search(lpublic,"error"));
				  flag=0;
				  break;
		}	
	}

	/*************config pppoe snooping enable cmd******************/	
	memset(if_name,0,sizeof(if_name));
	cgiFormStringNoNewlines("if_name",if_name,20); 
	memset(if_pppoe_snp_service,0,sizeof(if_pppoe_snp_service));
	cgiFormStringNoNewlines("if_pppoe_snp_service",if_pppoe_snp_service,10);
	if((strcmp(if_name,""))&&(strcmp(if_pppoe_snp_service,"")))
	{
 		ret=config_pppoe_snooping_enable_cmd(if_name,if_pppoe_snp_service); /*返回0表示失败，返回1表示成功*/
																			/*返回-1表示bad command parameter*/
																			/*返回-2表示Interface name is too long*/
																			/*返回-3表示if_name is not a ve-interface name*/
																			/*返回-4表示get local_slot_id error*/
																			/*返回-5表示error*/
		switch(ret)
		{
		  case 0:ShowAlert(search(lsystem,"set_if_pppoe_snp_service_fail"));
				 flag=0;
				 break;
		  case 1:break;
		  case -1:ShowAlert(search(lpublic,"input_para_illegal"));
				  flag=0;
				  break;
		  case -2:ShowAlert(search(lpublic,"interface_too_long"));
				  flag=0;
				  break;
		  case -3:ShowAlert(search(lsystem,"if_not_veif"));
				  flag=0;
				  break;
		  case -4:ShowAlert(search(lpublic,"error_open"));
				  flag=0;
				  break;
		  case -5:ShowAlert(search(lpublic,"error"));
				  flag=0;
				  break;
		}	
	}

	if(flag==1)
	{
		ShowAlert(search(lpublic,"oper_succ"));
	}
}



