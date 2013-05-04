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
* wp_usrlis.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/wait.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include <libxml/xpathInternals.h>


#define UsrNum 50000


int ShowUserlistPage(struct list *lpublic, struct list *lsystem, struct list *lcontrol);   /*n代表加密后的字符串*/


int cgiMain()
{
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lsystem;     /*解析help.txt文件的链表头*/
  struct list *lcontrol;     /*解析help.txt文件的链表头*/
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsystem=get_chain_head("../htdocs/text/system.txt"); 
  lcontrol=get_chain_head("../htdocs/text/control.txt"); 
  ShowUserlistPage(lpublic,lsystem,lcontrol);
  release(lcontrol);  
  release(lpublic);  
  release(lsystem);  
  return 0;
}

int ShowUserlistPage(struct list *lpublic, struct list *lsystem, struct list *lcontrol)
{ 
  int i;
  int retu;
  //int ret1;                           /*get_user管理员返回值*/
  //int ret2;                           /*get_user普通用户返回值*/
  char *encry=(char *)malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
  char *str;
  memset(encry,0,BUF_LEN);
  char menu[21]="menulist";
  char* i_char=(char *)malloc(10);
  char * usrName=(char *)malloc(33);
  memset(usrName,0,33);
  char * deleteOP=(char *)malloc(10);
  memset(deleteOP,0,10);
  char * command=(char *)malloc(100);
  memset(command,0,100);
  char *session_id= (char *)malloc(N);
  int ret,status;
  int search_result;
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  cgiFormStringNoNewlines("USERNAME", usrName, 33);
  cgiFormStringNoNewlines("DELRULE", deleteOP, 10);
  str=dcryption(encry);
  if(str==NULL)
  {
    ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
    return 0;
  }
 // char * procductId=readproductID();
  //fprintf(stderr,"procductId=%s",procductId);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lsystem,"user_manage"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
  	  "#div1{ width:80px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:78px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".usrlis {overflow-x:hidden;	overflow:auto; width: 690; height: 220px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
	"</head>"\
  "<script src=/ip.js>"\
  "</script>"\
    "<script type=\"text/javascript\">"\
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
			 "</script>"\
	"<form>");
	if(access(USER_SYSLOG_XML,0)!=0)
	{		
		init_user_syslog_xml();
	}
	
	if(strcmp(deleteOP,"delete")==0)
	{
		int del_success = 0;
		memset(session_id,0,N);
		search_result=Search_user_infor_byName(usrName,1,session_id);
		if(search_result==0)
		{
	    		strcat(command,"userdel.sh");		
	    		strcat(command," ");
	    		strcat(command,usrName);
	    		fprintf(stderr,"command=%s",command);
	    		status = system(command);		 /*删除用户*/
	    		ret = WEXITSTATUS(status);
	    		if(ret==0)
	         	{
	         	  del_success = 1;
	         	  ShowAlert(search(lpublic,"oper_succ"));
	         	}
	         	else
	         	{
	         	  ShowAlert(search(lpublic,"oper_fail"));
	         	}
			
		}
		else
		{

			int out_flag=0;
		    out_flag=if_user_outtime(usrName);
			if(out_flag==2)
			{
	    		strcat(command,"userdel.sh");		
	    		strcat(command," ");
	    		strcat(command,usrName);
	    		fprintf(stderr,"command=%s",command);
	    		status = system(command);		 /*删除用户*/
	    		ret = WEXITSTATUS(status);
	    		if(ret==0)
	         	{
	         	  del_success = 1;
	         	  ShowAlert(search(lpublic,"oper_succ"));
	         	}
	         	else
	         	{
	         	  ShowAlert(search(lpublic,"oper_fail"));
	         	}
			}
			else
			{
              ShowAlert(search(lpublic,"usr_has_login"));
			}
			
		}

		if(del_success == 1)
		{
			del_user_syslog_by_name(usrName);
		}			
	}
	fprintf(cgiOut,"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"); 
    fprintf(cgiOut,"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"user_manage"));
	  fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		
		     
			fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
			"<tr>"); 

			
		
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

			fprintf(cgiOut,"</tr>"\
			"</table>"); 
				  
		   	  
		fprintf(cgiOut,"</td>"\
	  "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
	  "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"); 
	
		fprintf(cgiOut,"<tr>"\
		  "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
		  "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); 
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				"<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				"<td><table width=120 border=0 cellspacing=0 cellpadding=0>");
					fprintf(cgiOut,"<tr height=25>"\
					  "<td id=tdleft>&nbsp;</td>"\
					"</tr>"\
					"<tr height=26>"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"user_list"));	 /*突出显示*/
					fprintf(cgiOut,"</tr>");
					retu=checkuser_group(str);
					if(retu==0)  /*管理员*/
					{
      					fprintf(cgiOut,"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_addusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"add_user"));
      					fprintf(cgiOut,"</tr>"\
      					"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modpass.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));
      					fprintf(cgiOut,"</tr>"\
      					"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modpri.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_privilege"));
      					fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modsyslog.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_loglevel"));
      					fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"login_info"));
					fprintf(cgiOut,"</tr>");
					}
					else
					{	
						fprintf(cgiOut,"<tr height=25>"\
      					  "<td align=left id=tdleft><a href=wp_modusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));
      					fprintf(cgiOut,"</tr>");
						
						fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_user.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"login_info"));
					fprintf(cgiOut,"</tr>");
					}				
					
					int rowsNum=0;
					if(retu==0)  /*管理员*/
						rowsNum=8;
					else rowsNum=11;
					for(i=0;i<8;i++)
					{
					  fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
					}
				  fprintf(cgiOut,"</table>");
				fprintf(cgiOut,"</td>");
				if(retu==0)  /*管理员*/
              		fprintf(cgiOut,"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");
              	else
              		fprintf(cgiOut,"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px;padding-top:25px\">");


               
				fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");
				  fprintf(cgiOut,"<tr>"\
          "<td align=left>");


		/*------------------------------用户名列表的那块----------------------------------------------*/
		
		 
		  fprintf(cgiOut,"<div class=usrlis><table frame=below rules=rows width=650 border=0 cellspacing=0 cellpadding=0>"\
               "<tr height=30 bgcolor=#eaeff9 style=font-size:16px>"\
               "<th width=80 align=left>&nbsp;</th>"\
                "<th align=left style=\"padding-left:20px\" width=100 style=font-size:16px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsystem,"username"));
                fprintf(cgiOut,"<th align=left style=\"padding-left:20px\" width=120 style=font-size:16px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsystem,"pri"));
				fprintf(cgiOut,"<th align=left style=\"padding-left:20px\" width=150 style=font-size:16px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsystem,"log_lever"));
				fprintf(cgiOut,"<th align=left style=\"padding-left:20px\" width=150 style=font-size:16px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsystem,"oper_lever"));
                fprintf(cgiOut,"<th width=80 align=left>&nbsp;</th>"\
                "</tr>");
   /*			  "<tr height=30 bgcolor=#eaeff9 style=font-size:16px>"\
				"<th align=left style=\"padding-left:20px\" width=100>%s</th>",ser_var("username"));
				  fprintf(cgiOut,"<th align=left style=\"padding-left:20px\" width=154>%s</th>",ser_var("pri"));
				  fprintf(cgiOut,"</tr>");*/
  /********************************************************************************************************/
					/*显示用户列表*/
					//ret1 = get_user(ADMINGROUP); 
					//ret2 = get_user(VIEWGROUP);
					//if((ret1 == 0)||(ret2 == 0))
					 // {
						  /*输出成功*/
					 // }
					//else
					 // {
						 //  ShowAlert(search(lpublic,"error_open"));
					 // } 									   
  
  /********************************************************************************************************/	
  			    struct group * grentry = NULL;
               char *ptr=NULL;
               int cl = 1;
               int userNameNum=50000;
               int adminNum=0;
			   char log_info[10] = { 0 };
			   char oper_info[10] = { 0 };
               grentry = getgrnam(ADMINGROUP);
               if (grentry)
               {
                   for(i=0;(ptr=grentry->gr_mem[i])!=NULL;i++)
                 	{  
                 	   memset(menu,0,21);
					  	strcpy(menu,"menulist");
					  	sprintf(i_char,"%d",i+1);
					  	strcat(menu,i_char);
                       fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left style=\"padding-left:20px\" font-size:14px>",setclour(cl));
                       fprintf(cgiOut,"<td>%d</td>",i+1);
					   fprintf(cgiOut,"<td>%s</td>",ptr);
                       fprintf(cgiOut,"<td>administrator</td>");
					   get_user_syslog_by_name(ptr,log_info,oper_info);
					   fprintf(cgiOut,"<td>%s</td>",log_info);
					   fprintf(cgiOut,"<td>%s</td>",oper_info);
                       if(retu==0)  /*管理员*/
                       {
                           fprintf(cgiOut,"<td>");
                           fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(userNameNum-i),menu,menu);
         				   fprintf(cgiOut,"<img src=/images/detail.gif>"\
         				   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
         				   fprintf(cgiOut,"<div id=div1>");
    			   		   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_addusr.cgi?UN=%s target=mainFrame>%s</a></div>",encry,search(lpublic,"create"));
         				   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_modpass.cgi?UN=%s&USERNAME=%s target=mainFrame>%s</a></div>",encry,ptr,search(lsystem,"modify_password"));
    			   		   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_modpri.cgi?UN=%s&USERNAME=%s target=mainFrame>%s</a></div>",encry,ptr,search(lsystem,"modify_privilege"));
						   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_modsyslog.cgi?UN=%s&USERNAME=%s target=mainFrame>%s</a></div>",encry,ptr,search(lsystem,"modify_loglevel"));
         				   fprintf(cgiOut,"</div>"\
         				   "</div>"\
         				   "</div>");
        				   fprintf(cgiOut,"</td>");
    				   }
    				   else
    				   		fprintf(cgiOut,"<td>&nbsp;</td>");
                       fprintf(cgiOut,"</tr>");
                       cl=!cl;
                       
                 	}
                 	adminNum=i;
               }
			  
			   
               grentry = getgrnam(VIEWGROUP);
               if (grentry)
               {
                   for(i=0;(ptr=grentry->gr_mem[i])!=NULL;i++)
                 	{  
                 		memset(menu,0,21);
					  	strcpy(menu,"menulist");
					  	sprintf(i_char,"%d",adminNum+i+1);
					  	strcat(menu,i_char);
                       fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left style=\"padding-left:20px\" font-size:14px>",setclour(cl));
                       fprintf(cgiOut,"<td>%d</td>",adminNum+i+1);
					   fprintf(cgiOut,"<td>%s</td>",ptr);
                       fprintf(cgiOut,"<td>user</td>");
					   fprintf(cgiOut,"<td></td>");
					   fprintf(cgiOut,"<td></td>");
                       if(retu==0)  /*管理员*/
                       {
                           fprintf(cgiOut,"<td>");
                           fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(userNameNum-i-adminNum),menu,menu);
         				   fprintf(cgiOut,"<img src=/images/detail.gif>"\
         				   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
         				   fprintf(cgiOut,"<div id=div1>");
         				   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_addusr.cgi?UN=%s target=mainFrame>%s</a></div>",encry,search(lpublic,"create"));
         				   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_usrlis.cgi?UN=%s&USERNAME=%s&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,ptr,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
         				   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_modpass.cgi?UN=%s&USERNAME=%s target=mainFrame>%s</a></div>",encry,ptr,search(lsystem,"modify_password"));
    			   		   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_modpri.cgi?UN=%s&USERNAME=%s target=mainFrame>%s</a></div>",encry,ptr,search(lsystem,"modify_privilege"));
    			   		   //fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_modsyslog.cgi?UN=%s&USERNAME=%s target=mainFrame>%s</a></div>",encry,ptr,search(lsystem,"modify_loglevel"));
         				   fprintf(cgiOut,"</div>"\
         				   "</div>"\
         				   "</div>");
        				   fprintf(cgiOut,"</td>");
    				   }
    				   else
    				   		fprintf(cgiOut,"<td>&nbsp;</td>");



					   
                       fprintf(cgiOut,"</tr>");
                       cl=!cl;
                 	}
				   for(i=0;i<3;i++){
				   fprintf(cgiOut,"<tr height=25><td colspan=6>&nbsp;</td></tr>");
				   	}
				  
               }
			   
			  		   
          fprintf(cgiOut,"</table></div></td>");	
		  
		  /*----------------end user list--------------------------------------------*/
		  
			fprintf(cgiOut,"</table>");		
             fprintf(cgiOut, "</td>"\
            "</tr>"\
			  "<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
			  "</tr>"\
			"</table>");
		  fprintf(cgiOut,"</td>"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
		"</tr>");
	  fprintf(cgiOut,"</table></td>");
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
//free(procductId);
free(i_char);
free(encry);
free(command);
free(usrName);
free(session_id);
return 0;
}

