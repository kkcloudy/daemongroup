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
* wp_apcon.c
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
#include "ws_dcli_ac.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"



int ShowAPconPage(char *m,struct list *lpublic,struct list *lwcontrol,struct list *lwlan);    
void AP_Config(instance_parameter *ins_para,struct list *lpublic,struct list *lwcontrol,struct list *lwlan);

int cgiMain()
{  
  char encry[BUF_LEN] = { 0 }; 
  char *str = NULL;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwcontrol = NULL;     /*解析wlan.txt文件的链表头*/  
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwcontrol=get_chain_head("../htdocs/text/wcontrol.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_conap",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
  	ShowAPconPage(encry,lpublic,lwcontrol,lwlan);

  release(lpublic);  
  release(lwcontrol);
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

int ShowAPconPage(char *m,struct list *lpublic,struct list *lwcontrol,struct list *lwlan)
{  
  int i = 0; 
  int ech = 0;
  int result1 = 0,result2 = 0;
  DCLI_AC_API_GROUP_FIVE *up_timer = NULL;
  char select_insid[10] = { 0 };
  DCLI_WTP_API_GROUP_THREE *WTPINFOZ = NULL;
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>AP Config</title>");
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
  if(cgiFormSubmitClicked("ap_con") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		AP_Config(paraHead1,lpublic,lwcontrol,lwlan);
	} 
  }
	fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=300 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>"\
    "<td width=590 align=right valign=bottom background=/images/di22.jpg>",search(lpublic,"title_style"),search(lwcontrol,"adv_conf"));
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=ap_con style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
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
                  "</tr>");	          
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wcsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"wc_info"));                       
                    fprintf(cgiOut,"</tr>");
  				    fprintf(cgiOut,"<tr height=26>"\
					  "<td align=left id=tdleft><a href=wp_wcwtp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"wc_config"));  
					fprintf(cgiOut,"<tr height=25>"\
				      "<td align=left id=tdleft><a href=wp_apsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"ap_info"));                       
				    fprintf(cgiOut,"</tr>");
				    fprintf(cgiOut,"<tr height=26>"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwcontrol,"ap_config"));   /*突出显示*/
				    fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_rrmsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"rrm_info"));                       
 				    fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_rrmcon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"rrm_config"));                       
   					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wcapill.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"illegal"),search(lpublic,"menu_san"),search(lwcontrol,"list"));                       
 				    fprintf(cgiOut,"</tr>");
                  for(i=0;i<0;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
				  if(paraHead1)
				  {
					  result2=show_ap_echotimer(paraHead1->parameter,paraHead1->connection,&WTPINFOZ);
					  if((result2==1)&&(WTPINFOZ))
						ech=WTPINFOZ->echotimer;
					  result1=show_ap_update_img_timer_cmd(paraHead1->parameter,paraHead1->connection,&up_timer);
				  }
				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">"\
              "<table width=350 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
				 "<td>%s ID:</td>",search(lpublic,"instance"));
				 fprintf(cgiOut,"<td colspan=2>"\
				  "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_apcon.cgi\",\"%s\")>",m);
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
    			  "<td width=100>%s:</td>",search(lwcontrol,"echotimer"));
				  fprintf(cgiOut,"<td width=150 align=left><input type=text name=echotimer value=%d maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>",ech);
    			  fprintf(cgiOut,"<td width=100><font color=red>(1--30)</font></td>"\
  				"</tr>"\
				"<tr height=30>"\
    			  "<td>%s:</td>",search(lwcontrol,"update_img_timer"));
				  if((result1 == 1)&&(up_timer))
				    fprintf(cgiOut,"<td align=left><input type=text name=update_img_timer value=%d maxLength=10 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>",up_timer->timer);
				  else
				  	fprintf(cgiOut,"<td align=left><input type=text name=update_img_timer value=0 maxLength=10 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				  fprintf(cgiOut,"<td><font color=red>(5--3600)</font></td>"\
  				"</tr>"\
				"<tr height=30>"\
    			  "<td>%s:</td>",search(lwcontrol,"auto_update_switch"));
				  fprintf(cgiOut,"<td align=left colspan=2><select name=auto_update_switch id=auto_update_switch style=width:130px>");
				 	fprintf(cgiOut,"<option value=>"\
  				    "<option value=open>open"\
  				    "<option value=close>close"\
		          "</select></td>"\
  				"</tr>"\
  				"<tr height=30>"\
    			  "<td>%s:</td>",search(lwcontrol,"ap_access_through_nat"));
				  fprintf(cgiOut,"<td align=left colspan=2><select name=ap_access_through_nat id=ap_access_through_nat style=width:130px>");
				 	fprintf(cgiOut,"<option value=>"\
  				    "<option value=enable>enable"\
  				    "<option value=disable>disable"\
		          "</select></td>"\
  				"</tr>"\
  				"<tr height=30>"\
    			  "<td>%s:</td>",search(lwlan,"ap_sta_info_report_switch"));
				  fprintf(cgiOut,"<td align=left colspan=2><select name=ap_sta_info_report_switch id=ap_sta_info_report_switch style=width:130px>");
				 	fprintf(cgiOut,"<option value=>"\
  				    "<option value=enable>enable"\
  				    "<option value=disable>disable"\
		          "</select></td>"\
  				"</tr>"\
			    "<tr>"\
			      "<td><input type=hidden name=encry_conap value=%s></td>",m);				  
				  fprintf(cgiOut,"<td colspan=2><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
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
if(result1 == 1)
{
	Free_ap_update_img_timer(up_timer);
}
if(result2 == 1)
{
	free_show_ap_echotimer(WTPINFOZ);
}
free_instance_parameter_list(&paraHead1);
return 0;
}


void AP_Config(instance_parameter *ins_para,struct list *lpublic,struct list *lwcontrol,struct list *lwlan)
{
	int ret = 0; 
    int flag = 1;
	char *endptr = NULL;  
    char echotimer[5] = { 0 };
	int echot = 0;
    char update_img_timer[15] = { 0 };
	char auto_update_switch[10] = { 0 };
	char ap_access_through_nat[10] = { 0 };
	char ap_sta_info_report_switch[10] = { 0 };
	char alt[100] = { 0 };
	char max_wtp_num[10] = { 0 };

	/***********************set ap echotimer*****************************/	
    memset(echotimer,0,sizeof(echotimer));
	cgiFormStringNoNewlines("echotimer",echotimer,5);   
    if(strcmp(echotimer,"")!=0)
  	{
  		echot= strtoul(echotimer,&endptr,10);
		if((echot<1)||(echot>30))
		{
			flag = 0;
			ShowAlert(search(lwcontrol,"set_ap_echotimer_err"));
		}
		else
		{
			ret = set_ap_echotimer(ins_para->parameter,ins_para->connection,0,echot);
			switch(ret)
			{
				case SNMPD_CONNECTION_ERROR:
				case 0:ShowAlert(search(lwcontrol,"set_ap_echotimer_fail"));
					   flag = 0;
					   break;	
				case 1: break;
				case -1:ShowAlert(search(lpublic,"error"));
						flag = 0;
						break;				
				case -2:{
						  memset(alt,0,sizeof(alt));
						  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
						  memset(max_wtp_num,0,sizeof(max_wtp_num));
						  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
						  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
						  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
						  ShowAlert(alt);
						  flag=0;
						  break;
					    }
				case -3:ShowAlert(search(lwcontrol,"set_ap_echotimer_err"));
						flag = 0;
						break;
			}
		}
    }


    /***********************set ap update img timer cmd*****************************/		
    memset(update_img_timer,0,sizeof(update_img_timer));
    cgiFormStringNoNewlines("update_img_timer",update_img_timer,15); 
	if(strcmp(update_img_timer,"")!=0)	
	{
		ret = set_ap_update_img_timer_cmd(ins_para->parameter,ins_para->connection,update_img_timer);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
		    case 0:ShowAlert(search(lwcontrol,"set_ap_update_img_timer_fail"));
				   flag = 0;
				   break;
			case 1:break;
			case -1:ShowAlert(search(lpublic,"unknown_id_format"));
					flag = 0;
					break;
			case -2:ShowAlert(search(lpublic,"error"));
					flag = 0;
					break;	
			case -3:ShowAlert(search(lwcontrol,"set_ap_update_img_timer_err"));
					flag = 0;
					break;
		}
	}

	/***********************old ap img data cmd*****************************/		
    memset(auto_update_switch,0,sizeof(auto_update_switch));
    cgiFormStringNoNewlines("auto_update_switch",auto_update_switch,10); 
	if(strcmp(auto_update_switch,"")!=0)	
	{
		ret = old_ap_img_data_cmd(ins_para->parameter,ins_para->connection,auto_update_switch);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
		    case 0:ShowAlert(search(lwcontrol,"set_auto_update_switch_fail"));
				   flag = 0;
				   break;
			case 1:break;
		}
	}

	/***********************set ap access through nat cmd*****************************/		
    memset(ap_access_through_nat,0,sizeof(ap_access_through_nat));
    cgiFormStringNoNewlines("ap_access_through_nat",ap_access_through_nat,10); 
	if(strcmp(ap_access_through_nat,"")!=0)	
	{
		ret = set_ap_access_through_nat_cmd(ins_para->parameter,ins_para->connection,ap_access_through_nat);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
		    case 0:ShowAlert(search(lwcontrol,"set_ap_access_through_nat_fail"));
				   flag = 0;
				   break;
			case 1:break;
			case -1:ShowAlert(search(lpublic,"input_para_illegal"));
					flag = 0;
					break;
			case -2:ShowAlert(search(lpublic,"error"));
					flag = 0;
					break;	
		}
	}

	/***********************set ap sta infomation report enable func*****************************/		
	memset(ap_sta_info_report_switch,0,sizeof(ap_sta_info_report_switch));
    cgiFormStringNoNewlines("ap_sta_info_report_switch",ap_sta_info_report_switch,10);
    if(strcmp(ap_sta_info_report_switch,"")!=0)
    {
	    ret=set_ap_sta_infomation_report_enable_func(ins_para->parameter,ins_para->connection,0,ap_sta_info_report_switch);   
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示input patameter only with 'enable' or 'disable'*/
																							/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run*/
																							/*返回-4表示error，返回-5示WTP ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
		switch(ret)										  
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:{
				      ShowAlert(search(lwlan,"con_ap_sta_info_report_switch_fail"));
		  		      flag = 0;
				      break;
		  		 }
		  case 1:break;
		  case -1:{
			  		  ShowAlert(search(lpublic,"input_para_illegal"));
			  		  flag = 0;
					  break;
		  		  }
		  case -2:{
					  ShowAlert(search(lwlan,"wtp_not_exist"));
					  flag = 0;
					  break;
		  		  }
		  case -3:{
					  ShowAlert(search(lwlan,"wtp_not_run"));
					  flag = 0;
					  break;
		  		  }	  
		  case -4:{
					  ShowAlert(search(lpublic,"error"));
					  flag = 0;
					  break;
		  		  }
		  case -5:{
					  memset(alt,0,sizeof(alt));
					  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
					  memset(max_wtp_num,0,sizeof(max_wtp_num));
					  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
					  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
					  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
					  ShowAlert(alt);
					  flag = 0;
					  break;
		  		  }
		}
  	}

    if(flag == 1)
  	{
		ShowAlert(search(lpublic,"oper_succ"));
  	}
}



