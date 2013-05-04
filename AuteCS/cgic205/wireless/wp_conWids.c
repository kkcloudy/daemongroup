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
* wp_conWids.c
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
#include "ws_dcli_ac.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

int ShowWidsconPage(char *m,struct list *lpublic,struct list *lwlan);  /*m代表加密字符串，n代表wlan id，t代表wlan name ，r代表security id，intf代表interafce，Hessid代表Hideessid，s代表service state*/
void ConWids(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;                
  char ID[5] = { 0 };
  char wlan_name[20] = { 0 };    
  char secID[5] = { 0 };
  char serSta[10] = { 0 };
  char inter[20] = { 0 };
  char hessid[5] = { 0 };  

  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(wlan_name,0,sizeof(wlan_name));
  memset(secID,0,sizeof(secID));
  memset(inter,0,sizeof(inter));
  memset(serSta,0,sizeof(serSta));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	cgiFormStringNoNewlines("ID", ID, 5);	
	cgiFormStringNoNewlines("Na", wlan_name, 20);  
	str=dcryption(encry);
	if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
	else
      ShowWidsconPage(encry,lpublic,lwlan);
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_conwlanser",encry,BUF_LEN);
    cgiFormStringNoNewlines("wlan_id",ID,5);  
    cgiFormStringNoNewlines("wlan_name",wlan_name,20);		
    cgiFormStringNoNewlines("sec_id",secID,5);
	cgiFormStringNoNewlines("bind_interface",inter,20);
	cgiFormStringNoNewlines("set_hessid",hessid,5); 
	cgiFormStringNoNewlines("wlan_service",serSta,10);
	str=dcryption(encry);
	if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
	else
      ShowWidsconPage(encry,lpublic,lwlan);
  }  

  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWidsconPage(char *m,struct list *lpublic,struct list *lwlan)
{  
  int i = 0;  
  char select_insid[10] = { 0 };
  char ins_id[5] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WIDS</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"</head>"\
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
  if(cgiFormSubmitClicked("widscon_apply") == cgiFormSuccess)
  {  		
  	if(paraHead1)
	{
		ConWids(paraHead1,lpublic,lwlan);
	}
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=%s>%s %s</font></td>",search(lpublic,"title_style"),search(lwlan,"wids"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
	   
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=widscon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
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
					"<td align=left id=tdleft><a href=wp_showWids.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"show_wids"));						
				  fprintf(cgiOut,"</tr>");				  
				  fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"con_wids"));	 /*突出显示*/
				  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_widsHistory.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"history"));						 
				  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_widsStatistics.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"stat_info")); 					  
				  fprintf(cgiOut,"</tr>");
                  for(i=0;i<7;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
			  "<table width=460 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
				 "<td>%s ID:</td>",search(lpublic,"instance"));
				 fprintf(cgiOut,"<td colspan=2>"\
				  "<select name=instance_id id=instance_id style=width:100px onchange=instanceid_change(this,\"wp_conWids.cgi\",\"%s\")>",m);
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
                "<tr height=30 style=\"padding-top:20px\">"\
                  "<td>%s:</td>",search(lpublic,"l_type"));
                  fprintf(cgiOut,"<td colspan=2 align=left>"\
			        "<select name=wids_type  id=wids_type multiple=multiple size=3 style=width:100px>"\
			          "<option value=flooding>flooding"\
			          "<option value=spoofing>spoofing"\
			          "<option value=weakiv>weakiv"\
		            "</select>"\
				  "</td>"\
                "</tr>"\
                "<tr height=30 style=\"padding-top:20px\">"\
				  "<td>%s:</td>",search(lpublic,"l_state"));
				  fprintf(cgiOut,"<td align=left>"\
				    "<select name=wids_state id=wids_state style=width:100px>"\
				      "<option value=>"\
					  "<option value=enable>enable"\
					  "<option value=disable>disable"\
	                "</select>"\
				  "</td>"\
                "</tr>"\
                "<tr height=30 style=\"padding-top:20px\">"\
				  "<td width=160>%s:</td>",search(lwlan,"interval"));
				  fprintf(cgiOut,"<td width=100 align=left><input type=text name=interval size=15 maxLength=1 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				  "<td width=200 align=left style=\"padding-left:20px\"><font color=red>(1--5)%s</font></td>",search(lpublic,"second"));
                fprintf(cgiOut,"</tr>"\
				"<tr height=30 style=\"padding-top:20px\">"\
				  "<td>%s:</td>",search(lwlan,"probe_threshold"));
				  fprintf(cgiOut,"<td align=left><input type=text name=probe_threshold size=15 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				  "<td align=left style=\"padding-left:20px\"><font color=red>(1--100)</font></td>"\
                "</tr>"\
                "<tr height=30 style=\"padding-top:20px\">"\
				  "<td>%s:</td>",search(lwlan,"other_threshold"));
				  fprintf(cgiOut,"<td align=left><input type=text name=other_threshold size=15 maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				  "<td align=left style=\"padding-left:20px\"><font color=red>(1--100)</font></td>"\
                "</tr>"\
                "<tr height=30 style=\"padding-top:20px\">"\
				  "<td>%s:</td>",search(lwlan,"last_time_in_black"));
				  fprintf(cgiOut,"<td align=left><input type=text name=last_time_in_black size=15 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>"\
				  "<td align=left style=\"padding-left:20px\"><font color=red>(1--36000)%s</font></td>",search(lpublic,"second"));
                fprintf(cgiOut,"</tr>"\
                "<tr>"\
                  "<td><input type=hidden name=encry_conwlanser value=%s></td>",m);
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
free_instance_parameter_list(&paraHead1);
return 0;
}

