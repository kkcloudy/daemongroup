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
* wp_radcon.c
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
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_wqos.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


int rRate1[] = {540,480,360,240,180,120,110,90,60,55,20,10};    /*11b/g radio rate*/
int rRate2[] = {3000,2700,2430,2400,2160,1800,1620,1500,1350,
				1300,1215,1200,1170,1080,1040,900,810,780,
				650,600,585,540,520,450,405,390,300,270,
				260,195,150,135,130,65};/*11n radio rate*/

char *rType[] = {"11a","11b","11g","11gn","11g/gn","11b/g","11b/g/n","11a/an","11an"};  /*radio type*/


int ShowRadioconPage(char *m,char *n,char *t,char *f,char *pn,char *rt,char *txp,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan); 
void config_radio(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);

int cgiMain()
{
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  int rad_id = 0,result = 0;
  char *endptr = NULL;  
  char encry[BUF_LEN] = { 0 };
  char ID[10] = { 0 };
  char wtp_id[10] = { 0 };  
  char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_radiolis.cgi,否则上一页为wp_wtpdta.cgi*/  
  char pno[10] = { 0 };  
  char RType[10] = { 0 }; 
  char txp[10] = { 0 };  
  char instance_id[10] = { 0 };
  int select_insid = 0;
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
  memset(wtp_id,0,sizeof(wtp_id));
  memset(flag,0,sizeof(flag));
  memset(pno,0,sizeof(pno));
  memset(RType,0,sizeof(RType));
  memset(txp,0,sizeof(txp));
  memset(instance_id,0,sizeof(instance_id));  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
    cgiFormStringNoNewlines("ID", ID, 10);		
    cgiFormStringNoNewlines("WID", wtp_id, 10);
    cgiFormStringNoNewlines("FL", flag, 5);
    cgiFormStringNoNewlines("PN",pno,10);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {    
    cgiFormStringNoNewlines("encry_conradio",encry,BUF_LEN);
    cgiFormStringNoNewlines("radio_id",ID,10);	  
    cgiFormStringNoNewlines("wtp_id",wtp_id,10);  
    cgiFormStringNoNewlines("flag",flag,5);
    cgiFormStringNoNewlines("page_no",pno,10);
	cgiFormStringNoNewlines("radio_mode",RType,10);
	cgiFormStringNoNewlines("txp",txp,10);	
	cgiFormStringNoNewlines("instance_id",instance_id,10);  
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

  if(strcmp(RType,"")==0)
  {
	  if(strcmp(instance_id,"")==0)
	  {
		memset(instance_id,0,sizeof(instance_id));
		strncpy(instance_id,"0",sizeof(instance_id)-1);
	  }  
	  rad_id= strtoul(ID,&endptr,10);	 /*char转成int，10代表十进制*/
	  select_insid= strtoul(instance_id,&endptr,10);	 /*char转成int，10代表十进制*/
	  if(paraHead1)
	  {
		  result=show_radio_one(paraHead1->parameter,paraHead1->connection,rad_id,&radio);
	  }
	  if((result==1)&&(radio->RADIO[0]))
	  {
		  Radio_Type(radio->RADIO[0]->Radio_Type,RType);
		  if(strcmp(RType,"11bg")==0)
		  {
			  memset(RType,0,sizeof(RType));
			  strncpy(RType,"11b/g",sizeof(RType)-1);
		  }
		  else if(strcmp(RType,"11bgn")==0)
		  {
			  memset(RType,0,sizeof(RType));
			  strncpy(RType,"11b/g/n",sizeof(RType)-1);
		  }
	  }
	  else
		  strncpy(RType,"11b/g",sizeof(RType)-1);
  }
  if(strcmp(txp,"")==0)
  {
  	strncpy(txp,"0",sizeof(txp)-1);
  }

  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));          /*用户非法*/
  else
    ShowRadioconPage(encry,ID,wtp_id,flag,pno,RType,txp,instance_id,paraHead1,lpublic,lwlan);
  if(result==1)
  	Free_radio_one_head(radio);
  	
  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowRadioconPage(char *m,char *n,char *t,char *f,char *pn,char *rt,char *txp,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  char IsSubmit[5] = { 0 };
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  int i = 0,result = 0,limit = 0;  
  char *endptr = NULL;  
  int rad_id = 0;   
  DCLI_WQOS *wqos = NULL;
  int result1 = 0,result2 = 0,result4 = 0,result5 = 0;
  DCLI_RADIO_API_GROUP_ONE *rad_qos_head = NULL;
  int ratetypeChoice = 0;
  int wnum = 0,result3 = 0;  
  DCLI_WLAN_API_GROUP *WLANINFO = NULL;
  int flag_11n = 0;            /*flag_11n==1表示radio类型为11gn,11g/gn,11b/g,11b/g/n,11a/an或11an*/
  DCLI_WTP_API_GROUP_ONE *wtp = NULL;
  char wtp_sta[WTP_ARRAY_NAME_LEN] = { 0 }; 
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script type=\"text/javascript\">"\
     "function sub_txp()"\
     "{"\
     	"var txpower = document.all.rad_txpower_offset.value;"\
     	"if(txpower>(-26))"\
     	"{"\
     		"var new_txp = txpower - 1;"\
     		"document.all.rad_txpower_offset.value = new_txp;"\
     		"document.all.txp.value = new_txp;"\
     	"}"\
     	"else"\
     	"{"\
     		"return;"\
     	"}"\
  	 "}"\
  	 "function add_txp()"\
     "{"\
     	"var txpower = document.all.rad_txpower_offset.value;"\
     	"if(txpower<0)"\
     	"{"\
     		"var new_txp = parseInt(txpower) + 1;"\
     		"document.all.rad_txpower_offset.value = new_txp;"\
     		"document.all.txp.value = new_txp;"\
     	"}"\
     	"else"\
     	"{"\
     		"return;"\
     	"}"\
  	 "}"\
  "</script>"\
  "<body>");
  rad_id= strtoul(n,&endptr,10);    /*char转成int，10代表十进制*/
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("default") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	if(ins_para)
	{
		result4=set_radio_default_config_cmd_func(ins_para->parameter,ins_para->connection,rad_id);/*返回0表示失败，返回1表示成功，返回-1表示recover default config fail*/
	} 
	switch(result4)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lpublic,"recover_deflt_fail"));
			   break;
		case 1:ShowAlert(search(lpublic,"recover_deflt_succ"));
			   break;
		case -1:ShowAlert(search(lpublic,"recover_deflt_fail"));
			    break;
	}
  }
  if((cgiFormSubmitClicked("radiocon_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	if(ins_para)
	{
		config_radio(ins_para,rad_id,lpublic,lwlan);
	} 
  }  
  fprintf(cgiOut,"<form method=post>"\
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
          "<td width=62 align=center><input id=but type=submit name=radiocon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
          if(strcmp(f,"0")==0)
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtpdta.cgi?UN=%s&ID=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,t,ins_id,search(lpublic,"img_cancel"));
          else
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_radiolis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> Radio</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"));   /*突出显示*/
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
				    limit=29;
				  else
				  	limit=37;
                  for(i=0;i<limit;i++)
                  {
                    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
                  }     		  
				  if(ins_para)
				  {
					  result=show_radio_one(ins_para->parameter,ins_para->connection,rad_id,&radio);
					  result1=show_wireless_qos_profile_list(ins_para->parameter,ins_para->connection,&wqos);
					  result2=show_radio_qos_cmd_func(ins_para->parameter,ins_para->connection,&rad_qos_head);
					  result3=show_wlan_list(ins_para->parameter,ins_para->connection,&WLANINFO);
				  } 
				  if(result3 == 1)
				  {
				  	wnum = WLANINFO->wlan_num;
				  }

				  if(ins_para)
				  {				  	
					if((radio)&&(radio->RADIO[0]))
					{
						result5=show_wtp_one(ins_para->parameter,ins_para->connection,radio->RADIO[0]->WTPID,&wtp);
					}
				  } 
				  if(result5 == 1)
				  {
					  memset(wtp_sta,0,sizeof(wtp_sta));					  
					  if(wtp->WTP[0])
					  {
						  CheckWTPState(wtp_sta,wtp->WTP[0]->WTPStat);
					  }
				  }
				  if((strcmp(rt,"11gn")==0)||(strcmp(rt,"11g/gn")==0)||(strcmp(rt,"11b/g/n")==0)||(strcmp(rt,"11a/an")==0)||(strcmp(rt,"11an")==0))
				  	flag_11n=1;
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                      "<table width=735 border=0 cellspacing=0 cellpadding=0>"\
                        "<tr>"\
				          "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
				        fprintf(cgiOut,"</tr>"\
                		"<tr>"\
                          "<td><table width=735 border=0 cellspacing=0 cellpadding=0>"\
                               "<tr height=30 align=left>"\
                               "<td id=thead5 align=left>%s radio %d</td>",search(lpublic,"configure"),rad_id);		
                               fprintf(cgiOut,"</tr>"\
                        	   "</table>"\
                		  "</td>"\
                		"</tr>"\
                        "<tr><td align=center style=\"padding-left:20px\"><table width=735 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr height=30>"\
                    "<td width=60>%s:</td>",search(lwlan,"channel_state"));
                    fprintf(cgiOut,"<td width=75 align=left>"\
						"<select name=rad_channel_state id=rad_channel_state style=width:72px>"\
                           "<option value=>"\
						   "<option value=enable>auto"\
                           "<option value=disable>manual"\
                        "</select>"\
					"</td>"\
                    "<td align=left colspan=5><input type=submit style=\"width:100px; margin-left:5px\" border=0 name=default style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lpublic,"default"));
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"channel"));
                    fprintf(cgiOut,"<td width=75 align=left colspan=6>");
					  if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
						fprintf(cgiOut,"<input type=text name=rad_channel size=10 style=\"background-color:#cccccc\" disabled>");
					  else
						fprintf(cgiOut,"<input type=text name=rad_channel size=10 maxLength=4>");
					fprintf(cgiOut,"</td>"\
                  "</tr>"\
                  "<tr height=30>"\
					"<td>%s:</td>",search(lwlan,"tx_power_offset_step"));
					fprintf(cgiOut,"<td align=left>");
				  	if(!((result == 1)&&(radio->wlan_num>0)))/*如果radio没有绑定wlan*/
                    	fprintf(cgiOut,"<input type=text name=rad_txpower_offset_step size=10 style=\"background-color:#cccccc\" disabled>");
					else
						fprintf(cgiOut,"<input type=text name=rad_txpower_offset_step size=10 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" onpaste=\"return false\">");
					fprintf(cgiOut,"</td>"\
					"<td align=left colspan=5><font color=red>(>0)DBm</font></td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"tx_power_offset"));
					fprintf(cgiOut,"<td colspan=6 align=left>");
				  	if(((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))||(!((result == 1)&&(radio->wlan_num>0))))/*如果AP是run状态，且radio的管理状态是disable，或者radio没有绑定wlan*/
                    	fprintf(cgiOut,"<input type=text name=rad_txpower_offset size=10 style=\"background-color:#cccccc\" disabled>");
					else
						fprintf(cgiOut,"<input type=text name=rad_txpower_offset size=10 maxLength=3 onkeypress=\"return ((event.keyCode>=48&&event.keyCode<=57)||event.keyCode==45)\" onpaste=\"return false\">");
					fprintf(cgiOut,"</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"radio_mode"));
                    fprintf(cgiOut,"<td colspan=6 align=left>");
									 if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
                                       fprintf(cgiOut,"<select name=radio_mode id=radio_mode style=width:72px onchange=\"javascript:this.form.submit();\" disabled>");
									 else
									   fprintf(cgiOut,"<select name=radio_mode id=radio_mode style=width:72px onchange=\"javascript:this.form.submit();\">");									 
									 for(i=0;i<9;i++)
									 {
                                       if(strcmp(rType[i],rt)==0)
                                         fprintf(cgiOut,"<option value=%s selected=selected>%s",rType[i],rType[i]);
                					   else
                                         fprintf(cgiOut,"<option value=%s>%s",rType[i],rType[i]);
									 }
                                     fprintf(cgiOut,"</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"rate"));
					cgiFormSelectSingle("radio_mode", rType, 9, &ratetypeChoice, 2);
                    fprintf(cgiOut,"<td align=left width=100>");
							if(ratetypeChoice==0||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11a*/        
                            	fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=10 disabled>1(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=10>1(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==0||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11a*/        
                            	fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=20 disabled>2(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=20>2(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==0||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11a*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=55 disabled>5.5(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=55>5.5(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=60 disabled>6(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=60>6(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=90 disabled>9(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=90>9(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==0||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11a*/  
                            	fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=110 disabled>11(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=110>11(M/bps)</td>");
                  fprintf(cgiOut,"</tr>"\
                   "<tr height=30>"\
                    "<td>&nbsp;</td>"\
                            "<td align=left width=100>");
				  			if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=120 disabled>12(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=120>12(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=180 disabled>18(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=180>18(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=240 disabled>24(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=240>24(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=360 disabled>36(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=360>36(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=480 disabled>48(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\" name=\"vote\" value=480>48(M/bps)</td>");
                        	fprintf(cgiOut,"<td width=100>");
							if(ratetypeChoice==1||(flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))/*11b*/   
                            	fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=540 disabled>54(M/bps)</td>");
							else
								fprintf(cgiOut,"<input type=\"checkbox\"  name=\"vote\" value=540>54(M/bps)</td>");
                  fprintf(cgiOut,"</tr>"\
                  "<tr height=30>"\
                    "<td>Beacon:</td>"\
                    "<td align=left>");
				  	  if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
				  	    fprintf(cgiOut,"<input type=text name=rad_beacon size=10 style=\"background-color:#cccccc\" disabled>");
					  else
					  	fprintf(cgiOut,"<input type=text name=rad_beacon size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"</td>"\
                    "<td align=left colspan=5><font color=red>(25--1000)ms</font></td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"fragment"));
                    fprintf(cgiOut,"<td align=left>");
					  if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
					    fprintf(cgiOut,"<input type=text name=rad_fragment size=10 style=\"background-color:#cccccc\" disabled>");
					  else
					  	fprintf(cgiOut,"<input type=text name=rad_fragment size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"</td>"\
                    "<td align=left colspan=5><font color=red>(256--2346)byte</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td>Dtim:</td>"\
                    "<td align=left>");
					  if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
					    fprintf(cgiOut,"<input type=text name=rad_dtim size=10 style=\"background-color:#cccccc\" disabled></td>");
					  else
					  	fprintf(cgiOut,"<input type=text name=rad_dtim size=10 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
                    fprintf(cgiOut,"<td align=left colspan=5><font color=red>(1--15)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td>RTS:</td>"\
                    "<td align=left>");
					  if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
					    fprintf(cgiOut,"<input type=text name=rad_rts size=10 style=\"background-color:#cccccc\" disabled>");
					  else
					    fprintf(cgiOut,"<input type=text name=rad_rts size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"</td>"\
                    "<td align=left colspan=5><font color=red>(256--2347)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"pream"));
                    fprintf(cgiOut,"<td colspan=6 align=left>");
					                 if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
                                       fprintf(cgiOut,"<select name=radio_pream id=radio_pream style=width:72px disabled>");
									 else
									   fprintf(cgiOut,"<select name=radio_pream id=radio_pream style=width:72px>");
                                     if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->IsShortPreamble==1))
                                     {
                                       fprintf(cgiOut,"<option value=short selected=selected>short"\
                                       "<option value=long>long");
                                     }
                					 else
            						 {
                                       fprintf(cgiOut,"<option value=short>short"\
                                       "<option value=long selected=selected>long");
                                     }
                                     fprintf(cgiOut,"</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>ShortRetry:</td>"\
                    "<td align=left>");
					  if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
					    fprintf(cgiOut,"<input type=text name=rad_shortretry size=10 style=\"background-color:#cccccc\" disabled>");
					  else
					  	fprintf(cgiOut,"<input type=text name=rad_shortretry size=10 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"</td>"\
                    "<td align=left colspan=5><font color=red>(1--15)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td>LongRetry:</td>"\
                    "<td align=left>");
					  if((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2))   /*如果AP是run状态，且radio的管理状态是disable*/
					    fprintf(cgiOut,"<input type=text name=rad_longretry size=10 style=\"background-color:#cccccc\" disabled>");
					  else
					  	fprintf(cgiOut,"<input type=text name=rad_longretry size=10 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					fprintf(cgiOut,"</td>"\
                    "<td align=left colspan=5><font color=red>(1--15)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"service"));
                    fprintf(cgiOut,"<td colspan=6 align=left>"\
                                     "<select name=radio_service id=radio_service style=width:72px>");
                                      if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat== 2))
                                      {
                                        fprintf(cgiOut,"<option value=disable selected=selected>disable"\
                                        "<option value=enable>enable");
                                      }
                					  else
            						  {
                                        fprintf(cgiOut,"<option value=disable>disable"\
                                        "<option value=enable selected=selected>enable");
                                      }
                                     fprintf(cgiOut,"</select>"\
                    "</td>"\
                  "</tr>"\
                   "<tr height=30>"\
                    "<td>MAX%s:</td>",search(lwlan,"rate"));
                    fprintf(cgiOut,"<td align=left>");
								     if((flag_11n==1)||((strcmp(wtp_sta,"run")==0)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat==2)))
									 	fprintf(cgiOut,"<select name=max_rate id=max_rate style=width:72px; disabled>");
									 else
                                     	fprintf(cgiOut,"<select name=max_rate id=max_rate style=width:72px>");
                                         fprintf(cgiOut,"<option value=%s>%s"," "," ");
										if(flag_11n==1)
										{
											for(i=0;i<34;i++)
	                                         fprintf(cgiOut,"<option value=%d>%d",rRate2[i],rRate2[i]);
										}
										else
										{
											for(i=0;i<12;i++)
	                                         fprintf(cgiOut,"<option value=%d>%d",rRate1[i],rRate1[i]);
										}                            			
                                     fprintf(cgiOut,"</select>"\
                    "</td>"\
                    "<td align=left style=\"padding-left:3px\" colspan=5><font color=red>M/bps</font></td>"\
                  "</tr>"\
                    "<tr height=30>"\
                    "<td>%s QOS:</td>",search(lpublic,"bind"));
                    fprintf(cgiOut,"<td align=left colspan=6>"\
                                     "<select name=bind_qos id=bind_qos style=width:72px>");
                                         fprintf(cgiOut,"<option value=>");
										if(result1 == 1)
										{
											for(i=0;i<wqos->qos_num;i++)
											{
												if(wqos->qos[i])
												{
													fprintf(cgiOut,"<option value=%d>%d",wqos->qos[i]->QosID,wqos->qos[i]->QosID);
												}
											}
										}
                                     fprintf(cgiOut,"</select>"\
                    "</td>"\
                  "</tr>"\
                    "<tr height=30>");
                    if(strcmp(search(lpublic,"bind"),"Binding")==0)  /*英文界面*/
                      fprintf(cgiOut,"<td>Unbinding QOS:</td>");
            		else
                      fprintf(cgiOut,"<td>解除绑定 QOS:</td>");
                    fprintf(cgiOut,"<td align=left colspan=6>"\
						"<select name=unbind_qos id=unbind_qos style=width:72px>");
						fprintf(cgiOut,"<option value=>");
						if((result2==1)&&(rad_qos_head->qos_num>0))
						{
							for(i=0;i<rad_qos_head->qos_num;i++)
							{
								if((rad_qos_head->RADIO[i])&&(rad_qos_head->RADIO[i]->Radio_G_ID == rad_id)&&(rad_qos_head->RADIO[i]->QOSID > 0))
								{
									fprintf(cgiOut,"<option value=%d>%d",rad_qos_head->RADIO[i]->QOSID,rad_qos_head->RADIO[i]->QOSID);
								}
							}
						}
						fprintf(cgiOut,"</select>"\
					"</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"rad_max_throughout"));
                    fprintf(cgiOut,"<td align=left><input type=text name=rad_max_through size=10 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
                    "<td align=left colspan=5><font color=red>(1--108)M</font></td>"\
                  "</tr>"
				  "<tr height=30>"\
				  "<td>%s WLAN:</td>",search(lpublic,"bind"));
				  fprintf(cgiOut,"<td align=left>"\
								   "<select name=bind_wlan id=bind_wlan style=width:72px>");
									   fprintf(cgiOut,"<option value=>");
									if(result3 == 1)
									{
										for(i=0;i<wnum;i++)
										{
										  if(WLANINFO->WLAN[i])
										  {
											  fprintf(cgiOut,"<option value=%d>%d",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanID);
										  }
										}
									}
								   fprintf(cgiOut,"</select>"\
				  "</td>"\
				  "<td align=left colspan=5>VLAN ID:<input type=text name=wlan_vlan_id size=10 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(1--4094)</font></td>"\
				"</tr>"\
				"<tr height=30>"\
				  "<td>%s:</td>",search(lpublic,"l2_isolation"));
				  fprintf(cgiOut,"<td align=left>"\
								   "<select name=l2_iso_wlan id=l2_iso_wlan style=width:72px>");
									   fprintf(cgiOut,"<option value=>");
									if(result3 == 1)
									{
										for(i=0;i<wnum;i++)
										{
										  if(WLANINFO->WLAN[i])
										  {
											  fprintf(cgiOut,"<option value=%d>%d",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanID);
										  }
										}
									}
								   fprintf(cgiOut,"</select>"\
				  "</td>"\
				  "<td align=left colspan=5>"\
				    "<select name=l2_iso_state id=l2_iso_state style=width:72px>"\
				    	"<option value=>"\
                        "<option value=disable>disable"\
                        "<option value=enable>enable"\
                     "</select>"\
				  "</td>"\
				"</tr>"\
				"<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"auto_channel"));
                    fprintf(cgiOut,"<td colspan=6 align=left>"\
                                     "<select name=auto_channel id=auto_channel style=width:72px>"\
                                       "<option value=>"\
									   "<option value=disable>disable"\
                                       "<option value=enable>enable"\
                                     "</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"diversity"));
                    fprintf(cgiOut,"<td colspan=6 align=left>"\
                                     "<select name=diversity id=diversity style=width:72px>"\
									   "<option value=>"\
									   "<option value=disable>disable"\
									   "<option value=enable>enable"\
                                     "</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"txantenna"));
                    fprintf(cgiOut,"<td colspan=6 align=left>"\
                                     "<select name=txantenna id=txantenna style=width:72px>"\
                                       "<option value=>"\
                                       "<option value=auto>auto"\
                                       "<option value=main>main"\
                                       "<option value=vice>vice"\
                                     "</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr>"\
                    "<td id=sec1 colspan=4 style=padding-top:20px style=\"border-bottom:2px solid #53868b\">11N %s</td>",search(lpublic,"l_item"));
					fprintf(cgiOut,"<td colspan=3>&nbsp;</td>"\
                  "</tr>"\
				  "<tr height=30 style=padding-top:10px>"\
                    "<td>ampdu/amsdu %s:</td>",search(lpublic,"switch"));
					fprintf(cgiOut,"<td>");
									 if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/
                                       fprintf(cgiOut,"<select name=ampdu_type id=ampdu_type style=width:72px>");
                                     else
									   fprintf(cgiOut,"<select name=ampdu_type id=ampdu_type style=width:72px disabled>");
                                       fprintf(cgiOut,"<option value=>"\
                                       "<option value=ampdu>ampdu"\
                                       "<option value=amsdu>amsdu"\
                                     "</select>"\
				   "</td>"\
				   "<td colspan=5 align=left>");
									 if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/
                                       fprintf(cgiOut,"<select name=ampdu_switch id=ampdu_switch style=width:72px>");
                                     else
									   fprintf(cgiOut,"<select name=ampdu_switch id=ampdu_switch style=width:72px disabled>");
                                       fprintf(cgiOut,"<option value=>"\
                                       "<option value=enable>enable"\
                                       "<option value=disable>disable"\
                                     "</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>ampdu/amsdu %s:</td>",search(lwlan,"limit"));
					fprintf(cgiOut,"<td>");
					  if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/
                       fprintf(cgiOut,"<select name=limit_type id=limit_type style=width:72px>");
                     else
					   fprintf(cgiOut,"<select name=limit_type id=limit_type style=width:72px disabled>");
                       fprintf(cgiOut,"<option value=>"\
                       "<option value=ampdu>ampdu"\
                       "<option value=amsdu>amsdu"\
                     "</select>"\
					 "</td>"
					 "<td align=left>");
					  if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/
						fprintf(cgiOut,"<input type=text name=ampdu_limit size=10 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					  else
					    fprintf(cgiOut,"<input type=text name=ampdu_limit size=10 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" style=\"background-color:#cccccc\" disabled>");
					fprintf(cgiOut,"</td>"\
                    "<td align=left colspan=5><font color=red>(ampdu:1024--65535,amsdu:2290--4096)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td>ampdu/amsdu subframe:</td>");
					fprintf(cgiOut,"<td>");
					  if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/
                       fprintf(cgiOut,"<select name=subframe_type id=subframe_type style=width:72px>");
                     else
					   fprintf(cgiOut,"<select name=subframe_type id=subframe_type style=width:72px disabled>");
                       fprintf(cgiOut,"<option value=>"\
                       "<option value=ampdu>ampdu"\
                       "<option value=amsdu>amsdu"\
                     "</select>"\
					 "</td>"
					 "<td align=left>");
					  if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/
						fprintf(cgiOut,"<input type=subframe name=subframe size=10 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					  else
					    fprintf(cgiOut,"<input type=subframe name=subframe size=10 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" style=\"background-color:#cccccc\" disabled>");
					fprintf(cgiOut,"</td>"\
                    "<td align=left colspan=5><font color=red>(2--64)</font></td>"\
                  "</tr>"
                  "<tr height=30>"\
                    "<td>11n %s:</td>",search(lwlan,"work_mode"));
					fprintf(cgiOut,"<td>");
								  if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/
								    fprintf(cgiOut,"<select name=work_mode_wlan id=work_mode_wlan style=width:72px>");
								  else
								    fprintf(cgiOut,"<select name=work_mode_wlan id=work_mode_wlan style=width:72px disabled>");
								    fprintf(cgiOut,"<option value=>");
								    if(result == 1)
								    {
									  for(i=0;i<radio->wlan_num;i++)
									  {
									  	if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->WlanId))
									  	{
											fprintf(cgiOut,"<option value=%d>%d",radio->RADIO[0]->WlanId[i],radio->RADIO[0]->WlanId[i]);
									  	}
									  }
								    }
								    fprintf(cgiOut,"</select>"\
					"</td>"
                    "<td colspan=5 align=left>");
									 if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/ 
                                       fprintf(cgiOut,"<select name=11n_work_mode id=11n_work_mode style=width:72px>");
									 else
									   fprintf(cgiOut,"<select name=11n_work_mode id=11n_work_mode style=width:72px disabled>");
                                       fprintf(cgiOut,"<option value=>"\
                                       "<option value=puren>puren"\
                                       "<option value=mixed>mixed"\
                                     "</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"chainmask"));
					fprintf(cgiOut,"<td>");
									 if(flag_11n==1) 
                                       fprintf(cgiOut,"<select name=chainmask_type id=chainmask_type style=width:98px>");
									 else
									   fprintf(cgiOut,"<select name=chainmask_type id=chainmask_type style=width:98px disabled>");
                                       fprintf(cgiOut,"<option value=>"\
                                       "<option value=tx_chainmask>tx_chainmask"\
                                       "<option value=rx_chainmask>rx_chainmask"\
                                     "</select>"\
                    "</td>"\
                    "<td colspan=5 align=left>");
									 if(flag_11n==1) 
                                       fprintf(cgiOut,"<select name=chainmask id=chainmask style=width:72px>");
									 else
									   fprintf(cgiOut,"<select name=chainmask id=chainmask style=width:72px disabled>");
                                       fprintf(cgiOut,"<option value=>"\
                                       "<option value='1.0.0'>1.0.0"\
                                       "<option value='0.1.0'>0.1.0"\
                                       "<option value='1.1.0'>1.1.0"\
                                       "<option value='0.0.1'>0.0.1"\
                                       "<option value='1.0.1'>1.0.1"\
                                       "<option value='0.1.1'>0.1.1"\
                                       "<option value='1.1.1'>1.1.1"\
                                     "</select>"\
                    "</td>"\
                  "</tr>"\
                  "<tr height=30>"\
                    "<td>%s:</td>",search(lwlan,"cwmode_channel_offset"));
                    fprintf(cgiOut,"<td colspan=6 align=left>");									 
									 if((flag_11n==1)&&((result == 1)&&(radio->wlan_num>0)))/*如果radio是11n模式，且已经绑定WLAN*/ 									 
                                       fprintf(cgiOut,"<select name=cwmode_channel_offset id=cwmode_channel_offset style=width:98px>");
									 else
									   fprintf(cgiOut,"<select name=cwmode_channel_offset id=cwmode_channel_offset style=width:98px disabled>");
                                       fprintf(cgiOut,"<option value=>"\
                                       "<option value='ht20^'>ht20"\
                                       "<option value='ht20/40^up'>ht20/40 up"\
                                       "<option value='ht20/40^down'>ht20/40 down"\
                                       "<option value='ht40^up'>ht40 up"\
                                       "<option value='ht40^down'>ht40 down"\
                                     "</select>"\
                    "</td>"\
                  "</tr>"\
				  "<tr height=30>"\
				    "<td>ShortGI:</td>");
				    fprintf(cgiOut,"<td colspan=6 align=left>");
								   if(flag_11n==1) 
								     fprintf(cgiOut,"<select name=ShortGI_mode id=ShortGI_mode style=width:72px>");
								   else
								     fprintf(cgiOut,"<select name=ShortGI_mode id=ShortGI_mode style=width:72px disabled>");
									 fprintf(cgiOut,"<option value=>"\
									 "<option value=800>800ns"\
									 "<option value=400>400ns"\
								   "</select>"\
				    "</td>"\
				  "</tr>"\
			 	  "<tr>"\
                    "<td><input type=hidden name=encry_conradio value=%s></td>",m);
                    fprintf(cgiOut,"<td><input type=hidden name=radio_id value=%s></td>",n);		
                    fprintf(cgiOut,"<td><input type=hidden name=wtp_id value=%s></td>",t); 
                    fprintf(cgiOut,"<td><input type=hidden name=flag value=%s></td>",f);
                    fprintf(cgiOut,"<td><input type=hidden name=page_no value=%s></td>",pn);
					fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
					fprintf(cgiOut,"<td><input type=hidden name=txp value=%d></td>",0);
                  fprintf(cgiOut,"</tr>"\
				  "<tr>"\
					"<td colspan=7><input type=hidden name=instance_id value=%s></td>",ins_id);
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
if(result==1)
{
  Free_radio_one_head(radio);
}  	
if(result1==1)
{
  Free_qos_head(wqos);
}
if(result2==1)
{
  Free_show_radio_qos(rad_qos_head);
}
if(result3 == 1)
{
  Free_wlan_head(WLANINFO);
}
if(result5==1)
{
  Free_one_wtp_head(wtp);
}
return 0;
}

void config_radio(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)
{
  char rad_txpower_offset_step[10] = { 0 };
  char content[10] = { 0 };
  int cont = 0,ret = 0,flag = 1;
  char *endptr = NULL;  
  char temp[100] = { 0 };
  char qos_id[10] = { 0 };
  char bind_qos[10] = { 0 };
  char unbind_qos[10] = { 0 };
  char maxThrouth[10] = { 0 };
  char bind_wlan[10] = { 0 };
  char vlan_id[10] = { 0 };
  char vid[10] = { 0 };
  char l_bss_num[10] = { 0 };
  char wlan_id[5] = { 0 };
  char state[10] = { 0 };
  char auto_channel[10] = { 0 };
  char diversity[10] = { 0 };
  char txantenna[10] = { 0 };  
  char max_wlan_num[10] = { 0 };
  char ntxp[10] = { 0 };
  char ampdu_type[10] = { 0 };
  char ampdu_switch[10] = { 0 };
  char limit_type[10] = { 0 };
  char ampdu_limit[10] = { 0 };
  char subframe_type[10] = { 0 };
  char subframe[10] = { 0 };
  char work_mode_wlan[10] = { 0 };
  char work_mode[10] = { 0 };
  char channel_offset[10] = { 0 };
  char chainmask_type[15] = { 0 };
  char chainmask[10] = { 0 };
  char cwmode[10] = { 0 };
  char max_radio_num[10] = { 0 };
  char ShortGI_mode[10] = { 0 };
  char cwmode_channel_offset[20] = { 0 };
  struct RadioList *RadioList_Head = NULL;

  /****************config radio channel state****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_channel_state",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))    /*radio channel不为空时*/  
  {
   	 ret=set_ap_radio_auto_channel_cont_cmd(ins_para->parameter,ins_para->connection,id,content);/*返回0表示失败，返回1表示成功*/
																								/*返回-1表示input patameter only with 'enable' or 'disable'*/
																								/*返回-2表示wtp id does not exist*/
																								/*返回-3表示radio id does not exist*，返回-4表示error*/
	switch(ret)
	{
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
	   			flag=0;
				ShowAlert(search(lwlan,"con_chan_state_fail"));
				break;
			  }
	   case 1:break;   
	   case -1:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"input_illegal"));			   
				 break; 
			   }
	   case -2:{
	   			 flag=0;
				 ShowAlert(search(lwlan,"wtp_not_exist"));				 
				 break; 
			   }
	   case -3:{
	   			 flag=0;
				 ShowAlert(search(lwlan,"radio_not_exist"));			   
				 break; 
			   }
	   case -4:{
	   			 flag=0;
				 ShowAlert(search(lpublic,"error"));			   
				 break; 
			   }
	 }
   }

  /****************config radio channel****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_channel",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))    /*radio channel不为空时*/  
  {
      ret= config_radio_channel(ins_para->parameter,ins_para->connection,id,content);/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
																				    /*返回-2表示Radio is disable,  please enable it first，返回-3表示error*/
																				    /*返回-4表示channel  is invalid in CHINA，返回-5表示channel  is invalid in EUROPE*/
																					/*返回-6表示channel  is invalid in USA，返回-7表示channel  is invalid in JAPAN*/
																				 	/*返回-8表示channel  is invalid in FRANCE，返回-9表示channel  is invalid in SPAIN*/
																				    /*返回-10表示input parameter error，返回-11表示11a receive channel list is:  36 ..;149 153 157 161*/
																				    /*返回-12表示radio type doesn,t support this channel，返回-13表示error，返回-14表示Radio ID非法*/  
																					/*返回-15表示illegal input:Input exceeds the maximum value of the parameter type*/
      switch(ret)               
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
                 flag=0;
                 ShowAlert(search(lwlan,"con_chan_fail"));
                 break;
               }
        case 1:break;
        case -1:{
                  flag=0;
                  ShowAlert(search(lwlan,"radio_id_not_exist"));
                  break; 
        	    }
        case -2:{
                  flag=0;
                  ShowAlert(search(lwlan,"enable_radio"));
                  break; 
        	    }
        case -3:{
                  flag=0;
                  ShowAlert(search(lpublic,"error"));
                  break;
                }
        case -4:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"channel_invaild_c"));
                    break;
        		}
        case -5:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"channel_invaild_e"));
                    break;
        			}
        case -6:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"channel_invaild_u"));
                    break;
        			}
        case -7:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"channel_invaild_j"));
                    break;
        			}
        case -8:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"channel_invaild_f"));
                    break;
        			}
        case -9:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"channel_invaild_s"));
                    break;
        			}
		case -10:{
                    flag=0;
                    ShowAlert(search(lpublic,"input_para_error"));
                    break;
                	}
		case -11:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"11a_rec_chan_list"));
                    break;
    			}
		case -12:
        		{
            		flag=0;
                     ShowAlert(search(lwlan,"rad_type_dont_supprot_this_channel"));
                    break;
    			}
		case -13:{
                    flag=0;
                    ShowAlert(search(lpublic,"error"));
                    break;
            	 }
		case -14:{
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
		case -15:{
                    flag=0;
                    ShowAlert(search(lpublic,"input_exceed_max_value"));
                    break;
            	 }
      }   
  }
  
  /****************config radio mode****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("radio_mode",content,10);
  if(strcmp(content,"")!=0)   
  {
	  ret= config_radio_mode(ins_para->parameter,ins_para->connection,id,content);
	  switch(ret)				/*返回0表示失败，返回1表示成功，返回-1表示mode非法，返回-2表示radio id 不存在，返回-3表示Radio is disable,	please enable it first，返回-4表示radio mode not allow to set with 11n，返回-5表示出错*/
	  {
	    case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
				 ShowAlert(search(lwlan,"con_mode_fail"));
				 break;
			   }
		case 1:break;
		case -1:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_mode_illegal"));
				  break; 
				}
		case -2:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_id_not_exist"));
				  break; 
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lwlan,"enable_radio"));
				  break;
				}
		case -4:{
				  flag=0;
				  ShowAlert(search(lwlan,"mode_not_allow_set_11n"));
				  break;
				}
		case -5:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
	  }
  }
  

  /****************config radio txpower offset step****************/
  memset(rad_txpower_offset_step,0,sizeof(rad_txpower_offset_step));
  cgiFormStringNoNewlines("rad_txpower_offset_step",rad_txpower_offset_step,10);
  if((strcmp(rad_txpower_offset_step,"")!=0)&&(strchr(rad_txpower_offset_step,' ')==NULL))
  {
	ret= set_radio_txpowerstep_cmd(ins_para->parameter,ins_para->connection,id,rad_txpower_offset_step);  
	switch(ret) 			
	{
	  case SNMPD_CONNECTION_ERROR:
	  case 0:{
			   flag=0;
			   ShowAlert(search(lwlan,"con_txp_off_step_fail"));
			   break;
			 }
	  case 1:break;
	  case -1:{
				flag=0;
				ShowAlert(search(lpublic,"input_exceed_max_value"));
				break; 
			  }
	  case -2:{
				flag=0;
				ShowAlert(search(lpublic,"unknown_id_format"));
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
				ShowAlert(search(lwlan,"radio_id_not_exist"));
				break; 
			  } 	  
	  case -5:{
				flag=0;
				ShowAlert(search(lwlan,"rad_dont_bind_wlan"));
				break;
			  }
	  case -9:{
				flag=0;
				ShowAlert(search(lpublic,"input_para_illegal"));
				break;
			  }
	}	
  }

  /****************config radio txpower offset****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_txpower_offset",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))     /*radio txpower不为空时*/  
  {
	cont = strtoul(content,&endptr,10); 
	ret= config_radio_txpower_offset(ins_para->parameter,ins_para->connection,id,cont);  
	switch(ret) 			
	{
	  case SNMPD_CONNECTION_ERROR:
	  case 0:{
			   flag=0;
			   ShowAlert(search(lwlan,"con_txp_off_fail"));
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
				ShowAlert(search(lwlan,"txp_conflict_country_code"));
				break; 
			  } 			 
	  case -3:{
				flag=0;
				ShowAlert(search(lwlan,"radio_id_not_exist"));
				break; 
			  } 	  
	  case -4:{
				flag=0;
				ShowAlert(search(lwlan,"enable_radio"));
				break;
			  }
	  case -5:{
				flag=0;
				ShowAlert(search(lwlan,"11n_not_allow_set_txpower"));
				break;
			  }
	  case -6:{
				flag=0;
				ShowAlert(search(lwlan,"max_txp_20"));
				break;
			  }
	  case -7:{
				flag=0;
				ShowAlert(search(lwlan,"max_txp_27"));
				break;
			  }
	  case -8:{
				flag=0;
				ShowAlert(search(lwlan,"wtp_not_run"));
				break;
			  }
	  case -9:{
				flag=0;
				ShowAlert(search(lwlan,"rad_dont_bind_wlan"));
				break;
			  }
	  case -10:{
				 flag=0;
				 ShowAlert(search(lpublic,"error"));
				 break;
			   }
	  case -11:{
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
	  case -15:{
				 flag=0;
				 ShowAlert(search(lwlan,"checkout_txpowerstep"));
				 break;
			   }
	}	
  }

   /****************config radio rate****************/
   char **responses;
   int result = cgiFormNotFound;  
   char con[50] = { 0 };
   memset(con,0,sizeof(con));
   result = cgiFormStringMultiple("vote", &responses);
    if (result != cgiFormNotFound) 
     {
        int i = 0;
        while (responses[i]) {
            if(i>0)
        		{
                    strncat(con,",",sizeof(con)-strlen(con)-1);
        		}
                strncat(con,responses[i],sizeof(con)-strlen(con)-1);
        	i++;
    	}
    }
	
  if(strcmp(con,"")!=0)
  {
	  ret= config_radio_rate(ins_para->parameter,ins_para->connection,id,con);   /*返回0表示失败，返回1表示成功，返回-1表示radio id does not exist，返回-2表示mode 11b support rate list:10 20 55 110*/
											                                    /*返回-3表示mode 11a support rate list:60 90 120 180 240 360 480 540，返回-4表示mode 11g support rate list:60 90 120 180 240 360 480 540*/
																				/*返回-5表示mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540，返回-6表示wtp radio does not support this rate,please check first*/	
																				/*返回-7表示radio is disable, please enable it first，返回-8表示radio list is empty，返回-9表示radio mode is 11n,not allow to set rate*/
																				/*返回-10表示radio support rate does not exist，返回-11表示radio type is conflict, please check it first，返回-12表示出错，返回-13表示Radio ID非法*/
																				/*返回-14表示mode 11an support rate list:60 90 120 180 240 360 480 540，返回-15表示mode 11gn support rate list:60 90 120 180 240 360 480 540*/
																				/*返回-16表示mode 11a/an support rate list:60 90 120 180 240 360 480 540，返回-17表示mode 11g/gn support rate list:60 90 120 180 240 360 480 540*/
																				/*返回-18表示mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
	    case 0:{
	             flag=0;
	             ShowAlert(search(lwlan,"con_rate_fail"));
	             break;
	           }
	    case 1:break;
	    case -1:{
	              flag=0;
	              ShowAlert(search(lwlan,"radio_id_not_exist"));
	              break; 
	            }
	    case -2:{
	              flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				  strncat(temp,"11b",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				  strncat(temp,"10 20 55 110",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	              ShowAlert(temp);
	              break; 
	            }
	    case -3:{
	              flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				  strncat(temp,"11a",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				  strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	              ShowAlert(temp);
	              break; 
	            }
	    case -4:{
	              flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				  strncat(temp,"11g",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				  strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	              ShowAlert(temp);
	              break; 
	            }
	    case -5:{
	              flag=0;
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				  strncat(temp,"11b/g",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				  strncat(temp,"10 20 55 60 90 110 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	              ShowAlert(temp);
	              break; 
	            }
		case -6:{
	              flag=0;
	              ShowAlert(search(lwlan,"radio_dont_sup_this_rate"));
	              break;
	            }
	    case -7:{
	              flag=0;
	              ShowAlert(search(lwlan,"enable_radio"));
	              break;
	            }
	    case -8:{
	              flag=0;
	              ShowAlert(search(lwlan,"radio_emp"));
	              break;
	            }
		case -9:{
	              flag=0;
	              ShowAlert(search(lwlan,"11n_not_allow_set_rate"));
	              break;
	            }
	    case -10:{
	              flag=0;
	              ShowAlert(search(lwlan,"radio_rate_illegal"));
	              break;
	            }
	    case -11:{
	              flag=0;
	              ShowAlert(search(lwlan,"radio_type_conflict"));
	              break;
	            }
	    case -12:{
	              flag=0;
	              ShowAlert(search(lpublic,"error"));
	              break;	
	            }
		case -13:{
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
		case -14:{
	               flag=0;
				   memset(temp,0,sizeof(temp));
				   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				   strncat(temp,"11an",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	               ShowAlert(temp);
	               break; 
	             }
		case -15:{
	               flag=0;
				   memset(temp,0,sizeof(temp));
				   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				   strncat(temp,"11gn",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	               ShowAlert(temp);
	               break; 
	             }
		case -16:{
	               flag=0;
				   memset(temp,0,sizeof(temp));
				   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				   strncat(temp,"11a/an",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	               ShowAlert(temp);
	               break; 
	             }
		case -17:{
	               flag=0;
				   memset(temp,0,sizeof(temp));
				   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				   strncat(temp,"11g/gn",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	               ShowAlert(temp);
	               break; 
	             }
		case -18:{
	               flag=0;
				   memset(temp,0,sizeof(temp));
				   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
				   strncat(temp,"11b/g/n",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
				   strncat(temp,"10 20 60 90 110 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
				   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	               ShowAlert(temp);
	               break; 
	             }
	  }
}
  cgiStringArrayFree(responses);

  /****************config radio beacon****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_beacon",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))    /*radio beacon不为空时*/  
  {
    cont= strtoul(content,&endptr,10);    /*char转成int，10代表十进制*/		
    if((cont>24)&&(cont<1001))             /*检查beacon合法性*/
    {      
      ret= config_radio_beaconinterval(ins_para->parameter,ins_para->connection,id,cont);/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
      switch(ret)               
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
                 flag=0;
                 ShowAlert(search(lwlan,"con_beacon_fail"));
                 break;
               }
        case 1:break;
        case -1:{
                  flag=0;
                  ShowAlert(search(lwlan,"radio_id_not_exist"));
                  break; 
        	    }
        case -2:{
                  flag=0;
                  ShowAlert(search(lwlan,"enable_radio"));
                  break; 
        	    }
        case -3:{
                  flag=0;
                  ShowAlert(search(lpublic,"error"));
                  break;
                }
      }   
    }
    else
    {
      flag=0;
      ShowAlert(search(lwlan,"radio_beacon_illegal"));
    }
  }

  /****************config radio fragment****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_fragment",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))   /*radio fragment不为空时*/  
  {
    cont= strtoul(content,&endptr,10);    /*char转成int，10代表十进制*/		
    if((cont>255)&&(cont<2347))             /*检查fragment合法性*/
    {      
      ret= config_radio_fragmentation(ins_para->parameter,ins_para->connection,id,cont); /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
      switch(ret)               
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
                 flag=0;
                 ShowAlert(search(lwlan,"con_fragment_fail"));
                 break;
               }
        case 1:break;
        case -1:{
                  flag=0;
                  ShowAlert(search(lwlan,"radio_id_not_exist"));
                  break; 
        	    }
        case -2:{
                  flag=0;
                  ShowAlert(search(lwlan,"enable_radio"));
                  break; 
        	    }
        case -3:{
                  flag=0;
                  ShowAlert(search(lpublic,"error"));
                  break;
                }
      }   
    }
    else
    {
      flag=0;
      ShowAlert(search(lwlan,"radio_fragment_illegal"));
    }
  }

  /****************config radio dtim****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_dtim",content,10);   
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))    /*radio dtim不为空时*/  
  {
    cont= strtoul(content,&endptr,10);    /*char转成int，10代表十进制*/		
    if((cont>0)&&(cont<16))             /*检查dtim合法性*/
    {      
      ret= config_radio_dtim(ins_para->parameter,ins_para->connection,id,cont);/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
      switch(ret)              
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
                 flag=0;
                 ShowAlert(search(lwlan,"con_dtim_fail"));
                 break;
               }
        case 1:break;
        case -1:{
                  flag=0;
                  ShowAlert(search(lwlan,"radio_id_not_exist"));
                  break; 
        	    }
        case -2:{
                  flag=0;
                  ShowAlert(search(lwlan,"enable_radio"));
                  break; 
        	    }
        case -3:{
                  flag=0;
                  ShowAlert(search(lpublic,"error"));
                  break;
                }
      }   
    }
    else
    {
      flag=0;
      ShowAlert(search(lwlan,"radio_dtim_illegal"));
    }
  }

  /****************config radio rts****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_rts",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))    /*radio rts不为空时*/  
  {
    cont= strtoul(content,&endptr,10);    /*char转成int，10代表十进制*/		
    if((cont>255)&&(cont<2348))             /*检查rts合法性*/
    {      
      ret= config_radio_rtsthreshold(ins_para->parameter,ins_para->connection,id,cont);  /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
      switch(ret)               
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
                 flag=0;
                 ShowAlert(search(lwlan,"con_rts_fail"));
                 break;
               }
        case 1:break;
        case -1:{
                  flag=0;
                  ShowAlert(search(lwlan,"radio_id_not_exist"));
                  break; 
        	    }
        case -2:{
                  flag=0;
                  ShowAlert(search(lwlan,"enable_radio"));
                  break; 
        	    }
        case -3:{
                  flag=0;
                  ShowAlert(search(lpublic,"error"));
                  break;
                }
      }   
    }
    else
    {
      flag=0;
      ShowAlert(search(lwlan,"radio_rts_illegal"));
    }
  }

  /****************config radio pream****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("radio_pream",content,10);
  if(strcmp(content,"")!=0)
  {
	  ret= config_radio_preamble(ins_para->parameter,ins_para->connection,id,content);  /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
	  switch(ret)			   
	  {
	    case SNMPD_CONNECTION_ERROR:
		case 0:{
				 flag=0;
				 ShowAlert(search(lwlan,"con_pream_fail"));
				 break;
			   }
		case 1:break;
		case -1:{
				  flag=0;
				  ShowAlert(search(lwlan,"radio_id_not_exist"));
				  break; 
				}
		case -2:{
				  flag=0;
				  ShowAlert(search(lwlan,"enable_radio"));
				  break;
				}
		case -3:{
				  flag=0;
				  ShowAlert(search(lpublic,"error"));
				  break;
				}
	  }
  }

  /****************config radio shortRetry****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_shortretry",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))    /*radio shortretry不为空时*/  
  {
    cont= strtoul(content,&endptr,10);    /*char转成int，10代表十进制*/		
    if((cont>0)&&(cont<16))             /*检查shortretry合法性*/
    {      
      ret= config_radio_shortretry(ins_para->parameter,ins_para->connection,id,cont);  /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
      switch(ret)              
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
                 flag=0;
                 ShowAlert(search(lwlan,"con_shortretry_fail"));
                 break;
               }
        case 1:break;
        case -1:{
                  flag=0;
                  ShowAlert(search(lwlan,"radio_id_not_exist"));
                  break; 
        	    }
        case -2:{
                  flag=0;
                  ShowAlert(search(lwlan,"enable_radio"));
                  break; 
        	    }
        case -3:{
                  flag=0;
                  ShowAlert(search(lpublic,"error"));
                  break;
                }
      }   
    }
    else
    {
      flag=0;
      ShowAlert(search(lwlan,"radio_shortretry_illegal"));
    }
  }

  /****************config radio longRetry****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("rad_longretry",content,10);
  if((strcmp(content,"")!=0)&&(strchr(content,' ')==NULL))   /*radio shortretry不为空时*/  
  {
    cont= strtoul(content,&endptr,10);    /*char转成int，10代表十进制*/		
    if((cont>0)&&(cont<16))             /*检查shortretry合法性*/
    {      
      ret= config_radio_longretry(ins_para->parameter,ins_para->connection,id,cont); /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
      switch(ret)              
      {
        case SNMPD_CONNECTION_ERROR:
        case 0:{
                 flag=0;
                 ShowAlert(search(lwlan,"con_longretry_fail"));
                 break;
               }
        case 1:break;
        case -1:{
                  flag=0;
                  ShowAlert(search(lwlan,"radio_id_not_exist"));
                  break; 
        	    }
        case -2:{
                  flag=0;
                  ShowAlert(search(lwlan,"enable_radio"));
                  break; 
        	    }
        case -3:{
                  flag=0;
                  ShowAlert(search(lpublic,"error"));
                  break;
                }
      }   
    }
    else
    {
      flag=0;
      ShowAlert(search(lwlan,"radio_longretry_illegal"));
    }
  }

  /****************config radio service****************/
  memset(content,0,sizeof(content));
  cgiFormStringNoNewlines("radio_service",content,10);
  ret= config_radio_service(ins_para->parameter,ins_para->connection,id,content);  /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示wtp not in run state，返回-3表示出错*/
  switch(ret)              
  {
    case SNMPD_CONNECTION_ERROR:
    case 0:{
             flag=0;
             ShowAlert(search(lwlan,"con_radio_service_fail"));
             break;
           }
    case 1:break;
    case -1:{
              flag=0;
              ShowAlert(search(lwlan,"radio_id_not_exist"));
              break; 
            }
    case -2:{
              flag=0;
              ShowAlert(search(lwlan,"wtp_not_run"));
              break; 
            }
    case -3:{
              flag=0;
              ShowAlert(search(lpublic,"error"));
              break;
            }
  }

  /****************config max rate****************/
   memset(content,0,sizeof(content));
   cgiFormStringNoNewlines("max_rate",content,10);
   if(strcmp(content,""))
    { 
        ret= config_max_rate(ins_para->parameter,ins_para->connection,id,content,&RadioList_Head);
        switch(ret)				
        {
            case 0:{
                	 flag=0;
                     ShowAlert(search(lwlan,"con_radio_maxrate_fail"));
                	 break;
        		   }
            case 1:break;
			case -1:{
					  flag=0;
					  ShowAlert(search(lwlan,"rate_illegal"));
					  break; 
					}
			case -2:{
					  flag=0;
					  ShowAlert(search(lwlan,"radio_id_not_exist"));
					  break; 
					}
			case -3:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					  strncat(temp,"11b",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					  strncat(temp,"10 20 55 110",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  break; 
					}
			case -4:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					  strncat(temp,"11a",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					  strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  break; 
					}
			case -5:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					  strncat(temp,"11g",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					  strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  break; 
					}
			case -6:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					  strncat(temp,"11b/g",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					  strncat(temp,"10 20 55 60 90 110 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  break; 
					}			
			case -7:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					  strncat(temp,"11b/g/n",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					  strncat(temp,"10 20 60 90 110 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  break; 
					}
			case -8:{
					  flag=0;
					  ShowAlert(search(lwlan,"radio_dont_sup_this_rate"));
					  break;
					}			
			case -9:{
					  flag=0;
					  ShowAlert(search(lwlan,"enable_radio"));
					  break;
					}			
			case -10:{
					   flag=0;
					   ShowAlert(search(lwlan,"wtp_not_bind_wlan"));
					   break;
					 }
			case -11:{
					   flag=0;
					   ShowAlert(search(lwlan,"radio_emp"));
					   break;
					 }
			case -12:{
					   flag=0;
					   ShowAlert(search(lwlan,"support_rate_not_exist"));
					   break;
					 }
			case -13:{
					   flag=0;
					   ShowAlert(search(lwlan,"radio_type_conflict"));
					   break;
					 }
			case -14:{
					   flag=0;
					   ShowAlert(search(lpublic,"error"));
					   break;	
					 }
			case -15:{
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
			case -16:{
					   flag=0;
					   memset(temp,0,sizeof(temp));
					   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					   strncat(temp,"11an",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					   ShowAlert(temp);
					   break; 
					 }
			case -17:{
					   flag=0;
					   memset(temp,0,sizeof(temp));
					   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					   strncat(temp,"11gn",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					   ShowAlert(temp);
					   break; 
					 }
			case -18:{
					   flag=0;
					   memset(temp,0,sizeof(temp));
					   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					   strncat(temp,"11a/an",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					   ShowAlert(temp);
					   break; 
					 }
			case -19:{
					   flag=0;
					   memset(temp,0,sizeof(temp));
					   strncpy(temp,search(lwlan,"model_sup_rate1"),sizeof(temp)-1);
					   strncat(temp,"11g/gn",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"model_sup_rate2"),sizeof(temp)-strlen(temp)-1);
					   strncat(temp,"60 90 120 180 240 360 480 540",sizeof(temp)-strlen(temp)-1);
					   strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
					   ShowAlert(temp);
					   break; 
					 }
        }
    } 

  /****************radio apply qos****************/
   memset(bind_qos,0,sizeof(bind_qos));
   cgiFormStringNoNewlines("bind_qos",bind_qos,10);
   if(strcmp(bind_qos,""))
   {
     ret=radio_apply_qos(ins_para->parameter,ins_para->connection,id,bind_qos);
     switch(ret)
     {
       case SNMPD_CONNECTION_ERROR:
       case 0:{
            	flag=0;
                ShowAlert(search(lwlan,"radio_apply_qos_fail"));
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
                 memset(temp,0,sizeof(temp));
                 strncpy(temp,search(lwlan,"qos_id_1"),sizeof(temp)-1);
                 memset(qos_id,0,sizeof(qos_id));
                 snprintf(qos_id,sizeof(qos_id)-1,"%d",QOS_NUM-1);
                 strncat(temp,qos_id,sizeof(temp)-strlen(temp)-1);
                 strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
                 ShowAlert(temp);
            	 break;
        	   }
       case -3:{
            	 flag=0;
                 ShowAlert(search(lwlan,"radio_id_not_exist"));
            	 break;
        	   }
       case -4:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wqos_not_exist"));
            	 break;
        	   }
     }
   }

  /****************radio delete qos****************/
   memset(unbind_qos,0,sizeof(unbind_qos));
   cgiFormStringNoNewlines("unbind_qos",unbind_qos,10);
   if(strcmp(unbind_qos,""))
   {
     ret=radio_delete_qos(ins_para->parameter,ins_para->connection,id,unbind_qos); 
     switch(ret)
     {
       case SNMPD_CONNECTION_ERROR:
       case 0:{
            	flag=0;
                ShowAlert(search(lwlan,"radio_delete_qos_fail"));
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
                 memset(temp,0,sizeof(temp));
                 strncpy(temp,search(lwlan,"qos_id_1"),sizeof(temp)-1);
                 memset(qos_id,0,sizeof(qos_id));
                 snprintf(qos_id,sizeof(qos_id)-1,"%d",QOS_NUM-1);
                 strncat(temp,qos_id,sizeof(temp)-strlen(temp)-1);
                 strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
                 ShowAlert(temp);
            	 break;
        	   }
       case -3:{
            	 flag=0;
                 ShowAlert(search(lwlan,"radio_id_not_exist"));
            	 break;
        	   }
       case -4:{
            	 flag=0;
                 ShowAlert(search(lwlan,"wqos_not_exist"));
            	 break;
        	   }
       case -5:{
            	 flag=0;
                 ShowAlert(search(lwlan,"dis_radio"));
            	 break;
        	   }
     }
   }

   

   /****************config radio max throughout****************/
   memset(maxThrouth,0,sizeof(maxThrouth));
   cgiFormStringNoNewlines("rad_max_through",maxThrouth,10);
   if(strcmp(maxThrouth,"")!=0) 
   {
   	 ret=set_radio_max_throughout_func(ins_para->parameter,ins_para->connection,id,maxThrouth);  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																								/*返回-2表示max throughout should be 1 to 108，返回-3表示WTP id does not exist*/
																								/*返回-4表示radio id does not exist，返回-5表示error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
	   			flag=0;
				ShowAlert(search(lwlan,"con_max_through_fail"));
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
				 ShowAlert(search(lwlan,"rad_max_throughout_illegal")); 			 
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
				 ShowAlert(search(lpublic,"error"));			   
				 break; 
			   }
	 }
   }

   
   /****************radio apply wlan  &&   radio apply wlan base vlan****************/
   memset(bind_wlan,0,sizeof(bind_wlan));
   cgiFormStringNoNewlines("bind_wlan",bind_wlan,10);   
   memset(vlan_id,0,sizeof(vlan_id));
   cgiFormStringNoNewlines("wlan_vlan_id",vlan_id,10);   
   if(strcmp(bind_wlan,""))
   {
     if(strcmp(vlan_id,"")==0)
     {
		 ret=radio_apply_wlan(ins_para->parameter,ins_para->connection,id,bind_wlan);
		 switch(ret)
		 {
		   case SNMPD_CONNECTION_ERROR:
		   case 0:{
					flag=0;
					ShowAlert(search(lwlan,"radio_apply_wlan_fail"));
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
					 ShowAlert(search(lwlan,"radio_not_exist"));
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
	                 strncpy(temp,search(lwlan,"bss_num_exhaust1"),sizeof(temp)-1);
					 memset(l_bss_num,0,sizeof(l_bss_num));
					 snprintf(l_bss_num,sizeof(l_bss_num)-1,"%d",L_BSS_NUM);
                 	 strncat(temp,l_bss_num,sizeof(temp)-strlen(temp)-1);
	                 strncat(temp,search(lwlan,"bss_num_exhaust2"),sizeof(temp)-strlen(temp)-1);
	                 ShowAlert(temp);
					 break;
				   }
		   case -6:{
					 flag=0;
					 ShowAlert(search(lwlan,"interface_not_match"));
					 break;
				   }
		   case -7:{
					 flag=0;
					 ShowAlert(search(lwlan,"wtp_dont_bind_inter"));
					 break;
				   }
		   case -8:{
					 flag=0;
					 ShowAlert(search(lwlan,"wlan_dont_bind_inter"));
					 break;
				   }
		   case -9:{
					 flag=0;
					 ShowAlert(search(lwlan,"wlan_create_bridge_fail"));
					 break;
				   }
		   case -10:{
					  flag=0;
					  ShowAlert(search(lwlan,"add_bss_inter_to_wlanbr_fail"));
					  break;
				    }
		   case -11:{
					  flag=0;
					  memset(temp,0,sizeof(temp));
	                  strncpy(temp,search(lwlan,"wtp_over_wep_wlan_count1"),sizeof(temp)-1);
                 	  strncat(temp,"4",sizeof(temp)-strlen(temp)-1);
	                  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	                  ShowAlert(temp);
					  break;
				    }
		   case -12:{
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
		   case -13:{
					  flag=0;
					  ShowAlert(search(lpublic,"input_exceed_max_value"));
					  break;
				    }
		   case -14:{
					  flag=0;
					  ShowAlert(search(lwlan,"secIndex_is_same_with_other"));
					  break;
				    }
		   case -18:{
					  flag=0;
					  ShowAlert(search(lwlan,"radio_has_bind_wlan"));
					  break;
				    }
		 }
     }
	 else
	 {
	 	ret=radio_apply_wlan_base_vlan_cmd(ins_para->parameter,ins_para->connection,id,bind_wlan,vlan_id);	/*返回表示失败，返回1表示成功，返回-1表示input parameter error*/
																											/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示vlan id should be 1 to VLANID_RANGE_MAX*/
																											/*返回-4表示radio id does not exist，返回-5表示wtp is in use, you should unused it first*/
																											/*返回-6表示binding wlan does not exist，返回-7表示wlan does not bind interface*/
																											/*返回-8表示wtp does not bind interface，返回-9表示wlan and wtp bind interface did not match*/
																											/*返回-10表示clear wtp binding wlan list successfully，返回-11表示wlan is enable,you should disable it first*/
																											/*返回-12表示wtp over max bss count，返回-13表示bss is enable, you should disable it first*/
																											/*返回-14表示wtp over max wep wlan count 4，返回-15表示error，返回-16表示Radio ID非法*/
																											/*返回-17表示illegal input:Input exceeds the maximum value of the parameter type*/
																											/*返回-18表示radio apply bingding securityindex is same with other*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:{
				   flag=0;
				   ShowAlert(search(lwlan,"radio_apply_wlan_fail"));
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
	                strncpy(temp,search(lwlan,"vlan_id_1to"),sizeof(temp)-1);
					memset(vid,0,sizeof(vid));
					snprintf(vid,sizeof(vid)-1,"%d",VLANID_RANGE_MAX);
                 	strncat(temp,vid,sizeof(temp)-strlen(temp)-1);
	                strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
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
					ShowAlert(search(lwlan,"unuse_wtp"));
					break;
				  }
		  case -6:{
					 flag=0;
					 ShowAlert(search(lwlan,"wlan_not_exist"));
					 break;
				  }
		  case -7:{
					 flag=0;
					 ShowAlert(search(lwlan,"wlan_dont_bind_inter"));
					 break;
				  }
		  case -8:{
					 flag=0;
					 ShowAlert(search(lwlan,"wtp_dont_bind_inter"));
					 break;
				  }
		  case -9:{
					 flag=0;
				     ShowAlert(search(lwlan,"interface_not_match"));
					 break;
				  }
		  case -10:{
				      flag=0;
					  ShowAlert(search(lwlan,"clear_bind_wlan_list_succ"));
					  break;
	  			   }
		  case -11:{
				      flag=0;
					  ShowAlert(search(lwlan,"dis_wlan"));
					  break;
	  			   }
		  case -12:{
				      flag=0;
					  ShowAlert(search(lwlan,"wtp_over_bss_count"));
					  break;
	  			   }
		  case -13:{
				      flag=0;
					  ShowAlert(search(lwlan,"unuse_bss"));
					  break;
	  			   }
		  case -14:{
				      flag=0;
					  memset(temp,0,sizeof(temp));
	                  strncpy(temp,search(lwlan,"wtp_over_wep_wlan_count1"),sizeof(temp)-1);
	             	  strncat(temp,"4",sizeof(temp)-strlen(temp)-1);
	                  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
	                  ShowAlert(temp);
					  break;
	  			   }
		  case -15:{
				      flag=0;
					  ShowAlert(search(lpublic,"error"));
					  break;
	  			   }
		  case -16:{
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
		  case -17:{
				      flag=0;
					  ShowAlert(search(lpublic,"input_exceed_max_value"));
					  break;
	  			   }
		  case -18:{
					  flag=0;
					  ShowAlert(search(lwlan,"secIndex_is_same_with_other"));
					  break;
				    }
		}
																						
	 }
   }


   /****************set radio l2 isolation*****************/
   memset(wlan_id,0,sizeof(wlan_id));
   cgiFormStringNoNewlines("l2_iso_wlan",wlan_id,5);
   memset(state,0,sizeof(state));
   cgiFormStringNoNewlines("l2_iso_state",state,10);
   if((strcmp(wlan_id,""))&&(strcmp(state,""))) 
   {
     ret=set_radio_l2_isolation_func(ins_para->parameter,ins_para->connection,id,wlan_id,state); /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																								/*返回-2表示max throughout should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
																								/*返回-4表示wlan not exist，返回-5表示wtp not binding wlan，返回-6表示wtp id does not run*/
																								/*返回-7表示binding wlan error，返回-8表示error，返回-9表示Radio ID非法*/
																								/*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_radio_l2_isolation_fail"));
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
				 ShowAlert(search(lpublic,"input_para_error")); 			  
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"wlan_not_exist"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lwlan,"wtp_not_bind_wlan"));				
				 break; 
			   }
	   case -6:{
				 flag=0;
				 ShowAlert(search(lwlan,"wtp_not_run")); 			  
				 break; 
			   }
	   case -7:{
				 flag=0;
				 ShowAlert(search(lwlan,"bind_wlan_err")); 			  
				 break; 
			   }
	   case -8:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	   case -9:{
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
	   case -10:{
				  flag=0;
				  ShowAlert(search(lpublic,"input_exceed_max_value")); 			  
				  break; 
			    }
	 }
   }

   /****************set ap radio auto channel func*****************/
   memset(auto_channel,0,sizeof(auto_channel));
   cgiFormStringNoNewlines("auto_channel",auto_channel,10);
   if(strcmp(auto_channel,""))
   {
   	 ret=set_ap_radio_auto_channel_func(ins_para->parameter,ins_para->connection,id,auto_channel);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
																								 /*返回-2表示wtp id does not exist，返回-3表示radio id does not exist，返回-4表示error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_auto_channel_fail"));
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
				 ShowAlert(search(lwlan,"wtp_not_exist"));				
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"radio_not_exist"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	 }	 
   }

   
   /****************set ap radio diversity func*****************/
   memset(diversity,0,sizeof(diversity));
   cgiFormStringNoNewlines("diversity",diversity,10);
   if(strcmp(diversity,""))
   {
   	 ret=set_ap_radio_diversity_func(ins_para->parameter,ins_para->connection,id,diversity); /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
																							/*返回-2表示wtp id does not exist，返回-3表示radio id does not exist*/
																							/*返回-4表示radio model not petmit to set diversity，返回-5表示error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_diversity_fail"));
			    break;
			  }
	   case 1:{
	   			if(strcmp(diversity,"enable")==0)
	   			{
					flag=0;
					ShowAlert(search(lwlan,"con_succ_reboot_ap"));
	   			}
	   			break;   
	   		  }
	   case -1:{
				 flag=0;
				 ShowAlert(search(lpublic,"input_para_error")); 			  
				 break; 
			   }
	   case -2:{
				 flag=0;
				 ShowAlert(search(lwlan,"wtp_not_exist"));				
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"radio_not_exist"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"not_permit_set_diversity"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	 }	 
   }

   /****************set ap radio txantenna func*****************/
   memset(txantenna,0,sizeof(txantenna));
   cgiFormStringNoNewlines("txantenna",txantenna,10);
   if(strcmp(txantenna,""))
   {
   	 ret=set_ap_radio_txantenna_func(ins_para->parameter,ins_para->connection,id,txantenna);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'auto' 'main' or 'vice'*/
																						   /*返回-2表示wtp id does not exist，返回-3表示radio id does not exist，返回-4表示error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_txantenna_fail"));
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
				 ShowAlert(search(lwlan,"wtp_not_exist"));				
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"radio_not_exist"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	 }	 
   }

   /****************set radio 11n ampdu able*****************/
   memset(ampdu_type,0,sizeof(ampdu_type));
   cgiFormStringNoNewlines("ampdu_type",ampdu_type,10);
   memset(ampdu_switch,0,sizeof(ampdu_switch));
   cgiFormStringNoNewlines("ampdu_switch",ampdu_switch,10);
   if((strcmp(ampdu_type,""))&&(strcmp(ampdu_switch,"")))
   {
   	 ret=set_radio_11n_ampdu_able_cmd(ins_para->parameter,ins_para->connection,id,ampdu_type,ampdu_switch); /*返回0表示失败，返回1表示成功*/
																									       /*返回-1表示input patameter only with 'enable' or 'disable'*/
																									       /*返回-2表示radio id does not exist*/
																									       /*返回-3表示radio is not binding wlan, please bind it first*/
																									       /*返回-4表示radio is disable, please enable it first*/
																										   /*返回-5表示radio mode is not 11n,don't support this op*/
																										   /*返回-6表示error，返回-7表示Radio ID非法*/
																										   /*返回-8表示input patameter only with 'ampdu' or 'amsdu'*/
																										   /*返回-9表示amsdu switch is enable, please disable it first*/
																										   /*返回-10表示ampdu switch is enable, please disable it first*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
				if(strcmp(ampdu_type,"ampdu") == 0)
				{
					ShowAlert(search(lwlan,"set_ampdu_switch_fail"));
				}
				else if(strcmp(ampdu_type,"amsdu") == 0)
				{
					ShowAlert(search(lwlan,"set_amsdu_switch_fail"));
				}
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
				 ShowAlert(search(lwlan,"radio_not_exist")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"rad_dont_bind_wlan"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"enable_radio"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lwlan,"11n_support_this_op"));				
				 break; 
			   }
	   case -6:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	   case -7:{
				 flag=0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				 memset(max_radio_num,0,sizeof(max_radio_num));
				 snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM);
				 strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				 strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
				 ShowAlert(temp); 			  
				 break; 
			   }
	   case -8:{
				 flag=0;
				 ShowAlert(search(lpublic,"input_para_illegal")); 			  
				 break; 
			   }
	   case -9:{
				 flag=0;
				 ShowAlert(search(lwlan,"amsdu_switch_is_enable")); 			  
				 break; 
			   }
	   case -10:{
				  flag=0;
				  ShowAlert(search(lwlan,"ampdu_switch_is_enable")); 			  
				  break; 
			    }
	 }	 
   }

   /****************set radio 11n ampdu limit*****************/
   memset(limit_type,0,sizeof(limit_type));
   cgiFormStringNoNewlines("limit_type",limit_type,10);
   memset(ampdu_limit,0,sizeof(ampdu_limit));
   cgiFormStringNoNewlines("ampdu_limit",ampdu_limit,10);
   if((strcmp(limit_type,""))&&(strcmp(ampdu_limit,"")))
   {
   	 ret=set_radio_11n_ampdu_limit_cmd(ins_para->parameter,ins_para->connection,id,limit_type,ampdu_limit); /*返回0表示失败，返回1表示成功*/
																									       /*返回-1表示input patameter error*/
																									       /*返回-2表示input patameter error,ampdu limit should be 1024-65535*/
																									       /*返回-3表示radio id does not exist*/
																									       /*返回-4表示radio is not binding wlan, please bind it first*/
																										   /*返回-5表示radio is disable, please enable it first*/
																										   /*返回-6表示radio mode is not 11n,don't support this op*/
																										   /*返回-7表示error，返回-8表示Radio ID非法*/
																										   /*返回-9表示input patameter only with 'ampdu' or 'amsdu'*/
																										   /*返回-10表示input patameter error,amsdu limit should be 2290-4096*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
				if(strcmp(ampdu_type,"ampdu") == 0)
				{
					ShowAlert(search(lwlan,"set_ampdu_limit_fail"));
				}
				else if(strcmp(ampdu_type,"amsdu") == 0)
				{
					ShowAlert(search(lwlan,"set_amsdu_limit_fail"));
				}			    
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
				 ShowAlert(search(lwlan,"ampdu_limit_illegal")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"radio_not_exist")); 			  
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"rad_dont_bind_wlan"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lwlan,"enable_radio"));				
				 break; 
			   }
	   case -6:{
				 flag=0;
				 ShowAlert(search(lwlan,"11n_support_this_op"));				
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
				 snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM);
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
				  ShowAlert(search(lwlan,"amsdu_limit_illegal")); 			  
				  break; 
			    }
	 }	 
   }
   
   /****************set radio 11n ampdu subframe cmd*****************/
   memset(subframe_type,0,sizeof(subframe_type));
   cgiFormStringNoNewlines("subframe_type",subframe_type,10);
   memset(subframe,0,sizeof(subframe));
   cgiFormStringNoNewlines("subframe",subframe,10);
   if((strcmp(subframe_type,""))&&(strcmp(subframe,"")))
   {
   	 ret=set_radio_11n_ampdu_subframe_cmd(ins_para->parameter,ins_para->connection,id,subframe_type,subframe);/*返回0表示失败，返回1表示成功*/
																										     /*返回-1表示input patameter only with 'ampdu' or 'amsdu'*/
																										     /*返回-2表示input patameter error*/
																										     /*返回-3表示input patameter error,limit should be 2-64*/
																										     /*返回-4表示Radio ID非法，返回-5表示radio id does not exist*/
																											 /*返回-6表示radio is not binding wlan, please bind it first*/
																											 /*返回-7表示radio is disable, please enable it first*/
																											 /*返回-8表示radio mode is not 11n,don't support this op，返回-9表示error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    if(strcmp(ampdu_type,"ampdu") == 0)
				{
					ShowAlert(search(lwlan,"set_ampdu_subframe_fail"));
				}
				else if(strcmp(ampdu_type,"amsdu") == 0)
				{
					ShowAlert(search(lwlan,"set_amsdu_subframe_fail"));
				}	
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
				 ShowAlert(search(lpublic,"input_para_error")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"ampdu_subframe_illegal")); 			  
				 break; 
			   }
	   case -4:{
				 flag=0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				 memset(max_radio_num,0,sizeof(max_radio_num));
				 snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM);
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
				 ShowAlert(search(lwlan,"rad_dont_bind_wlan"));				
				 break; 
			   }	   
	   case -7:{
				 flag=0;
				 ShowAlert(search(lwlan,"enable_radio"));				
				 break; 
			   }	   
	   case -8:{
				 flag=0;
				 ShowAlert(search(lwlan,"11n_support_this_op"));				
				 break; 
			   }
	   case -9:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	 }	 
   }

   
   /****************set radio 11n puren mixed*****************/
   memset(work_mode_wlan,0,sizeof(work_mode_wlan));
   cgiFormStringNoNewlines("work_mode_wlan",work_mode_wlan,10);
   memset(work_mode,0,sizeof(work_mode));
   cgiFormStringNoNewlines("11n_work_mode",work_mode,10);
   if((strcmp(work_mode_wlan,""))&&(strcmp(work_mode,"")))
   {
   	 ret=set_radio_11n_puren_mixed_cmd(ins_para->parameter,ins_para->connection,id,work_mode_wlan,work_mode);/*返回0表示失败，返回1表示成功*/
																									        /*返回-1表示input patameter error*/
																									        /*返回-2表示input patameter only with 'puren' or 'mixed'*/
																									        /*返回-3表示radio id does not exist*/
																									        /*返回-4表示radio is not binding wlan, please bind it first*/
																										    /*返回-5表示radio is disable, please enable it first*/
																										    /*返回-6表示radio mode is not 11n,don't support this op*/
																										    /*返回-7表示error，返回-8表示Radio ID非法*/
																											/*返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
																											/*返回-10表示now radio mode is an or gn, belong to puren,you can set it to mixed*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_11n_work_mode_fail"));
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
				 ShowAlert(search(lpublic,"input_para_error")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"radio_not_exist")); 			  
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"rad_dont_bind_wlan"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lwlan,"enable_radio"));				
				 break; 
			   }
	   case -6:{
				 flag=0;
				 ShowAlert(search(lwlan,"11n_support_this_op"));				
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
				  ShowAlert(search(lwlan,"puren_mode_can_not_set")); 			  
				  break; 
			    }
	 }	 
   }

   /****************set tx chainmask*****************/
   memset(chainmask_type,0,sizeof(chainmask_type));
   cgiFormStringNoNewlines("chainmask_type",chainmask_type,15);
   memset(chainmask,0,sizeof(chainmask));
   cgiFormStringNoNewlines("chainmask",chainmask,10);
   if((strcmp(chainmask_type,""))&&(strcmp(chainmask,"")))
   {
   	 ret=set_tx_chainmask_v2_cmd(ins_para->parameter,ins_para->connection,id,chainmask_type,chainmask); /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with '0.0.1','0.1.0','0.1.1','1.0.0','1.0.1','1.1.0' or '1.1.1'*/
																									   /*返回-2表示radio id does not exist，返回-3表示radio not support this command*/			
																									   /*返回-4表示radio mode is not 11N ,don't support this command，返回-5表示error，返回-6表示Radio ID非法*/
																									   /*返回-7表示input patameter only with 'tx_chainmask' or 'rx_chainmask'*/
																									   /*返回-8表示radio chainmask number is 1, don't support this value*/
																									   /*返回-9表示radio chainmask number is 2, don't support this value*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_chainmask_fail"));
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
				 ShowAlert(search(lwlan,"radio_not_exist")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"rad_not_support_this_command"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"11n_support_this_op"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	   case -6:{
				 flag = 0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				 memset(max_radio_num,0,sizeof(max_radio_num));
				 snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				 strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				 strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  		 ShowAlert(temp);
	    	     break; 
			   }
	   case -7:{
				 flag=0;
				 ShowAlert(search(lpublic,"input_para_illegal")); 			  
				 break; 
			   }
	   case -8:{
				 flag = 0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"radio_chainmask_dont_support1"),sizeof(temp)-1);
				 strncat(temp,"1",sizeof(temp)-strlen(temp)-1);
				 strncat(temp,search(lwlan,"radio_chainmask_dont_support2"),sizeof(temp)-strlen(temp)-1);
		  		 ShowAlert(temp);
	    	     break; 
			   }
	   case -9:{
				 flag = 0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"radio_chainmask_dont_support1"),sizeof(temp)-1);
				 strncat(temp,"2",sizeof(temp)-strlen(temp)-1);
				 strncat(temp,search(lwlan,"radio_chainmask_dont_support2"),sizeof(temp)-strlen(temp)-1);
		  		 ShowAlert(temp);
	    	     break;
			   }
	 }	 
   }

   /****************set radio cmmode*****************/
   memset(cwmode_channel_offset,0,sizeof(cwmode_channel_offset));
   cgiFormStringNoNewlines("cwmode_channel_offset",cwmode_channel_offset,20);
   memset(cwmode,0,sizeof(cwmode));
   memset(channel_offset,0,sizeof(channel_offset));
   if(strcmp(cwmode_channel_offset,""))
   {
	   endptr = strchr( cwmode_channel_offset, '^' );
	   if(endptr)
	   {
		   strncpy(cwmode,cwmode_channel_offset,endptr-cwmode_channel_offset);
		   endptr++;
		   strncpy(channel_offset,endptr,sizeof(channel_offset)-1);
	   }
   }
		
   if(strcmp(cwmode,""))
   {
   	 ret=set_radio_cmmode_cmd(ins_para->parameter,ins_para->connection,id,cwmode);/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																			     /*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
																			     /*返回-4表示radio is not binging wlan，返回-5表示error，返回-6表示Radio ID非法*/
																				 /*返回-7表示radio mode is not 11N ,don't support this command*/
																				 /*返回-12表示you are not allowed to set channel offset up*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_cwmode_fail"));
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
				 ShowAlert(search(lwlan,"radio_not_exist")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"enable_radio"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"rad_dont_bind_wlan"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	   case -6:{
				 flag = 0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				 memset(max_radio_num,0,sizeof(max_radio_num));
				 snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				 strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				 strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  		 ShowAlert(temp);
	    	     break; 
			   }
	   case -7:{
				 flag=0;
				 ShowAlert(search(lwlan,"11n_support_this_op"));				
				 break; 
			   }
	   case -12:{
				  flag=0;
				  ShowAlert(search(lwlan,"channel_not_allowed_up"));				
				  break; 
			    }
	 }	 
   }

   /****************set radio 11n channel offset*****************/
   if(strcmp(channel_offset,""))
   {
   	 ret=set_radio_11n_channel_offset_cmd(ins_para->parameter,ins_para->connection,id,channel_offset);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'down'*/
																									 /*返回-2表示radio id does not exist，返回-3表示radio is not binding wlan, please bind it first*/			
																									 /*返回-4表示radio is disable, please enable it first，返回-5表示radio mode is not 11n,don't support this op*/
																									 /*返回-6表示error，返回-7表示Radio ID非法，返回-8表示radio channel bandwidth is not 40,don't support this op*/
																									 /*返回-9表示the current radio channel is larger than the max channel,you are not allowed to set channel offset up*/
																									 /*返回-10表示the current radio channel is less than the min channel ,you are not allowed to set channel offset down*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_channel_offset_fail"));
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
				 ShowAlert(search(lwlan,"radio_not_exist")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"rad_dont_bind_wlan"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"enable_radio"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lwlan,"11n_support_this_op"));				
				 break; 
			   }
	   case -6:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	   case -7:{
				 flag = 0;
				 memset(temp,0,sizeof(temp));
				 strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				 memset(max_radio_num,0,sizeof(max_radio_num));
				 snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				 strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				 strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  		 ShowAlert(temp);
	    	     break; 
			   }
	   case -8:{
				 flag=0;
				 ShowAlert(search(lwlan,"channel_bandwidth_not_40")); 			  
				 break; 
			   }
	   case -9:{
				 flag=0;
				 ShowAlert(search(lwlan,"channel_not_allowed_up")); 			  
				 break; 
			   }
	   case -10:{
				  flag=0;
				  ShowAlert(search(lwlan,"channel_not_allowed_down")); 			  
				  break; 
			    }
	 }	 
   }

   /****************set radio guard interval*****************/
   memset(ShortGI_mode,0,sizeof(ShortGI_mode));
   cgiFormStringNoNewlines("ShortGI_mode",ShortGI_mode,10);
   if(strcmp(ShortGI_mode,""))
   {
   	 ret=set_radio_guard_interval_cmd(ins_para->parameter,ins_para->connection,id,ShortGI_mode);/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																							   /*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
																							   /*返回-4表示radio is not binging wlan，返回-5表示error*/
	 switch(ret)
	 {
	   case SNMPD_CONNECTION_ERROR:
	   case 0:{
			    flag=0;
			    ShowAlert(search(lwlan,"set_ShortGI_mode_fail"));
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
				 ShowAlert(search(lwlan,"radio_not_exist")); 			  
				 break; 
			   }
	   case -3:{
				 flag=0;
				 ShowAlert(search(lwlan,"enable_radio"));				
				 break; 
			   }
	   case -4:{
				 flag=0;
				 ShowAlert(search(lwlan,"rad_dont_bind_wlan"));				
				 break; 
			   }
	   case -5:{
				 flag=0;
				 ShowAlert(search(lpublic,"error")); 			  
				 break; 
			   }
	 }	 
   }

  if(flag==1)
    ShowAlert(search(lpublic,"oper_succ"));
}



