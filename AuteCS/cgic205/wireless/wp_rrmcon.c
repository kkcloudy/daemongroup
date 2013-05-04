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
* wp_rrmcon.c
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
#include "ws_dcli_ac.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


int ShowRrmconPage(char *m,struct list *lpublic,struct list *lwcontrol);    
void RrmConfig(instance_parameter *ins_para,struct list *lpublic,struct list *lwcontrol);

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
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_conrrm",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
  	ShowRrmconPage(encry,lpublic,lwcontrol);

  release(lpublic);  
  release(lwcontrol);
  destroy_ccgi_dbus();
  return 0;
}

int ShowRrmconPage(char *m,struct list *lpublic,struct list *lwcontrol)
{ 
  int i = 0;
  int result1 = 0,result2 = 0;
  DCLI_AC_API_GROUP_FIVE *resource_mg = NULL;
  DCLI_AC_API_GROUP_FIVE *tx_control = NULL;
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>RrmConfig</title>");
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
  if(cgiFormSubmitClicked("rrm_con") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		RrmConfig(paraHead1,lpublic,lwcontrol);
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
          "<td width=62 align=center><input id=but type=submit name=rrm_con style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
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
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wcwtp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"wc_config"));                       
   					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
				      "<td align=left id=tdleft><a href=wp_apsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"ap_info"));                       
				    fprintf(cgiOut,"</tr>");
				    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_apcon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"ap_config")); 					
				    fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_rrmsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"rrm_info"));                       
 				    fprintf(cgiOut,"</tr>");
  				    fprintf(cgiOut,"<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwcontrol,"rrm_config"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wcapill.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"illegal"),search(lpublic,"menu_san"),search(lwcontrol,"list"));                       
 				    fprintf(cgiOut,"</tr>");
                  for(i=0;i<5;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
				  if(paraHead1)
				  {
					  result1=show_ap_rrm_config_func(paraHead1->parameter,paraHead1->connection,&resource_mg);
					  result2=show_ap_txpower_control(paraHead1->parameter,paraHead1->connection,&tx_control);
				  }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:12px\">"\
              "<table width=450 border=0 cellspacing=0 cellpadding=0>"\
	"<tr height=30>"\
	 "<td>%s ID:</td>",search(lpublic,"instance"));
	 fprintf(cgiOut,"<td colspan=2>"\
	  "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_rrmcon.cgi\",\"%s\")>",m);
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
    "<td width=150>%s:</td>",search(lwcontrol,"rrm_switch"));
	if((result1 == 1)&&(resource_mg->rrm_state==1))
	{
		fprintf(cgiOut,"<td width=150 align=left><input name=radiobutton type=radio value=enable checked=checked >%s</td>",search(lwcontrol,"open"));
		fprintf(cgiOut,"<td><input type=radio name=radiobutton value=disable >%s</td>",search(lwcontrol,"close"));
	}
	else
	{
		fprintf(cgiOut,"<td width=150 align=left><input name=radiobutton type=radio value=enable >%s</td>",search(lwcontrol,"open"));
		fprintf(cgiOut,"<td><input type=radio name=radiobutton value=disable checked=checked>%s</td>",search(lwcontrol,"close"));
	}
 fprintf(cgiOut,"</tr>");
 fprintf(cgiOut,"<tr height=30>"\
    "<td width=150>%s:</td>",search(lwcontrol,"rm_time"));
fprintf(cgiOut,"<td width=150 align=left><input type=text name=rm_time maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
    "<td width=150><font color=red>(30--32767)</font></td>");
  fprintf(cgiOut,"</tr>");
fprintf(cgiOut,"<tr height=30>"\
    "<td width=150>%s:</td>",search(lwcontrol,"dynamic"));
