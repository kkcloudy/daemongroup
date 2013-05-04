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
* wp_modpri.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos 
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


int ShowModifySyslogPage(struct list *lpublic, struct list *lsystem);
void modifySyslog_hand(struct list *lpublic, struct list *lsystem); 

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt"); 

	ShowModifySyslogPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem); 
	
	return 0;
}

int ShowModifySyslogPage(struct list *lpublic, struct list *lsystem)
{ 
  int i = 0;
  char log_info[10] = { 0 };
  char oper_info[10] = { 0 };
  char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
  char *str;	
  char mod_encry[BUF_LEN];
   char * usrName=(char *)malloc(33);
  memset(usrName,0,33);
  if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    cgiFormStringNoNewlines("USERNAME", usrName, 33);
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
      return 0;
	}
  }
  else
  {
  	cgiFormStringNoNewlines("a_name", usrName, 33);
  }
  char * procductId=readproductID();
  cgiFormStringNoNewlines("encry_modadm",mod_encry,BUF_LEN);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsystem,"user_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<body>");
  
  
  
if(cgiFormSubmitClicked("submit_modifyadmin") == cgiFormSuccess)
{
  modifySyslog_hand(lpublic,lsystem);
}
fprintf(cgiOut,"<form method=post>"\
"<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"user_manage"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	 
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=submit_modifyadmin style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			
		if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
		{
		 
		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

		}
		else
		{
		
           fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",mod_encry,search(lpublic,"img_cancel"));

		}

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
				if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
				{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));						 
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_addusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"add_user"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_modpass.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));
					fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modpri.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_privilege"));
      					fprintf(cgiOut,"</tr>"\
						"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"modify_loglevel"));	/*突出显示*/					
					fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"login_info"));
					fprintf(cgiOut,"</tr>");
				}
				else if(cgiFormSubmitClicked("submit_modifyadmin") == cgiFormSuccess)				
				{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));						 
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_addusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"add_user"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_modpass.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modpri.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lsystem,"modify_privilege"));
      					fprintf(cgiOut,"</tr>"\	
						"<tr height=26>"\
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"modify_loglevel"));	/*突出显示*/					
					fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",mod_encry,search(lpublic,"menu_san"),search(lpublic,"login_info"));
					fprintf(cgiOut,"</tr>");

				}

			  fprintf(cgiOut,"</table>"\
			"</td>"\
			"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                "<table width=280 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=30>"\
						  "<td id=tdprompt>%s:</td>",search(lsystem,"user_na"));
						  if(strcmp(usrName,"")==0)
						  	fprintf(cgiOut,"<td><input type=text name=a_name size=20></td>");
						  else 
						  	fprintf(cgiOut,"<td><input type=text name=a_name size=20 value=%s enable></td>",usrName);
						fprintf(cgiOut,"</tr>");
						if(strcmp(usrName,"")!=0)
						{
							get_user_syslog_by_name(usrName,log_info,oper_info);
						}
						fprintf(cgiOut,"<tr height=30>"\
						  "<td id=tdprompt>%s:</td>",search(lsystem,"log_lever"));
        				  fprintf(cgiOut,"<td><select name=log_lever style=width:130px>");
						  for(i=0;i<6;i++)
						  {
						  	  if(strcmp(log_info,syslog_level[i]) == 0)
							    fprintf(cgiOut,"<option value=%s selected=selected>%s",syslog_level[i],syslog_level[i]);
							  else
							  	fprintf(cgiOut,"<option value=%s>%s",syslog_level[i],syslog_level[i]);
						  }
						  fprintf(cgiOut,"</select></td>"\
						"</tr>"\
						"<tr height=30>"\
						  "<td id=tdprompt>%s:</td>",search(lsystem,"oper_lever"));
        				  fprintf(cgiOut,"<td><select name=oper_lever style=width:130px>");
						  for(i=0;i<3;i++)
						  {
						  	  if(strcmp(oper_info,sysoper_level[i]) == 0)
							    fprintf(cgiOut,"<option value=%s selected=selected>%s",sysoper_level[i],sysoper_level[i]);
							  else
							  	fprintf(cgiOut,"<option value=%s>%s",sysoper_level[i],sysoper_level[i]);
						  }
						  fprintf(cgiOut,"</select></td>"\
						"</tr>"\
						"<tr>");
						if(cgiFormSubmitClicked("submit_modifyadmin") != cgiFormSuccess)
						{
						  fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_modadm value=%s></td>",encry);
						}
						else if(cgiFormSubmitClicked("submit_modifyadmin") == cgiFormSuccess)
						{
						  fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_modadm value=%s></td>",mod_encry);
						}
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

free(usrName);
free(procductId);
free(encry);
return 0;
}

void modifySyslog_hand(struct list *lpublic, struct list *lsystem)
{
  char na[N]={0};  
  char log[10]={0};
  char oper[10]={0};
  int ret = 0;
  
  memset(na,0,N);					 /*清空临时变量*/
  memset(log,0,10);
  memset(oper,0,10);
  cgiFormStringNoNewlines("a_name", na, N);
  cgiFormStringNoNewlines("log_lever",log,10);  
  cgiFormStringNoNewlines("oper_lever",oper,10);  

  ret = mod_user_syslog_by_name(na,log,oper);
  
  if(ret == 1)
  {
	  ShowAlert(search(lpublic,"oper_succ"));
  }
  else 
  {
	  ShowAlert(search(lpublic,"oper_fail"));
  }
}
  


