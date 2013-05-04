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
* wp_add_access.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for add access
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_err.h"
#include <sys/wait.h>


int ShowAddAccessPage(); 
int addaccess_hand(char * prefix, char * opt, char * target, struct list * lcontrol); 

int cgiMain()
{
 ShowAddAccessPage();
 return 0;
}

int ShowAddAccessPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	FILE *fp;
	char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char add_encry[BUF_LEN];  
	char * access_prefix = (char *)malloc(25);
	char * access_opt = (char *)malloc(10);
	char * access_target = (char *)malloc(30);
	char * access_ipaddr_text = (char *)malloc(30);
	memset(access_prefix, 0, 25);
	memset(access_opt, 0, 10);
	memset(access_target, 0, 30);
	memset(access_ipaddr_text, 0, 30);
	if(cgiFormSubmitClicked("submit_addaccess") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(add_encry,0,BUF_LEN);					 /*清空临时变量

*/
	}

  cgiFormStringNoNewlines("encry_addaccess",add_encry,BUF_LEN);
  cgiFormStringNoNewlines("access_prefix",access_prefix,25);
  cgiFormStringNoNewlines("access_opt",access_opt,10);
  cgiFormStringNoNewlines("access_ipaddr",access_target,30);
  cgiFormStringNoNewlines("access_ipaddr_text",access_ipaddr_text,30);
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script languge=javascript>"\
  "function select_output()"\
  "{"\
  		"if( document.all.form_add_access.access_ipaddr.value == 'ip_addr' )"\
  			"{"\
  				"document.getElementById('access_ip_tr').style.display = '';"\
  			"}"\
		"else"\
			"{"\
				"document.getElementById('access_ip_tr').style.display = 'none';"\
			"}"\
  	"}"\
  "</script>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_addaccess") == cgiFormSuccess)
  {
  	if( !strcmp(access_target,"any") )
    	addaccess_hand(access_prefix, access_opt, access_target, lcontrol);
	else
	{
		addaccess_hand(access_prefix, access_opt, access_ipaddr_text, lcontrol);
	}
  }

  fprintf(cgiOut,"<form name=form_add_access method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lcontrol,"route_manage"));
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
	
	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td width=62 align=center><input id=but type=submit name=submit_addaccess style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
	  if(cgiFormSubmitClicked("submit_addaccess") != cgiFormSuccess)
        fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	  else                                         
 		fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",add_encry,search(lpublic,"img_cancel"));
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
            		if(cgiFormSubmitClicked("submit_addaccess") != cgiFormSuccess)
            		{
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_show_access.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"show_access"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_access"));   /*突出显示*/
         				fprintf(cgiOut,"</tr>");
            		}
            		else if(cgiFormSubmitClicked("submit_addaccess") == cgiFormSuccess)				
            		{
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_show_access.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lcontrol,"show_access"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_access"));   /*突出显示*/
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
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  "<table width=360 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>"\
							 "<tr height=30>"\
								"<td align=left id=tdprompt style=font-size:12px>%s: </td>","ACCESS LIST Prefix");
								fprintf(cgiOut,"<td><input type=text name=access_prefix size=21 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
								"<td style=color:red>(1-99)</td>"\
							  "</tr>"\
							  "<tr height=30>"\
								"<td align=left id=tdprompt style=font-size:12px>%s: </td>",search(lcontrol,"operation"));
								fprintf(cgiOut,"<td colspan=2><select name=access_opt>"\
									"<option value=permit>permit"\
									"<option value=deny>deny");
									#if 0
									fprintf(cgiOut,"<option value=remark>remark");
									#endif
									fprintf(cgiOut,"</select></td>"\
							  "</tr>"\
							  "<tr height=30>"\
								"<td align=left id=tdprompt style=font-size:12px>%s: </td>",search(lcontrol,"target_value"));
								fprintf(cgiOut,"<td colspan=2><select name=access_ipaddr onChange=select_output()>"\
									"<option value=ip_addr>ip addr"\
									"<option value=any>any");
									
									
									fprintf(cgiOut,"</select></td>"\
							  "</tr>"\
							  "<tr height=30 id=access_ip_tr>"\
								"<td align=left id=tdprompt style=font-size:12px>%s: </td>",search(lcontrol,"ip_addr"));
								fprintf(cgiOut,"<td><input type=text name=access_ipaddr_text size=21></td>"\
							  "</tr>"\
							
							  "<tr>");
							  if(cgiFormSubmitClicked("submit_addaccess") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td><input type=hidden name=encry_addaccess value=%s></td>",encry);

							  }
							  else if(cgiFormSubmitClicked("submit_addaccess") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addaccess value=%s></td>",add_encry);
							  }
							  fprintf(cgiOut,"</tr>"\
							"</table>"\
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
free(access_prefix);
free(access_opt);
free(access_target);
free(access_ipaddr_text);
release(lpublic);  
release(lcontrol);
return 0;
}

															 
int addaccess_hand(char * prefix, char * opt, char * target, struct list * lcontrol)
{
	char * endptr = NULL;
	int prefix_temp =strtoul(prefix, &endptr, 10);
	if( prefix_temp < 1 || prefix_temp > 100 )
	{
		ShowAlert(search(lcontrol,"illegal_input"));
		return -1;
	}
	
	char cmdstr[300];
	memset (cmdstr, 0, 300);
	//FILE * fd = NULL;
	sprintf(cmdstr, "add_access_list.sh %s %s %s >/dev/null 2>&1", prefix, opt, target);
	int status = system(cmdstr);
	int ret = WEXITSTATUS(status);

	if(0 != ret)
		ShowAlert(search(lcontrol,"add_access_fail"));
	else
		ShowAlert(search(lcontrol,"add_access_suc"));
	
	return 0;
}

