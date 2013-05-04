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
* wp_ftpcli.c
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
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"

int ShowFtpcliPage(struct list *lpublic,struct list *lsys);     /*m代表加密后的字符串*/
void File(struct list *lpublic,struct list *lsys);

int cgiMain()
{
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lsys;      /*解析system.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsys=get_chain_head("../htdocs/text/system.txt");
  ShowFtpcliPage(lpublic,lsys);
  release(lpublic);  
  release(lsys);
  return 0;
}

int ShowFtpcliPage(struct list *lpublic,struct list *lsys)
{ 
  FILE *fp;
  char *encry=(char *)malloc(BUF_LEN);				
  char *str;
  char lan[3];
  int i;
  char ftp_encry[BUF_LEN];  
  if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	       /*用户非法*/
      return 0;
	}
	memset(ftp_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  cgiFormStringNoNewlines("encry_ftpcli",ftp_encry,BUF_LEN);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsys,"ftp_cli"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>"\
  "<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsys,"ftp_cli"));
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
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=upload_file style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",ftp_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		}		
		else			
		{	
		  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=upload_file style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",ftp_encry);
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
                  "</tr>"\
                  "<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\">%s</td>",search(lsys,"ftp_cli"));    /*突出显示*/
                  fprintf(cgiOut,"</tr>");
	              for(i=0;i<1;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
					"<table width=400 height=40 border=0 cellspacing=0 cellpadding=0>"\
					  "<tr>"\
						"<td align=center>"\
						"<p>%s<input type=\"file\" size=\"30\" name=\"file\" value=\"\"></p>",search(lsys,"file_upload"));
						fprintf(cgiOut,"<table width=350 border=0 cellspacing=0 cellpadding=0>");
					   if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)
					   {
						 File(lpublic,lsys);
					   }
					   if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
					   {
						 fprintf(cgiOut,"<tr><td><input type=hidden name=encry_ftpcli value=%s></td></tr>",encry);
					   }
					   else if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)
					   {
						 fprintf(cgiOut,"<tr><td><input type=hidden name=encry_ftpcli value=%s></td></tr>",ftp_encry);
					   }	 
					   fprintf(cgiOut,"</table>"\
							 "</td>"\
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
free(encry); 
return 0;
}          

void File(struct list *lpublic,struct list *lsys)
{
	char name[1024];
	if (cgiFormFileName("file", name, sizeof(name)) != cgiFormSuccess) 
    {
        ShowAlert(search(lpublic,"no_upload"));
		return;
	} 
	fprintf(cgiOut, "<tr><td><p>%s:",search(lsys,"upload"));
	cgiHtmlEscape(name);
	fprintf(cgiOut,"</p></td></tr>");
}


