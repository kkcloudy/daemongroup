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
* wp_snmp_summary.c
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


void ShowSnmpSummaryPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd);  


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
	ShowSnmpSummaryPage(encry,str,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowSnmpSummaryPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd)
{    
  int i = 0,retu = 1,limit = 0;     
  int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/  
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  int ret1 = AC_MANAGE_SUCCESS, ret2 = AC_MANAGE_SUCCESS, ret3 = AC_MANAGE_SUCCESS;
  STSNMPSysInfo snmp_info;
  unsigned int snmp_state = 0;  
  SNMPINTERFACE *interface_array = NULL;
  unsigned int interface_num = 0;  
  unsigned int port = 0;
  int j = 0;
  char *endptr = NULL;	

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
  fprintf(cgiOut,"<title>SNMP</title>");
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
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>SNMP</td>"\
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>SNMP</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"info"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");				  
				  retu=checkuser_group(n);		  
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_snmp_config.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> SNMP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config"));                       
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_community_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"community_list"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_community_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_community"));                       
                    fprintf(cgiOut,"</tr>");
				  }	
				  
				  if(snmp_cllection_mode(ccgi_dbus_connection))
			      {
					cllection_mode = 1;
					limit = 6;
				  }
				  else
				  {
				  	cllection_mode = 0;
				  	limit = 4;
				  }

				  if(1 == cllection_mode)/*开启分板采集*/
				  {
				  	  slot_id=strtoul(select_slotid,&endptr,10);
					  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
					  ret1 = ac_manage_show_snmp_base_info(select_connection, &snmp_info);
					  if(AC_MANAGE_SUCCESS == ret1) 
					  {
						ret3 = ac_manage_show_snmp_pfm_interface(select_connection, &interface_array, &interface_num, &port);
					  }				  
					  if(AC_MANAGE_SUCCESS == ret1 && AC_MANAGE_SUCCESS == ret2) 
					  {
						  limit+=interface_num;/*bind interface*/
					  }
				  }
				  else					/*关闭分板采集*/
				  {
					  ret1 = ac_manage_show_snmp_base_info(ccgi_dbus_connection, &snmp_info);
					  if(AC_MANAGE_SUCCESS == ret1) 
					  {
						ret3 = ac_manage_show_snmp_pfm_interface(ccgi_dbus_connection, &interface_array, &interface_num, &port);
					  }				  
					  if(AC_MANAGE_SUCCESS == ret1 && AC_MANAGE_SUCCESS == ret2) 
					  {
						  limit+=interface_num;/*bind interface*/
					  }
				  }

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
 "<table width=350 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");
  if(1 == cllection_mode)/*开启分板采集*/
  {
	  fprintf(cgiOut,"<tr style=\"padding-bottom:15px\">"\
		"<td width=70><b>SLOT ID:</b></td>"\
		  "<td width=280>"\
			  "<select name=slot_id id=slot_id style=width:45px onchange=slotid_change(this,\"wp_snmp_summary.cgi\",\"%s\")>",m);
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
	  "</tr>");
  }
  fprintf(cgiOut,"<tr>"\
    "<td colspan=2 valign=top align=center style=\"padding-bottom:5px\">");
	if(1 == cllection_mode)/*开启分板采集*/
	{
		slot_id=strtoul(select_slotid,&endptr,10);
		get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
		ret1 = ac_manage_show_snmp_base_info(select_connection, &snmp_info);
		ret2 = ac_manage_show_snmp_state(select_connection, &snmp_state);
		if(AC_MANAGE_SUCCESS == ret1) 
		{
		  ret3 = ac_manage_show_snmp_pfm_interface(select_connection, &interface_array, &interface_num, &port);
		}
	  
		if(AC_MANAGE_SUCCESS == ret1 && AC_MANAGE_SUCCESS == ret2) 
		{
		 fprintf(cgiOut,"<table frame=below rules=rows width=350 border=1>"\
		   "<tr align=left>"\
		     "<td id=td1 width=170>%s</td>",search(lsnmpd,"snmp_state"));
		  	 fprintf(cgiOut,"<td id=td2 width=180>%s</td>",snmp_state ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>system name</td>"\
			 "<td id=td2>%s</td>",snmp_info.sys_name);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>system describtion</td>"\
			 "<td id=td2>%s</td>",snmp_info.sys_description);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>system OID</td>"\
			 "<td id=td2>%s</td>",snmp_info.sys_oid);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>Collection mode</td>"\
			 "<td id=td2>%s</td>",snmp_info.collection_mode ? "decentral" : "concentre");/*decentral表示开启分板采集*/
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>cachetime</td>"\
			 "<td id=td2>%d</td>",snmp_info.cache_time ? snmp_info.cache_time : 300);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>SNMP V1 mode</td>"\
			 "<td id=td2>%s</td>",(snmp_info.v1_status) ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>SNMP V2C mode</td>"\
			 "<td id=td2>%s</td>",(snmp_info.v2c_status) ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>SNMP V3 mode</td>"\
			 "<td id=td2>%s</td>",(snmp_info.v3_status) ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>Bind Port</td>");				     
		     if(0 == interface_num) 
			   fprintf(cgiOut,"<td id=td2>Not Set</td>");
			 else
			   fprintf(cgiOut,"<td id=td2>%d</td>",snmp_info.agent_port ? snmp_info.agent_port : 161);
		   fprintf(cgiOut,"</tr>");
		   for(j = 0; j < interface_num; j++) 
		   {
		   	 if(interface_array)
		   	 {
				 fprintf(cgiOut,"<tr align=left>");
				 if(0 == j)
				 {
					 fprintf(cgiOut,"<td id=td1>Interface</td>"\
					 "<td id=td2>%s</td>",interface_array[j].ifName);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td id=td1>&nbsp;</td>"\
					 "<td id=td2>%s</td>",interface_array[j].ifName);
				 }
				 fprintf(cgiOut,"</tr>");
		   	 }
		   }				   
		 fprintf(cgiOut,"</table>");
	  }
	  else if(AC_MANAGE_DBUS_ERROR == ret1 || AC_MANAGE_DBUS_ERROR == ret2) 
	  	  fprintf(cgiOut,"%s",search(lpublic,"contact_adm")); 
      else if(AC_MANAGE_MALLOC_ERROR == ret1) 
	  	  fprintf(cgiOut,"%s",search(lpublic,"malloc_error")); 
      else 
	  	  fprintf(cgiOut,"%s",search(lsnmpd,"get_base_info_fail")); 
	}
	else		/*关闭分板采集*/
	{
		ret1 = ac_manage_show_snmp_base_info(ccgi_dbus_connection, &snmp_info);
		ret2 = ac_manage_show_snmp_state(ccgi_dbus_connection, &snmp_state);
		if(AC_MANAGE_SUCCESS == ret1) 
		{
		  ret3 = ac_manage_show_snmp_pfm_interface(ccgi_dbus_connection, &interface_array, &interface_num, &port);
		}

		if(AC_MANAGE_SUCCESS == ret1 && AC_MANAGE_SUCCESS == ret2) 
		{
		 fprintf(cgiOut,"<table frame=below rules=rows width=350 border=1>"\
		   "<tr align=left>"\
		     "<td id=td1 width=170>%s</td>",search(lsnmpd,"snmp_state"));
		  	 fprintf(cgiOut,"<td id=td2 width=180>%s</td>",snmp_state ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>%s</td>",search(lpublic,"name"));
			 fprintf(cgiOut,"<td id=td2>%s</td>",snmp_info.sys_name);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>%s</td>",search(lsnmpd,"description"));
			 fprintf(cgiOut,"<td id=td2>%s</td>",snmp_info.sys_description);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>%s OID</td>",search(lpublic,"system"));
			 fprintf(cgiOut,"<td id=td2>%s</td>",snmp_info.sys_oid);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>%s</td>",search(lsnmpd,"collect_mode"));
			 fprintf(cgiOut,"<td id=td2>%s</td>",snmp_info.collection_mode ? "decentral" : "concentre");/*decentral表示开启分板采集*/
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>%s</td>",search(lsnmpd,"cache_time"));
			 fprintf(cgiOut,"<td id=td2>%d</td>",snmp_info.cache_time ? snmp_info.cache_time : 300);
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>SNMP V1 %s</td>",search(lpublic,"mode"));
			 fprintf(cgiOut,"<td id=td2>%s</td>",(snmp_info.v1_status) ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>SNMP V2C %s</td>",search(lpublic,"mode"));
			 fprintf(cgiOut,"<td id=td2>%s</td>",(snmp_info.v2c_status) ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>SNMP V3 %s</td>",search(lpublic,"mode"));
			 fprintf(cgiOut,"<td id=td2>%s</td>",(snmp_info.v3_status) ? "enable" : "disable");
		   fprintf(cgiOut,"</tr>"\
		   "<tr align=left>"\
		     "<td id=td1>%s</td>",search(lsnmpd,"bind_port"));
			 fprintf(cgiOut,"<td id=td2>%d</td>",snmp_info.agent_port ? snmp_info.agent_port : 161);
		   fprintf(cgiOut,"</tr>");
		   for(j = 0; j < interface_num; j++) 
		   {
		     if(interface_array)
		   	 {
				 fprintf(cgiOut,"<tr align=left>");
				 if(0 == j)
				 {
					 fprintf(cgiOut,"<td id=td1>%s</td>",search(lsnmpd,"interface"));
					 fprintf(cgiOut,"<td id=td2>%s</td>",interface_array[j].ifName);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td id=td1>&nbsp;</td>"\
					 "<td id=td2>%s</td>",interface_array[j].ifName);
				 }
				 fprintf(cgiOut,"</tr>");
		   	 }
		   }				   
		 fprintf(cgiOut,"</table>");
	  }
	  else if(AC_MANAGE_DBUS_ERROR == ret1 || AC_MANAGE_DBUS_ERROR == ret2) 
	  	  fprintf(cgiOut,"%s",search(lpublic,"contact_adm")); 
      else if(AC_MANAGE_MALLOC_ERROR == ret1) 
	  	  fprintf(cgiOut,"%s",search(lpublic,"malloc_error")); 
      else 
	  	  fprintf(cgiOut,"%s",search(lsnmpd,"get_base_info_fail")); 
	}
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
free_instance_parameter_list(&paraHead);
}