void ConWids(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{
	int flag = 1;
	int i = 0,ret = 0,result = cgiFormNotFound;
	char **responses;  	
	char type1[15] = { 0 };
	char type2[15] = { 0 };
	char type3[15] = { 0 };
	char state[10] = { 0 };
	char interval[5] = { 0 };
	char probe_threshold[5] = { 0 };
	char other_threshold[5] = { 0 };
	char last_time_in_black[10] = { 0 };

	memset(type1,0,sizeof(type1));
	memset(type2,0,sizeof(type2));
	memset(type3,0,sizeof(type3));
	memset(state,0,sizeof(state));
	
	/****************set ap wids set cmd****************/
	cgiFormStringNoNewlines("wids_state",state,10);

	result = cgiFormStringMultiple("wids_type", &responses);

	if((strcmp(state,"")==0)||(result == cgiFormNotFound))
	{
		if(!((strcmp(state,"")==0)&&(result == cgiFormNotFound)))
		{
			if(result == cgiFormNotFound)
			{
				ShowAlert(search(lpublic,"select_type"));
				flag=0;
			}
			else if(strcmp(state,"")==0)
			{
				ShowAlert(search(lpublic,"select_state"));
				flag=0;
			}
		}
	}
	else if((strcmp(state,""))&&(result != cgiFormNotFound))
	{
		i = 0;	
		while(responses[i])
		{
			switch(i)
			{
			  case 0:strncpy(type1,responses[i],sizeof(type1)-1);
			  	     break;
			  case 1:strncpy(type2,responses[i],sizeof(type2)-1);
			  	     break;	
			  case 2:strncpy(type3,responses[i],sizeof(type3)-1);
			  	     break;	
			}
			i++;
		}
		cgiStringArrayFree(responses);
		 
		ret=set_ap_wids_set_cmd_func(ins_para->parameter,ins_para->connection,type1,type2,type3,state);/*返回0表示失败，返回1表示成功*/
																									  /*返回-1表示input parameter error*/
																									  /*返回-2表示error*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"con_wids_fail"));
		  		 flag=0;
		  		 break;
		  case 1:break;	
		  case -1:ShowAlert(search(lpublic,"input_para_error"));
		 		  flag=0; 
		  		  break;	
		  case -2:ShowAlert(search(lpublic,"error"));
		  		  flag=0;
		  		  break;
		}
	}	

	/****************set wtp wids interval cmd****************/
    memset(interval,0,sizeof(interval));
    cgiFormStringNoNewlines("interval",interval,5);
	if(strcmp(interval,""))
	{
		ret=set_wtp_wids_interval_cmd_func(ins_para->parameter,ins_para->connection,interval);/*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示unknown id format*/
																							 /*返回-2表示wtp wids interval error,should be 1 to 5 second*/
																							 /*返回-3表示wids switch is enable，返回-4表示error*/
																							 /*返回-5表示illegal input:Input exceeds the maximum value of the parameter type*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"con_interval_fail"));
		  		 flag=0;
		  		 break;
		  case 1:break;	
		  case -1:ShowAlert(search(lpublic,"unknown_id_format"));
		 		  flag=0; 
		  		  break;	
		  case -2:ShowAlert(search(lwlan,"wids_interval_illegal"));
		 		  flag=0; 
		  		  break;
		  case -3:ShowAlert(search(lwlan,"wids_switch_enable"));
		 		  flag=0; 
		  		  break;
		  case -4:ShowAlert(search(lpublic,"error"));
		  		  flag=0;
		  		  break;
		  case -5:ShowAlert(search(lpublic,"input_exceed_max_value"));
		  		  flag=0;
		  		  break;
		}
	}


	/****************set wtp wids threshold cmd****************/
    memset(probe_threshold,0,sizeof(probe_threshold));
    cgiFormStringNoNewlines("probe_threshold",probe_threshold,5);
	if(strcmp(probe_threshold,""))
	{
		ret=set_wtp_wids_threshold_cmd_func(ins_para->parameter,ins_para->connection,"probe",probe_threshold);/*返回0表示失败，返回1表示成功*/
																											 /*返回-1表示input patameter should only be 'probe' or 'other'*/
																											 /*返回-2表示unknown id format*/
																											 /*返回-3表示wtp wids threshold error,should be 1 to 100*/
																											 /*返回-4表示wids switch is enable，返回-5表示error*/
																											 /*返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"con_p_thr_fail"));
		  		 flag=0;
		  		 break;
		  case 1:break;	
		  case -1:ShowAlert(search(lpublic,"input_para_error"));
		 		  flag=0; 
		  		  break;
		  case -2:ShowAlert(search(lpublic,"unknown_id_format"));
		 		  flag=0; 
		  		  break;	
		  case -3:ShowAlert(search(lwlan,"wids_threshold_illegal"));
		 		  flag=0; 
		  		  break;
		  case -4:ShowAlert(search(lwlan,"wids_switch_enable"));
		 		  flag=0; 
		  		  break;
		  case -5:ShowAlert(search(lpublic,"error"));
		  		  flag=0;
		  		  break;
		  case -6:ShowAlert(search(lpublic,"input_exceed_max_value"));
		  		  flag=0;
		  		  break;
		}		
	}		

	memset(other_threshold,0,sizeof(other_threshold));
    cgiFormStringNoNewlines("other_threshold",other_threshold,5);
	if(strcmp(other_threshold,""))
	{
		ret=set_wtp_wids_threshold_cmd_func(ins_para->parameter,ins_para->connection,"other",other_threshold);/*返回0表示失败，返回1表示成功*/
																											 /*返回-1表示input patameter should only be 'probe' or 'other'*/
																											 /*返回-2表示unknown id format*/
																											 /*返回-3表示wtp wids threshold error,should be 1 to 100*/
																											 /*返回-4表示wids switch is enable，返回-5表示error*/
																											 /*返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"con_o_thr_fail"));
		  		 flag=0;
		  		 break;
		  case 1:break;	
		  case -1:ShowAlert(search(lpublic,"input_para_error"));
		 		  flag=0; 
		  		  break;
		  case -2:ShowAlert(search(lpublic,"unknown_id_format"));
		 		  flag=0; 
		  		  break;	
		  case -3:ShowAlert(search(lwlan,"wids_threshold_illegal"));
		 		  flag=0; 
		  		  break;
		  case -4:ShowAlert(search(lwlan,"wids_switch_enable"));
		 		  flag=0; 
		  		  break;
		  case -5:ShowAlert(search(lpublic,"error"));
		  		  flag=0;
		  		  break;
		  case -6:ShowAlert(search(lpublic,"input_exceed_max_value"));
		  		  flag=0;
		  		  break;
		}
	}

	/****************set wtp wids lasttime cmd****************/
    memset(last_time_in_black,0,sizeof(last_time_in_black));
    cgiFormStringNoNewlines("last_time_in_black",last_time_in_black,10);
	if(strcmp(last_time_in_black,""))
	{
		ret=set_wtp_wids_lasttime_cmd_func(ins_para->parameter,ins_para->connection,last_time_in_black);/*返回0表示失败，返回1表示成功*/
																									   /*返回-1表示unknown id format*/
																									   /*返回-2表示wtp wids lasttime in black error,should be 1 to 36000*/
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"con_lasttime_fail"));
		  		 flag=0;
		  		 break;
		  case 1:break;	
		  case -1:ShowAlert(search(lpublic,"unknown_id_format"));
		 		  flag=0; 
		  		  break;	
		  case -2:ShowAlert(search(lwlan,"wids_lasttime_illegal"));
		 		  flag=0; 
		  		  break;
		}
	}
	
	if(flag==1)
	  ShowAlert(search(lpublic,"oper_succ"));	
}


