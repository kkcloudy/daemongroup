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
* wp_wtpcon.c
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
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"


int ShowWtpconPage(char *m,char *n,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan); 
void config_wtp(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char ID[10] = { 0 };
  char pno[10] = { 0 };  
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
  memset(pno,0,sizeof(pno));  
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("ID", ID, 10);		
	cgiFormStringNoNewlines("PN",pno,10);
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {    
	cgiFormStringNoNewlines("encry_conwtp",encry,BUF_LEN);
	cgiFormStringNoNewlines("wtp_id",ID,10);	  
	cgiFormStringNoNewlines("page_no",pno,10);
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

  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
  else
	ShowWtpconPage(encry,ID,pno,instance_id,paraHead1,lpublic,lwlan);
  
  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWtpconPage(char *m,char *n,char *pn,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  char IsSubmit[5] = { 0 };
  FILE *fp = NULL;
  DCLI_WTP_API_GROUP_ONE *wtp = NULL;
  int i = 0,status = 1,result = 0;
  char BindInter[20] = { 0 };
  char *endptr = NULL;
  char *retu = NULL;
  int wtpID = 0;
  wtpID= strtoul(n,&endptr,10);	  /*char转成int，10代表十进制*/  
  int ap_reboot_result = 0,clear_update_config_result = 0;
  int ret = 0;
  DCLI_WTP_API_GROUP_TWO *WTPINFO = NULL;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>");     
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("wtpcon_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
    if(ins_para)
	{
		config_wtp(ins_para,wtpID,lpublic,lwlan);	 
	}
  }  
  if((cgiFormSubmitClicked("ap_reboot") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	if(ins_para)
	{
		ap_reboot_result=set_ap_reboot_func(ins_para->parameter,ins_para->connection,wtpID); /*返回0表示失败，返回1表示成功，返回-1表示wtp is not in run state*/
																							 /*返回-2表示wtp id does not exist，返回-3表示error*/
	}
	switch(ap_reboot_result)
	{
		case SNMPD_CONNECTION_ERROR:
    	case 0:ShowAlert(search(lpublic,"ap_reboot_fail"));
			   break;
		case 1:ShowAlert(search(lpublic,"ap_reboot_succ"));
			   break;
		case -1:ShowAlert(search(lwlan,"wtp_not_run"));
			    break;
		case -2:ShowAlert(search(lwlan,"wtp_not_exist"));
				break;
		case -3:ShowAlert(search(lpublic,"error"));
				break;
    }
  }
  if((cgiFormSubmitClicked("clear_upgrade_config") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	if(ins_para)
	{
		clear_update_config_result=clear_wtp_img_cmd_func(ins_para->parameter,ins_para->connection,wtpID);	/*返回0表示失败，返回1表示成功，返回-1表示error*/
	}
	switch(clear_update_config_result)
	{
		case SNMPD_CONNECTION_ERROR:
    	case 0:ShowAlert(search(lwlan,"clear_force_update_config_fail"));
			   break;
		case 1:ShowAlert(search(lwlan,"clear_force_update_config_succ"));
			   break;
		case -1:ShowAlert(search(lpublic,"error"));
				break;
    }
  }
  fprintf(cgiOut,"<body>"\
  "<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=wtpcon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
   fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtplis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pn,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> AP</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
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
				  for(i=0;i<8;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                      "<table width=720 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
						fprintf(cgiOut,"</tr>"\
						"<tr>"\
						  "<td>");
				  status = system("bind_inter.sh"); 
				  if(ins_para)
				  {
					  result=show_wtp_one(ins_para->parameter,ins_para->connection,wtpID,&wtp);
					  ret = show_ap_if_info_func(ins_para->parameter,ins_para->connection,wtpID,&WTPINFO);
				  }
				             fprintf(cgiOut,"<table width=720 border=0 cellspacing=0 cellpadding=0>"\
							   "<tr height=30 align=left>"\
							   "<td id=thead5 align=left>%s AP %d</td>",search(lpublic,"configure"),wtpID);		
							   fprintf(cgiOut,"</tr>"\
							   "</table>"\
						  "</td>"\
						"</tr>"\
						"<tr><td align=center style=\"padding-left:20px\">");
				fprintf(cgiOut,"<table width=720 border=0 cellspacing=0 cellpadding=0>");
				fprintf(cgiOut,"<tr height=30>");
				   fprintf(cgiOut,"<td width=150>%s:</td>",search(lwlan,"bind_interface"));				 
                 fprintf(cgiOut,"<td width=100 align=left>");
				 if(status==0)
				 {
				   if((result == 1)&&(wtp)&&(wtp->WTP[0])&&(wtp->WTP[0]->isused==1))
					 fprintf(cgiOut,"<select name=bind_interface id=bind_interface style=width:100px disabled>");  /*如果AP状态为used，绑定接口的下拉框不可用*/
				   else
					 fprintf(cgiOut,"<select name=bind_interface id=bind_interface style=width:100px>");
				   if((fp=fopen("/var/run/apache2/bind_inter.tmp","r"))==NULL)		 /*以只读方式打开资源文件*/
				   {
					   ShowAlert(search(lpublic,"error_open"));
			       }
				   else
				   {
					   memset(BindInter,0,sizeof(BindInter));
					   retu=fgets(BindInter,20,fp);
					   while(retu!=NULL)
					   {
						 if((result == 1)&&(wtp)&&(wtp->WTP[0])&&(wtp->WTP[0]->apply_interface_name)&&(strncmp(retu,wtp->WTP[0]->apply_interface_name,(strlen(retu)-1))==0))
						   fprintf(cgiOut,"<option value=%s selected=selected>%s",retu,retu);
						 else
						   fprintf(cgiOut,"<option value=%s>%s",retu,retu);
						 memset(BindInter,0,sizeof(BindInter));
						 retu=fgets(BindInter,20,fp);
					   }				   
					   fclose(fp);	
				   }
				   fprintf(cgiOut,"</select>");				   
				 }
				 else
				 {
  				    fprintf(cgiOut,"%s",search(lpublic,"exec_shell_fail"));
				 }
		        fprintf(cgiOut,"</td>");
				  fprintf(cgiOut,"<td width=470 align=left style=\"padding-left:30px\"><font color=red>(%s)</font>"\
				  				 "<input type=submit style=\"width:100px; margin-left:20px\" border=0 name=ap_reboot style=background-image:url(/images/SubBackGif.gif) value=\"%s\">"\
				  				 "</td>",search(lwlan,"wtp_bind"),search(lpublic,"ap_reboot"));
                fprintf(cgiOut,"</tr>");
                fprintf(cgiOut,"<tr height=30>"\
                 "<td>AP %s:</td>",search(lwlan,"state"));
                 fprintf(cgiOut,"<td align=left><select name=wtp_use id=wtp_use style=width:100px>");
				 if((result == 1)&&(wtp)&&(wtp->WTP[0])&&(wtp->WTP[0]->isused==1))
				 {
  				    fprintf(cgiOut,"<option value=unused>unused"\
  				    "<option value=used selected=selected>used");
				 }
				 else
				 {
  				    fprintf(cgiOut,"<option value=unused selected=selected>unused"\
  				    "<option value=used>used");
				 }
	             fprintf(cgiOut,"</select></td>");
				  fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"wtp_able"));
                fprintf(cgiOut,"</tr>"\
				"<tr height=30>"\
						"<td>AP %s:</td>",search(lpublic,"name"));
				fprintf(cgiOut,"<td><input name=ap_new_num size=15 maxLength=%d onkeypress=\"return event.keyCode!=32\" value=\"\"></td>",DEFAULT_LEN-1);
				 				fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(%s)</font></td>",search(lwlan,"mod_ap_name"));
				fprintf(cgiOut,"</tr>"
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"wlan_sta_max"));
				fprintf(cgiOut,"<td><input name=max_num size=15 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\"></td>"\
				 				"<td align=left style=\"padding-left:30px\"><font color=red>(0--32767)</font></td>"\
				"</tr>"
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"wtp_triger_num"));
				fprintf(cgiOut,"<td><input name=wtp_triger_num size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\"></td>"\
				 				"<td align=left style=\"padding-left:30px\"><font color=red>(1--64)</font></td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"wtp_flow_triger"));
				fprintf(cgiOut,"<td><input name=wtp_flow_triger size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\"></td>"\
				 				"<td align=left style=\"padding-left:30px\"><font color=red>(0--1024)</font></td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lpublic,"ntp_syn"));
                fprintf(cgiOut,"<td><select name=ntp_synch_type id=ntp_synch_type style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=start>start</option>"\
                                  "<option value=stop>stop</option>"\
                     			"</td>"\
                                "<td align=left style=\"padding-left:30px\"><input name=ntp_synch_value size=15 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"><font color=red>(60--65535)s</font></td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"ap_sta_info_report_switch"));
                fprintf(cgiOut,"<td colspan=2><select name=ap_sta_info_report_switch id=ap_sta_info_report_switch style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=enable>enable</option>"\
                                  "<option value=disable>disable</option>"\
                     			"</td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"ap_extension_info_switch"));
                fprintf(cgiOut,"<td colspan=2><select name=ap_extension_info_switch id=ap_extension_info_switch style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=enable>enable</option>"\
                                  "<option value=disable>disable</option>"\
                     			"</td>"\
				"</tr>"\
				"<tr height=30>"\
						"<td>%s:</td>",search(lwlan,"ap_eth_rate"));
                fprintf(cgiOut,"<td><select name=ap_eth_ifindex id=ap_eth_ifindex style=width:100px>"\
                                  "<option value=></option>");
								  if((1 == ret)&&(WTPINFO)&&(WTPINFO->WTP[0]))
								  {
								  	for(i=0; i<WTPINFO->WTP[0]->apifinfo.eth_num; i++)
								  	{
										fprintf(cgiOut,"<option value=%d>eth%d</option>",i,i);
								  	}
								  }
								  else
								  {
								  	fprintf(cgiOut,"<option value=0>eth0</option>");
								  }
                     			fprintf(cgiOut,"</td>"\
                                "<td align=left style=\"padding-left:30px\"><select name=ap_eth_rate id=ap_eth_rate style=width:100px>"\
                                  "<option value=></option>"\
                                  "<option value=10>10M</option>"\
                                  "<option value=100>100M</option>"\
                                  "<option value=1000>1000M</option>"\
                                "</td>"\
				"</tr>"\
				"<tr valign=top style=\"padding-top:10px\">"\
				  "<td colspan=2 width=250>"\
				    "<fieldset align=left>"\
					  "<legend><font color=Navy>%s</font></legend>",search(lwlan,"force_update"));
					  fprintf(cgiOut,"<table width=232 border=0 cellspacing=0 cellpadding=0>"\
					    "<tr height=30>"\
				   		  "<td width=132>%s:</td>",search(lpublic,"filename"));
						  fprintf(cgiOut,"<td width=100><input name=filename size=15 maxLength=63 value=\"\"></td>"\
						"</tr>"\
						"<tr height=30>"\
				   		  "<td>%s:</td>",search(lpublic,"version"));
						  fprintf(cgiOut,"<td><input name=version size=15 maxLength=63 value=\"\"></td>"\
						"</tr>"\
						"<tr height=30>"\
				   		  "<td>&nbsp;</td>"\
						  "<td><select name=time id=time style=width:100px>"\
						  		"<option value=>"\
			  				    "<option value=now>now"\
			  				    "<option value=later>later"\
				          "</select></td>"\
						"</tr>"\
					  "</table>"\
					"</fieldset>"); /*框下边*/
				  fprintf(cgiOut,"</td>");
				    if(strcmp(search(lpublic,"filename"),"Filename")==0)
				      fprintf(cgiOut,"<td style=\"padding-left:185px\"><input type=submit style=\"width:200px; margin-left:20px\" border=0 name=clear_upgrade_config style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"clear_force_update_config"));
					else
					  fprintf(cgiOut,"<td style=\"padding-left:110px\"><input type=submit style=\"width:120px; margin-left:20px\" border=0 name=clear_upgrade_config style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"clear_force_update_config"));
				fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr>"\
				  "<td><input type=hidden name=encry_conwtp value=%s></td>",m);
				  fprintf(cgiOut,"<td><input type=hidden name=wtp_id value=%s></td>",n); 
				  fprintf(cgiOut,"<td><input type=hidden name=page_no value=%s></td>",pn);
				  fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				  "<td colspan=4><input type=hidden name=instance_id value=%s></td>",ins_id);
				fprintf(cgiOut,"</tr>"\
			    "</table>");
				fprintf(cgiOut,"</td></tr>"\
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
  Free_one_wtp_head(wtp);
}
if(ret == 1)
{
  free_show_ap_if_info(WTPINFO);
}
return 0;
}


void config_wtp(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)   /*返回0表示失败，返回1表示成功*/
{
  int use_state = 0,ret1 = 0,ret2 = 0,ret3 = 0,ret4 = 0,ret5 = 0,ret6 = 0,ret7 = 0,ret8 = 0,ret9 = 0,ret10 = 0,ret11 = 0,flag1 = 1,flag2 = 1;
  char interf[20] = { 0 };
  char state[20] = { 0 };  
  char *endptr = NULL;  
  int hidden = 1;
  char new_name[DEFAULT_LEN+5] = { 0 };
  char maxnum[10] = { 0 };
  int sta_num = 0;
  char temp[100] = { 0 };
  char triger_num[10] = { 0 };
  char flow_triger[10] = { 0 };
  char ntp_synch_type[10] = { 0 };
  char ntp_synch_value[10] = { 0 };
  int ntp_synch_v = 3600;
  char ap_sta_info_report_switch[10] = { 0 };
  char ap_extension_info_switch[10] = { 0 };
  char ap_eth_ifindex[5] = { 0 };
  char ap_eth_rate[10] = { 0 };
  char alt[100] = { 0 };
  char max_wtp_num[10] = { 0 };
  int trNum = 0;
  int flowNum = 0;
  char default_len[10] = { 0 };
  char file_name[64] = { 0 };
  char version[64] = { 0 };
  char time[10] = { 0 };
  
  memset(state,0,sizeof(state));
  cgiFormStringNoNewlines("wtp_use",state,20);	
  if(strcmp(state,"unused")==0)
  	use_state=1;
  else
  	use_state=0;
  if(use_state==1)
  {
    ret2=wtp_used(ins_para->parameter,ins_para->connection,id,use_state);         /*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed，返回-2表示You should be apply interface first，返回-3表示You should be apply wlan id first，返回-4表示map L3 interace error，返回-5表示BSS interface policy conflict，返回-6表示error*/
	switch(ret2)
	{
	  case SNMPD_CONNECTION_ERROR:
      case 0:{
	  	       ShowAlert(search(lwlan,"set_wtp_use_fail"));
	  		   hidden = 0;
	           break;
      	     }
	  case 1:break;
      case -1:{
	  	        ShowAlert(search(lwlan,"wtp_not_exist"));
	  			hidden = 0;
				break;
      	      }
	  case -2:{
	  	        ShowAlert(search(lwlan,"bind_interface_first"));
	  		    hidden = 0;
	            break;
	  	      }
	  case -3:{
	  	        ShowAlert(search(lwlan,"bind_wlan_first"));
	  		    hidden = 0;
	            break;
	  	      }
	  case -4:{
	  	        ShowAlert(search(lwlan,"map_l3_inter_error"));
	  		    hidden = 0;
			    break;
	  	      }
	  case -5:{
	  	        ShowAlert(search(lwlan,"bss_ifpolicy_conflict"));			
			    hidden = 0;
			    break;
	  	      }
	  case -6:{
	  	        ShowAlert(search(lpublic,"error"));
			    hidden = 0;
			    break;
	  	      }
	}
  }
  else
  {
    /*******************wtp apply interface ******************/
	  memset(interf,0,sizeof(interf));
	  cgiFormStringNoNewlines("bind_interface",interf,20);
	  if(strcmp(interf,""))
	  {
		  ret1=wtp_apply_interface(ins_para->parameter,ins_para->connection,id,interf);/*返回0表示失败，返回1表示成功，返回-1表示the length of interface name excel 16*/
																					  /*返回-2表示interface does not exist，返回-3表示if you want to change binding interface, please delete binding wlan id first*/
																					  /*返回-4表示error，返回-5示WTP ID非法*/
																					  /*返回-8表示is no local interface, permission denial*/
																					  /*返回-9表示if you want to change binding interface, please unused wtp  first*/
																					  /*返回-10表示interface has be binded in other hansi*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
		  switch(ret1)
		  {
		    case SNMPD_CONNECTION_ERROR:
			case 0:{
					  ShowAlert(search(lwlan,"bind_interface_fail"));
					  hidden = 0;
					  break;
				   }
			case 1:break;
			case -1:{
					  ShowAlert(search(lpublic,"interface_too_long"));	
					  hidden = 0;
					  break; 
					}
			case -2:{
					  ShowAlert(search(lpublic,"interface_not_exist"));
					  hidden = 0;
					  break;
					}
			case -3:{
					  ShowAlert(search(lwlan,"delete_bind_wlan_first"));
					  hidden = 0;
					  break;
					}
			case -4:{
			   	      ShowAlert(search(lpublic,"error"));
			   		  hidden = 0;
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
					  hidden = 0;
					  break;
				    }
			case -8:{
				      ShowAlert(search(lpublic,"no_local_interface"));
					  hidden = 0;
				      break;
				    }
			case -9:{
				      ShowAlert(search(lwlan,"unuse_wtp_first"));
					  hidden = 0;
				      break;
				    }
			case -10:{
					   ShowAlert(search(lwlan,"interface_bind_in_other_hansi"));
					   hidden = 0;
					   break;	
					 }
		  }
	  }


	
   if(hidden==1)  /*成功绑定接口*/
   {
     ret2=wtp_used(ins_para->parameter,ins_para->connection,id,use_state);          /*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed，返回-2表示You should be apply interface first，返回-3表示You should be apply wlan id first，返回-4表示map L3 interace error，返回-5表示BSS interface policy conflict，返回-6表示error*/
     switch(ret2)
     {
       case SNMPD_CONNECTION_ERROR:
       case 0:{
	   	        ShowAlert(search(lwlan,"set_wtp_use_fail"));
	   		    hidden = 0;
                break;
       	      }
       case 1:break;
	   case -1:{
	   	         ShowAlert(search(lwlan,"wtp_not_exist"));
	   			 hidden = 0;
                 break;
	   	       }
       case -2:{
	   	         ShowAlert(search(lwlan,"bind_interface_first"));
	   			 hidden = 0;
                 break;
       	       }
	   case -3:{
	   	         ShowAlert(search(lwlan,"bind_wlan_first"));
	   			 hidden = 0;
                 break;
	   	       }
	   case -4:{
	   	         ShowAlert(search(lwlan,"map_l3_inter_error"));
	   			 hidden = 0;
                 break;
	   	       }
	   case -5:{
	   	         ShowAlert(search(lwlan,"bss_ifpolicy_conflict"));
	   			 hidden = 0;
                 break;
	   	       }
	   case -6:{
	   	         ShowAlert(search(lpublic,"error"));
	   			 hidden = 0;
                 break;
	   	       }
     }
       }
  }


  memset(new_name,0,sizeof(new_name));
  cgiFormStringNoNewlines("ap_new_num",new_name,DEFAULT_LEN+5);
  if(strcmp(new_name,"")!=0)
  {
  	if(strchr(new_name,' ')==NULL)/*不包含空格*/
  	{
		ret6=set_wtp_wtpname(ins_para->parameter,ins_para->connection,id,new_name);
		switch(ret6)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:hidden=0;
				   ShowAlert(search(lwlan,"mod_ap_name_fail"));
				   break;
			case 1:break;
			case -1:hidden=0;
					memset(temp,0,sizeof(temp));
					strncpy(temp,search(lwlan,"most1"),sizeof(temp)-1);
					memset(default_len,0,sizeof(default_len));
					snprintf(default_len,sizeof(default_len)-1,"%d",DEFAULT_LEN-1);
					strncat(temp,default_len,sizeof(temp)-strlen(temp)-1);
					strncat(temp,search(lwlan,"most2"),sizeof(temp)-strlen(temp)-1);
					ShowAlert(temp);
					break;
			case -2:hidden=0;
				    ShowAlert(search(lwlan,"wtp_not_exist"));
				    break;
		}
  	}
	else
	{
		hidden=0;
	    ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
	}
  }

	
	 cgiFormStringNoNewlines("max_num",maxnum,10);
	 if(strcmp(maxnum,""))
	 {
		 sta_num=strtoul(maxnum,&endptr,10);	/*char转成int，10代表十进制*/
		 if((sta_num>=0)&&(sta_num<32768))
		 {
			ret3=config_wtp_max_sta_num(ins_para->parameter,ins_para->connection,id,maxnum);
		    switch (ret3)
		    {
		    	case SNMPD_CONNECTION_ERROR:
		    	case 0:{
						 memset(temp,0,sizeof(temp));
						 strncpy(temp,search(lwlan,"con_bss_max_sta_num_fail1"),sizeof(temp)-1);
						 strncat(temp,"WTP",sizeof(temp)-strlen(temp)-1);
						 strncat(temp,search(lwlan,"con_bss_max_sta_num_fail2"),sizeof(temp)-strlen(temp)-1);
						 ShowAlert(temp);
						 hidden = 0;
					     break;
					   }
				case 1:break;
				case -1:{
						  ShowAlert(search(lwlan,"wtp_not_exist"));   		/*wtp not exist*/
						  hidden = 0;
						  break; 
						}
				case -2:{
				          ShowAlert(search(lwlan,"more_sta_has_access")); 	/*more sta(s) has accessed before you set max sta num*/
						  hidden = 0;
			              break; 
				        }
				case -3:{
						  ShowAlert(search(lpublic,"oper_fail"));		 	/*operation fail*/
						  hidden = 0;
						  break; 
						}
				case -4:{
						  ShowAlert(search(lpublic,"error"));		  		 /*error*/
						  hidden = 0;
						  break;  
						}
				case -5:{													 /*WTP ID非法*/
						  memset(temp,0,sizeof(temp));
						  strncpy(temp,search(lwlan,"wtp_id_illegal1"),sizeof(temp)-1);
						  memset(max_wtp_num,0,sizeof(max_wtp_num));
						  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
						  strncat(temp,max_wtp_num,sizeof(temp)-strlen(temp)-1);
						  strncat(temp,search(lwlan,"wtp_id_illegal2"),sizeof(temp)-strlen(temp)-1);
						  ShowAlert(temp);
						  hidden=0;
						  break;
					    }
				case -10:{													 /*input num should be 0-64*/
						  memset(temp,0,sizeof(temp));
					      strncpy(temp,search(lwlan,"max_sta_num_illegal1"),sizeof(temp)-1);
					      strncat(temp,"32767",sizeof(temp)-strlen(temp)-1);
					      strncat(temp,search(lwlan,"max_sta_num_illegal2"),sizeof(temp)-strlen(temp)-1);
					      ShowAlert(temp);
						  hidden = 0;
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
		   hidden = 0;
		 }
	 }
  	
  memset(triger_num,0,sizeof(triger_num));
  cgiFormStringNoNewlines("wtp_triger_num",triger_num,10);
  if(strcmp(triger_num,"")!=0) 
  {
    trNum= strtoul(triger_num,&endptr,10);					   /*char转成int，10代表十进制*/		
    if((trNum>0)&&(trNum<65)) 
    {
      ret4=config_wtp_triger_num(ins_para->parameter,ins_para->connection,id,triger_num);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示operation fail，返回-3表示triger num must be little than max sta num，返回-4表示error，返回-5示WTP ID非法*/
								  														/*返回-9表示unknown id format，返回-10表示input triger num should be 1~64*/
	  switch(ret4)
	  {
	    case SNMPD_CONNECTION_ERROR:
	    case 0:{
			     ShowAlert(search(lwlan,"con_triger_num_fail"));
			     hidden = 0;
                 break;
	    	   } 
        case 1:break;
	    case -1:{
			      ShowAlert(search(lwlan,"wtp_not_exist"));               /*wtp id not exist*/
  	              hidden = 0;
			      break; 
	    	    }
	    case -2:{
			      ShowAlert(search(lpublic,"oper_fail"));                 /*operation fail*/
			      hidden = 0;
			      break; 
	    	    }
	    case -3:{
			      ShowAlert(search(lwlan,"triger_num_little_sta_num"));   /*triger num must be little than max sta num*/
		 	      hidden = 0;
			      break; 
	    	    }
	    case -4:{
			      ShowAlert(search(lpublic,"error"));                     /*error*/
		 	      hidden = 0;
			      break;  
	    	    }
		case -5:{														  /*WTP ID非法*/
				  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				  memset(max_wtp_num,0,sizeof(max_wtp_num));
				  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				  ShowAlert(alt);
				  hidden = 0;
				  break;
	  		    }
		case -9:{
			      ShowAlert(search(lpublic,"unknown_id_format"));   	 /*unknown id format*/
		 	      hidden = 0;
			      break; 
	    	    }
		case -10:{
			       ShowAlert(search(lwlan,"triger_num_illegal"));   	 /*input triger num should be 1~64*/
		 	       hidden = 0;
			       break; 
	    	     }
	  }
    }
    else
    {
	  ShowAlert(search(lwlan,"triger_num_illegal"));
	  flag1=0;
    }
  }

  memset(flow_triger,0,sizeof(flow_triger));
  cgiFormStringNoNewlines("wtp_flow_triger",flow_triger,10);
  if(strcmp(flow_triger,"")!=0) 
  {
    flowNum= strtoul(flow_triger,&endptr,10);					   /*char转成int，10代表十进制*/		
    if((flowNum>=0)&&(flowNum<1025)) 
    {
      ret5=set_wtp_flow_trige(ins_para->parameter,ins_para->connection,id,flow_triger);  /*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示operation fail，返回-3表示flow triger must be <0-30>，返回-4表示error，返回-5示WTP ID非法*/
								  														/*返回-9表示unknown id format，返回-10表示input flow triger should be 0~1024*/
	  switch(ret5)
	  {
	    case SNMPD_CONNECTION_ERROR:
	    case 0:{
			     ShowAlert(search(lwlan,"con_flow_triger_fail"));
			     hidden = 0;
                 break;
	    	   }
        case 1:break;
	    case -1:{
			      ShowAlert(search(lwlan,"wtp_not_exist"));               /*wtp id not exist*/
  	              hidden = 0;
			      break; 
	    	    }
	    case -2:{
			      ShowAlert(search(lpublic,"oper_fail"));                 /*operation fail*/
			      hidden = 0;
			      break; 
	    	    }
	    case -3:{
			      ShowAlert(search(lwlan,"flow_triger_num_illegal"));     /*flow triger must be <0-30>*/
		 	      hidden = 0;  
			      break; 
	    	    }
	    case -4:{
			      ShowAlert(search(lpublic,"error"));                     /*error*/
		 	      hidden = 0;
			      break;  
	    	    }
		case -5:{														  /*WTP ID非法*/
				  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				  memset(max_wtp_num,0,sizeof(max_wtp_num));
				  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				  ShowAlert(alt);
				  hidden = 0;
				  break;
	  		    }
		case -9:{
			      ShowAlert(search(lpublic,"unknown_id_format"));   	 /*unknown id format*/
		 	      hidden = 0;
			      break; 
	    	    }
		case -10:{
			       ShowAlert(search(lwlan,"flow_triger_num_illegal"));   	 /*input flow triger should be 0~1024*/
		 	       hidden = 0;
			       break; 
	    	     }
	  }
    }
    else
    {
	  ShowAlert(search(lwlan,"flow_triger_num_illegal"));
	  flag2=0;
    }
  }

  memset(ntp_synch_type,0,sizeof(ntp_synch_type));
  cgiFormStringNoNewlines("ntp_synch_type",ntp_synch_type,10);
  memset(ntp_synch_value,0,sizeof(ntp_synch_value));
  cgiFormStringNoNewlines("ntp_synch_value",ntp_synch_value,10);
  if((strcmp(ntp_synch_type,"")!=0) && (strcmp(ntp_synch_value,"")!=0))
  {
  	ntp_synch_v = strtoul(ntp_synch_value,&endptr,10);	
	ret8=set_ac_ap_ntp_func(ins_para->parameter,ins_para->connection,id,ntp_synch_type,ntp_synch_value);   /*返回0表示失败，返回1表示成功*/
																										  /*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
																										  /*返回-5表示input interface only with 'start' or 'stop'，返回-6表示interval should be 60-65535*/
	switch(ret8)										  
	{
	  case SNMPD_CONNECTION_ERROR:
	  case 0:{
			      ShowAlert(search(lpublic,"ntp_syn_fail"));
	  		      hidden = 0;
			      break;
	  		 }
	  case 1:break;
	  /*case -1:{
		  		  ShowAlert(search(lwlan,"wtp_not_run"));
		  		  hidden = 0;
				  break;
	  		  }*/
	  case -2:{
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  hidden = 0;
				  break;
	  		  }
	  case -3:{
				  ShowAlert(search(lpublic,"error"));
				  hidden = 0;
				  break;
	  		  }
	  case -4:{
				  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				  memset(max_wtp_num,0,sizeof(max_wtp_num));
				  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				  ShowAlert(alt);
				  hidden = 0;
				  break;
	  		  }
	  case -5:{
				  ShowAlert(search(lpublic,"input_para_error"));
				  hidden = 0;
				  break;
	  		  }
	  case -6:{
				  ShowAlert(search(lwlan,"invalid_ntp_interval"));
				  hidden = 0;
				  break;
	  		  }
	}
  }

  memset(ap_sta_info_report_switch,0,sizeof(ap_sta_info_report_switch));
  cgiFormStringNoNewlines("ap_sta_info_report_switch",ap_sta_info_report_switch,10);
  if(strcmp(ap_sta_info_report_switch,"")!=0)
  {
	ret9=set_ap_sta_infomation_report_enable_func(ins_para->parameter,ins_para->connection,id,ap_sta_info_report_switch);   
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示input patameter only with 'enable' or 'disable'*/
																							/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run*/
																							/*返回-4表示error，返回-5示WTP ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
	switch(ret9)										  
	{
	  case SNMPD_CONNECTION_ERROR:
	  case 0:{
			      ShowAlert(search(lwlan,"con_ap_sta_info_report_switch_fail"));
	  		      hidden = 0;
			      break;
	  		 }
	  case 1:break;
	  case -1:{
		  		  ShowAlert(search(lpublic,"input_para_illegal"));
		  		  hidden = 0;
				  break;
	  		  }
	  case -2:{
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  hidden = 0;
				  break;
	  		  }
	  case -3:{
				  ShowAlert(search(lwlan,"wtp_not_run"));
				  hidden = 0;
				  break;
	  		  }	  
	  case -4:{
				  ShowAlert(search(lpublic,"error"));
				  hidden = 0;
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
				  hidden = 0;
				  break;
	  		  }
	}
  }

  memset(ap_extension_info_switch,0,sizeof(ap_extension_info_switch));
  cgiFormStringNoNewlines("ap_extension_info_switch",ap_extension_info_switch,10);
  if(strcmp(ap_extension_info_switch,"")!=0)
  {
	ret10=set_ap_extension_infomation_enable(ins_para->parameter,ins_para->connection,id,ap_extension_info_switch);   
																					/*返回0表示失败，返回1表示成功*/
																					/*返回-1表示input patameter only with 'enable' or 'disable'*/
																					/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run*/
																					/*返回-4表示error，返回-5示WTP ID非法*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
	switch(ret10)										  
	{
	  case SNMPD_CONNECTION_ERROR:
	  case 0:{
			      ShowAlert(search(lwlan,"con_ap_sta_info_report_switch_fail"));
	  		      hidden = 0;
			      break;
	  		 }
	  case 1:break;
	  case -1:{
		  		  ShowAlert(search(lpublic,"input_para_illegal"));
		  		  hidden = 0;
				  break;
	  		  }
	  case -2:{
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  hidden = 0;
				  break;
	  		  }
	  case -3:{
				  ShowAlert(search(lwlan,"wtp_not_run"));
				  hidden = 0;
				  break;
	  		  }	  
	  case -4:{
				  ShowAlert(search(lpublic,"error"));
				  hidden = 0;
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
				  hidden = 0;
				  break;
	  		  }
	}
  }

  memset(ap_eth_ifindex,0,sizeof(ap_eth_ifindex));
  cgiFormStringNoNewlines("ap_eth_ifindex",ap_eth_ifindex,5);
  memset(ap_eth_rate,0,sizeof(ap_eth_rate));
  cgiFormStringNoNewlines("ap_eth_rate",ap_eth_rate,10);
  if((strcmp(ap_eth_ifindex,"")!=0)&&(strcmp(ap_eth_rate,"")!=0))
  {
	ret11=set_ap_if_rate_cmd(ins_para->parameter,ins_para->connection,id,ap_eth_ifindex,ap_eth_rate);   
																				  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																				  /*返回-2表示input interface only with '10' or '100' or '1000'，返回-3表示wtp is not in run state*/
																				  /*返回-4表示wtp id does not exist，返回-5表示eth if_index does not exist*/
																				  /*返回-6表示error，返回-7示WTP ID非法*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
	switch(ret11)										  
	{
	  case SNMPD_CONNECTION_ERROR:
	  case 0:{
			      ShowAlert(search(lwlan,"con_ap_eth_rate_fail"));
	  		      hidden = 0;
			      break;
	  		 }
	  case 1:break;
	  case -1:{
		  		  ShowAlert(search(lpublic,"unknown_id_format"));
		  		  hidden = 0;
				  break;
	  		  }
	  case -2:{
		  		  ShowAlert(search(lpublic,"input_para_illegal"));
		  		  hidden = 0;
				  break;
	  		  }	 
	  case -3:{
				  ShowAlert(search(lwlan,"wtp_not_run"));
				  hidden = 0;
				  break;
	  		  }	  
	  case -4:{
				  ShowAlert(search(lwlan,"wtp_not_exist"));
				  hidden = 0;
				  break;
	  		  }
	  case -5:{
				  ShowAlert(search(lwlan,"eth_ifindex_not_exist"));
				  hidden = 0;
				  break;
	  		  }
	  case -6:{
				  ShowAlert(search(lpublic,"error"));
				  hidden = 0;
				  break;
	  		  }
	  case -7:{
				  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				  memset(max_wtp_num,0,sizeof(max_wtp_num));
				  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				  ShowAlert(alt);
				  hidden = 0;
				  break;
	  		  }
	}
  }

  memset(file_name,0,sizeof(file_name));
  cgiFormStringNoNewlines("filename",file_name,64);
  memset(version,0,sizeof(version));
  cgiFormStringNoNewlines("version",version,64);
  memset(time,0,sizeof(time));
  cgiFormStringNoNewlines("time",time,10);
  if((strcmp(file_name,""))&&(strcmp(version,""))&&(strcmp(time,"")))
  {
    ret7=update_wtp_img_cmd_func(ins_para->parameter,ins_para->connection,id,file_name,version,time);   /*返回0表示失败，返回1表示成功*/
																									   /*返回-1表示set update failed due to system cann't find file*/
																									   /*返回-2表示set update failed due to file version error*/
																									   /*返回-3表示error*/
	switch(ret7)
    {
      case SNMPD_CONNECTION_ERROR:
      case 0:{
		       ShowAlert(search(lwlan,"upgrade_wtp_fail"));
		       hidden = 0;
               break;
    	     }
      case 1:break;
      case -1:{
		        ShowAlert(search(lwlan,"sys_cant_find_file"));               
	            hidden = 0;
		        break; 
    	      }
      case -2:{
		        ShowAlert(search(lwlan,"file_version_error"));                
		        hidden = 0;
		        break; 
    	      }
      case -3:{
		        ShowAlert(search(lpublic,"error"));                    
	 	        hidden = 0;
		        break;  
    	      }
    }
  }
  
  if((hidden==1)&&(flag1==1)&&(flag2==1))
  	ShowAlert(search(lpublic,"oper_succ"));  
}


