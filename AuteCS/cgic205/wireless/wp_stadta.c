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
* wp_stadta.c
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
#include "ws_dbus_list_interface.h"

void ShowStadtaPage(char *m,char *mac,char *pn,struct list *lpublic,struct list *lwlan);  

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;      
  char mac[20] = { 0 };
  char pn[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(mac,0,sizeof(mac));
  memset(pn,0,sizeof(pn));
  
  cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  cgiFormStringNoNewlines("MAC", mac, 20);
  cgiFormStringNoNewlines("PN", pn, 10);
  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowStadtaPage(encry,mac,pn,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowStadtaPage(char *m,char *mac,char *pn,struct list *lpublic,struct list *lwlan)
{
  int i = 0,j = 0,result = 0,limit = 0,find = 0;     
  int bnum = 0;
  char tem_mac[20] = { 0 };
  char select_insid[10] = { 0 };
  struct dcli_ac_info *ac = NULL;
  struct dcli_bss_info *bss = NULL;  
  struct dcli_sta_info *sta = NULL;
  unsigned char ieee80211_state[20] = { 0 };
  unsigned char PAE[20] = { 0 };
  unsigned char BACKEND[20] = { 0 };
  //time_t 	now,now_sysrun,statime;
  time_t online_time;
  int 	hourz = 0,minz = 0,secz = 0;
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Station Detail</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
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
     	  "<td width=62 align=center><a href=wp_stalis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,select_insid,search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_stalis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,select_insid,search(lpublic,"img_cancel"));
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
						"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"sta_det"));   /*突出显示*/
					fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_seachsta.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"search_sta"));                       
					fprintf(cgiOut,"</tr>");			
					if(paraHead1)
					{
						result=show_station_list_by_group(paraHead1->parameter,paraHead1->connection,&ac);
					}
				    if(result == 1)
						bnum = ac->num_bss_wireless;
					
					limit=11;
					for(i=0;i<limit;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						"<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
			 "<table width=370 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");
			  fprintf(cgiOut,"<tr>"\
                     "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),select_insid);
                  fprintf(cgiOut,"</tr>");			
			 fprintf(cgiOut,"<tr valign=middle>"\
			    "<td align=center>");			
			if((result==1)&&(bnum>0))			
			{	
			   fprintf(cgiOut,"<table width=370 border=0 cellspacing=0 cellpadding=0>"\
	           "<tr align=left height=10 valign=top>"\
	           "<td id=thead1>%s</td>",search(lwlan,"sta_det"));
	           fprintf(cgiOut,"</tr>");
			   for(i=0,bss = ac->bss_list; ((i<ac->num_bss_wireless)&&(NULL != bss)); i++,bss = bss->next)
			   {
				   for(j=0,sta = bss->sta_list; ((j<bss->num_sta)&&(NULL != sta)); j++,sta = sta->next)
				   {
						 memset(tem_mac,0,sizeof(tem_mac));
						 snprintf(tem_mac,sizeof(tem_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STRZ(sta->addr));

						 if(strcmp(tem_mac,mac)==0)
						 {
						   fprintf(cgiOut,"<tr>"\
							 "<td align=center style=\"padding-left:20px\">");
						     if(strcmp(search(lwlan,"sta_det"),"Station Detail")==0)
							   fprintf(cgiOut,"<table frame=below rules=rows width=370 border=1>");
							 else
							   fprintf(cgiOut,"<table frame=below rules=rows width=320 border=1>");
								 fprintf(cgiOut,"<tr align=left>");
								 if(strcmp(search(lwlan,"sta_det"),"Station Detail")==0)
								   fprintf(cgiOut,"<td id=td1 width=220>MAC</td>");
								 else
								   fprintf(cgiOut,"<td id=td1 width=170>MAC</td>");
								   fprintf(cgiOut,"<td id=td2 width=150>%02X:%02X:%02X:%02X:%02X:%02X</td>",MAC2STRZ(sta->addr));
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
								   "<td id=td1>STA VLAN ID</td>"\
								   "<td id=td2>%d</td>",sta->vlan_id);
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>IP</td>");
								   if(sta->ip)
								   {
									   fprintf(cgiOut,"<td id=td2>%s</td>",sta->ip);
								   }
								 memset(ieee80211_state, 0, sizeof(ieee80211_state));
								 memset(PAE, 0, sizeof(PAE));
								 memset(BACKEND, 0, sizeof(BACKEND));
								 asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			

								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"sta_IEEE80211_state"));
								   fprintf(cgiOut,"<td id=td2>%s</td>",ieee80211_state);
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"pae_state"));
								   fprintf(cgiOut,"<td id=td2>%s</td>",PAE);
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"backend_state"));
								   fprintf(cgiOut,"<td id=td2>%s</td>",BACKEND);
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"resbyte"));
								   fprintf(cgiOut,"<td id=td2>%lld</td>",sta->retrybytes);
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"trabyte"));
								   fprintf(cgiOut,"<td id=td2>%lld</td>",sta->txbytes);
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"acc_time"));	
								 /*time(&now);
								 get_sysruntime(&now_sysrun);
								 online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
								 statime = now - online_time;*/
								 fprintf(cgiOut,"<td id=td2>%s</td>",ctime(&sta->sta_access_time));
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"on_time"));
								 online_time=sta->sta_online_time_new;
								 hourz=online_time/3600;
								 minz=(online_time-hourz*3600)/60;
								 secz=(online_time-hourz*3600)%60;								 
								 fprintf(cgiOut,"<td id=td2>%d:%d:%d</td>",hourz,minz,secz);
								 fprintf(cgiOut,"</tr>"\
								 "<tr align=left>"\
								   "<td id=td1>%s</td>",search(lwlan,"acc_type"));
								   fprintf(cgiOut,"<td id=td2>%s</td>","Wireless");
								 fprintf(cgiOut,"</tr>"\
							   "</table></td>"\
							"</tr>");
							find=1;
							break;
						 }
				   }
				   if(find==1)
				   	break;
			   }
			  fprintf(cgiOut,"</table>");
			}
			else if(bnum==0)
			  fprintf(cgiOut,"%s",search(lwlan,"no_bss"));
			else if((result==0)||result == SNMPD_CONNECTION_ERROR)
              fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
	        else
	          fprintf(cgiOut,"%s",search(lpublic,"error"));	
	fprintf(cgiOut,"</td>"\
 " </tr>"\
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
if(result == 1)	
{
  Free_sta_summary(ac);
}
free_instance_parameter_list(&paraHead1);
}


