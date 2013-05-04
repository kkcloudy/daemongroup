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
* wp_login_limit.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for login user limit
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
#include "ws_user_limit.h"


int ShowExportConfPage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	char cmd[128];
	memset(cmd,0,128);
	sprintf(cmd,"sudo %s",LIMIT_INIT);
	system(cmd);

	ShowExportConfPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowExportConfPage(struct list *lpublic, struct list *lsystem)
{ 
	 
	  char *encry=(char *)malloc(BUF_LEN);				
	  char *str;
	  char buff[128];
	  memset(buff,0,128);

	  int i;
	  /*处理文本框内容和下拉框内容*/
	  char domain[32];
	  memset(domain,0,32);

	  char type[32];
	  memset(type,0,32);

	  char item[32];
	  memset(item,0,32);

	  char value[32];
	  memset(value,0,32);

	  char zstring[128];
	  memset(zstring,0,128);  /*总串*/
	
      char addn[N]="";	
	  
	  ST_LIMIT_ALL limitall;   //初始化总的
      memset(&limitall,0,sizeof(limitall));

     // ST_LIMIT_INFO limitkeyl; //初始化单个的
     // memset(&limitkeyl,0,sizeof(limitkeyl));

      int limnum=0;

	  char menu[21]="menulist";
      char* i_char=(char *)malloc(10);

	  int cl=1;
	  char log_encry[BUF_LEN];
	 
	  if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
	  {
		  memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
		  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		  return 0;
		}
		strcpy(addn,str);
		memset(log_encry,0,BUF_LEN);				   /*清空临时变量*/
	  }
	  else
	  	{
    	cgiFormStringNoNewlines("encry_import",log_encry,BUF_LEN);
        str=dcryption(log_encry);
        if(str==NULL)
        {
          ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
  	    }
  		 strcpy(addn,str);
  	     memset(log_encry,0,BUF_LEN);                   /*清空临时变量*/
  	
    	}
	  cgiFormStringNoNewlines("encry_import",log_encry,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  /***********************2008.5.26*********************/
	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
	  fprintf(cgiOut,"<title>%s</title>\n",search(lsystem," "));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".usrlis {overflow-x:hidden;	overflow:auto; width: 416px; height: 220px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
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
	  "</head>"\
	  "<body>");

    	  if(cgiFormSubmitClicked("conf") == cgiFormSuccess)
    	  	{
            char cmd[128];
			memset(cmd,0,128);
			sprintf(cmd,"sudo %s",LIMIT_INIT);
			system(cmd);            
    	  	}
		
	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>\n"\
	  "<div align=center>\n"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n");  //1111111111111111
	  fprintf(cgiOut,"<tr>\n"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  //2222222222222
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=conf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	
		if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
		{
     		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

		}
     	else
     	{
       	    fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",log_encry,search(lpublic,"img_cancel"));

			}
		fprintf(cgiOut,"</tr>"\
		"</table>");  //22222222222
	 
	 
	  
	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>"); //333333333333333
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 valign=top cellspacing=0 cellpadding=0>"); //4444444444
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 valign=top border=0 cellspacing=0 cellpadding=0>"); //55555555555555							

			  fprintf(cgiOut,"<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>"); 
                     /*管理员*/
                     if(checkuser_group(addn)==0)
                     	{
					if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
					{					 
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
					    "<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
					    fprintf(cgiOut,"</tr>");						
						//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					     "<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
					    fprintf(cgiOut,"</tr>");
						
						//boot upgrade 
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
						fprintf(cgiOut,"</tr>");


						//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					     "<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
					    fprintf(cgiOut,"</tr>");

						
					    fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"l_user"));  /*突出显示*/
					    fprintf(cgiOut,"</tr>");

						
					//新增时间条目
					fprintf(cgiOut,"<tr height=25>\n"\
					  "<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>\n");

					//新增NTP条目
					fprintf(cgiOut,"<tr height=25>\n"\
					  "<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
					fprintf(cgiOut,"</tr>\n");
					//新增pppoe条目
        					fprintf(cgiOut,"<tr height=25>"\
        					  "<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE");
        					fprintf(cgiOut,"</tr>");
					

					
					}
					else if(cgiFormSubmitClicked("conf") == cgiFormSuccess)			
					{
						fprintf(cgiOut,"<tr height=25>\n"\
						  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
						fprintf(cgiOut,"</tr>\n"\
						"<tr height=25>\n"\
						  "<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>\n");
					    fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));;
					    fprintf(cgiOut,"</tr>\n");					  						
						//新增条目
					    fprintf(cgiOut,"<tr height=25>\n"\
					       "<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",log_encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
					    fprintf(cgiOut,"</tr>");

						//boot upgrade 
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
						fprintf(cgiOut,"</tr>");


						//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					       "<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",log_encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
					    fprintf(cgiOut,"</tr>");
					
					    fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n",search(lpublic,"menu_san"),search(lpublic,"l_user"));  /*突出显示*/
						fprintf(cgiOut,"</tr>");

						
					//新增时间条目
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");

						//新增NTP条目
					fprintf(cgiOut,"<tr height=25>\n"\
					  "<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
					fprintf(cgiOut,"</tr>\n");

					//新增pppoe条目
        					fprintf(cgiOut,"<tr height=25>"\
        					  "<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",log_encry,search(lpublic,"menu_san"),"PPPOE");
        					fprintf(cgiOut,"</tr>");
					}
                     	}
					else
						{

                       if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
					{					 
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
						fprintf(cgiOut,"</tr>");					
					  					
					//新增时间条目
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");
					}
					else if(cgiFormSubmitClicked("conf") == cgiFormSuccess)			
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",log_encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
						fprintf(cgiOut,"</tr>");					
					
					 
					//新增时间条目
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>\n",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");

						}
						}

                      if(cgiFormSubmitClicked("view") == cgiFormSuccess || cgiFormSubmitClicked("current") == cgiFormSuccess)
                      	{						
                                for(i=0;i<2;i++)
             					{
             					    fprintf(cgiOut,"<tr height=25>"\
             						"<td id=tdleft>&nbsp;</td>"\
             					  "</tr>");
             					}               

                      	}
                   
				       fprintf(cgiOut,"</table>"); //555555555555555555
				        fprintf(cgiOut,"</td>"\
				        "<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">\n");

						fprintf(cgiOut,"<table width=560 border=0 cellspacing=0 cellpadding=0>"); //666666666666

///////////////////////////////  显示内容，并进行配置修改  是在一个table中间


fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","STATE");
fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","NAME");
fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","RULER");
fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","DEVICE");
fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","NUMBER");
fprintf(cgiOut,"<th  style=font-size:12px></th>");
fprintf(cgiOut,"</tr>");

read_limit_xml(LIMIT_XML_PATH, &limitall);
limnum=limitall.l_num;

for(i=0;i<limnum;i++)
{
 memset(menu,0,21);
 strcpy(menu,"menulist");
 sprintf(i_char,"%d",i+1);
 strcat(menu,i_char);
	
fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
fprintf(cgiOut,"<td align=left>%s</td>",limitall.limit_info[i].state);
fprintf(cgiOut,"<td align=left>%s</td>",limitall.limit_info[i].name);
fprintf(cgiOut,"<td align=left>%s</td>",limitall.limit_info[i].ruler);
fprintf(cgiOut,"<td align=left>%s</td>",limitall.limit_info[i].device);
fprintf(cgiOut,"<td align=left>%s</td>",limitall.limit_info[i].number);															
fprintf(cgiOut,"<td>");
fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(10-i),menu,menu);
fprintf(cgiOut,"<img src=/images/detail.gif>"\
"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
fprintf(cgiOut,"<div id=div1>");
fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_login_config.cgi?UN=%s&ID=%d target=mainFrame>%s</a></div>",encry,i+1,search(lpublic,"log_mod"));
fprintf(cgiOut,"</div>"\
"</div>"\
"</div>");
fprintf(cgiOut,"</td>");	 	
fprintf(cgiOut,"</tr>\n");	

cl = !cl;
}
		
///////////////////////////////

				   if(cgiFormSubmitClicked("conf") != cgiFormSuccess)
				   {
					 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>\n",encry);
				   }
				   else if(cgiFormSubmitClicked("conf") == cgiFormSuccess)
				   {
					 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>\n",log_encry);
				   }	 
		
					
   fprintf(cgiOut,"</table>"); //666666666666666
			  fprintf(cgiOut,"</td>\n"\
			  "</tr>\n"\
			  "<tr height=4 valign=top>\n"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
			  "</tr>"\
			"</table>");//444444444444
		  fprintf(cgiOut,"</td>\n"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
		"</tr>\n"\
	  "</table></td>\n"); //333333333333
	fprintf(cgiOut,"</tr>\n"\
	"<tr>\n"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
	  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
	"</tr>\n"\
	"</table>\n");//111111111111111
	fprintf(cgiOut,"</div>\n"\
	"</form>\n"\
	"</body>\n"\
	"</html>\n");
free(encry); 
free(i_char);
return 0;
}  

