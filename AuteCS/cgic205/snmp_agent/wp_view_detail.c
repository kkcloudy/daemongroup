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
* wp_view_detail.c
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
#include <dbus/dbus.h>
#include "ws_snmpd_engine.h"
#include "ac_manage_interface.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"


void ShowViewDetailPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd);  


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;      
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsnmpd = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsnmpd=get_chain_head("../htdocs/text/snmpd.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowViewDetailPage(encry,str,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowViewDetailPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd)
{    
  int i = 0,retu = 1,limit = 0;     
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  char *endptr = NULL;	
  char ViewName[25] = { 0 };
  int ret = AC_MANAGE_DBUS_ERROR;
  STSNMPView *view_array = NULL;
  unsigned int view_num = 0;
  struct oid_list *oid_node = NULL;

  memset(select_slotid,0,sizeof(select_slotid));
  cgiFormStringNoNewlines( "SLOT_ID", select_slotid, 10 );
  memset(ViewName,0,sizeof(ViewName));
  cgiFormStringNoNewlines("ViewName", ViewName, 25);
  list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
  if(strcmp(select_slotid,"")==0)
  { 
	if(paraHead)
	{
		snprintf(select_slotid,sizeof(select_slotid)-1,"%d",paraHead->parameter.slot_id);
	} 
  }  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SNMP V3</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script src=/slotid_onchange.js>"\
  "</script>"\
  "<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>SNMP V3</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
     	  "<td width=62 align=center><a href=wp_view_list.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_view_list.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
				  fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsnmpd,"view_detail"));   /*突出显示*/
				  fprintf(cgiOut,"</tr>");	
				  retu=checkuser_group(n);		  
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_view")); 					  
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_group_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"group_list"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_group_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_group"));                       
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_v3user_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"v3user_list"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_v3user_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_v3user"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  
				  slot_id=strtoul(select_slotid,&endptr,10);
				  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
				  if(ViewName)
				  {
					  ret = ac_manage_show_snmp_view(select_connection, &view_array, &view_num, ViewName);
				  }
				  
				  limit = 0;				  
				  if((AC_MANAGE_SUCCESS == ret)&&(view_array))
				  {
				  	if((view_array[0].view_included.oid_num + view_array[0].view_excluded.oid_num) > 3)
				  	{
				  		limit = view_array[0].view_included.oid_num + view_array[0].view_excluded.oid_num - 3;
				  	}
				  }
				  
				  if(retu == 1)  /*普通用户*/
				  	limit+=3;
				  
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
 "<table width=350 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr style=\"padding-bottom:15px\">"\
	"<td width=70><b>SLOT ID:</b></td>"\
	  "<td width=280>"\
		  "<select name=slot_id id=slot_id style=width:45px onchange=slotid_change(this,\"wp_view_detail.cgi\",\"%s\")>",m);
		  for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
		  {
			 memset(temp,0,sizeof(temp));
			 snprintf(temp,sizeof(temp)-1,"%d",paraNode->parameter.slot_id);
  
			 if(strcmp(select_slotid,temp) == 0)
			   fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
			 else
			   fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
		  } 		  
		  fprintf(cgiOut,"</select>"\
	  "</td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=2 valign=top align=center style=\"padding-bottom:5px\">");
		fprintf(cgiOut,"<table frame=below rules=rows width=350 border=1>");
		if((AC_MANAGE_SUCCESS == ret)&&(view_array))
		{
		   fprintf(cgiOut,"<tr align=left>"\
		     "<td id=td1 width=110>%s</td>",search(lsnmpd,"view_name"));
		  	 fprintf(cgiOut,"<td id=td2 width=240>%s</td>",ViewName);
		   fprintf(cgiOut,"</tr>");
		   fprintf(cgiOut,"<tr align=left>"\
			 "<td id=td1>%s</td>",search(lsnmpd,"view_included_num"));
			 fprintf(cgiOut,"<td id=td2>%d</td>", view_array[0].view_included.oid_num);
	       fprintf(cgiOut,"</tr>");
		   for(i = 0, oid_node = view_array[0].view_included.oidHead;
			   i < view_array[0].view_included.oid_num && NULL != oid_node;
			   i++, oid_node = oid_node->next)
		   {
			   fprintf(cgiOut,"<tr align=left>");
			   	 if(0 == i)
				 	fprintf(cgiOut,"<td id=td1>%s</td>",search(lsnmpd,"view_included_oid"));
				 else
				 	fprintf(cgiOut,"<td id=td1>&nbsp;</td>");
				 fprintf(cgiOut,"<td id=td2>%s</td>", oid_node->oid);
			   fprintf(cgiOut,"</tr>");
		   }
		   fprintf(cgiOut,"<tr align=left>"\
			 "<td id=td1>%s</td>",search(lsnmpd,"view_excluded_num"));
			 fprintf(cgiOut,"<td id=td2>%d</td>", view_array[0].view_excluded.oid_num);
	       fprintf(cgiOut,"</tr>");
		   for(i = 0, oid_node = view_array[0].view_excluded.oidHead;
			   i < view_array[0].view_excluded.oid_num && NULL != oid_node;
			   i++, oid_node = oid_node->next)
		   {
			   fprintf(cgiOut,"<tr align=left>");
			   	 if(0 == i)
				 	fprintf(cgiOut,"<td id=td1>%s</td>",search(lsnmpd,"view_excluded_oid"));
				 else
				 	fprintf(cgiOut,"<td id=td1>&nbsp;</td>");
				 fprintf(cgiOut,"<td id=td2>%s</td>", oid_node->oid);
			   fprintf(cgiOut,"</tr>");
		   }
		   free_ac_manage_show_snmp_view(&view_array, view_num);
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
free_instance_parameter_list(&paraHead);
}


