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
* wp_wdsBssidList.c
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
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"


void ShowWdsBssidListPage(char *m,char *n,char *t,char *wid,char *f,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan); 
void DeleteWdsBssidMac(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };			  
  char rid[10] = { 0 };  
  char wlanid[10] = { 0 };
  char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_radiolis.cgi,否则上一页为wp_wtpdta.cgi*/  
  char *str = NULL;        
  char instance_id[10] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");  
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(rid,0,sizeof(rid));
  memset(wlanid,0,sizeof(wlanid));
  memset(flag,0,sizeof(flag));
  memset(instance_id,0,sizeof(instance_id)); 
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("RID", rid, 10);
	cgiFormStringNoNewlines("WLANID", wlanid, 10);
	cgiFormStringNoNewlines("FL", flag, 5);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {    
	cgiFormStringNoNewlines("encry_delwdsmac",encry,BUF_LEN);
	cgiFormStringNoNewlines("radio_id",rid,10);  
	cgiFormStringNoNewlines("wlan_id",wlanid,10);  
	cgiFormStringNoNewlines("flag",flag,5);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  
  if(strcmp(instance_id,"")==0)
  {	
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);	
	if(paraHead1)
	{
		snprintf(instance_id,sizeof(instance_id)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id); 
	}
  }
  else
  {
	get_slotID_localID_instanceID(instance_id,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
  }

  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
  else
    ShowWdsBssidListPage(encry,str,rid,wlanid,flag,instance_id,paraHead1,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWdsBssidListPage(char *m,char *n,char *t,char *wid,char *f,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  int i = 0,limit = 0,retu = 1;  
  char *endptr = NULL;  
  int rid = 0,result = 0,bssid_num = 0;      
  char wtp_id[10] = { 0 };
  char pno[10] = { 0 };
  DCLI_RADIO_API_GROUP_ONE *mac_head = NULL;
  struct wds_bssid *q = NULL;
  char delete_mac[20] = { 0 };
  char alt[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  char max_radio_num[10] = { 0 };
  memset(wtp_id,0,sizeof(wtp_id));
  memset(pno,0,sizeof(pno));
  cgiFormStringNoNewlines("WID", wtp_id, 10); 
  cgiFormStringNoNewlines("PN",pno,10);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WDS BSSID List</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script type=\"text/javascript\">"\
  "function deleteWdsMac(rid,wid,mac)"\
  "{"\
     "myform.Radio_id.value=rid;"\
     "myform.Wlan_id.value=wid;"\
     "myform.wds_mac.value=mac;"\
     "myform.submit();"\
  "}"\
  "</script>"\
  "<body>");  
  if(ins_para)
  {
	  DeleteWdsBssidMac(ins_para,lpublic,lwlan);
  } 
  fprintf(cgiOut,"<form id=myform method=post>"\
  "<input type=hidden name='Radio_id' id='Radio_id' value=''/>"\
  "<input type=hidden name='Wlan_id' id='Wlan_id' value=''/>"\
  "<input type=hidden name='wds_mac' id='wds_mac' value=''/>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>");
    if(strcmp(f,"0")==0)
      fprintf(cgiOut,"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
	else
	  fprintf(cgiOut,"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RF</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
		  "<td width=62 align=center><a href=wp_rdtail.cgi?UN=%s&FL=%s&ID=%s&WID=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,f,t,wtp_id,pno,ins_id,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_rdtail.cgi?UN=%s&FL=%s&ID=%s&WID=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,f,t,wtp_id,pno,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>WDS BSSID </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");					  
				  retu=checkuser_group(n);	 
				  if(strcmp(f,"0")==0)
				  {
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                    fprintf(cgiOut,"</tr>");
				    if(retu==0)  /*管理员*/
				    {
                      fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                      fprintf(cgiOut,"</tr>");
				    }
		fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgrouplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"ap_group_list"));			  
		  fprintf(cgiOut,"</tr>");
		  if(retu==0)  /*管理员*/
		  {
		fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));			  
		  fprintf(cgiOut,"</tr>");
		  	}
				    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
                    fprintf(cgiOut,"</tr>");
				    if(retu==0)  /*管理员*/
				    {
					  fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
                      fprintf(cgiOut,"</tr>");
				    }
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                    fprintf(cgiOut,"</tr>"\
				    "<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                    fprintf(cgiOut,"</tr>");
					if(retu==0)  /*管理员*/
				    {
					  fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                      fprintf(cgiOut,"</tr>");
				    }
				    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                    fprintf(cgiOut,"</tr>");
				    if(retu==0) /*管理员*/
				    {
                      fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                      fprintf(cgiOut,"</tr>");
				    }			
				  }
				  else
				  {
				    if(retu==0) /*管理员*/
				  	{
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_bssbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter")); 					  
					  fprintf(cgiOut,"</tr>");
				  	}
				  }
				  
				  rid= strtoul(t,&endptr,10);	/*char转成int，10代表十进制*/	
				  if(ins_para)
				  {
					  result=show_wlan_wds_bssid_list_cmd(ins_para->parameter,ins_para->connection,rid,wid,"wds_bssid_list",&mac_head);
				  } 
				  if(result == 1)
				  {
				     if((mac_head)&&(mac_head->BSS[0])&&(mac_head->BSS[0]->wds_bss_list))
					 {
						q = mac_head->BSS[0]->wds_bss_list;
						while(q)
						{
							bssid_num++;
							q = q->next;
						}
					 }
				  }
				  
				  if(strcmp(f,"0")==0)
				  {
					  limit+=bssid_num-8;
					  if(retu==1)  /*普通用户*/
						limit+=5;
				  }
				  else
				  {
					  limit+=bssid_num+0;
					  if(retu==1)  /*普通用户*/
						limit+=1;
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
				 "<table width=220 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
				  "<tr valign=middle>"\
					"<td align=left>"\
					  "<table width=220 border=0 cellspacing=0 cellpadding=0>"\
					  "<tr style=\"padding-bottom:20px\">"\
					    "<td colspan=2 id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
					  fprintf(cgiOut,"</tr>"\
					  "<tr style=\"padding-bottom:20px\">"\
					    "<td width=100>WDS BSSID%s:</td>",search(lpublic,"num"));
					    fprintf(cgiOut,"<td width=120>%d</td>",bssid_num);
					  fprintf(cgiOut,"</tr>");
						if(result==1)
						{	
						  if((bssid_num>0)&&(mac_head)&&(mac_head->BSS[0])&&(mac_head->BSS[0]->wds_bss_list))
						  {
							 q = mac_head->BSS[0]->wds_bss_list;
							 fprintf(cgiOut,"<tr>"
							 "<td colspan=2>"
							 "<table width=220 border=0 cellspacing=0 cellpadding=0>"\
							 "<tr align=left height=10 valign=top>"\
								"<td id=thead1>Radio%s WLAN%s WDS BSSID %s</td>",t,wid,search(lpublic,"list"));
							 fprintf(cgiOut,"</tr>"\
							 "<tr>"\
							 "<td style=\"padding-left:20px\">");
							 if(retu==0)  /*管理员*/
							   fprintf(cgiOut,"<table frame=below rules=rows width=200 border=1>");
							 else
							   fprintf(cgiOut,"<table frame=below rules=rows width=150 border=1>");
							 while(q)
							 {	
							  fprintf(cgiOut,"<tr align=left>"\
								  "<td id=td1 width=150>%02X:%02X:%02X:%02X:%02X:%02X</td>",q->BSSID[0],q->BSSID[1],q->BSSID[2],q->BSSID[3],q->BSSID[4],q->BSSID[5]);
							  
							  if(retu==0)  /*管理员*/
							  {
							  	memset(delete_mac,0,sizeof(delete_mac));
								snprintf(delete_mac,sizeof(delete_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",q->BSSID[0],q->BSSID[1],q->BSSID[2],q->BSSID[3],q->BSSID[4],q->BSSID[5]);
								fprintf(cgiOut,"<td id=td2 width=50><a id=link href=# target=mainFrame onclick=deleteWdsMac(\"%s\",\"%s\",\"%s\");>%s</a></td>",t,wid,delete_mac,search(lpublic,"delete"));
							  }
							  fprintf(cgiOut,"</tr>");
							  q = q->next;
							 }
							  fprintf(cgiOut,"</table></td>"\
							  "</tr>"\
							  "</table></td></tr>");
						  }
						  else
							fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lwlan,"no_wds_bssid"));
						}
						else if(result==-1)
							fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lpublic,"unknown_id_format"));
						else if(result==-2)
						{
						  memset(alt,0,sizeof(alt));
						  strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
						  memset(max_wlan_num,0,sizeof(max_wlan_num));
						  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
						  strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
						  strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
						  fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",alt);
						}
						else if(result==-3)
						  fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lwlan,"wlan_not_exist"));
						else if(result==-4)
						  fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lpublic,"error"));
						else if(result==-5)
						{
							memset(alt,0,sizeof(alt));
						    strncpy(alt,search(lwlan,"radio_id_illegal1"),sizeof(alt)-1);
						    memset(max_radio_num,0,sizeof(max_radio_num));
						    snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
						    strncat(alt,max_radio_num,sizeof(alt)-strlen(alt)-1);
						    strncat(alt,search(lwlan,"radio_id_illegal2"),sizeof(alt)-strlen(alt)-1);
							fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",alt);
						}
						else if(result==-6)
						  fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lpublic,"input_exceed_max_value"));
						else
						  fprintf(cgiOut,"<tr><td colspan=2>%s</td></tr>",search(lpublic,"contact_adm"));
						fprintf(cgiOut,"<tr>"\
						  "<td><input type=hidden name=encry_delwdsmac value=%s></td>",m);
						  fprintf(cgiOut,"<td><input type=hidden name=radio_id value=%s></td>",t); 
						fprintf(cgiOut,"</tr>"\
						"<tr>"\
						  "<td><input type=hidden name=wlan_id value=%s></td>",wid);
						  fprintf(cgiOut,"<td><input type=hidden name=flag value=%s></td>",f);
						fprintf(cgiOut,"</tr>"\
						"</table>"
					"</td>"\
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
if(result==1)
{
  Free_wlan_wds_bssid_list_head(mac_head);
}
}

