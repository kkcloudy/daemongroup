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
* wp_log_view.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for syslog config
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
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_log_conf.h"


int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic);

void show_content(char *color,char *file_name);

int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ShowVersionDelPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic)
{ 
  char *encry=(char *)malloc(BUF_LEN);
 
  char *str;
 
  char showtype[N];

  char file_name[128];  //读取文件
  memset(file_name,0,128);

  char color[128];  //读取文件
  memset(color,0,128);
  
  char *cmd = (char *)malloc(PATH_LENG);   
  memset(cmd,0,PATH_LENG); 
 
  int i = 0;   

//  int lnumber=0;

  int ret;

  char key_word[128];
  memset(key_word,0,128);
 
  cgiFormStringNoNewlines("Nb", file_name, 128);  
  cgiFormStringNoNewlines("Nc", color, 128);  

  if(cgiFormSubmitClicked("log_view") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}

	
  }
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	 "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".usrlis {overflow-x:hidden;	overflow:auto; width: 750px; height: 420px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
  "</head>"\
  "<script type='text/javascript'>"\
  "function changestate(){"\
  "var a1 = document.getElementsByName('showtype')[0];"\
  "var a2 = document.getElementsByName('showtype')[1];"\
  "}"\
  "</script>"\
  "<body>");  

  	 
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>");    //  11111111111111111111111111111
  fprintf(cgiOut,"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lpublic,"log_info"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
        // 鉴权
          fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
		  fprintf(cgiOut,"<input type=hidden name=Nb value=%s />",file_name);
	      fprintf(cgiOut,"<input type=hidden name=Nc value=%s />",color);

    	   fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  // 222222222
           fprintf(cgiOut,"<tr>");		
		   
		   fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=log_view style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
           fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

		   fprintf(cgiOut,"</tr>"\
          "</table>");  //222222222222
				
		
	fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>");
    fprintf(cgiOut,"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"); //444444444444444444
      fprintf(cgiOut,"<tr>");
        fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>");	 //555555555555555555555	
            fprintf(cgiOut,"<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>");
	          fprintf(cgiOut,"<tr>");  //次内
              fprintf(cgiOut,"<td><table width=120 border=0 cellspacing=0 cellpadding=0>"); // 6666666666666
                   fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
			  fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
			  fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");       		

								for(i=0;i<18;i++)
								{
									fprintf(cgiOut,"<tr height=25>"\
									  "<td id=tdleft>&nbsp;</td>"\
									"</tr>");
								}
								
								   fprintf(cgiOut,"</table>"); //6666666666666666666666
							   fprintf(cgiOut,"</td>");
							   fprintf(cgiOut,"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

				  //lnumber=line_num(temppath,"wc -l");	

                  //改变显示状态
			      fprintf(cgiOut,"<script type='text/javascript'>"\
							"changestate();"\
							"</script>");		
				  fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>");	  //777777777777777

				  fprintf(cgiOut,"<tr><th width=150 align=left style=font-size:14px>%s</th></tr>",search(lpublic,"log_detail"));
				  fprintf(cgiOut,"<tr>");
				  fprintf(cgiOut,"<td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
				   "</tr>");
				  
                  fprintf(cgiOut,"<tr height=5><td></td></tr>");

				  /*
				  fprintf(cgiOut,"<tr>");
				  fprintf(cgiOut,"<td>");
		          fprintf(cgiOut,"%s :  %d ","日志信息条数",lnumber);
				  fprintf(cgiOut,"</td>");
				  fprintf(cgiOut,"</tr>");
				  */

                memset(showtype,0,N);
				cgiFormStringNoNewlines("showtype", showtype, N);
				memset(key_word,0,128);
				cgiFormStringNoNewlines("keys", key_word, N);
				if(strcmp(showtype,"1")==0)
				{

				  fprintf(cgiOut,"<tr>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\" checked>&nbsp;&nbsp;");
                  fprintf(cgiOut,"%s",search(lpublic,"log_all_item"));
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"</tr>");
                  
                  fprintf(cgiOut,"<tr height=5><td></td></tr>");
                  
                  fprintf(cgiOut,"<tr>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\" >&nbsp;&nbsp;");
                  fprintf(cgiOut,"%s",search(lpublic,"log_key"));
				  fprintf(cgiOut,"&nbsp;&nbsp;<input type=text name=keys value=\"\">");
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"</tr>");
				}
				else if(strcmp(showtype,"2")==0)
				{
				  fprintf(cgiOut,"<tr>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">&nbsp;&nbsp;");
                  fprintf(cgiOut,"%s",search(lpublic,"log_all_item"));
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"</tr>");
                  
                  fprintf(cgiOut,"<tr height=5><td></td></tr>");
                  
                  fprintf(cgiOut,"<tr>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\" checked>&nbsp;&nbsp;");
                  fprintf(cgiOut,"%s",search(lpublic,"log_key"));
				  fprintf(cgiOut,"&nbsp;&nbsp;<input type=text name=keys value=\"%s\">",key_word);
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"</tr>");

				}
                else{				
                  fprintf(cgiOut,"<tr>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\" checked>&nbsp;&nbsp;");
                  fprintf(cgiOut,"%s",search(lpublic,"log_all_item"));
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"</tr>");
                  
                  fprintf(cgiOut,"<tr height=5><td></td></tr>");
                  
                  fprintf(cgiOut,"<tr>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"<input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\" >&nbsp;&nbsp;");
                  fprintf(cgiOut,"%s",search(lpublic,"log_key"));
				  fprintf(cgiOut,"&nbsp;&nbsp;<input type=text name=keys value=\"\">");
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"<td>");
                  fprintf(cgiOut,"</td>");
                  fprintf(cgiOut,"</tr>");
				
                	}

				  fprintf(cgiOut,"<tr>");
				  fprintf(cgiOut,"<td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
				   "</tr>");
				  
		           if(cgiFormSubmitClicked("log_view") == cgiFormSuccess)

		           	{
		           	memset(file_name,0,128);
		            cgiFormStringNoNewlines("Nb", file_name, 128);  
					memset(showtype,0,N);
					cgiFormStringNoNewlines("showtype", showtype, N);
					memset(key_word,0,128);
					cgiFormStringNoNewlines("keys",key_word,128);

                    if(strcmp(showtype,"1")==0)
                                {
     						    sprintf(cmd,"cat /var/log/%s | sort -r > %s",file_name,temppath);  
								ret=system(cmd);
								if(ret==0)
								show_content(color,file_name);                 
                                }
					  if(strcmp(showtype,"2")==0)
                                {
                                if(strcmp(key_word,"")==0)
                                	{
									ShowAlert(search(lpublic,"log_key_null"));
                                	}
								else{
     						    sprintf(cmd,"cat /var/log/%s |grep \"%s\" |sort -r > %s",file_name,key_word,temppath); 	
                                ret=system(cmd);
								if(ret==0)
								show_content(color,file_name);              
									}
                                }
		           	}
	
		 			fprintf(cgiOut,"</table>");		//7777777777777777777777	
			 
			fprintf(cgiOut,"</td></tr>");
            fprintf(cgiOut,"<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>");
			
         fprintf(cgiOut, "</table>"); // 555555555555555
        fprintf(cgiOut,"</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>");
      fprintf(cgiOut,"</tr>");  //次内
    fprintf(cgiOut,"</table></td>"); //444444444444444
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"); // 1111111111111111111111111111111111111111111
fprintf(cgiOut,"</div>"\
"</form>"\
"</body>");
fprintf(cgiOut,"</html>");  

