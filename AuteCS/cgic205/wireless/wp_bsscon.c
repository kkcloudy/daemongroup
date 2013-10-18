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
* wp_bsscon.c
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
#include "ws_dcli_wlans.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"


int ShowBSSconPage(char *m,char *n,char *t,char *wid,char *f,char *ins_id,char *wtp_id,char *BSSindex,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan); 
int config_bss(instance_parameter *ins_para,int Rid,char *Bid,char *Wid,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };			  
  char ID[10] = { 0 };
  char rid[10] = { 0 };  
  char wlanid[10] = { 0 };
  char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_rdtail.cgi,否则上一页为wp_wtpdta.cgi*/  
  char wtp_id[10] = { 0 };
  char BSSindex[10] = { 0 };  
  char instance_id[10] = { 0 };
  char *str = NULL;        
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt"); 
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(rid,0,sizeof(rid));
  memset(wlanid,0,sizeof(wlanid));
  memset(flag,0,sizeof(flag));  
  memset(wtp_id,0,sizeof(wtp_id));
  memset(BSSindex,0,sizeof(BSSindex));  
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("ID", ID, 10);		
	cgiFormStringNoNewlines("RID", rid, 10);
	cgiFormStringNoNewlines("WLANID", wlanid, 10);
	cgiFormStringNoNewlines("FL", flag, 5);	
	cgiFormStringNoNewlines("WID", wtp_id, 10); 
	cgiFormStringNoNewlines("BSSindex", BSSindex, 10);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {    
	cgiFormStringNoNewlines("encry_conbss",encry,BUF_LEN);
	cgiFormStringNoNewlines("bss_id",ID,10);	  
	cgiFormStringNoNewlines("radio_id",rid,10);  
	cgiFormStringNoNewlines("wlan_id",wlanid,10);  
	cgiFormStringNoNewlines("flag",flag,5);
	cgiFormStringNoNewlines("wtp_id", wtp_id, 10); 
	cgiFormStringNoNewlines("bss_index", BSSindex, 10);
	cgiFormStringNoNewlines("instance_id", instance_id, 10);
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

  if(strcmp(instance_id,"")==0)
  {
	memset(instance_id,0,sizeof(instance_id));
	strncpy(instance_id,"0",sizeof(instance_id)-1);
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
  else
    ShowBSSconPage(encry,ID,rid,wlanid,flag,instance_id,wtp_id,BSSindex,paraHead1,lpublic,lwlan);

  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowBSSconPage(char *m,char *n,char *t,char *wid,char *f,char *ins_id,char *wtp_id,char *BSSindex,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  int i = 0,limit = 0,no_operate = 1;  
  char *endptr = NULL;  
  int rid = 0;   
  char pno[10] = { 0 };
  memset(pno,0,sizeof(pno));
  cgiFormStringNoNewlines("PN",pno,10);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  rid= strtoul(t,&endptr,10);	/*char转成int，10代表十进制*/	
  
  if(cgiFormSubmitClicked("bsscon_apply") == cgiFormSuccess)
  {
  	if(ins_para)
  	{
		no_operate=config_bss(ins_para,rid,n,wid,lpublic,lwlan);
		if(no_operate==1)/*没有任何操作，返回上级页面*/
		{
		  fprintf(cgiOut, "<script type=text/javascript>"\
							"window.location.href = 'wp_rdtail.cgi?UN=%s&FL=%s&ID=%s&WID=%s&PN=%s&INSTANCE_ID=%s';\n",m,f,t,wtp_id,pno,ins_id);
						  fprintf(cgiOut, "</script>");
		}
  	}
  }  
  fprintf(cgiOut,"<form>"\
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
		  "<td width=62 align=center><input id=but type=submit name=bsscon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> BSS</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");					
				  if(strcmp(f,"0")==0)
				  {
                    fprintf(cgiOut,"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
	                fprintf(cgiOut,"</tr>"\
	                "<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
	                fprintf(cgiOut,"</tr>");

			fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_wtpgrouplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"ap_group_list"));			  
			  fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));			  
			  fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
	                fprintf(cgiOut,"</tr>"\
				    "<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
	                fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                    fprintf(cgiOut,"</tr>"\
				    "<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                    fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
	                fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
	                fprintf(cgiOut,"</tr>"\
	                "<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
	                fprintf(cgiOut,"</tr>");
				  }
				  else
				  {
				    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_bssbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  if(strcmp(f,"0")==0)
				  	limit=6;
				  else
				  	limit=12;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
                      "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
                      "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
					 fprintf(cgiOut,"</tr>"\
						"<tr>"\
						  "<td><table width=700 border=0 cellspacing=0 cellpadding=0>"\
							   "<tr height=30 align=left>"\
							   "<td id=thead5 align=left>%s BSS %s</td>",search(lpublic,"configure"),BSSindex);		
							   fprintf(cgiOut,"</tr>"\
							   "</table>"\
						  "</td>"\
						"</tr>"\
						"<tr><td align=center style=\"padding-left:20px\"><table width=700 border=0 cellspacing=0 cellpadding=0>"\
				  "<tr height=30>"\
					"<td width=130>%s:</td>",search(lwlan,"wlan_sta_max"));
					fprintf(cgiOut,"<td width=120 align=left><input type=text name=bssnum size=17 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
					 "<td width=450 align=left style=\"padding-left:10px\"><font color=red>(0--32767)</font></td>"\
				  "</tr>"\
				  "<tr height=30>"\
					"<td>WDS ANY %s:</td>",search(lpublic,"mode"));
					fprintf(cgiOut,"<td align=left><select name=wds_any id=wds_any style=width:115px>"\
						"<option value=>"\
  				        "<option value=disable>disable"\
  				    	"<option value=enable>enable"\
					 "</td>"\
					 "<td align=left style=\"padding-left:10px\"><font color=red>(%s)</font></td>",search(lwlan,"con_wds_service"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=30>"\
					"<td>WDS SOME %s:</td>",search(lpublic,"mode"));
					fprintf(cgiOut,"<td align=left><input type=text name=wds_some size=17 maxLength=17 onkeypress=\"return event.keyCode!=32\"></td>"\
					 "<td align=left style=\"padding-left:10px\"><font color=red>(xx:xx:xx:xx:xx:xx,%s)</font></td>",search(lwlan,"con_wds_some"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=30>"\
					"<td>BSS%s:</td>",search(lpublic,"traffic_limit_switch"));
					fprintf(cgiOut,"<td align=left colspan=2><select name=traffic_limit_switch id=traffic_limit_switch style=width:115px>"\
						"<option value=>"\
  				        "<option value=disable>disable"\
  				    	"<option value=enable>enable"\
					 "</td>"\
				  "</tr>"\
				  "<tr height=30>"\
					"<td>BSS%s:</td>",search(lpublic,"uplink_traffic_limit_threshold"));
					fprintf(cgiOut,"<td align=left><input type=text name=uplink_traffic_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
					"<td align=left style=\"padding-left:10px\"><font color=red>(1--884736)kbps</font></td>"
				  "</tr>"\
				  "<tr height=30>"\
					"<td>STA%s:</td>",search(lpublic,"average_uplink_traffic_limit_threshold"));
					fprintf(cgiOut,"<td align=left><input type=text name=average_uplink_traffic_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				    "<td align=left style=\"padding-left:10px\"><font color=red>(1--884736)kbps</font></td>"
				  "</tr>"\
				  "<tr height=30>"\
					"<td>BSS%s:</td>",search(lpublic,"downlink_traffic_limit_threshold"));
					fprintf(cgiOut,"<td align=left><input type=text name=downlink_traffic_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
					"<td align=left style=\"padding-left:10px\"><font color=red>(1--884736)kbps</font></td>"
				  "</tr>"\
				  "<tr height=30>"\
					"<td>STA%s:</td>",search(lpublic,"average_downlink_traffic_limit_threshold"));
					fprintf(cgiOut,"<td align=left><input type=text name=average_downlink_traffic_limit_threshold size=17 maxLength=9 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				    "<td align=left style=\"padding-left:10px\"><font color=red>(1--884736)kbps</font></td>"
				  "</tr>"\
				  "<tr height=30>"\
					"<td>%s:</td>",search(lwlan,"sta_access_rssi_limit"));
					fprintf(cgiOut,"<td align=left><input type=text name=rssi size=17 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				    "<td align=left style=\"padding-left:10px\"><font color=red>(0--95)</font></td>"
				  "</tr>"\
				  "<tr height=30>"\
					"<td>%s:</td>",search(lwlan,"multi_user_switch"));
					fprintf(cgiOut,"<td align=left colspan=2><select name=multi_user_switch id=multi_user_switch style=width:115px>"\
						"<option value=>"\
  				        "<option value=disable>disable"\
  				    	"<option value=enable>enable"\
					 "</td>"\
				  "</tr>"\
				  "<tr>"\
					"<td><input type=hidden name=encry_conbss value=%s></td>",m);
					fprintf(cgiOut,"<td><input type=hidden name=bss_id value=%s></td>",n);		
					fprintf(cgiOut,"<td><input type=hidden name=radio_id value=%s></td>",t); 
				  fprintf(cgiOut,"</tr>"\
				  "<tr>"\
					"<td><input type=hidden name=wlan_id value=%s></td>",wid);
					fprintf(cgiOut,"<td><input type=hidden name=flag value=%s></td>",f);
					fprintf(cgiOut,"<td><input type=hidden name=wtp_id value=%s></td>",wtp_id);
				  fprintf(cgiOut,"</tr>"\
				  "<tr>"\
					"<td><input type=hidden name=bss_index value=%s></td>",BSSindex);
				    fprintf(cgiOut,"<td colspan=2><input type=hidden name=instance_id value=%s></td>",ins_id);
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
return 0;
}

int config_bss(instance_parameter *ins_para,int Rid,char *Bid,char *Wid,struct list *lpublic,struct list *lwlan)
{  
  int ret = 0,flag = 1,no_operate = 1;/*no_operate==1表示没有任何bss配置操作*/
  char *endptr = NULL;  
  char temp[100] = { 0 };
  char bss_num[5] = { 0 };
  char bssnum[10] = { 0 };
  char wds_any[10] = { 0 };
  char max_radio_num[10] = { 0 };
  char mac_add[20] = { 0 };
  int sta_num = 0;
  char max_wlan_num[10] = { 0 };
  char traffic_limit_switch[10] = { 0 };
  char uplink_traffic_limit_threshold[10] = { 0 };
  char average_uplink_traffic_limit_threshold[10] = { 0 };
  char downlink_traffic_limit_threshold[10] = { 0 };
  char average_downlink_traffic_limit_threshold[10] = { 0 };
  char rssi[10] = { 0 };
  char multi_user_switch[10] = { 0 };

  /*************************max sta count**********************************/
  memset(bssnum,0,sizeof(bssnum));
  cgiFormStringNoNewlines("bssnum",bssnum,10);
  if(strcmp(bssnum,"")!=0)
  { 
    no_operate=0;
    sta_num=strtoul(bssnum,&endptr,10);	/*char转成int，10代表十进制*/
	if((sta_num>=0)&&(sta_num<32768))
	{
      ret=set_bss_max_sta_num(ins_para->parameter,ins_para->connection,Rid,Wid,bssnum);   /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
										                                                 /*返回-2表示wlanid should be 1 to WLAN_NUM，返回-3表示max station number should be greater than 0,and not cross 32767*/
										                                                 /*返回-4表示bss not exist，返回-5表示more sta(s) has accessed before you set max sta num */
										                                                 /*返回-6表示operation fail!，返回-7表示Radio ID非法，返回-11表示wlan is not binded radio*/
      switch(ret)				
	  {
	    case SNMPD_CONNECTION_ERROR:
	    case 0:{
	 	  	     flag=0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"con_bss_max_sta_num_fail1"),sizeof(temp)-1);
				 strncat(temp,"BSS",sizeof(temp)-strlen(temp)-1);
				 strncat(temp,search(lwlan,"con_bss_max_sta_num_fail2"),sizeof(temp)-strlen(temp)-1);
				 ShowAlert(temp);
			     break;
			   }
	    case 1:break;
	    case -1:{
	 		      flag=0;
				  ShowAlert(search(lpublic,"input_para_error"));
			      break; 
		        }
	    case -2:{
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
	    case -3:{
			      flag=0;
		          ShowAlert(search(lwlan,"bss_num"));
	              break; 
		        }
	    case -4:{
			      flag=0;
		          ShowAlert(search(lwlan,"bss_not_exist"));
	              break; 
		        }
	    case -5:{
			      flag=0;
		          ShowAlert(search(lwlan,"more_sta_has_access"));
	              break; 
		        }
	    case -6:{
	  		      flag=0;
				  ShowAlert(search(lpublic,"oper_fail"));
			      break; 
		        }
		case -7:{
	  		      flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				  memset(max_radio_num,0,sizeof(max_radio_num));
				  snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				  strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  		  ShowAlert(temp);
			      break; 
		        }
		case -11:{
	  		      flag=0;
				  ShowAlert(search(lwlan,"radio_not_bind_wlan"));
			      break; 
		        }
	  }
	}
	else
	{
	  memset(temp,0,sizeof(temp));
	  strncpy(temp,search(lwlan,"max_sta_num_illegal1"),sizeof(temp)-1);
	  strncat(temp,"32767",sizeof(temp)-strlen(temp)-1);
	  strncat(temp,search(lwlan,"max_sta_num_illegal2"),sizeof(temp)-strlen(temp)-1);
	  ShowAlert(temp);
      flag=0;
	}
  }

  /*************************set wds service cmd**********************************/
  memset(wds_any,0,sizeof(wds_any));
  cgiFormStringNoNewlines("wds_any",wds_any,10);
  if(strcmp(wds_any,"")!=0)
  {
    no_operate=0;
  	ret=set_wds_service_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,"wds",wds_any);
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_wds_any_fail"));
			     break;
			   }
		case 1:break;
		case -1:{
				  flag=0;
				  ShowAlert(search(lpublic,"input_para_illegal"));
				  break;
				}
		case -2:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_not_exist"));
				  break;
				}
		case -3:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);  
				  break;
				}
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_not_exist"));
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"wtp_not_run"));
				  break;
				}
		case -6:{
				  flag=0;
				  ShowAlert(search(lwlan,"another_wds_mode_used"));
				  break;
				}
		case -7:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
		case -8:{
				  flag=0;
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
				  flag=0;
				  ShowAlert(search(lpublic,"input_exceed_max_value"));
				  break;
				}
		case -10:{
				  flag=0;
				  ShowAlert(search(lpublic,"unknown_id_format"));
				  break;
				}
		case -11:{
				  flag=0;
				  ShowAlert(search(lpublic,"input_para_illegal"));
				  break;
				}
		case -12:{
				  flag=0;
				  ShowAlert(search(lwlan,"another_mesh_mode_used"));
				  break;
				}
	}
  }


  /*************************radio wlan wds bssid cmd**********************************/
  memset(mac_add,0,sizeof(mac_add));
  cgiFormStringNoNewlines("wds_some",mac_add,20);
  if(strcmp(mac_add,"")!=0)
  {
  	if(strchr(mac_add,' ')==NULL)   /*不包含空格*/
  	{
		no_operate=0;
		ret=radio_wlan_wds_bssid_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,"add","wds_bssid",mac_add);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:{
					 flag=0;
					 ShowAlert(search(lwlan,"con_wds_some_fail"));
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
					  ShowAlert(search(lpublic,"input_para_illegal"));
					  break;
					}
			case -3:{
					  flag=0;
					  ShowAlert(search(lpublic,"unknown_mac_format"));
					  break;
					}		
			case -4:{
					  flag=0;
					  ShowAlert(search(lwlan,"wlan_not_exist"));
					  break;
					}
			case -5:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
					  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);	
					  break;
					}
			case -6:{
					  flag=0;
					  ShowAlert(search(lwlan,"another_wds_mode_used_dis_first"));
					  break;
					}
			case -7:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,"WDS ",sizeof(temp)-1);
					  strncat(temp,search(lpublic,"oper_fail"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  break;
					}
			case -8:{
				  	  flag=0;
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
					  flag=0;
					  ShowAlert(search(lpublic,"input_para_illegal"));
					  break;
					}
			case -10:{
					   flag=0;
					   ShowAlert(search(lwlan,"another_mesh_mode_used_dis_first"));
					   break;
					 }
			case -11:{
					   flag=0;
					   memset(temp,0,sizeof(temp));
					   strncpy(temp,"Mesh ",sizeof(temp)-1);
					   strncat(temp,search(lpublic,"oper_fail"),sizeof(temp)-strlen(temp)-1);
					   ShowAlert(temp);
					   break;
					 }
		}
  	}
	else
	{
	  flag=0;
	  ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
	}
  }

  
  /*************************radio bss traffic limit cmd**********************************/
  memset(traffic_limit_switch,0,sizeof(traffic_limit_switch));
  cgiFormStringNoNewlines("traffic_limit_switch",traffic_limit_switch,10);
  if(strcmp(traffic_limit_switch,"")!=0)
  {
    no_operate=0;
  	ret=radio_bss_traffic_limit_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,traffic_limit_switch);
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_traffic_limit_switch_fail"));
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
				  ShowAlert(search(lpublic,"input_para_illegal"));
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_not_exist"));  
				  break;
				}
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_not_exist"));
				  break;
				}
		case -6:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}
		case -7:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
	}
  }


  /*************************radio bss traffic limit value cmd**********************************/
  memset(uplink_traffic_limit_threshold,0,sizeof(uplink_traffic_limit_threshold));
  cgiFormStringNoNewlines("uplink_traffic_limit_threshold",uplink_traffic_limit_threshold,10);
  if(strcmp(uplink_traffic_limit_threshold,"")!=0)
  {
    no_operate=0;
  	ret=radio_bss_traffic_limit_value_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,uplink_traffic_limit_threshold);

	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_uplink_traffic_limit_threshold_fail"));
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
				  strncat(temp,uplink_traffic_limit_threshold,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lpublic,"input_para_error2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_not_exist"));  
				  break;
				}
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_not_exist"));
				  break;
				}
		case -6:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}
		case -7:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
		case -9:{
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

  /*************************radio bss traffic limit average valuecmd**********************************/
  memset(average_uplink_traffic_limit_threshold,0,sizeof(average_uplink_traffic_limit_threshold));
  cgiFormStringNoNewlines("average_uplink_traffic_limit_threshold",average_uplink_traffic_limit_threshold,10);
  if(strcmp(average_uplink_traffic_limit_threshold,"")!=0)
  {
    no_operate=0;
  	ret=radio_bss_traffic_limit_average_value_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,average_uplink_traffic_limit_threshold);

	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_average_uplink_traffic_limit_threshold_fail"));
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
				  strncat(temp,average_uplink_traffic_limit_threshold,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lpublic,"input_para_error2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_not_exist"));  
				  break;
				}
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_not_exist"));
				  break;
				}
		case -6:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}
		case -7:{
				  flag=0;
				  ShowAlert(search(lwlan,"sta_tralim_morethan_bss_tralim_value"));
				  break;
				}
		case -8:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
		case -13:{
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

  /*************************radio bss traffic limit send value cmd**********************************/
  memset(downlink_traffic_limit_threshold,0,sizeof(downlink_traffic_limit_threshold));
  cgiFormStringNoNewlines("downlink_traffic_limit_threshold",downlink_traffic_limit_threshold,10);
  if(strcmp(downlink_traffic_limit_threshold,"")!=0)
  {
    no_operate=0;
  	ret=radio_bss_traffic_limit_send_value_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,downlink_traffic_limit_threshold);

	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_downlink_traffic_limit_threshold_fail"));
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
				  strncat(temp,downlink_traffic_limit_threshold,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lpublic,"input_para_error2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  break;
				}
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_not_exist"));
				  break;
				}
		case -5:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}		
		case -6:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_not_exist"));  
				  break;
				}
		case -7:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
		case -12:{
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

  /*************************radio bss traffic limit average valuecmd**********************************/
  memset(average_downlink_traffic_limit_threshold,0,sizeof(average_downlink_traffic_limit_threshold));
  cgiFormStringNoNewlines("average_downlink_traffic_limit_threshold",average_downlink_traffic_limit_threshold,10);
  if(strcmp(average_downlink_traffic_limit_threshold,"")!=0)
  {
    no_operate=0;
  	ret=radio_bss_traffic_limit_average_send_value_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,average_downlink_traffic_limit_threshold);

	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_average_downlink_traffic_limit_threshold_fail"));
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
				  strncat(temp,average_downlink_traffic_limit_threshold,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lpublic,"input_para_error2"),sizeof(temp)-strlen(temp)-1);
		  	      ShowAlert(temp);
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"sta_tralim_morethan_bss_tralim_value"));
				  break;
				}
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_not_exist"));
				  break;
				}
		case -6:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}
		case -7:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_not_exist"));  
				  break;
				}
		case -8:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
		case -13:{
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
  
  /*************************set radio wlan limit rssi access sta cmd**********************************/
  memset(rssi,0,sizeof(rssi));
  cgiFormStringNoNewlines("rssi",rssi,10);
  if(strcmp(rssi,"")!=0)
  {
    no_operate=0;
  	ret=set_radio_wlan_limit_rssi_access_sta_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,rssi);

	switch(ret)
	{
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"con_sta_access_rssi_limit_fail"));
			     break;
			   }
		case 1:break;
		case -1:{
				  flag=0;
				  ShowAlert(search(lpublic,"unknown_id_format"));
				  break;
				}
		case -2:{
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
		case -3:{
				  flag=0;
				  ShowAlert(search(lpublic,"input_para_illegal"));
				  break;
				}
		case -4:{
	  		      flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				  memset(max_radio_num,0,sizeof(max_radio_num));
				  snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				  strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  		  ShowAlert(temp);
			      break; 
		        }	 
		case -5:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_not_exist"));
				  break;
				}
		
		case -6:{
				  flag=0;
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  break;
				}
		case -7:{
				  flag=0;
				  ShowAlert(search(lwlan,"wlan_not_exist"));  
				  break;
				}
		case -8:{
				  flag=0;
				  ShowAlert(search(lwlan,"bss_not_exist"));  
				  break;
				}
		case -9:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}
	}
  }

  
  /*************************set bss multi_user optimize cmd**********************************/
  memset(multi_user_switch,0,sizeof(multi_user_switch));
  cgiFormStringNoNewlines("multi_user_switch",multi_user_switch,10);
  if(strcmp(multi_user_switch,"")!=0)
  {
    no_operate=0;
  	ret=set_bss_multi_user_optimize_cmd(ins_para->parameter,ins_para->connection,Rid,Wid,multi_user_switch);
	switch(ret)
	{
		case 0:{
				 flag=0;
			     ShowAlert(search(lwlan,"set_multi_user_switch_fail"));
			     break;
			   }
		case 1:break;
		case -1:{
				  flag=0;
				  ShowAlert(search(lpublic,"input_para_error"));
				  break;
				}
		case -2:{
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
		case -3:{
	  		      flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				  memset(max_radio_num,0,sizeof(max_radio_num));
				  snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				  strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  		  ShowAlert(temp);
			      break; 
		        }	 
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"bss_not_exist"));
				  break;
				}
		
		case -5:{
				  flag=0;
				  ShowAlert(search(lpublic,"oper_fail"));
				  break;
				}		
		case -6:{
				  flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"rad_dont_bind_wlan"),sizeof(temp)-1);
				  strncat(temp,Wid,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
				  ShowAlert(temp);
				  break;
				}		
		case -7:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));  
				  break;
				}
	}
  }
  if((flag==1)&&(no_operate==0))
  	ShowAlert(search(lpublic,"oper_succ"));

  return no_operate;
}


