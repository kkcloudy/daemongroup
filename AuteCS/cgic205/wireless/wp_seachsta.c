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
* wp_seachsta.c
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
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

void ShowSearchStationPage(char *m,char *n,int f,struct list *lpublic,struct list *lwlan);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 }; 
  char staMac[20] = { 0 };
  char *str = NULL;        
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));  
  memset(staMac,0,sizeof(staMac));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
      ShowSearchStationPage(encry,"",0,lpublic,lwlan);
  }
  else
  {    
    cgiFormStringNoNewlines("encry_searbymac",encry,BUF_LEN);
	cgiFormStringNoNewlines("sta_mac",staMac,20);
	str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
      ShowSearchStationPage(encry,staMac,1,lpublic,lwlan);
  }
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowSearchStationPage(char *m,char *n,int f,struct list *lpublic,struct list *lwlan)  /*f==0表示首次进入该页*/
{  
  int i = 0,result = 0;                /*颜色初值为#f9fafe*/
  int limit = 0;
  char select_insid[10] = { 0 };
  struct dcli_sta_info *sta = NULL;
  unsigned char ieee80211_state[20] = { 0 };
  unsigned char PAE[20] = { 0 };
  unsigned char BACKEND[20] = { 0 };
  //time_t now,now_sysrun,statime;
  time_t online_time;
  int hour = 0,min = 0,sec = 0;  
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
		"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"station"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=searbymac_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=center><a href=wp_stasumary.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,select_insid,search(lpublic,"img_cancel"));
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
						  "<td align=left id=tdleft><a href=wp_stalis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"sta_list"));						 
						fprintf(cgiOut,"</tr>"\
						"<tr height=26>"\
						  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"search_sta"));   /*突出显示*/
                        fprintf(cgiOut,"</tr>");
						if((strcmp(n,"")!=0)&&(strchr(n,' ')==NULL)&&(f)) 
						  limit=14;
						else
						  limit=2;
						for(i=0;i<limit;i++)
						{
						  fprintf(cgiOut,"<tr height=25>"\
							"<td id=tdleft>&nbsp;</td>"\
						  "</tr>");
						}
					  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">"\
 "<table width=400 border=0 cellspacing=0 cellpadding=0>"\
   "<tr>"\
     "<td><table width=400 border=0 cellspacing=0 cellpadding=0>"\
     	  "<tr height=30>"\
			"<td>%s ID:</td>",search(lpublic,"instance"));
			fprintf(cgiOut,"<td align=left colspan=2>"\
		    "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_seachsta.cgi\",\"%s\")>",m);
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
          "<tr height=30>"\
            "<td width=80>MAC:</td>"\
            "<td width=70 align=left><input type=text name=sta_mac size=20 maxLength=17 value=%s></td>",n);
			  fprintf(cgiOut,"<td width=250 align=left><font color=red>%s</font></td>",search(lpublic,"mac_format"));
		  fprintf(cgiOut,"</tr>"\
     "</table></td>"\
   "</tr>");

  if((strcmp(n,"")!=0)&&(strchr(n,' ')==NULL))
  {
  	if(paraHead1)
	{
		result=show_sta_bymac(paraHead1->parameter,paraHead1->connection,n,&sta);
	}
	
    fprintf(cgiOut,"<tr style=\"padding-top:20px\">"\
      "<td>");	
			
	        if(result==1)
	        {
				memset(ieee80211_state, 0, sizeof(ieee80211_state));
				memset(PAE, 0, sizeof(PAE));
				memset(BACKEND, 0, sizeof(BACKEND));
		        asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);
		        fprintf(cgiOut,"<table frame=below rules=rows width=320 border=1>"\
                "<tr align=left>"\
			     "<td id=td1 width=170>MAC</td>"\
			     "<td id=td2 width=150>%s</td>",n);
			    fprintf(cgiOut,"</tr>"\
			    "<tr align=left>"\
			     "<td id=td1>WLAN ID</td>"\
			     "<td id=td2>%d</td>",sta->wlan_id);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>AP ID</td>"\
				  "<td id=td2>%d</td>",sta->wtp_id);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>Radio_G_ID</td>"\
				  "<td id=td2>%d</td>",sta->radio_g_id);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>Radio_L_ID</td>"\
				  "<td id=td2>%d</td>",sta->radio_l_id);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>BSS%s</td>",search(lwlan,"index"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",sta->bssindex);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%sID</td>",search(lwlan,"security")); 		  
				  fprintf(cgiOut,"<td id=td2>%d</td>",sta->security_id);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>VLAN ID</td>"\
				  "<td id=td2>%d</td>",sta->vlan_id);
				fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>IEEE80211 %s</td>",search(lwlan,"state"));			  
				  fprintf(cgiOut,"<td id=td2>%s</td>",ieee80211_state);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>PAE_%s</td>",search(lwlan,"state"));			  
				  fprintf(cgiOut,"<td id=td2>%s</td>",PAE);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>Backend_%s</td>",search(lwlan,"state"));			  
				  fprintf(cgiOut,"<td id=td2>%s</td>",BACKEND);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>STA%s</td>",search(lpublic,"uplink_traffic_limit_threshold"));			  
				  fprintf(cgiOut,"<td id=td2>%u kbps</td>",sta->sta_traffic_limit);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>STA%s</td>",search(lpublic,"downlink_traffic_limit_threshold"));			  
				  fprintf(cgiOut,"<td id=td2>%u kbps</td>",sta->sta_send_traffic_limit);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"acc_time"));	
				/*time(&now);
				get_sysruntime(&now_sysrun);
				online_time=now_sysrun-sta->StaTime+sta->sta_online_time;
				statime = now - online_time;*/
				online_time = sta->sta_online_time_new;
				hour=online_time/3600;
				min=(online_time-hour*3600)/60;
				sec=(online_time-hour*3600)%60;
				fprintf(cgiOut,"<td id=td2>%s</td>",ctime(&sta->sta_access_time));
				fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"on_time"));			  
				  fprintf(cgiOut,"<td id=td2>%d:%d:%d</td>",hour,min,sec);
				fprintf(cgiOut,"</tr>"\
			  "</table>");

	        }
			else if((result==0)||result == SNMPD_CONNECTION_ERROR)
			  fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
			else if(result==-1)
			  fprintf(cgiOut,"%s",search(lwlan,"no_sta"));			
			else
			  fprintf(cgiOut,"%s",search(lpublic,"error"));	
      fprintf(cgiOut,"</td>"\
    "</tr>");
    Free_bss_bymac(sta);
  }
  else if(f)
    ShowAlert(search(lwlan,"mac_not_null"));
  fprintf(cgiOut,"<tr>"\
    "<td><input type=hidden name=encry_searbymac value=%s></td>",m);
  fprintf(cgiOut,"</tr>"\
  "<tr>"
    "<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
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
free_instance_parameter_list(&paraHead1);
}


