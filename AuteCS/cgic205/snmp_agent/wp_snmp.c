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
* wp_snmpd_summary.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"


void ShowSnmpPage(char *m,struct list *lpublic,struct list *lsnmpd);

int cgiMain()
{
	char encry[BUF_LEN] = { 0 };
	char *str = NULL;
	struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
	struct list *lsnmpd = NULL;     /*解析snmpd.txt文件的链表头*/  
	lpublic = get_chain_head("../htdocs/text/public.txt");
	lsnmpd= get_chain_head("../htdocs/text/snmpd.txt");

    ccgi_dbus_init();
	memset(encry,0,sizeof(encry));
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
  	if(str==NULL)
    	ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
	else
		ShowSnmpPage(encry,lpublic,lsnmpd);
	release(lpublic);
	release(lsnmpd); 
	destroy_ccgi_dbus();
	return 0;
}

void ShowSnmpPage(char *m,struct list *lpublic,struct list *lsnmpd)
{   
  int i = 0;
  int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/
  int limit = 0;
  int slot_num = 0;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  unsigned int snmp_state = 0;  
  int ret1 = AC_MANAGE_DBUS_ERROR;
  int ret2 = AC_MANAGE_DBUS_ERROR;
  unsigned int trap_state = 0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SNMP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"SNMP");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
     
			 fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));	
		
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
         			"<td align=left id=tdleft><a href=wp_snmp_summary.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>SNMP</font></a></td>",m);
         			fprintf(cgiOut,"</tr>");
					
					fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_trap_summary.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>TRAP</font></a></td>",m);
         			fprintf(cgiOut,"</tr>"); 
					
					fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_view_list.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>SNMP V3</font></a></td>",m);
         			fprintf(cgiOut,"</tr>"); 	
					if(snmp_cllection_mode(ccgi_dbus_connection))
					{
						cllection_mode = 1;
					}
					else
					{
						cllection_mode = 0;
					}

					list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
					for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
					{
						slot_num++;
					}
					if(slot_num>1)
					{
						limit=slot_num-1;
					}
					if(1 == cllection_mode)/*开启分板采集*/
					{
						limit+=slot_num;
						limit--;
					}
					for(i=0;i<limit;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}			
			fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
			"<table frame=below rules=rows width=200 border=1>");
				if(1 == cllection_mode)/*开启分板采集*/
				{
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1 width=150>SNMP %s</td>",search(lpublic,"decentralized_collect"));
					  fprintf(cgiOut,"<td id=td2 width=50>%s</td>",search(lpublic,"log_start_n"));
					fprintf(cgiOut,"</tr>");
					for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
					{
						ret1 = ac_manage_show_snmp_state(paraNode->connection, &snmp_state);
						fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>Slot%d SNMP %s</td>",paraNode->parameter.slot_id,search(lpublic,"l_state"));
						  if((AC_MANAGE_SUCCESS == ret1)&&(1 == snmp_state))
						  	fprintf(cgiOut,"<td id=td2>%s</td>","start");
						  else
						  	fprintf(cgiOut,"<td id=td2>%s</td>","stop");
						fprintf(cgiOut,"</tr>");
					}
				}
				else				 /*关闭分板采集*/
				{
					ac_manage_show_snmp_state(ccgi_dbus_connection, &snmp_state);
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1 width=150>SNMP %s</td>",search(lpublic,"decentralized_collect"));
					  fprintf(cgiOut,"<td id=td2 width=50>%s</td>",search(lpublic,"stop"));
					fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>SNMP %s</td>",search(lpublic,"l_state"));
					  if(1 == snmp_state)
					  	fprintf(cgiOut,"<td id=td2>%s</td>","start");
					  else
					  	fprintf(cgiOut,"<td id=td2>%s</td>","stop");
					fprintf(cgiOut,"</tr>");					
				}
				for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
				{
					trap_state = 0;
	                ret2 = ac_manage_show_trap_state(paraNode->connection, &trap_state);					
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>Slot%d TRAP %s</td>",paraNode->parameter.slot_id,search(lpublic,"l_state"));
					  if((AC_MANAGE_SUCCESS == ret2)&&(1 == trap_state))
					    fprintf(cgiOut,"<td id=td2>%s</td>","start");
					  else
					    fprintf(cgiOut,"<td id=td2>%s</td>","stop");
					fprintf(cgiOut,"</tr>");
				}
			  fprintf(cgiOut,"</table>");
fprintf(cgiOut,"</td>"\
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


