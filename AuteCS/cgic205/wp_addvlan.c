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
* wp_addvlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for add vlan 
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"

#include "ws_dcli_vlan.h"
#include "ws_dbus_list_interface.h"

int ShowAddvlanPage(); 
int addvlan_hand(struct list *lpublic); 

int cgiMain()
{
 ShowAddvlanPage();
 return 0;
}

int ShowAddvlanPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	char add_encry[BUF_LEN];  
	if(cgiFormSubmitClicked("submit_addvlan") != cgiFormSuccess)
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

  cgiFormStringNoNewlines("encry_addvlan",add_encry,BUF_LEN);
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_addvlan") == cgiFormSuccess)
  {
    addvlan_hand(lcontrol);
  }

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>VLAN</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td width=62 align=center><input id=but type=submit name=submit_addvlan style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
	  if(cgiFormSubmitClicked("submit_addvlan") != cgiFormSuccess)
        fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	  else                                         
 		fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",add_encry,search(lpublic,"img_cancel"));
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
            		if(cgiFormSubmitClicked("submit_addvlan") != cgiFormSuccess)
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_configvlan.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>VLAN </font><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"config"));					   
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> VLAN</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_show_pvlan.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));		  
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"pvlan_add"));			  
						fprintf(cgiOut,"</tr>");
            		}
            		else if(cgiFormSubmitClicked("submit_addvlan") == cgiFormSuccess)				
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_configvlan.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>VLAN </font><font id=%s>%s<font></a></td>",add_encry,search(lpublic,"menu_san"),search(lcontrol,"config"));						 
            		  fprintf(cgiOut,"</tr>"\
            		  "<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font><font id=yingwen_san> VLAN</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add"));	 /*突出显示*/
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_show_pvlan.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));		  
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",add_encry,search(lpublic,"menu_san"),search(lcontrol,"pvlan_add"));			  
						fprintf(cgiOut,"</tr>");
            		}
					for(i=0;i<1;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=115 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  "<table width=260 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>"\
							 "<tr height=50>"\
								"<td align=left id=tdprompt style=font-size:12px>%s:</td>","VLAN ID");
								fprintf(cgiOut,"<td><input type=text name=V_ID size=21></td>"\
								"<td align=left style=color:#FF0000;font-size:12px;padding-left:2px>(2-4094)</td>"\
							  "</tr>"\
							  "<tr height=50>"\
								"<td align=left id=tdprompt style=font-size:12px>%s:</td>",search(lcontrol,"vlan_name"));
								fprintf(cgiOut,"<td colspan=2><input type=text name=V_name size=21></td>"\
							  "</tr>"\
							  "<tr>");
							  if(cgiFormSubmitClicked("submit_addvlan") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",encry);
							  }
							  else if(cgiFormSubmitClicked("submit_addvlan") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_addvlan value=%s></td>",add_encry);
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
release(lpublic);  
release(lcontrol);
return 0;
}

															 
int addvlan_hand(struct list * lcontrol)
{
															 
	char VID[4] = {0};
	char Vname[100] = {0};
	int retu = 0;
	unsigned short vIDtypeInt;			  /*清空临时变量*/
	cgiFormStringNoNewlines("V_ID",VID,N);
	cgiFormStringNoNewlines("V_name",Vname,100);
	if(0==strcmp(Vname,""))
	{
			ShowAlert(search(lcontrol,"name_no_null"));
			return 0;
	}
	vIDtypeInt=atoi(VID);
	ccgi_dbus_init();
	instance_parameter *paraHead2 = NULL;
	instance_parameter *pq = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
    for(pq=paraHead2;(NULL != pq);pq=pq->next)
	{
		retu = create_vlan(pq->connection,vIDtypeInt,Vname);
	}
	free_instance_parameter_list(&paraHead2);
	switch(retu)
	{
		case -1:
			ShowAlert(search(lcontrol,"illegal_vID"));
		break;
		case -10:
			ShowAlert(search(lcontrol,"change_vlanName"));
		break;
		case -2:
			ShowAlert(search(lcontrol,"illegal_vID"));
		break;
		case -3:
			ShowAlert(search(lcontrol,"name_long"));
		break;
		case -4:
			ShowAlert(search(lcontrol,"name_begin_illegal"));	
		break;
		case -5:
			ShowAlert(search(lcontrol,"name_body_illegal"));
		break;
		case 0:
			ShowAlert(search(lcontrol,"opt_fail"));
		break;
		case 1:
			ShowAlert(search(lcontrol,"create_vlan_success"));
		break;
		case -6:
			ShowAlert(search(lcontrol,"vID_Exist"));
		break;
		case -7:
			ShowAlert(search(lcontrol,"vname_conflict"));
		break;
		case -8:
			ShowAlert(search(lcontrol,"HW_error"));
		break;
		case -9:
			ShowAlert(search(lcontrol,"SW_error"));
		break;
		default:
			ShowAlert(search(lcontrol,"opt_fail"));
		break;
	}
	return 1;
}
