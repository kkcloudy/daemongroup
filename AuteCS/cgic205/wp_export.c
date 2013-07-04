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
* wp_export.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for export file
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
#include "ws_version_param.h"


int ShowExportConfPage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowExportConfPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowExportConfPage(struct list *lpublic, struct list *lsystem)
{ 
	FILE *fp1;
	char *encry=(char *)malloc(BUF_LEN);				
	char *str;

	int j;

	char *fpath=(char *)malloc(1024);
	memset(fpath,0,1024);
	char *path_conf=(char *)malloc(1024);
	memset(path_conf,0,1024);
	strcpy(path_conf,"/mnt/conf_xml.conf");

	memset(encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		return 0;
	}
	  /***********************2008.5.26*********************/
	cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lsystem,"export_config"));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		"<style type=text/css>"\
		".a3{width:30;border:0; text-align:center}"\
		"</style>"\
		"</head>"\
		"<body>");

		fprintf(cgiOut,"<form method=post encType=multipart/form-data action='/cgi-bin/wp_expconf.cgi'>");
		fprintf(cgiOut,"<div align=center>"\
		"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	

		fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>");
        fprintf(cgiOut,"<td width=62 align=center><a href=wp_export.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
			"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
			fprintf(cgiOut,"</tr>"\
			"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
			fprintf(cgiOut,"</tr>"\
			"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
			fprintf(cgiOut,"</tr>"\
			"<tr height=26>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"export_config"));  /*突出显示*/
			fprintf(cgiOut,"</tr>");
			//新增条目
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));

						//boot upgrade 
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
						fprintf(cgiOut,"</tr>");
			/*日志信息*/
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
			fprintf(cgiOut,"</tr>");


			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
			fprintf(cgiOut,"</tr>");


			fprintf(cgiOut,"</tr>");
			//新增时间条目
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
			fprintf(cgiOut,"</tr>");

			//新增NTP条目
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
			fprintf(cgiOut,"</tr>");
			//新增pppoe条目
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE");
			fprintf(cgiOut,"</tr>");
			//新增时间条目
			fprintf(cgiOut,"<tr height=26>"\
			"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service"));
			fprintf(cgiOut,"</tr>");
					

					for(j=0;j<2;j++)
					{
					  fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
					}
				  fprintf(cgiOut,"</table>"\
				"</td>"\
				"<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
					"<table width=280 border=0 cellspacing=0 cellpadding=0>"\
				  "<tr align=left>"\
					"<td>");
					if((fp1=fopen(path_conf,"r"))==NULL)	 //以只读方式打开配置文件
					{
							ShowAlert(search(lpublic,"export_w"));					
						
					}
					else
					{

					fprintf(cgiOut,"<input type=submit name=exportfile value=\"%s\">",search(lsystem,"export_config"));/*导出配置文件的连接*/
					//if (0 == strcmp(exp_encry,""))
					//{
					//strcpy(exp_encry,encry);
					//}
					//fprintf(cgiOut,"<p><a href=wp_expconf.cgi?UN=%s&FPATH=%s target=mainFrame class=top><font color=blue size=2>%s</font></a></p>",exp_encry,path_conf,search(lsystem,"export_config"));/*导出配置文件的连接*/

					fclose(fp1);
					}
                    fprintf(cgiOut,"<p></p>");
					fprintf(cgiOut,"<table width=350 border=0 cellspacing=0 cellpadding=0>");
		  
				 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=UN value=%s></td></tr>",encry);
				   fprintf(cgiOut,"</table>"\
					 "</td>"\
					 "</tr>"\
					 "<tr>"\
					 "<td  id=sec1 style=\"padding-left:23px;width=500; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
					 fprintf(cgiOut,"</tr>");
					 
					
						 fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
						   "<td style=font-size:14px;color:#FF0000>%s</td>"\
						 "</tr>",search(lpublic,"export_des"));
					 
					 
					   
   fprintf(cgiOut,"</table>"\
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
free(encry); 
free(path_conf);
free(fpath);
return 0;
}  





