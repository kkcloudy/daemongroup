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
* wp_wtpupgrade.c
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
#include <time.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


int ShowWtpUpgradePage(char *m,char *n,struct list *lpublic,struct list *lwlan);   
void Wtp_Upgrade(instance_parameter *ins_para,char *m,struct list * lpublic,struct list *lwlan);



int cgiMain()
{  
  char encry[BUF_LEN] = { 0 };	  
  char *str = NULL; 			   
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  memset(encry,0,sizeof(encry));
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {    
	cgiFormStringNoNewlines("encry_wtpupgrade",encry,BUF_LEN);
  } 
  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
	ShowWtpUpgradePage(encry,str,lpublic,lwlan);
  
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWtpUpgradePage(char *m,char *n,struct list *lpublic,struct list *lwlan)
{    
  char IsClearConfig[10] = { 0 };
  char IsSubmit[5] = { 0 };
  int i = 0,retu = 1,limit = 0,cl = 1;
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int result = 0,result1 = 0,result2 = 0,result3 = 0;
  int wtpupgrade_page_no = 0;/*wtpupgrade_page_no表示批量升级页面显示的内容
    							     wtpupgrade_page_no==1显示"配置同时升级个数"、"配置型号页面"
    							     wtpupgrade_page_no==2显示"配置同时升级个数"、"配置型号页面"、"显示升级配置信息"
    							     wtpupgrade_page_no==3显示"显示AP升级信息"*/
  DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL,*q = NULL;
  DCLI_WTP_API_GROUP_ONE *ap_head = NULL;
  struct model_tar_file head,*vq = NULL;
  int model_tar_file_num = 0;      
  int wtp_update_num=0;/*升级配置信息条数*/
  int wtp_num=0;/*AP升级信息条数*/
  char ap_model[256] = { 0 };
  int ret_ip = 0;
  char ip[WTP_WTP_IP_LEN+1] = { 0 };
  char wtp_state[20] = { 0 };
  time_t now,online_time;
  int hour = 0,min = 0,sec = 0;
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;

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

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
    "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
  "</style>"\
  "</head>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>"\
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
  "<body>");    
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("wtpupgrade_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  { 
    if(paraHead1)
	{
		Wtp_Upgrade(paraHead1,m,lpublic,lwlan);
	} 
  }
  memset(IsClearConfig,0,sizeof(IsClearConfig));
  cgiFormStringNoNewlines("IsClearConfig", IsClearConfig, 10);
  if((strcmp(IsClearConfig,"true")==0)&&(strcmp(IsSubmit,"")))
  {
	  memset(ap_model,0,sizeof(ap_model));
	  cgiFormStringNoNewlines("ApModel", ap_model, 256);
	  result=wtp_clear_ap_one_model_update_config(paraHead1->parameter,paraHead1->connection,ap_model);
	  																	/*返回0表示失败，返回1表示成功*/
																	    /*返回-1表示buf malloc failed*/
																	    /*返回-2表示upgrade is in process,changement of configuration is not allowed now*/
																	    /*返回-3表示no update config information of model*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
	  switch(result)
	  {
	  	  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"clear_batch_update_config_fail"));
				 break;
		  case 1:ShowAlert(search(lwlan,"clear_batch_update_config_succ"));
				 break;
		  case -1:ShowAlert(search(lpublic,"malloc_error"));
				 break;
		  case -2:ShowAlert(search(lwlan,"ap_upgrade_is_in_process"));
				 break;
		  case -3:ShowAlert(search(lwlan,"no_upgrade_config_info"));
				  break;
	  }
  }

  if((cgiFormSubmitClicked("clear_upgrade_config_all") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	  result=wtp_clear_ap_update_config_func(paraHead1->parameter,paraHead1->connection);
	  													   /*返回0表示失败，返回1表示成功，返回-1表示no update config information*/
	  													   /*返回-2表示upgrade is in process,changement of configuration is not allowed now*/
														   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
	  switch(result)
	  {
	  	  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"clear_batch_update_config_fail"));
				 break;
		  case 1:ShowAlert(search(lwlan,"clear_batch_update_config_succ"));
				 break;
		  case -1:ShowAlert(search(lwlan,"no_upgrade_config_info"));
				  break;
		  case -2:ShowAlert(search(lwlan,"ap_upgrade_is_in_process"));
				  break;
	  }
  }

  if((cgiFormSubmitClicked("set_ap_update_start") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	  result=wtp_set_ap_update_control_config(paraHead1->parameter,paraHead1->connection,"start");
	  															   /*返回0表示失败，返回1表示成功*/
																   /*返回-1表示input parameter can only be 'start' or 'stop'*/
																   /*返回-2表示there's no upgrade configuration,it should be configured first*/
																   /*返回-3表示error*/
																   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
	  switch(result)
	  {
	  	  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"start_ap_update_fail"));
				 break;
		  case 1:ShowAlert(search(lwlan,"start_ap_update_succ"));
				 break;
		  case -1:ShowAlert(search(lpublic,"input_para_illegal"));
				  break;
		  case -2:ShowAlert(search(lwlan,"no_upgrade_config_info"));
				  break;
		  case -3:ShowAlert(search(lpublic,"error"));
				  break;					  
	  }
  }

  if((cgiFormSubmitClicked("set_ap_update_stop") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	  result=wtp_set_ap_update_control_config(paraHead1->parameter,paraHead1->connection,"stop");
	  															   /*返回0表示失败，返回1表示成功*/
																   /*返回-1表示input parameter can only be 'start' or 'stop'*/
																   /*返回-2表示there's no upgrade configuration,it should be configured first*/
																   /*返回-3表示error*/
																   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
	  switch(result)
	  {	  	  
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"stop_ap_update_fail"));
				 break;
		  case 1:ShowAlert(search(lwlan,"stop_ap_update_succ"));
				 break;
		  case -1:ShowAlert(search(lpublic,"input_para_illegal"));
				  break;
		  case -2:ShowAlert(search(lwlan,"no_upgrade_config_info"));
				  break;
		  case -3:ShowAlert(search(lpublic,"error"));
				  break;					  
	  }
  }
  
  result1=wtp_show_ap_update_config_func(paraHead1->parameter,paraHead1->connection,&WTPINFO);
  if((result1==1)&&(WTPINFO))
  {
  	wtp_update_num = 0;
	for(q = WTPINFO; (NULL !=  q); q = q->next)
    {				  
	   wtp_update_num++;
    }
  }
  
  result3=show_update_wtp_list_func(paraHead1->parameter,paraHead1->connection,&ap_head);
  if((result3 == 1)&&(ap_head))
  {
	wtp_num = ap_head->num;
  }  

  if((result3 == 1)&&(wtp_num > 0))/*存在AP升级信息*/
  {
	wtpupgrade_page_no = 3;
  }
  else if((result1==1)&&(wtp_update_num > 0))/*存在升级配置信息*/
  {
	wtpupgrade_page_no = 2;
  }
  else/*不存在升级信息*/
  {
  	wtpupgrade_page_no = 1;
  }
  
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
	  "<tr>");	
	
		retu=checkuser_group(n);
	    if(((wtpupgrade_page_no == 1)||(wtpupgrade_page_no == 2))&&(retu==0))/*不存在升级信息且是管理员*/
	      fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=wtpupgrade_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		else
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
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
  					"<td align=left id=tdleft><a href=wp_wtplis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                    fprintf(cgiOut,"</tr>");
		    
		    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpgrouplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"ap_group_list"));                       
                    fprintf(cgiOut,"</tr>");
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
				  "<tr height=26>"\
  					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></a></td>",search(lpublic,"menu_san"),search(lwlan,"batch_update"));    /*突出显示*/             
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
				  
				  if(wtpupgrade_page_no == 3)/*存在AP升级信息*/
				  {
					if((result1 == 1)&&(wtp_num > 9))
					{
						limit+=(wtp_num-9);
					}
				  }
				  else if(wtpupgrade_page_no == 2)/*存在升级配置信息*/
				  {
					if((result1 == 1)&&(wtp_update_num > 1))
					{
						limit+=(wtp_update_num-1);
					}
				  }
				  else/*不存在升级信息*/
				  {
				  	limit=0;
				  }
				  	
				  if(retu==1)  /*普通用户*/
				  	limit+=4;
                  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:8px\">"\
			  "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
			  "<tr height=30>");
				    if(strcmp(search(lwlan,"path"),"PATH")==0)
				    {
						fprintf(cgiOut,"<td width=158>%s ID:</td>",search(lpublic,"instance"));
						fprintf(cgiOut,"<td width=100 align=left>");
				    }
					else
					{
						fprintf(cgiOut,"<td width=89>%s ID:</td>",search(lpublic,"instance"));
						fprintf(cgiOut,"<td width=100 align=left>");
				    }
						fprintf(cgiOut,"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wtpupgrade.cgi\",\"%s\")>",m);  
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
					"</td>");
					if(wtpupgrade_page_no == 1)
					{
						if(strcmp(search(lwlan,"path"),"PATH")==0)
						  fprintf(cgiOut,"<td align=left width=442><a href=wp_bind_file.cgi?UN=%s&INSTANCE_ID=%s><font color=blue size=2>%s</font></a></td>",m,select_insid,search(lwlan,"version_bind"));
						else
						  fprintf(cgiOut,"<td align=left width=511><a href=wp_bind_file.cgi?UN=%s&INSTANCE_ID=%s><font color=blue size=2>%s</font></a></td>",m,select_insid,search(lwlan,"version_bind"));
					}
					else if(wtpupgrade_page_no == 3)
					{
						if(strcmp(search(lwlan,"path"),"PATH")==0)
						  fprintf(cgiOut,"<td align=left width=442><input type=submit style=\"width:100px; margin-right:20px\" border=0 name=set_ap_update_stop style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"set_ap_update_stop"));
						else
						  fprintf(cgiOut,"<td align=left width=511><input type=submit style=\"width:80px; margin-right:20px\" border=0 name=set_ap_update_stop style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"set_ap_update_stop"));
					}
					else
					{
						if(strcmp(search(lwlan,"path"),"PATH")==0)
						  fprintf(cgiOut,"<td width=442>&nbsp;</td>");
						else
						  fprintf(cgiOut,"<td width=511>&nbsp;</td>");
					}
			  fprintf(cgiOut,"</tr>"\
			  "<tr><td colspan=3>");
			  if(((wtpupgrade_page_no == 1)||(wtpupgrade_page_no == 2))&&(retu==0))
			  {
			  	  if(paraHead1)
				  {
				  	result2=show_model_tar_file_bind_info(paraHead1->parameter,paraHead1->connection,&head,&model_tar_file_num);
				  } 
				  if(strcmp(search(lwlan,"path"),"PATH")==0)
					fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");
				  else
					fprintf(cgiOut,"<table width=630 border=0 cellspacing=0 cellpadding=0>");
					fprintf(cgiOut,"<tr height=35>");
					  if(strcmp(search(lwlan,"path"),"PATH")==0)
						fprintf(cgiOut,"<td width=160>%s:</td>",search(lpublic,"count_onetime"));
					  else
						fprintf(cgiOut,"<td width=90>%s:</td>",search(lpublic,"count_onetime"));
					  fprintf(cgiOut,"<td width=540 align=left>");
					  	if(WTPINFO)
					  	{
							fprintf(cgiOut,"<input type=text name=count value=%d maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">",WTPINFO->Count_onetimeupdt);
					  	}
						else
						{
							fprintf(cgiOut,"<input type=text name=count maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\">");
					  	}	
					    fprintf(cgiOut,"<font color=red>(1-50)</font></td>"\
					"</tr>"\
					"<tr height=35>"\
					  "<td>%s:</td>",search(lwlan,"model"));
					  fprintf(cgiOut,"<td align=left>");
					    if(result2 ==1 )
						{			
							if(model_tar_file_num > 0)
							{
								fprintf(cgiOut,"<select name=model id=model multiple=multiple size=5 style=width:500px>");
								for(i=0,vq=head.next; ((i<model_tar_file_num)&&(vq));i++,vq=vq->next)
								{
									if((vq->apmodel))
									{
										fprintf(cgiOut,"<option value='%s'>%s",vq->apmodel,vq->apmodel);
									}
								}
								fprintf(cgiOut,"</select>");
							}
							else
							{
								fprintf(cgiOut,"%s",search(lwlan,"no_bind_info"));
							}
						}
					  fprintf(cgiOut,"<td>"\
					"</tr>"\
				  "</table>");
			  }
			  if((wtpupgrade_page_no == 2)&&(retu==0))
			  {
			  	fprintf(cgiOut,"</td></tr>"\
				"<tr style=\"padding-top:30px\"><td colspan=3>");
					fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
					 "<tr align=left height=10 valign=top>"\
					   "<td id=thead1>%s%s</td>",search(lpublic,"upgrade_config"),search(lpublic,"info"));
					 fprintf(cgiOut,"</tr>"\
					 "<tr>"\
					   "<td width=700 align=left style=\"padding-left:20px\">"\
						 "<table frame=below rules=rows width=700 border=1>"\
						   "<tr align=left>"\
							   "<th width=200><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"model"));					
							   fprintf(cgiOut,"<th width=170><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"ap_version"));	
							   fprintf(cgiOut,"<th width=317><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"path"));	
							   fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
						   "</tr>");
						   for(i = 0,q = WTPINFO; ((i<wtp_update_num)&&(NULL !=  q)); i++,q = q->next)
						   {			
						   	   memset(menu,0,15);
							   strcat(menu,"menuLists");
							   sprintf(menu_id,"%d",i+1); 
							   strcat(menu,menu_id);
							   fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
							     if(q->model)
							     {
									 fprintf(cgiOut,"<td>%s</td>",q->model);
							     }
								 if(q->versionname)
							     {
									 fprintf(cgiOut,"<td>%s</td>",q->versionname);
							     }
								 if(q->versionpath)
							     {
									 fprintf(cgiOut,"<td>%s</td>",q->versionpath);
							     }
								 if(q->model)
							     {
									 fprintf(cgiOut,"<td>"\
									   "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(wtp_update_num-i),menu,menu);
										 fprintf(cgiOut,"<img src=/images/detail.gif>"\
										 "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
										   fprintf(cgiOut,"<div id=div1>"\
											 "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpupgrade.cgi?UN=%s&ApModel=%s&IsClearConfig=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame>%s</a></div>",m,q->model,"true",select_insid,search(lpublic,"rm_conf"));							   
										   fprintf(cgiOut,"</div>"\
										 "</div>"\
									   "</div>"\
									 "</td>");
							     }
								 else
								 {
								   	 fprintf(cgiOut,"<td>&nbsp;</td>");
								 }
					           fprintf(cgiOut,"</tr>");
							   cl=!cl;
						   }
						 fprintf(cgiOut,"</table>"\
					  "</td>"\
					"</tr>"\
					"<tr style=\"padding-top:15px\">"\
					  "<td align=left>"\
					    "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
					 	  "<tr>");
					        if(strcmp(search(lwlan,"path"),"PATH")==0)
					        {
						      fprintf(cgiOut,"<td align=center><input type=submit style=\"width:100px; margin-right:20px\" border=0 name=set_ap_update_start style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"set_ap_update_start"));
						      fprintf(cgiOut,"<td align=center><input type=submit style=\"width:240px; margin-left:20px\" border=0 name=clear_upgrade_config_all style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"clear_batch_update_config"));
					        }
					        else
					        {
						      fprintf(cgiOut,"<td align=center><input type=submit style=\"width:80px; margin-right:20px\" border=0 name=set_ap_update_start style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"set_ap_update_start"));
						      fprintf(cgiOut,"<td align=center><input type=submit style=\"width:150px; margin-left:20px\" border=0 name=clear_upgrade_config_all style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"clear_batch_update_config"));
					        }
					      fprintf(cgiOut,"</tr>"\
						"</table>"\
					"</td></tr>"\
				   "</table>");
			    }
			    else if(wtpupgrade_page_no == 3)
			    {	
				  fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
				   "<tr align=left style=\"padding-top:10px\">");		  
					 fprintf(cgiOut,"<td id=thead1>%s</td>",search(lpublic,"ap_upgrade_info"));
				   fprintf(cgiOut,"</tr>"\
				   "<tr>"\
					 "<td align=center style=\"padding-left:20px\">"\
					   "<table frame=below rules=rows width=700 border=1>"\
						 "<tr align=left>"\
						   "<th width=40><font id=yingwen_thead>ID</font></th>"\
						   "<th width=130><font id=yingwen_thead>MAC</font></th>"\
						   "<th width=150><font id=yingwen_thead>IP</font></th>"\
						   "<th width=110><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"model"));					
						   fprintf(cgiOut,"<th width=90><font id=%s>%s</font><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"run"),search(lpublic,"menu_thead"),search(lwlan,"state"));
						   fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"uptinfo"));
						   fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"time"));
						 fprintf(cgiOut,"</tr>");
						 if((result3 == 1)&&(ap_head))
						 {
							 cl = 1;
							 for(i=0;((i<wtp_num)&&(ap_head->WTP[i]));i++)
							 { 			   
								fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
								  fprintf(cgiOut,"<td>%d</td>",ap_head->WTP[i]->WTPID);								  
								  fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",ap_head->WTP[i]->WTPMAC[0],ap_head->WTP[i]->WTPMAC[1],ap_head->WTP[i]->WTPMAC[2],ap_head->WTP[i]->WTPMAC[3],ap_head->WTP[i]->WTPMAC[4],ap_head->WTP[i]->WTPMAC[5]);
								  if(ap_head->WTP[i]->WTPIP)
								  {
									  ret_ip= wtp_check_wtp_ip_addr(ip,ap_head->WTP[i]->WTPIP);
									  if(ret_ip != 1)
										fprintf(cgiOut,"<td>%s</td>",ap_head->WTP[i]->WTPIP);
									  else
										fprintf(cgiOut,"<td>%s</td>",ip);
								  }
								  if(ap_head->WTP[i]->WTPModel)
								  {
									  fprintf(cgiOut,"<td>%s</td>",ap_head->WTP[i]->WTPModel);
								  }					
								  
								  memset(wtp_state,0,sizeof(wtp_state));
								  CheckWTPState(wtp_state,ap_head->WTP[i]->WTPStat);
								  fprintf(cgiOut,"<td>%s</td>",wtp_state);
							 
								  if(ap_head->WTP[i]->updateStat==1)
									fprintf(cgiOut,"<td>update</td>");
								  else if(ap_head->WTP[i]->updateStat==2)
									fprintf(cgiOut,"<td>success</td>"); 			   
								  else
									fprintf(cgiOut,"<td>ready</td>");
							 
								  time(&now);
								  online_time = now - ap_head->WTP[i]->manual_update_time;
								  hour = online_time/3600;
								  min = (online_time-hour*3600)/60;
								  sec = (online_time-hour*3600)%60;
								  fprintf(cgiOut,"<td>%02d:%02d:%02d</td>",hour,min,sec);
								fprintf(cgiOut,"</tr>");
								cl=!cl;
							 }		 
						 }
					   fprintf(cgiOut,"</table>"\
					 "</td>"\
				   "</tr>"\
				  "</table>");
			    }
			    fprintf(cgiOut,"</td></tr>"\
				"<tr>"\
				  "<td><input type=hidden name=encry_wtpupgrade value=\"%s\"></td>",m);
				  fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
				  fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
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
if(result1==1)
{
  free_wtp_show_ap_update_config(WTPINFO);
}
if((result2==1)&&(model_tar_file_num>0))
{
  Free_show_model_tar_file_bind_info(&head);
}
if(result3==1)
{
  Free_show_update_wtp_list(ap_head);
}
free_instance_parameter_list(&paraHead1);
return 0;
}
void Wtp_Upgrade(instance_parameter *ins_para,char *m,struct list *lpublic,struct list *lwlan)
{
	int ret = 0,flag = 1;
	char count[STRING_LEN] = { 0 };
	int result = cgiFormNotFound, i = 0, set_result = 1;
	char **responses;
	char temp[256] = { 0 };

	memset(count,0,sizeof(count));
	cgiFormStringNoNewlines("count",count,STRING_LEN);

	if(strcmp(count,"") != 0)
	{
	    ret=wtp_set_ap_update_count_config(ins_para->parameter,ins_para->connection,count);
		switch(ret)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:ShowAlert(search(lwlan,"set_count_onetime_fail"));
		  		 flag=0;
		         break;
		  case 1:break;
		  case -1:ShowAlert(search(lpublic,"unknown_id_format"));
		          flag=0;
				  break;
		  case -2:ShowAlert(search(lwlan,"upgrade_has_already_started"));
		          flag=0;
		          break;
		  case -3:ShowAlert(search(lpublic,"error"));
		          flag=0;
				  break;
		}
	}

	result = cgiFormStringMultiple("model", &responses);
	if(result != cgiFormNotFound)
	{
	   i = 0;	
	   while((responses[i])&&(set_result==1))
	   {
		    ret=wtp_set_ap_update_base_model_config(ins_para->parameter,ins_para->connection,responses[i]);
			switch(ret)
			{
			  case SNMPD_CONNECTION_ERROR:
			  case 0:ShowAlert(search(lwlan,"upgrade_wtp_fail"));
					 set_result=0;			  		 
			         break;
			  case 1:break;
			  case -1:ShowAlert(search(lpublic,"malloc_error"));
					  set_result=0;
					  break;
			  case -2:ShowAlert(search(lwlan,"does_not_surport_model"));
					  set_result=0;
			          break;
			  case -3:ShowAlert(search(lwlan,"upgrade_is_process"));
					  set_result=0;
			          break;
			  case -4:{
			  			memset(temp,0,sizeof(temp));
					    strncpy(temp,search(lwlan,"model_has_been_set_ever1"),sizeof(temp)-1);
					    strncat(temp,responses[i],sizeof(temp)-strlen(temp)-1);
					    strncat(temp,search(lwlan,"model_has_been_set_ever2"),sizeof(temp)-strlen(temp)-1);
		                ShowAlert(temp);
						set_result=0;
		                break; 
		              }
			  case -5:ShowAlert(search(lpublic,"error"));
					  set_result=0;
					  break;
			}
			i++;
	   }
	   cgiStringArrayFree(responses);

	   if(set_result==0) 
	     flag=0;
	}

	if(flag==1)
	  ShowAlert(search(lpublic,"oper_succ"));
}

