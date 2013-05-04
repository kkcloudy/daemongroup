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
* wp_delusr.c
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


int ShowDeleteuserPage(struct list *lpublic, struct list *lsystem);
void deleteuser_hand(struct list *lpublic, struct list *lsystem);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt"); 

	ShowDeleteuserPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowDeleteuserPage(struct list *lpublic, struct list *lsystem)
{ 
  char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
  char *str;
  FILE *fp;
  char lan[3];
  char del_encry[BUF_LEN]; 
  char addn[N];         

  int ret2;           /*set_deluser()的返回值*/
  
  st_deluser *user_this=NULL;/*链表操的当前指针*/
  st_deluser ulist_node; 
  st_deluser *ulist_head=&ulist_node;/*链表头结点指针*/  
  memset(ulist_head, 0, sizeof(st_deluser));

  int i;              
  if(cgiFormSubmitClicked("submit_deleteuser") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
      return 0;
	}
	strcpy(addn,str);
	memset(del_encry,0,BUF_LEN);                   /*清空临时变量*/
  }  
  char * procductId=readproductID();
  cgiFormStringNoNewlines("encry_delusr",del_encry,BUF_LEN);
  if(cgiFormSubmitClicked("submit_deleteuser") == cgiFormSuccess)
  	cgiFormStringNoNewlines("log_name", addn, N);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsystem,"user_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".delusr {overflow-x:hidden;  overflow:auto; width: 446px; height: 220px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
  	"</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_deleteuser") == cgiFormSuccess)
    deleteuser_hand(lpublic,lsystem);
  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"user_manage"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lpublic,"error_open"));
	    }
	    else
	    {
			fseek(fp,4,0);						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp);	   
			fclose(fp);
	    }
	    if(strcmp(lan,"ch")==0)
    	{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_deleteuser style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  	if(cgiFormSubmitClicked("submit_deleteuser") != cgiFormSuccess)
          	{
          
                  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

			}
      		else
      		{
      		
      		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",del_encry,search(lpublic,"img_cancel"));

			}
			fprintf(cgiOut,"</tr>"\
          "</table>");
		}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_deleteuser style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  	if(cgiFormSubmitClicked("submit_deleteuser") != cgiFormSuccess)
		  	{
		  
      	      fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
            }
      		else
      		{
      	  	      			  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",del_encry);

				}
      	  	fprintf(cgiOut,"</tr>"\
		  "</table>");
		}		
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
				  if(cgiFormSubmitClicked("submit_deleteuser") != cgiFormSuccess)
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
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"delete_user"));   /*突出显示*/
                    fprintf(cgiOut,"</tr>");
				  }
				  else if(cgiFormSubmitClicked("submit_deleteuser") == cgiFormSuccess)				  
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",del_encry,search(lpublic,"menu_san"),search(lsystem,"user_list"));                       
                    fprintf(cgiOut,"</tr>"\
                    "<tr height=25>"\
                      "<td align=left id=tdleft><a href=wp_addusr.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",del_encry,search(lpublic,"menu_san"),search(lsystem,"add_user"));
                    fprintf(cgiOut,"</tr>"\
   				    "<tr height=25>"\
                      "<td align=left id=tdleft><a href=wp_modpass.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",del_encry,search(lpublic,"menu_san"),search(lsystem,"modify_password"));
                    fprintf(cgiOut,"</tr>"\
  				    "<tr height=25>"\
                      "<td align=left id=tdleft><a href=wp_modpri.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",del_encry,search(lpublic,"menu_san"),search(lsystem,"modify_privilege"));
                    fprintf(cgiOut,"</tr>"\
  				    "<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"delete_user"));   /*突出显示*/
                    fprintf(cgiOut,"</tr>");
				  }
                  for(i=0;i<4;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                "<table width=280 border=0 cellspacing=0 cellpadding=0>"\
        "<tr>"\
          "<td align=left>"\
		  "<div class=delusr><table frame=below rules=rows width=500 border=1 cellspacing=0 cellpadding=0>"\
               "<tr height=30 bgcolor=#eaeff9 style=font-size:16px align=left>"\
                "<th width=30>&nbsp;</th>"\
                "<th align=left style=\"padding-left:0px\" width=100 style=font-size:16px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsystem,"username"));
                fprintf(cgiOut,"<th align=left style=\"padding-left:0px\" width=154 style=font-size:16px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsystem,"pri"));
                fprintf(cgiOut,"</tr>");
                
/******************************************************************************************************/
				//ret1 = set_deluser(ADMINGROUP,addn);      /*显示可删除用户列表*/
				ret2 = set_deluser(VIEWGROUP,addn, ulist_head);
                
				if(ret2 == 0){
                    int cl = 1;
                    while(NULL != user_this){
						fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
						fprintf(cgiOut,"<td><input type=checkbox name=check value=%s></td>",user_this->name);
						fprintf(cgiOut,"<td align=left style=font-size:14px>%s</td>",user_this->name);
						fprintf(cgiOut,"<td>%s</td>",user_this->group);
						fprintf(cgiOut,"</tr>");
						cl = !cl;
						user_this=user_this->next;					
					}
            	}
				else{
					   ShowAlert(search(lpublic,"error_open"));
				}
				
				free_deluser(ulist_head);/*set_deluser后用此函数释放链表*/
/******************************************************************************************************/				
          fprintf(cgiOut,"</table></div></td>"\
          "</tr>"\
		"<tr>"\
		"<td>"\
		"<input type=hidden name=log_name value=%s>",addn);
		fprintf(cgiOut,"</td>"\
		"</tr>"\
	  	"<tr>");
		if(cgiFormSubmitClicked("submit_deleteuser") != cgiFormSuccess)
        {
		  fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_delusr value=%s></td>",encry);
	    }
		else if(cgiFormSubmitClicked("submit_deleteuser") == cgiFormSuccess)
            {
              fprintf(cgiOut,"<td colspan=3><input type=hidden name=encry_delusr value=%s></td>",del_encry);
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
free(procductId);
free(encry);
return 0;
}

void deleteuser_hand(struct list *lpublic, struct list *lsystem)
{
  int result;   
  char **responses;
  int ret,status;
  int count = 0;            /*记录删除成功的个数*/

  
  result = cgiFormStringMultiple("check", &responses);
  if(result == cgiFormNotFound)           /*如果没有选择任何用户*/
    ShowAlert(search(lsystem,"select_user"));
  else                  
  {
    int i = 0;	

    while(responses[i])
    {
		char *command = (char*)malloc(PATH_LENG);  /*command存放命令行参数*/
		memset(command,0,PATH_LENG);        
		strcat(command,"userdel.sh");
		strcat(command," ");
		strcat(command,responses[i]);
		status = system(command);		 /*删除用户*/
		ret = WEXITSTATUS(status);

        if(ret==0)
        {
 		  count++;
        }
		
		free(command); 
		i++;
	}
	
    if(count)
	{
	  ShowAlert(search(lpublic,"oper_succ"));     	       	
	}
	else
	{
	  ShowAlert(search(lpublic,"oper_fail"));
	}
	cgiStringArrayFree(responses);
  }
}



