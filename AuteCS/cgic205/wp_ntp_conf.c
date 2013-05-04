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
* wp_ntp_conf.c
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
	 
	 char *encry=(char *)malloc(BUF_LEN);				

	 char selename[N];    /*下拉框内容*/
     memset(selename,0,N); 
	
     FILE *pp;
	 char buff[128];
	 memset(buff,0,128);
   
	 int j;

     int cl=1;
   ///////////////////////////
	  

  char cmd[128];  //要执行的命令
  memset(cmd,0,128);  
  ///////////////////////////
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  /***********************2008.5.26*********************/
	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"ntp_s"));
	 fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
   	  "<style type=text/css>"\
   	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
 	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
 	  "#link{ text-decoration:none; font-size: 12px}"\
 	  ".usrlis {overflow-x:hidden;	overflow:auto; width: 780px; height: 300px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
 	  "</style>"\
		"<script type='text/javascript'>"\
		"function changestate(){"\
		"var a1 = document.getElementsByName('showtype')[0];"\
		"var a2 = document.getElementsByName('showtype')[1];"\
		"var a3 = document.getElementsByName('showtype')[2];"\
		"}"\
		"</script>"\
		"</head>"\
		"<body>");
	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  //1111111111111111
	  fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  //2222222222222
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=log_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
        fprintf(cgiOut,"<td width=62 align=left><a href=wp_ntp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		
     	
		fprintf(cgiOut,"</tr>"\
		"</table>");  //22222222222
	 
	 
	  
	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>"); //333333333333333
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); //4444444444
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 border=0 cellspacing=0 cellpadding=0>"); //55555555555555							

			  fprintf(cgiOut,"<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>"); 
                 
					for(j=0;j<11;j++)
					{
					    fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
					  "</tr>");
					}

                 
				       fprintf(cgiOut,"</table>"); //555555555555555555
				        fprintf(cgiOut,"</td>"\
				        "<td  align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");

////////////////////// 点击查看的内容

		   fprintf(cgiOut,"<div class=usrlis><table frame=below rules=rows width=700 border=0 cellspacing=0 cellpadding=0>"\
		   "<tr height=30 bgcolor=#eaeff9 style=font-size:16px>"\
			"</tr>");  //8888888888888888888
	 
		  fprintf(cgiOut,"<tr height=10><td></td></tr>");	
 		 
   pp=popen("cat /etc/ntp.conf |sed '/#/d' ","r");  
					  if(pp==NULL)
					  fprintf(cgiOut,"error open the pipe");
				  else
					  {
				  fgets( buff, sizeof(buff), pp );	//很重要 ，不然与条目不匹配 						 
		  do
		  {										   
			  fprintf(cgiOut,"<tr bgcolor=%s><td>",setclour(cl));		
		      fprintf(cgiOut,"<br>");   					     
			  fprintf(cgiOut,"%s",buff); 
			  fprintf(cgiOut,"</td></tr>");
			  fgets( buff, sizeof(buff), pp ); 	

			  cl = !cl;
			   }while( !feof(pp) ); 					   
			  pclose(pp);
							  }

	  
	 fprintf(cgiOut,"</table></div>");	//88888888888888888888888888

////////////////点击查看的内容


			  fprintf(cgiOut,"</td>"\
			  "</tr>"\
			  "<tr height=4 valign=top>"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
			  "</tr>"\
			"</table>");//444444444444
		  fprintf(cgiOut,"</td>"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
		"</tr>"\
	  "</table></td>"); //333333333333
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
	  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
	"</tr>"\
	"</table>");//111111111111111
	fprintf(cgiOut,"</div>"\
	"</form>"\
	"</body>"\
	"</html>");
free(encry); 
return 0;
}  


