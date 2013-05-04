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
* wp_stacon.c
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
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"


void ShowConStaPage(char *m,char *radio_id,char *wlan_id,char *mac,char *pn,struct list *lpublic,struct list *lwlan);  
void config_sta(instance_parameter *ins_para,int Rid,char *Wid,char *MAC,struct list *lpublic,struct list *lwlan);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;      
  char radid[10] = { 0 };
  char wlan_id[10] = { 0 };
  char mac[20] = { 0 };
  char pn[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(radid,0,sizeof(radid));
  memset(wlan_id,0,sizeof(wlan_id));
  memset(mac,0,sizeof(mac));
  memset(pn,0,sizeof(pn));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	  cgiFormStringNoNewlines("RID", radid, 10);
	  cgiFormStringNoNewlines("WLANID", wlan_id, 10);
	  cgiFormStringNoNewlines("MAC", mac, 20);
	  cgiFormStringNoNewlines("PN", pn, 10);
  }
  else
  {
  	  cgiFormStringNoNewlines("encry_consta", encry, BUF_LEN);
	  cgiFormStringNoNewlines("radio_id", radid, 10);
	  cgiFormStringNoNewlines("wlan_id", wlan_id, 10);
	  cgiFormStringNoNewlines("mac_address", mac, 20);
	  cgiFormStringNoNewlines("page_no", pn, 10);
  }
  
  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowConStaPage(encry,radid,wlan_id,mac,pn,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowConStaPage(char *m,char *radio_id,char *wlan_id,char *mac,char *pn,struct list *lpublic,struct list *lwlan)
{
  int i = 0,rid = 0,result = 0;
  char *endptr = NULL;
  char temp[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  char select_insid[10] = { 0 };  
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Config Station</title>");
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
	rid= strtoul(radio_id,&endptr,10);	/*char转成int，10代表十进制*/

  if(cgiFormSubmitClicked("reset_average_uplink_tralimthr") == cgiFormSuccess)
  {
  	result = 0;
  	if(paraHead1)
	{
		result=radio_bss_traffic_limit_cancel_sta_value_cmd(paraHead1->parameter,paraHead1->connection,rid,wlan_id,mac);
	}
	switch(result)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"reset_average_uplink_tralimthr_fail"));
			   break;
		case 1:ShowAlert(search(lwlan,"reset_average_uplink_tralimthr_succ"));
			   break;
		case -1:{
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
			  	  ShowAlert(temp);
				  break;
			    }
		case -2:ShowAlert(search(lwlan,"wlan_dont_work"));  
				break;
		case -3:{
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"no_sta_under_wlan1"),sizeof(temp)-1);
				  strncat(temp,wlan_id,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"no_sta_under_wlan2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -4:ShowAlert(search(lwlan,"wlan_not_exist"));  
				break;
		case -5:ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
		case -6:ShowAlert(search(lwlan,"radio_not_exist"));
				break;
		case -7:{
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,wlan_id,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}
		case -8:ShowAlert(search(lpublic,"error"));
				break;
	}
  }
  if(cgiFormSubmitClicked("reset_average_downlink_tralimthr") == cgiFormSuccess)
  {
  	result = 0;
  	if(paraHead1)
	{
		result=radio_bss_traffic_limit_cancel_sta_send_value_cmd(paraHead1->parameter,paraHead1->connection,rid,wlan_id,mac);
	}
	switch(result)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"reset_average_downlink_tralimthr_fail"));
			   break;
		case 1:ShowAlert(search(lwlan,"reset_average_downlink_tralimthr_succ"));
			   break;
		case -1:{
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
			  	  ShowAlert(temp);
				  break;
			    }
		case -2:ShowAlert(search(lwlan,"wlan_dont_work"));  
				break;
		case -3:{
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"no_sta_under_wlan1"),sizeof(temp)-1);
				  strncat(temp,wlan_id,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"no_sta_under_wlan2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -4:ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
		case -5:ShowAlert(search(lwlan,"radio_not_exist"));
				break;
		case -6:{
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,wlan_id,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}
		case -7:ShowAlert(search(lwlan,"wlan_not_exist"));  
				break;
		case -8:ShowAlert(search(lpublic,"error"));
				break;
	}
  }
  if(cgiFormSubmitClicked("stacon_apply") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		config_sta(paraHead1,rid,wlan_id,mac,lpublic,lwlan);
	}
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
     	  "<td width=62 align=center><input id=but type=submit name=stacon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
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
				    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"con_sta"));   /*突出显示*/
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				    "<td align=left id=tdleft><a href=wp_seachsta.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"search_sta"));                       
                  fprintf(cgiOut,"</tr>");				  
				  for(i=0;i<2;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
 				"<table width=690 border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
				  "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),select_insid);
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
						  "<td><table width=690 border=0 cellspacing=0 cellpadding=0>"\
							   "<tr height=30 align=left>"\
							   "<td id=thead5 align=left>%s</td>",search(lwlan,"con_sta"));		
							   fprintf(cgiOut,"</tr>"\
							   "</table>"\
						  "</td>"\
						"</tr>"\
						"<tr><td align=left style=\"padding-left:20px\">");
					if(strcmp(search(lwlan,"con_sta"),"Config Station")==0)
					{
						fprintf(cgiOut,"<table width=690 border=0 cellspacing=0 cellpadding=0>"\
						  "<tr height=30>"\
							"<td width=240>STA%s:</td>",search(lpublic,"uplink_traffic_limit_threshold"));
							fprintf(cgiOut,"<td width=120 align=left><input type=text name=sta_uplink_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
							fprintf(cgiOut,"<td width=330 align=left style=\"padding-left:10px\">"\
											 "<font color=red>kbps</font>"\
											 "<input type=submit style=\"width:280px; margin-left:15px\" border=0 name=reset_average_uplink_tralimthr style=background-image:url(/images/SubBackGif.gif) value=\"%s\">"\
										   "</td>",search(lwlan,"reset_average_uplink_tralimthr"));
						  fprintf(cgiOut,"</tr>"\
						  "<tr height=30>"\
							"<td>STA%s:</td>",search(lpublic,"downlink_traffic_limit_threshold"));
							fprintf(cgiOut,"<td align=left><input type=text name=sta_downlink_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
							fprintf(cgiOut,"<td align=left style=\"padding-left:10px\">"\
											 "<font color=red>kbps</font>"\
											 "<input type=submit style=\"width:290px; margin-left:15px\" border=0 name=reset_average_downlink_tralimthr style=background-image:url(/images/SubBackGif.gif) value=\"%s\">"\
										   "</td>",search(lwlan,"reset_average_downlink_tralimthr"));
						  fprintf(cgiOut,"</tr>");
					}
					else
					{
						fprintf(cgiOut,"<table width=550 border=0 cellspacing=0 cellpadding=0>"\
						  "<tr height=30>"\
							"<td width=110>STA%s:</td>",search(lpublic,"uplink_traffic_limit_threshold"));
							fprintf(cgiOut,"<td width=120 align=left><input type=text name=sta_uplink_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
							fprintf(cgiOut,"<td width=320 align=left style=\"padding-left:10px\">"\
										     "<font color=red>(1--884736)kbps</font>"\
										     "<input type=submit style=\"width:150px; margin-left:15px\" border=0 name=reset_average_uplink_tralimthr style=background-image:url(/images/SubBackGif.gif) value=\"%s\">"\
										     "</td>",search(lwlan,"reset_average_uplink_tralimthr"));
						  fprintf(cgiOut,"</tr>"\
						  "<tr height=30>"\
							"<td>STA%s:</td>",search(lpublic,"downlink_traffic_limit_threshold"));
							fprintf(cgiOut,"<td align=left><input type=text name=sta_downlink_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
							fprintf(cgiOut,"<td align=left style=\"padding-left:10px\">"\
										     "<font color=red>(1--884736)kbps</font>"\
										     "<input type=submit style=\"width:150px; margin-left:15px\" border=0 name=reset_average_downlink_tralimthr style=background-image:url(/images/SubBackGif.gif) value=\"%s\">"\
										     "</td>",search(lwlan,"reset_average_downlink_tralimthr"));
						  fprintf(cgiOut,"</tr>");
					}
				  fprintf(cgiOut,"<tr>"\
					"<td><input type=hidden name=encry_consta value=%s></td>",m);
					fprintf(cgiOut,"<td><input type=hidden name=radio_id value=%s></td>",radio_id);		
					fprintf(cgiOut,"<td><input type=hidden name=wlan_id value=%s></td>",wlan_id); 
				  fprintf(cgiOut,"</tr>"\
				  "<tr>"\
					"<td><input type=hidden name=mac_address value=%s></td>",mac);
					fprintf(cgiOut,"<td><input type=hidden name=page_no value=%s></td>",pn);
					fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
				  fprintf(cgiOut,"</tr>"\
				"</table></td></tr>"\
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


void config_sta(instance_parameter *ins_para,int Rid,char *Wid,char *MAC,struct list *lpublic,struct list *lwlan)
{
  int ret = 0,flag = 1;
  char sta_uplink_limit_threshold[10] = { 0 };
  char sta_downlink_limit_threshold[10] = { 0 };
  char temp[100] = { 0 };
  char max_wlan_num[10] = { 0 };
 
  /*************************radio bss traffic limit value cmd**********************************/
  memset(sta_uplink_limit_threshold,0,sizeof(sta_uplink_limit_threshold));
  cgiFormStringNoNewlines("sta_uplink_limit_threshold",sta_uplink_limit_threshold,10);
  if(strcmp(sta_uplink_limit_threshold,"")!=0)
  {
  	ret=radio_bss_traffic_limit_sta_value_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,MAC,sta_uplink_limit_threshold);
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_sta_uplink_limit_threshold_fail"));
			     break;
			   }
		case 1:break;
		case -1:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
			  	  break;
				}
		case -2:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lpublic,"input_para_error1"),sizeof(temp)-1);
				  strncat(temp,sta_uplink_limit_threshold,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lpublic,"input_para_error2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_dont_work"));  
				  break;
				}
		case -4:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"no_sta_under_wlan1"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"no_sta_under_wlan2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"sta_tralim_morethan_bss_tralim_value"));
				  break;
				}
		case -6:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
		case -8:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
                  strncpy(temp,search(lpublic,"input_para_1to"),sizeof(temp)-1);
                  strncat(temp,"884736",sizeof(temp)-strlen(temp)-1);
                  strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
                  ShowAlert(temp);
				  break;
				}
	}
  }

  /*************************radio bss traffic limit value cmd**********************************/
  memset(sta_downlink_limit_threshold,0,sizeof(sta_downlink_limit_threshold));
  cgiFormStringNoNewlines("sta_downlink_limit_threshold",sta_downlink_limit_threshold,10);
  if(strcmp(sta_downlink_limit_threshold,"")!=0)
  {
  	ret=radio_bss_traffic_limit_sta_send_value_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,MAC,sta_downlink_limit_threshold);
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_sta_downlink_limit_threshold_fail"));
			     break;
			   }
		case 1:break;
		case -1:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
			  	  break;
				}
		case -2:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lpublic,"input_para_error1"),sizeof(temp)-1);
				  strncat(temp,sta_downlink_limit_threshold,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lpublic,"input_para_error2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_dont_work"));  
				  break;
				}
		case -4:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"no_sta_under_wlan1"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"no_sta_under_wlan2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"sta_tralim_morethan_bss_tralim_value"));
				  break;
				}
		case -6:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
		case -11:{
				   flag=0;
				   memset(temp,0,sizeof(temp));
                   strncpy(temp,search(lpublic,"input_para_1to"),sizeof(temp)-1);
                   strncat(temp,"884736",sizeof(temp)-strlen(temp)-1);
                   strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
                   ShowAlert(temp);
				   break;
				 }
	}
  }

  if(flag==1)
  	ShowAlert(search(lpublic,"oper_succ"));
}