free(encry);
return 0;
}

//接受的命令不同，一个是全部显示，一个是按关键字过滤
void show_content(char *color,char *file_name)
{
     FILE *pp;
     char buff[128]; //读取popen中的块大小

     fprintf(cgiOut,"<tr>");
     fprintf(cgiOut,"<td align=left>");
 
     fprintf(cgiOut,"<div class=usrlis><table frame=below rules=rows width=750 border=0 cellspacing=0 cellpadding=0>"\
 		   "<tr height=30 bgcolor=#eaeff9 style=font-size:16px>"\
 			"</tr>"); 
 	 
     fprintf(cgiOut,"<tr height=10><td></td></tr>");	                                               			  
 
 	 pp=fopen(temppath,"r");  
     if(pp==NULL)
     fprintf(cgiOut,"error open the pipe");
          else
 			    {
 				  fgets( buff, sizeof(buff), pp );	//很重要 ，不然与条目不匹配 						 
 		  do
 		  {										   
 			  fprintf(cgiOut,"<tr bgcolor=%s><td>",color);		
 
 			  strcpy(file_name,buff);					  
 		      fprintf(cgiOut,"<br>");   					     
 			  fprintf(cgiOut,"%s",file_name); 
 			  fprintf(cgiOut,"</td></tr>");
 			  fgets( buff, sizeof(buff), pp ); 									   
 			   }while( !feof(pp) ); 					   
			  fclose(pp);
 							  }
 
     fprintf(cgiOut,"</table></div></td>");	
     fprintf(cgiOut,"</tr>");			

}