if((result1 == 1)&&(resource_mg->d_channel_state==1))
{
	fprintf(cgiOut,"<td width=150 align=left><input name=dynamic type=radio value=open checked=checked >%s</td>",search(lwcontrol,"open"));
	fprintf(cgiOut,"<td><input type=radio name=dynamic value=close >%s</td>",search(lwcontrol,"close"));
}
else
{
	fprintf(cgiOut,"<td width=150 align=left><input name=dynamic type=radio value=open>%s</td>",search(lwcontrol,"open"));
	fprintf(cgiOut,"<td><input type=radio name=dynamic value=close checked=checked>%s</td>",search(lwcontrol,"close"));
}

 fprintf(cgiOut,"</tr>");
 fprintf(cgiOut,"<tr height=30>"\
    "<td>%s:</td>",search(lwcontrol,"dynamic_power"));
 if((result2 == 1)&&(tx_control->tx_control)&&(tx_control->tx_control->state==1))
 {
	fprintf(cgiOut,"<td align=left><input name=dynamic_power type=radio value=open checked=checked >%s</td>",search(lwcontrol,"open"));
	fprintf(cgiOut,"<td><input type=radio name=dynamic_power value=close >%s</td>",search(lwcontrol,"close"));
 }
 else
 {
	fprintf(cgiOut,"<td align=left><input name=dynamic_power type=radio value=open >%s</td>",search(lwcontrol,"open"));
	fprintf(cgiOut,"<td><input type=radio name=dynamic_power value=close checked=checked >%s</td>",search(lwcontrol,"close"));
 }
 fprintf(cgiOut,"</tr>");
 fprintf(cgiOut,"<tr height=30>"\
    "<td>%s:</td>",search(lwcontrol,"dynamic_power_scope"));
 if((result2 == 1)&&(tx_control->tx_control)&&(tx_control->tx_control->scope==0))
 {
	fprintf(cgiOut,"<td align=left><input name=dynamic_power_scope type=radio value=own checked=checked >own</td>"\
	"<td><input type=radio name=dynamic_power_scope value=all >all</td>");
 }
 else
 {
	fprintf(cgiOut,"<td align=left><input name=dynamic_power_scope type=radio value=own >own</td>"\
	"<td><input type=radio name=dynamic_power_scope value=all checked=checked >all</td>");
 }
 fprintf(cgiOut,"</tr>"\
  "<tr height=30>"\
	 "<td width=150>%s:</td>",search(lwcontrol,"coverage_threshold"));
 fprintf(cgiOut,"<td width=150 align=left><input type=text name=cov_thr maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
	 "<td width=150><font color=red>(5--15)</font></td>");
   fprintf(cgiOut,"</tr>"\
 "<tr height=30>"\
    "<td width=150>%s:</td>",search(lwcontrol,"txpower_threshold"));
fprintf(cgiOut,"<td width=150 align=left><input type=text name=txp_thr maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
    "<td width=150><font color=red>(20--35)</font></td>");
  fprintf(cgiOut,"</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lwcontrol,"rogue_ap_counter_switch"));
 if((result1 == 1)&&(resource_mg->countermeasures_switch==1))
 {
	fprintf(cgiOut,"<td align=left><input name=rogue_ap_counter_switch type=radio value=enable checked=checked >%s</td>",search(lwcontrol,"open"));
	fprintf(cgiOut,"<td><input type=radio name=rogue_ap_counter_switch value=disable >%s</td>",search(lwcontrol,"close"));
 }
 else
 {
	fprintf(cgiOut,"<td align=left><input name=rogue_ap_counter_switch type=radio value=enable >%s</td>",search(lwcontrol,"open"));
	fprintf(cgiOut,"<td><input type=radio name=rogue_ap_counter_switch value=disable checked=checked >%s</td>",search(lwcontrol,"close"));
 }
 fprintf(cgiOut,"</tr>");
 fprintf(cgiOut,"<tr height=30>"\
    "<td>%s:</td>",search(lwcontrol,"rogue_ap_counter_mode"));
    fprintf(cgiOut,"<td align=left colspan=2>"\
 	  "<select name=rogue_ap_counter_mode id=rogue_ap_counter_mode style=width:130px>");
 		if((result1 == 1)&&(resource_mg->countermeasures_mode==0))
 		{
		  fprintf(cgiOut,"<option value=ap selected=selected>ap"\
		  "<option value=adhoc>adhoc"\
		  "<option value=all>all");
  		}
		else if((result1 == 1)&&(resource_mg->countermeasures_mode==1))
		{
		  fprintf(cgiOut,"<option value=ap>ap"\
		  "<option value=adhoc selected=selected>adhoc"\
		  "<option value=all>all");
  		}
		else
		{
		  fprintf(cgiOut,"<option value=ap>ap"\
		  "<option value=adhoc>adhoc"\
		  "<option value=all selected=selected>all");
  		}
     fprintf(cgiOut,"</select>"\
   "</td>"\
  "</tr>"\
  "<tr>"\
    "<td><input type=hidden name=encry_conrrm value=%s></td>",m);
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
  Free_ap_rrm_config(resource_mg);
}
if(result2 == 1)
{
  Free_ap_txpower_control(tx_control);
}
free_instance_parameter_list(&paraHead1);
return 0;
}