void DeleteWdsBssidMac(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{
    char rid[10] = { 0 };
	char wid[10] = { 0 };
	char mac[20] = { 0 };
	char temp[100] = { 0 };
    char max_wlan_num[10] = { 0 };
	char max_radio_num[10] = { 0 };
	char *endptr = NULL;  
	int id = 0;
	int ret = 0;

	memset(rid,0,sizeof(rid));
	cgiFormStringNoNewlines("Radio_id",rid,10);
	memset(wid,0,sizeof(wid));
	cgiFormStringNoNewlines("Wlan_id",wid,10);
	memset(mac,0,sizeof(mac));
	cgiFormStringNoNewlines("wds_mac",mac,20);
	if(strcmp(rid,""))
	{
		id= strtoul(rid,&endptr,10);	/*char转成int，10代表十进制*/	
		
		ret=radio_wlan_wds_bssid_cmd(ins_para->parameter,ins_para->connection,id,wid,"delete","wds_bssid",mac);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:{
					 ShowAlert(search(lwlan,"del_wds_bssid_fail"));
					 break;
				   }
			case 1:break;
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
			case -2:{
					  ShowAlert(search(lpublic,"input_para_illegal"));
					  break;
					}
			case -3:{
					  ShowAlert(search(lpublic,"unknown_mac_format"));
					  break;
					}		
			case -4:{
					  ShowAlert(search(lwlan,"wlan_not_exist"));
					  break;
					}
			case -5:{
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
					  strncat(temp,wid,sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);	
					  break;
					}
			case -6:{
					  ShowAlert(search(lwlan,"another_wds_mode_used_dis_first"));
					  break;
					}
			case -7:{
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,"WDS ",sizeof(temp)-1);
					  strncat(temp,search(lpublic,"oper_fail"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  break;
					}
			case -8:{
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
					  memset(max_radio_num,0,sizeof(max_radio_num));
					  snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
					  strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
			  		  ShowAlert(temp);
					  break;
					}
			case -9:{
					  ShowAlert(search(lpublic,"input_para_illegal"));
					  break;
					}
			case -10:{
					   ShowAlert(search(lwlan,"another_mesh_mode_used_dis_first"));
					   break;
					 }
			case -11:{
					   memset(temp,0,sizeof(temp));
					   strncpy(temp,"Mesh ",sizeof(temp)-1);
					   strncat(temp,search(lpublic,"oper_fail"),sizeof(temp)-strlen(temp)-1);
					   ShowAlert(temp);
					   break;
					 }
		}
	}
}


