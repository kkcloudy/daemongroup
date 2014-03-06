/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_add_facl.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
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
#include "ws_init_dbus.h"
#include "facl/facl_db.h"
#include "facl/facl_errcode.h"

int ShowAddFaclPage(char *m,struct list *lpublic,struct list *lfirewall);    
void AddFacl(struct list *lpublic,struct list *lfirewall);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };			
  char *str = NULL;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lfirewall = NULL;     /*解析firewall.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lfirewall=get_chain_head("../htdocs/text/firewall.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {
    cgiFormStringNoNewlines("encry_addfacl",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
    ShowAddFaclPage(encry,lpublic,lfirewall);
  release(lpublic);  
  release(lfirewall);
  destroy_ccgi_dbus();
  return 0;
}

int ShowAddFaclPage(char *m,struct list *lpublic,struct list *lfirewall)
{  
  char IsSubmit[5] = { 0 };
  int i = 0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>AddFACL</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");  
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("addfacl_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	AddFacl(lpublic,lfirewall);
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>FACL</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=addfacl_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
     	  fprintf(cgiOut,"<td width=62 align=center><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                  "</tr>"\
  				    "<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_facl_list.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FACL</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                    fprintf(cgiOut,"</tr>"\
  				    "<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lfirewall,"add_facl"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_add_faclrule.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lfirewall,"tc_addrule"));					   
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
              "<table width=500 border=0 cellspacing=0 cellpadding=0>"\
  "<tr height=30>"\
    "<td width=40>Index:</td>");
    fprintf(cgiOut,"<td width=260 align=left><input type=text name=facl_index maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" size=40></td>");
    fprintf(cgiOut,"<td width=200><font color=red>(1--2048)</font></td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lpublic,"name"));
    fprintf(cgiOut,"<td align=left><input type=text name=facl_name size=40 maxLength=256 onkeypress=\"return event.keyCode!=32\"></td>"\
    "<td align=left><font color=red>(%s)</font></td>",search(lfirewall,"most_256_char"));
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
    "<td><input type=hidden name=encry_addfacl value=%s></td>",m);
    fprintf(cgiOut,"<td colspan=2><input type=hidden name=SubmitFlag value=%d></td>",1);
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
return 0;
}

void AddFacl(struct list *lpublic,struct list *lfirewall)
{
  int id = 0,ret = 0;  
  char *endptr = NULL;  
  char facl_index[10] = { 0 };
  char facl_name[257] = { 0 };

  memset(facl_index,0,sizeof(facl_index));
  cgiFormStringNoNewlines("facl_index",facl_index,10);   
  memset(facl_name,0,sizeof(facl_name));
  cgiFormStringNoNewlines("facl_name",facl_name,257);  
  if(strcmp(facl_index,"")!=0) /*facl_index不能为空*/
  {
	  id= strtoul(facl_index,&endptr,10);
	  if((id>0)&&(id<2049))
	  {
		  if(strcmp(facl_name,"")!=0)			 /*facl name不能为空*/
		  {
			if(strchr(facl_name,' ')==NULL) 	/*不包含空格*/
			{
				ret=facl_interface_create_policy(ccgi_dbus_connection, facl_name, id);
				if (FACL_RETURN_OK == ret) 
				{
					ShowAlert(search(lfirewall,"add_success"));
				}
				else if (FACL_TAG_VALUE_ERR == ret)
				{
					ShowAlert(search(lfirewall,"facl_index_illegal"));
				} 
				else if (FACL_NAME_LEN_ERR == ret)
				{
					ShowAlert(search(lpublic,"input_too_long"));
				} 
				else if (FACL_POLICY_TAG_ALREADY_EXIST == ret) 
				{
					ShowAlert(search(lfirewall,"facl_exist"));
				} 
				else if (FACL_POLICY_NAME_ALREADY_EXIST == ret) 
				{
					ShowAlert(search(lfirewall,"facl_exist"));
				} 
				else if (FACL_TOTAL_RULE_NUM_OVER == ret) 
				{
					ShowAlert(search(lfirewall,"facl_rule_outsize"));
				} 
				else 
				{
					ShowAlert(search(lpublic,"error"));
				}
			}
			else
			{
				ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
			}
		  }
		  else
		  {
			  ShowAlert(search(lfirewall,"facl_name_not_null"));
		  }
	  }
	  else
	  {
		ShowAlert(search(lfirewall,"facl_index_illegal"));
	  }
  }
  else
  {
	  ShowAlert(search(lfirewall,"facl_index_not_null"));  
  }
}

