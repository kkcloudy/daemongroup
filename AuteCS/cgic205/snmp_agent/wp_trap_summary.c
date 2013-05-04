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
* wp_trap_summary.c
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


void ShowTrapSummaryPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd);  


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
	ShowTrapSummaryPage(encry,str,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowTrapSummaryPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd)
{    
  int i = 0,retu = 1,limit = 0;     
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  char *endptr = NULL;	
  int ret1 = AC_MANAGE_DBUS_ERROR;
  unsigned int trap_state = 0;
  TRAPParameter *parameter_array = NULL;
  unsigned int parameter_num = 0;
  unsigned int ret2 = AC_MANAGE_DBUS_ERROR;

  memset(select_slotid,0,sizeof(select_slotid));
  cgiFormStringNoNewlines( "SLOT_ID", select_slotid, 10 );
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
  fprintf(cgiOut,"<title>TRAP</title>");
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
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>TRAP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
     	  "<td width=62 align=center><a href=wp_snmp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_snmp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>TRAP</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"info"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");				  
				  retu=checkuser_group(n);		  
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_trap_config.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> TRAP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config"));
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_list.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_trap_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s </font><font id=yingwen_san>TRAP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"ntp_add"));
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_detail.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"switch"));
                  fprintf(cgiOut,"</tr>");
				  
				  slot_id=strtoul(select_slotid,&endptr,10);
				  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
				  ret1 = ac_manage_show_trap_state(select_connection, &trap_state);
				  ret2 = ac_manage_show_trap_parameter(select_connection, &parameter_array, &parameter_num);
				  
				  limit = 0;
				  
				  if(retu == 1)  /*普通用户*/
				  	limit+=2;
				  
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
		  "<select name=slot_id id=slot_id style=width:45px onchange=slotid_change(this,\"wp_trap_summary.cgi\",\"%s\")>",m);
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
		if(AC_MANAGE_SUCCESS == ret1)
		{
		   fprintf(cgiOut,"<tr align=left>"\
		     "<td id=td1 width=170>%s</td>",search(lsnmpd,"trap_state"));
		  	 fprintf(cgiOut,"<td id=td2 width=180>%s</td>",trap_state ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>");
		}
		if((AC_MANAGE_SUCCESS == ret2)&&(parameter_array))
		{
		   fprintf(cgiOut,"<tr align=left>"\
			 "<td id=td1>%s</td>",search(lsnmpd,"heart_time"));
			 fprintf(cgiOut,"<td id=td2>%d</td>", parameter_array[0].data);
	       fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
			 "<td id=td1>%s</td>",search(lsnmpd,"heart_mode"));
			 fprintf(cgiOut,"<td id=td2>%d</td>", parameter_array[1].data);
	       fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
			 "<td id=td1>%s</td>",search(lsnmpd,"resend_interval"));
			 fprintf(cgiOut,"<td id=td2>%d</td>", parameter_array[2].data);
	       fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
			 "<td id=td1>%s</td>",search(lsnmpd,"resend_times"));
			 fprintf(cgiOut,"<td id=td2>%d</td>", parameter_array[3].data);
	       fprintf(cgiOut,"</tr>");
		   MANAGE_FREE(parameter_array);
	    }
	    else if(AC_MANAGE_DBUS_ERROR == ret2)
	    {
	  	  fprintf(cgiOut,"<tr>"\
    	    "<td colspan=2>%s</td>",search(lpublic,"contact_adm"));
		  fprintf(cgiOut,"</tr>");
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

