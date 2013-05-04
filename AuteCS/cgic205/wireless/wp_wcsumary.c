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
* wp_wcsumary.c
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
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


void ShowStationPage(char *m,char* n,struct list *lpublic,struct list *lwcontrol);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 }; 
  char *str = NULL;        
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwcontrol = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwcontrol=get_chain_head("../htdocs/text/wcontrol.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {
   
      ShowStationPage(encry,str,lpublic,lwcontrol);
  }
  release(lpublic);  
  release(lwcontrol);
  destroy_ccgi_dbus();
  return 0;
}

void ShowStationPage(char *m,char* n,struct list *lpublic,struct list *lwcontrol)
{   
  int i = 0,retu = 1,limit = 0;
  DCLI_AC_API_GROUP_FIVE *interval = NULL;
  int result1 = 0,result2 = 0;
  DCLI_AC_API_GROUP_FIVE *wirelessconfig = NULL;
  wireless_config *head = NULL;
  char caauth_security[2][20] = {"CW_PRESHARED","CW_X509_CERTIFICATE"};
  char calev3_protocol[2][8] = {"CW_IPv6","CW_IPv4"};
  char clog_swith[2][4] = {"OFF","ON"};
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Station</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
	  "<script type=\"text/javascript\">"\
	   "function popMenu(objId)"\
	   "{"\
		  "var obj = document.getElementById(objId);"\
		  "if (obj.style.display == 'none')"\
		  "{"\
			"obj.style.display = 'block';"\
		  "}"\
		  "else"\
		  "{"\
			"obj.style.display = 'none';"\
		  "}"\
	  "}"\
	  "</script>"\
	  "<script src=/instanceid_onchange.js>"\
  	  "</script>"\
  "<body>");
  memset(select_insid,0,sizeof(select_insid));
  cgiFormStringNoNewlines( "INSTANCE_ID", select_insid, 10 );
  if(strcmp(select_insid,"")==0)
  { 
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB); 
	if(paraHead1)
	{
		snprintf(select_insid,sizeof(select_insid)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id);
	}
  }  
  else
  {
	get_slotID_localID_instanceID(select_insid,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
  }
	fprintf(cgiOut,"<form>"\
		"<div align=center>"\
		"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
        "<tr>"\
          "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
          "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		  "<td width=300 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwcontrol,"adv_conf"));
		  fprintf(cgiOut,"<td width=590 align=right valign=bottom background=/images/di22.jpg>");  
				fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
				"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
				fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                   "<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwcontrol,"wc_info"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
					retu=checkuser_group(n);
					if(retu == 0)
        			{
                		fprintf(cgiOut,"<tr height=25>"\
  					      "<td align=left id=tdleft><a href=wp_wcwtp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"wc_config"));                       
   						fprintf(cgiOut,"</tr>");
				 	}
				   fprintf(cgiOut,"<tr height=25>"\
				      "<td align=left id=tdleft><a href=wp_apsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"ap_info"));                       
				   fprintf(cgiOut,"</tr>");
				   if(retu == 0)
				   {
					   fprintf(cgiOut,"<tr height=25>"\
						 "<td align=left id=tdleft><a href=wp_apcon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"ap_config")); 					
					   fprintf(cgiOut,"</tr>");
				   }
		           fprintf(cgiOut,"<tr height=25>"\
				      "<td align=left id=tdleft><a href=wp_rrmsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"rrm_info"));                       
				   fprintf(cgiOut,"</tr>");
				   if(retu == 0)
				   {
					   fprintf(cgiOut,"<tr height=25>"\
						 "<td align=left id=tdleft><a href=wp_rrmcon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"rrm_config")); 					
					   fprintf(cgiOut,"</tr>");
				   }
				   fprintf(cgiOut,"<tr height=25>"\
				      "<td align=left id=tdleft><a href=wp_wcapill.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"illegal"),search(lpublic,"menu_san"),search(lwcontrol,"list"));                       
				   fprintf(cgiOut,"</tr>");
				   		if(paraHead1)
						{
							result1=show_wc_config(paraHead1->parameter,paraHead1->connection,&wirelessconfig);
							result2=show_neighbordead_interval_cmd_func(paraHead1->parameter,paraHead1->connection,&interval);
						}
						
					  	limit=3;
						if(retu==1)     /*普通用户*/
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
 "<table width=760 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
 "<tr style=\"padding-bottom:15px\">"\
	"<td width=70>%s ID:</td>",search(lpublic,"instance"));
	fprintf(cgiOut,"<td width=693>"\
		"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wcsumary.cgi\",\"%s\")>",m);
		list_instance_parameter(&paraHead2, INSTANCE_STATE_WEB);    
		for(pq=paraHead2;(NULL != pq);pq=pq->next)
		{
		   memset(temp,0,sizeof(temp));
		   snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
		
		   if(strcmp(select_insid,temp) == 0)
			 fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
		   else
			 fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
		}			
		free_instance_parameter_list(&paraHead2);
		fprintf(cgiOut,"</select>"\
	"</td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=2>");
		if(result1 == 1)
		{
			if((wirelessconfig != NULL)&&(wirelessconfig->wireless_control != NULL))
			{
				head = wirelessconfig->wireless_control;			
				fprintf(cgiOut,"<table width=470 border=0 cellspacing=0 cellpadding=0>"\
				 "<tr>"\
				   "<td align=left style=\"padding-left:20px\">");
						fprintf(cgiOut,"<table frame=below rules=rows width=350 border=1>");
						  /*fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s</td>",search(lwcontrol,"hwversion"));
							fprintf(cgiOut,"<td id=td2>%d</td>",head.hw_version);
						 fprintf(cgiOut,"</tr>"\
						   "<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"swversion"));
					   fprintf(cgiOut,"<td id=td2>%d</td>",head.sw_version);
						 fprintf(cgiOut,"</tr>"\*/
						   fprintf(cgiOut,"<tr align=left>"\
							 "<td id=td1 width=170>%s</td>",search(lwcontrol,"auth_security"));
							 fprintf(cgiOut,"<td id=td2 width=180>%s</td>",caauth_security[head->auth_security]);
						 fprintf(cgiOut,"</tr>"\
						   "<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"protocol"));
							 fprintf(cgiOut,"<td id=td2>%s</td>",calev3_protocol[head->uclev3_protocol]);
						 fprintf(cgiOut,"</tr>"\
						   /*"<tr align=left>"\
						   "<td id=td1>%s</td>",search(lwcontrol,"ac_name"));
						   if(head->ac_name)
						   {
							   fprintf(cgiOut,"<td id=td2>%s</td>",head->ac_name);			   
						   }
						 fprintf(cgiOut,"</tr>"\*/
						   "<tr align=left>"\
						   "<td id=td1>%s</td>",search(lwcontrol,"max_sta"));			 
						   fprintf(cgiOut,"<td id=td2>%d</td>",head->sta_count);
						 fprintf(cgiOut,"</tr>"\
						   "<tr align=left>"\
						   "<td id=td1>%s</td>",search(lwcontrol,"max_wtp"));			 
					   fprintf(cgiOut,"<td id=td2>%d</td>",head->max_wtp);
						 fprintf(cgiOut,"</tr>"\
						   "<tr align=left>"\
						   "<td id=td1>%s</td>",search(lwcontrol,"static_wtp"));			 
						   fprintf(cgiOut,"<td id=td2>%d</td>",head->static_wtp);
						 fprintf(cgiOut,"</tr>"\
						   /*"<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"mtu"));
							 fprintf(cgiOut,"<td id=td2>%d</td>",head.force_mtu);
						 fprintf(cgiOut,"</tr>"\*/
						   "<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"log_switch"));
							 fprintf(cgiOut,"<td id=td2>%s</td>",clog_swith[head->log_switch]);
						 fprintf(cgiOut,"</tr>"\
						   "<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"log_size"));
							 fprintf(cgiOut,"<td id=td2>%d</td>",head->log_size);
						 fprintf(cgiOut,"</tr>"\
						 "<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"ap_static_state"));
						     if(head->apstaticstate == 1)
							   fprintf(cgiOut,"<td id=td2>enable</td>");
							 else
							   fprintf(cgiOut,"<td id=td2>disable</td>");
						 fprintf(cgiOut,"</tr>"\
						 /*"<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"trap_level"));
							 fprintf(cgiOut,"<td id=td2>%d</td>",head->trapflag);
						 fprintf(cgiOut,"</tr>"\*/
						 "<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"ac_exten_infor_switch"));
						 	 if(head->g_ac_all_extention_information_switch == 1)
							   fprintf(cgiOut,"<td id=td2>enable</td>");
							 else
							   fprintf(cgiOut,"<td id=td2>disable</td>");
						 fprintf(cgiOut,"</tr>"\
						   "<tr align=left>"\
							 "<td id=td1>%s</td>",search(lwcontrol,"neidead_inter"));
						 	 if(result2 == 1)
							   fprintf(cgiOut,"<td id=td2>%d%s</td>",interval->neighbordead_interval,search(lpublic,"second"));
							 else
							   fprintf(cgiOut,"<td id=td2>0%s</td>",search(lpublic,"second"));
						 fprintf(cgiOut,"</tr>"\
						  "</table></td>"\
						"</tr>"\
					  "</td>"\
				 " </tr>"\
				"</table>");
			}
		}
		else if(result1 == -1)
			fprintf(cgiOut,"%s",search(lpublic,"error"));
		else
			fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
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
if(result1 == 1)
{
	Free_wc_config(wirelessconfig);
}
if(result2 == 1)
{
	Free_neighbordead_interval(interval);
}
free_instance_parameter_list(&paraHead1);
}


