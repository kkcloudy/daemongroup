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
* wp_routert.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for tools route
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
#include "ws_nm_status.h"

#define PATH_LENG 512  

int ShowSystemPerformancePage(struct list *lpublic, struct list *llocal);   /*n代表加密后的字符串*/

int cgiMain()
{
 struct list *lpublic = NULL;
 struct list *llocal = NULL;
 
 ShowSystemPerformancePage(lpublic, llocal);
 return 0;
}

int ShowSystemPerformancePage(struct list *lpublic, struct list *llocal)
{ 
  
     FILE *pp;
	
	 char buff[1024];
	 char address[PATH_LENG];
	 memset(address,0,PATH_LENG); //清空临时变量

	 char ip1[PATH_LENG];
	 memset(ip1,0,PATH_LENG); 
	 char ip2[PATH_LENG];
	 memset(ip2,0,PATH_LENG); 
	 char ip3[PATH_LENG];
	 memset(ip3,0,PATH_LENG); 
	 char ip4[PATH_LENG];
	 memset(ip4,0,PATH_LENG); 
	 
	 char *cmd = (char *)malloc(PATH_LENG); 
	 
	 cgiFormStringNoNewlines("gate_ip1",ip1,PATH_LENG);
	 cgiFormStringNoNewlines("gate_ip2",ip2,PATH_LENG);
	 cgiFormStringNoNewlines("gate_ip3",ip3,PATH_LENG);
	 cgiFormStringNoNewlines("gate_ip4",ip4,PATH_LENG);
	 
	 sprintf(address,"%s.%s.%s.%s ",ip1,ip2,ip3,ip4);
	 memset(cmd,0,PATH_LENG);
	 
	 sprintf(cmd,"sudo route.sh %s",address);

	/*
	 strcat(cmd,"traceroute");	 
	 strcat(cmd," ");
	 strcat(cmd,address);
	 strcat(cmd," ");
	 strcat(cmd,"-m");
	 strcat(cmd," ");
	 strcat(cmd,"4");  
	 */
 
  int i;
  char *encry=malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
  char *str = NULL;        
  
  lpublic = get_chain_head("../htdocs/text/public.txt");
  llocal = get_chain_head("../htdocs/text/performance.txt");
  struct list * lcontrol;
  lcontrol = get_chain_head("../htdocs/text/control.txt");

if(cgiFormSubmitClicked("submit") != cgiFormSuccess){
  memset(encry,0,BUF_LEN);
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  str=dcryption(encry);
  if(str==NULL)
  {
    ShowErrorPage(search(lpublic, "ill_user")); 	       /*用户非法*/
    return 0;
  }
}
 cgiFormStringNoNewlines("UN", encry, BUF_LEN); 

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>System Performance</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script language=javascript src=/ip.js>"\
  "</script>"\
  "<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lpublic,"tool"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
	   
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>");  	
          fprintf(cgiOut,"<tr>");
	      fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=submit style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));

		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
    //其他条幅
	  fprintf(cgiOut,"<tr height=25>"\
		  "<td align=left id=tdleft><a href=wp_command.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(llocal,"ping")); 
	  fprintf(cgiOut,"</tr>");
		   //突出条幅
      fprintf(cgiOut,"<tr height=26>"\
             "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(llocal,"traceroute"));   //突出显示 
      fprintf(cgiOut,"</tr>");

	  fprintf(cgiOut,"<tr height=25>"\
		  "<td align=left id=tdleft><a href=wp_mirror_tool.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"mirror")); 
		  fprintf(cgiOut,"</tr>");

	               for(i=0;i<12;i++) 
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
          "<table width=260 border=0 cellspacing=0 cellpadding=0>"\
	       "<tr>"\
            "<td align=left>");
  fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);  //鉴权
  fprintf(cgiOut,"<table width=500>");  //上部分
  fprintf(cgiOut,"<tr><td>");
  fprintf(cgiOut,"<fieldset align=left>");  
  fprintf(cgiOut, "<legend><font color=Navy>Traceroute %s</font></legend>",search(lpublic,"cmd_select")); 
  fprintf(cgiOut,"<table><tr><td>&nbsp;&nbsp;&nbsp;IP:&nbsp;&nbsp;");  //上表
 // fprintf(cgiOut,"<input type=text name=ip value=%s></td><tr>",address); 

  fprintf(cgiOut,"<td colspan=4 width=140>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
  fprintf(cgiOut,"<input type=text	name=gate_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
  fprintf(cgiOut,"<input type=text	name=gate_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
  fprintf(cgiOut,"<input type=text	name=gate_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
  fprintf(cgiOut,"<input type=text	name=gate_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
  fprintf(cgiOut,"</div></td>");

  
  fprintf(cgiOut,"<tr><td><input type=submit name=do value=%s id=do></td>",search(lpublic,"cmd_exec"));   //关于点击按钮 	
	
  fprintf(cgiOut,"</tr>");	 
  fprintf(cgiOut,"</table>"); //上表
	
  fprintf(cgiOut,"</fieldset>"); //框下边
  fprintf(cgiOut,"</td></tr>");
  fprintf(cgiOut,"</table>"); //上部分 
  fprintf(cgiOut,"<table width=650>");  //下表
  fprintf(cgiOut, "<tr height=30 bgcolor=E7E7E7><td><font color=Navy>%s:</font></td></tr> ",search(lpublic,"cmd_res") ); 
  
	if(cgiFormSubmitClicked("do")==cgiFormSuccess|| cgiFormSubmitClicked("submit")==cgiFormSuccess || (strchr(address,' ')==NULL)){

	 if((strcmp(ip1,"")==0)&&(strcmp(ip2,"")==0)&&(strcmp(ip3,"")==0)&&(strcmp(ip4,"")==0)){
	  	 ShowAlert(search(lpublic,"ip_not_null"));  
	  	}else{	  
      if((strcmp(ip1,"")==0)||(strcmp(ip2,"")==0)||(strcmp(ip3,"")==0)||(strcmp(ip4,"")==0)){
         ShowAlert(search(lpublic,"ip_error"));  
      	}else{
	  //执行命令和读取信息
	  pp=popen(cmd,"r");
			if(pp==NULL)
				fprintf(cgiOut,search(lpublic,"error"));
			else
				{
				fgets( buff, sizeof(buff), pp );
			fprintf(cgiOut,"<tr bgcolor=E7E7E7><td><font color=Navy>");	
	do
	{    
		fprintf( cgiOut, "%s", buff);
	    fprintf(cgiOut,"<br>");	  
		fgets( buff, sizeof(buff), pp );
	}while( !feof(pp) );
	fprintf(cgiOut,"</font></td></tr>");			
	pclose(pp);
	}
      		}
	  		}
			}	
	fprintf(cgiOut,"</table>"); //下 表 

//*****************


fprintf(cgiOut,"</td>"\
		  	  "</tr>"\
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

release(lcontrol);
release(lpublic);
release(llocal);
free(encry);
return 0;
}