void RrmConfig(instance_parameter *ins_para,struct list *lpublic,struct list *lwcontrol)
{
	int ret = 0; 
    int flag = 1,state = 0;
    int flag1 = 1,flag2 = 1;
    char stat[20] = { 0 };
    char rm_time[20] = { 0 };
    char channel[10] = { 0 };
    char power[10] = { 0 };
    char power_scope[10] = { 0 };
	char cov_thr[10] = { 0 };
	char txp_thr[10] = { 0 };
	char counter_switch[10] = { 0 };
	char counter_mode[10] = { 0 };

	/***********************config rrm switch*****************************/	
    memset(stat,0,sizeof(stat));
 	cgiFormStringNoNewlines("radiobutton",stat,20);   
	if(!strcmp(stat,"disable"))
	 	state = 0;
	else
	 	state = 1;
	ret = set_radio_resource_management(ins_para->parameter,ins_para->connection,state);
	if(ret == 1)
		;
	else if((ret == 0)||(ret == SNMPD_CONNECTION_ERROR))
	{
		flag = 0;
		ShowAlert(search(lwcontrol,"ap_scan_fail"));
	}
	else
	{
		flag = 0;
		ShowAlert(search(lpublic,"error"));
	}


    /***********************set ap scanning report interval*****************************/		
    memset(rm_time,0,sizeof(rm_time));
	cgiFormStringNoNewlines("rm_time",rm_time,20); 
	if(strcmp(rm_time,"")!=0)
	{
		 ret = set_ap_scanning_report_interval_cmd(ins_para->parameter,ins_para->connection,rm_time);
		 switch(ret)
		 {
		 	case -1:ShowAlert(search(lwcontrol,"time_err"));
				   flag = 0;
				   break;
			case 1:break;
			case -2:ShowAlert(search(lpublic,"oper_fail"));
					flag = 0;
					break;
	 	 }
	}


	/***********************dynamic channel selection*****************************/	
    memset(channel,0,sizeof(channel));
	cgiFormStringNoNewlines("dynamic",channel,10); 
	if(strcmp(channel,"")!=0)
	{
		ret = dynamic_channel_selection_cmd(ins_para->parameter,ins_para->connection,channel);
		switch(ret)
		{
			case 1:break;
			case -2:flag = 0;
					flag1 = 0;
					ShowAlert(search(lwcontrol,"enable_radio_resource"));
					break;
		}
	}


	/***********************open or close transmit power control*****************************/		
    memset(power,0,sizeof(power));
	cgiFormStringNoNewlines("dynamic_power",power,10); 
	if((strcmp(power,"")!=0)&&(flag1==1))
	{
		ret=dynamic_power_selection_cmd(ins_para->parameter,ins_para->connection,power);
		switch(ret)
		{			
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   flag2=0;
				   if(strcmp(power,"open")==0)
					 ShowAlert(search(lwcontrol,"open_power_fail"));
				   else
				     ShowAlert(search(lwcontrol,"close_power_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
					flag2=0;
			 		ShowAlert(search(lwcontrol,"enable_radio_resource"));  /*you should enable radio resource management first*/
					break;
		}
	}


	/***********************set transmit power control scope*****************************/
    memset(power_scope,0,sizeof(power_scope));
	cgiFormStringNoNewlines("dynamic_power_scope",power_scope,10); 
	if(strcmp(power_scope,"")!=0)
	{
		ret=set_transmit_power_control_scope(ins_para->parameter,ins_para->connection,power_scope);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   ShowAlert(search(lwcontrol,"set_power_scope_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
			 		ShowAlert(search(lpublic,"error"));
					break;
		}
	}

	

	/***********************set coverage threshold*****************************/
	memset(cov_thr,0,sizeof(cov_thr));
	cgiFormStringNoNewlines("cov_thr",cov_thr,10); 
	if(strcmp(cov_thr,"")!=0)
	{
		ret=set_coverage_threshold_cmd(ins_para->parameter,ins_para->connection,cov_thr);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   ShowAlert(search(lwcontrol,"set_coverage_threshold_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
			 		ShowAlert(search(lwcontrol,"cov_thr_error"));
					break;
			case -2:flag=0;
			 		ShowAlert(search(lpublic,"error"));
					break;
		}
	}


	
	/***********************set txpower threshold*****************************/
	memset(txp_thr,0,sizeof(txp_thr));
	cgiFormStringNoNewlines("txp_thr",txp_thr,10); 
	if(strcmp(txp_thr,"")!=0)
	{
		ret=set_txpower_threshold_cmd(ins_para->parameter,ins_para->connection,txp_thr);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   ShowAlert(search(lwcontrol,"set_txpower_threshold_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
			 		ShowAlert(search(lwcontrol,"txp_thr_error"));
					break;
			case -2:flag=0;
			 		ShowAlert(search(lpublic,"error"));
					break;
		}
	}

	
	/***********************set ap countermeasures cmd*****************************/
	memset(counter_switch,0,sizeof(counter_switch));
	cgiFormStringNoNewlines("rogue_ap_counter_switch",counter_switch,10); 
	if(strcmp(counter_switch,"")!=0)
	{
		ret=set_ap_countermeasures_cmd(ins_para->parameter,ins_para->connection,counter_switch);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   ShowAlert(search(lwcontrol,"set_rogue_ap_counter_switch_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
					ShowAlert(search(lpublic,"input_para_error"));
					break;
			case -2:flag=0;
					ShowAlert(search(lpublic,"error"));
					break;
		}
	}

	/***********************set ap countermeasures mode cmd*****************************/
	memset(counter_mode,0,sizeof(counter_mode));
	cgiFormStringNoNewlines("rogue_ap_counter_mode",counter_mode,10); 
	if(strcmp(counter_mode,"")!=0)
	{
		ret=set_ap_countermeasures_mode_cmd(ins_para->parameter,ins_para->connection,counter_mode);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   ShowAlert(search(lwcontrol,"set_rogue_ap_counter_mode_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
					ShowAlert(search(lpublic,"input_para_error"));
					break;
			case -2:flag=0;
					ShowAlert(search(lpublic,"error"));
					break;
		}
	}
		
    if((flag == 1)&&(flag1==1)&&(flag2==1))
  	{
		ShowAlert(search(lpublic,"oper_succ"));
  	}
}


