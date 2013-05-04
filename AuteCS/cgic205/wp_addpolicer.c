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
* wp_addpolicer.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for qos config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"

#include "ws_dcli_qos.h"

#define AMOUNT 512
int ShowAddQosPage(); 

int cgiMain()
{
 ShowAddQosPage();
 return 0;
}

int ShowAddQosPage()
{
    int retz;
	ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char addpolicer_encry[BUF_LEN];  
	char * policer_index=(char *)malloc(10);
	char * cir=(char *)malloc(20);
	char * cbs=(char *)malloc(20);
	char * mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);

	
	if(cgiFormSubmitClicked("submit_addpolicer") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(addpolicer_encry,0,BUF_LEN);					 /*清空临时变量*/
	}
	
  cgiFormStringNoNewlines("encry_addvlan",addpolicer_encry,BUF_LEN);
  cgiFormStringNoNewlines("policer_index",policer_index,10);
  cgiFormStringNoNewlines("CIR",cir,20);
  cgiFormStringNoNewlines("CBS",cbs,20);

   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"qos_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_addpolicer") == cgiFormSuccess)
  {
  	if(strcmp(policer_index,"")!=0 && strcmp(cir,"")!=0 && strcmp(cbs,"")!=0)
  	{
  		int temp1=atoi(policer_index);
  		long temp2=atoi(cir);
  		long temp3=atoi(cbs);
  		if(temp1<1 || temp1>255)
  			ShowAlert(search(lcontrol,"illegal_input"));
  		else if(temp2<1 || temp2>100000000)
  			ShowAlert(search(lcontrol,"illegal_input"));
  		else if(temp3<1 || temp3>2000000000)
  			ShowAlert(search(lcontrol,"illegal_input"));
  		else
  		{
			retz=set_policer(policer_index);
			switch(retz)
			{
				case 0:
					ShowAlert(search(lpublic,"oper_succ"));
					break;
				default:
					ShowAlert(search(lpublic,"oper_fail"));
					break;
			}
			retz=set_cir_cbs(cir,cbs,policer_index,lcontrol);
			switch(retz)
			{
				case 0:
					ShowAlert(search(lpublic,"oper_succ"));
					break;
				default:
					ShowAlert(search(lpublic,"oper_fail"));
					break;
			}
		}
	}
	else ShowAlert(search(lcontrol,"blank_input"));
  }

	show_qos_mode(mode);
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_addpolicer style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_addpolicer") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_policer.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_policer.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",addpolicer_encry,search(lpublic,"img_cancel"));
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
            		if(cgiFormSubmitClicked("submit_addpolicer") != cgiFormSuccess)
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_policer.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policer"));					   
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_policer"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");
			  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qoscounter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"counter"));					   
            		  fprintf(cgiOut,"</tr>");
					  
            		}
            		else if(cgiFormSubmitClicked("submit_addpolicer") == cgiFormSuccess)				
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_policer.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",addpolicer_encry,search(lpublic,"menu_san"),search(lcontrol,"policer"));						 
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_policer"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");
			  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qoscounter.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",addpolicer_encry,search(lpublic,"menu_san"),search(lcontrol,"counter"));						 
            		  fprintf(cgiOut,"</tr>");
            		  
            		}
					for(i=0;i<5;i++)
						{
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						}

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=190 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  "<table width=350 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>"\
						  	 "<tr height=25>\n");
			  		fprintf(cgiOut,"<td style=\"font-size:14px\" colspan='3'><font color='red'><b>%s</b></font></td>\n",search(lcontrol,mode));
			  	  fprintf(cgiOut,"</tr>"\
							 "<tr height=25>"\
								"<td align=left id=tdprompt style=font-size:12px>%s:</td>",search(lcontrol,"Policer_Index"));
								fprintf(cgiOut,"<td><input type=text name=policer_index size=16></td>"\
								"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(1-255)</td>"\
							  "</tr>"\
							  "<tr height=25>"\
								"<td align=left id=tdprompt style=font-size:12px>%s(CIR):</td>",search(lcontrol,"CIR"));
								fprintf(cgiOut,"<td><input type=text name=CIR size=16></td>"\
								"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(1-100000000 kbps)</td>"\
							  "</tr>"\
							  "<tr height=25>"\
								"<td align=left id=tdprompt style=font-size:12px>%s(CBS):</td>",search(lcontrol,"CBS"));
								fprintf(cgiOut,"<td><input type=text name=CBS size=16></td>"\
								"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(1-2000000000 byte)</td>"\
							  "</tr>"\
							  "<tr>");
							  if(cgiFormSubmitClicked("submit_addpolicer") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",encry);
							  }
							  else if(cgiFormSubmitClicked("submit_addpolicer") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",addpolicer_encry);
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
free(policer_index);
free(cir);
free(cbs);
free(mode);
release(lpublic);  
release(lcontrol);
return 0;
}

