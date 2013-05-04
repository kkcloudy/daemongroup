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
* wp_modusr.c
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


int ShowModifyuserPage(char *m,char *n,struct list *lpublic, struct list *lsystem);
void modifyuser_hand(char *mu,struct list *lpublic, struct list *lsystem); 

int cgiMain()
{     
	char *encry=(char *)malloc(BUF_LEN);			  /*存储从wp_usrmag.cgi带入的加密字符串*/	
	char *str;	  
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt"); 
    memset(encry,0,BUF_LEN);
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN) !=cgiFormNotFound)  /*首次进入该页*/
	{
	    str=dcryption(encry);
	    if(str==NULL)
	    {
	      ShowErrorPage(search(lpublic,"ill_user"));    /*用户非法*/
	      return 0;
	    }
		else
		  ShowModifyuserPage(encry,str,lpublic,lsystem);
	}
	else
	{      
		cgiFormStringNoNewlines("encry_modusr",encry,BUF_LEN);
		str=dcryption(encry);
	    if(str==NULL)
	    {
	      ShowErrorPage(search(lpublic,"ill_user"));    /*用户非法*/
	      return 0;
	    }
		else
		  ShowModifyuserPage(encry,str,lpublic,lsystem);
	}
	free(encry);
  	release(lpublic);  
  	release(lsystem);
	return 0;
}

int ShowModifyuserPage(char *m,char *n,struct list *lpublic, struct list *lsystem)
{ 
	
	int i;
	char * procductId=readproductID();
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


	if(cgiFormSubmitClicked("submit_modifyuser") == cgiFormSuccess)
	{
		modifyuser_hand(n,lpublic,lsystem);
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
			"<td width=62 align=center><input id=but type=submit name=submit_modifyuser style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
			  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
      					  "<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"user_list"));
      					fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\">%s</td>",search(lsystem,"modify_password"));	/*突出显示*/
						fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"login_info"));
					fprintf(cgiOut,"</tr>");
					for(i=0;i<4;i++)
					{
					  fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
					}
				  fprintf(cgiOut,"</table>"\
				"</td>"\
				"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
			  "<table border=0 cellspacing=0 cellpadding=0>");
				  fprintf(cgiOut,"<tr height=30>"\
				  "<td width=105 id=tdprompt align=left>%s:</td>",search(lsystem,"user_na"));
				  fprintf(cgiOut,"<td align=left><input type=text name=u_name size=20 value=%s disabled=true></td>",n);
				  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=30>"\
					"<td id=tdprompt align=left>%s:</td>",search(lsystem,"cur_pass"));
					fprintf(cgiOut,"<td><input type=password name=u_pass1 size=21></td>"\
					"<td style=font-size:14px;color:#FF0000>%s</td>",search(lsystem,"pwd_dep"));
					fprintf(cgiOut,"</tr>"\
				  "<tr height=30>"\
					"<td id=tdprompt align=left>%s:</td>",search(lsystem,"new_pass"));
					fprintf(cgiOut,"<td><input type=password name=u_pass2 size=21></td>"\
					"<td style=font-size:14px;color:#FF0000>%s</td>",search(lsystem,"pwd_dep"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=30>"\
					"<td id=tdprompt align=left>%s:</td>",search(lsystem,"con_pass"));
					fprintf(cgiOut,"<td><input width=140 type=password name=u_pass3 size=21></td>"\
					"<td style=font-size:14px;color:#FF0000>%s</td>",search(lsystem,"pwd_dep"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr>");
					fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_modusr value=%s></td>",m);
				  fprintf(cgiOut,"</tr>"\
			  "</table>");
	     fprintf(cgiOut,"</td>"\
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
	free(procductId);
	return 0;
}

void modifyuser_hand(char *mu,struct list *lpublic, struct list *lsystem)
{
  char name[N],pass1[N],pass2[N],pass3[N]; 
  memset(name,0,N);
  memset(pass1,0,N);
  memset(pass2,0,N);
  memset(pass3,0,N);
  int ret=-1;
  int ret1;
  int status,status1;
  char *command = (char*)malloc(PATH_LENG);
  char *command1 = (char *)malloc(PATH_LENG);
  cgiFormStringNoNewlines("u_name", name, N);
  cgiFormStringNoNewlines("u_pass1", pass1, N);
  cgiFormStringNoNewlines("u_pass2", pass2, N);
  cgiFormStringNoNewlines("u_pass3", pass3, N);

  memset(command1,0,PATH_LENG);
  strcat(command1,"checkpwd.sh");   
  strcat(command1," ");
  strcat(command1,mu);
  strcat(command1," ");
  strcat(command1,pass1);
  strcat(command1," ");
  strcat(command1,"11");
  
  status1 = system(command1); 	   /*检查修改密码操作是否为用户本人*/
  ret1 = WEXITSTATUS(status1);
  memset(command, 0 ,PATH_LENG);
  if(ret1 == 1)
  {
	  if((strcmp(pass1,"")!=0)&&(strcmp(pass2,"")!=0)&&(strcmp(pass3,"")!=0))
	  {
		  if((strlen(pass2)>=6)&&(strlen(pass2)<=20))
		  {
		  		if(checkpassword(pass2)==0)
		  		{
				  	if(strcmp(pass2,pass3)==0)
					{
				          /************************************************************************************/
				          strcat(command,"chpass.sh");
				          strcat(command," ");
						  strcat(command,mu);
						  strcat(command," ");
						  strcat(command,pass2);
						  strcat(command," ");
						  strcat(command,"normal");
				  	      /*调用修改密码脚本*/
						  status = system(command);
						  ret = WEXITSTATUS(status);
						  if(ret == 0)
							 ShowAlert(search(lpublic,"oper_succ"));
						  else
							 ShowAlert(search(lpublic,"oper_fail"));	 

				          /************************************************************************************/
				  	}
					else
					{
						 	ShowAlert(search(lsystem,"pass_incon"));  /*新密码不一致*/ 
					}
		  		}
				else
				{
					ShowAlert(search(lsystem,"pwd_dep"));//密码含有非法字符
				}
		  }
		  else
		  {
		 		ShowAlert(search(lsystem,"pwd_len"));
		  }	
	  }
	  else
	  {
	  		ShowAlert(search(lpublic,"pass_not_null"));  /*密码不能为空*/ 
	  }
  }
  else
  {
  	  ShowAlert(search(lsystem,"curpass_error"));			/*用户当前密码错误*/
  }
  free(command);
  free(command1);
}

