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
* wp_conAutoAp.c
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
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


int ShowAutoApconPage(char *m,struct list *lpublic,struct list *lwlan); 
void config_AutoAp(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };			  
  char *str = NULL; 
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
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
	cgiFormStringNoNewlines("encry_conAutoAp",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
  else
	ShowAutoApconPage(encry,lpublic,lwlan);

  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

int ShowAutoApconPage(char *m,struct list *lpublic,struct list *lwlan)
{  
  FILE *fp = NULL;
  DCLI_WLAN_API_GROUP *WLANINFO = NULL;
  DCLI_AC_API_GROUP_FIVE *auto_ap_login = NULL;  
  wid_auto_ap_if *autoap_head = NULL;
  int i = 0,wnum = 0,status = -1,result1 = 0,result2 = 0;
  char BindInter[20] = { 0 };
  char *retu = NULL;
  int clear_autoap_config_result = 0;
  char select_insid[10] = { 0 };
  int flag1 = 0,flag2 = 0;
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Auto AP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>");  
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
  if(cgiFormSubmitClicked("AutoApcon_apply") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		config_AutoAp(paraHead1,lpublic,lwlan);    
	}
  }
  if(cgiFormSubmitClicked("clear_autoap_config") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		clear_autoap_config_result=clear_auto_ap_config_func(paraHead1->parameter,paraHead1->connection); /*返回0表示失败，返回1表示成功*/
		switch(clear_autoap_config_result)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:ShowAlert(search(lwlan,"clear_autoap_config_fail"));
				   break;
			case 1:ShowAlert(search(lwlan,"clear_autoap_config_succ"));
				   break;
		}
	}
  }
  fprintf(cgiOut,"<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=AutoApcon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
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
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>");
                  fprintf(cgiOut,"<tr height=25>"\
  				    "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
                  fprintf(cgiOut,"</tr>");
			      fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
				  	"<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                  fprintf(cgiOut,"</tr>");
			      fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  for(i=0;i<5;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
          "<table width=590 border=0 cellspacing=0 cellpadding=0>"\
			"<tr style=\"padding-bottom:15px\">");
			   if(strcmp(search(lwlan,"bind_interface"),"Binding Interface")==0)	
			   {			     
			     fprintf(cgiOut,"<td width=126>%s ID:</td>",search(lpublic,"instance"));
				 fprintf(cgiOut,"<td width=464>");
			   }
			   else
			   {
			     fprintf(cgiOut,"<td width=105>%s ID:</td>",search(lpublic,"instance"));
				 fprintf(cgiOut,"<td width=485>");
			   }			   	 
				   fprintf(cgiOut,"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_conAutoAp.cgi\",\"%s\")>",m);
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
			 "<tr>"\
			   "<td colspan=2>");
				  status = system("bind_inter.sh"); 
				  if(paraHead1)
				  {
					  result1=show_wlan_list(paraHead1->parameter,paraHead1->connection,&WLANINFO);
				  }
				  if((result1 == 1)&&(WLANINFO))
				  {
				  	wnum = WLANINFO->wlan_num;
				  }

				  if(paraHead1)
				  {
					  result2=show_auto_ap_config(paraHead1->parameter,paraHead1->connection,&auto_ap_login);
				  }
				  if(result2 == 1)
				  {
				  	 if((auto_ap_login != NULL)&&(auto_ap_login->auto_login != NULL))
					 {
						flag1 = 1;
						if(auto_ap_login->auto_login->auto_ap_if)
							flag2 = 1;
					 }
				  }
			  if(strcmp(search(lwlan,"bind_interface"),"Binding Interface")==0)
				fprintf(cgiOut,"<table width=750 border=0 cellspacing=0 cellpadding=0>");
			  else
			  	fprintf(cgiOut,"<table width=580 border=0 cellspacing=0 cellpadding=0>");
				fprintf(cgiOut,"<tr height=30 valign=top>");
				   if(strcmp(search(lwlan,"bind_interface"),"Binding Interface")==0)
				     fprintf(cgiOut,"<td width=190>%s:</td>",search(lwlan,"bind_interface"));
				   else
				   	 fprintf(cgiOut,"<td width=110>%s:</td>",search(lwlan,"bind_interface"));
                 fprintf(cgiOut,"<td width=100 align=left>");
				 if(status==0)
				 {
				   if((flag1 == 1)&&(auto_ap_login->auto_login)&&(auto_ap_login->auto_login->auto_ap_switch == 1))
				     fprintf(cgiOut,"<select name=bind_interface id=bind_interface multiple=multiple size=8 style=width:100px disabled>");
				   else
				   	 fprintf(cgiOut,"<select name=bind_interface id=bind_interface multiple=multiple size=8 style=width:100px>");
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
				  if(strcmp(search(lwlan,"bind_interface"),"Binding Interface")==0)
				    fprintf(cgiOut,"<td width=460 align=left style=\"padding-left:20px\"><font color=red>(%s)</font><input type=submit style=\"width:190px; margin-left:20px\" border=0 name=clear_autoap_config style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"auto_ap_bind_inter"),search(lwlan,"clear_autoap_config"));
				  else
				  	fprintf(cgiOut,"<td width=370 align=left style=\"padding-left:20px\"><font color=red>(%s)</font><input type=submit style=\"width:130px; margin-left:20px\" border=0 name=clear_autoap_config style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lwlan,"auto_ap_bind_inter"),search(lwlan,"clear_autoap_config"));
                fprintf(cgiOut,"</tr>");
				if(result1 == 1)
				{
					fprintf(cgiOut,"<tr height=30 valign=top style=\"padding-top:10px; padding-bottom:10px\">"\
				  "<td>%s:</td>",search(lwlan,"bind_wlan"));
                fprintf(cgiOut,"<td align=left>");
				if(wnum>0)
				{
				  if((flag1 == 1)&&(auto_ap_login->auto_login)&&((auto_ap_login->auto_login->auto_ap_switch==1)||(auto_ap_login->auto_login->ifnum<=0)))
				    fprintf(cgiOut,"<select name=bind_wlan id=bind_wlan multiple=multiple size=8 style=width:100px disabled>");
				  else
					fprintf(cgiOut,"<select name=bind_wlan id=bind_wlan multiple=multiple size=8 style=width:100px>");
				  for(i=0;i<wnum;i++)
				  {
				  	if((result1 == 1)&&(WLANINFO)&&(WLANINFO->WLAN[i]))
				  	{
						fprintf(cgiOut,"<option value=%d>wlan %d",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanID);
				  	}
			  	  }
				  fprintf(cgiOut,"</select>");
				}
				else
				  fprintf(cgiOut,"no wlan");  /*WLAN不存在，显示no wlan*/
		        fprintf(cgiOut,"</td>"\
				"<td align=left style=\"padding-left:20px\">");
				 if(status==0)
				 {
				   if((flag1 == 1)&&(auto_ap_login->auto_login)&&((auto_ap_login->auto_login->auto_ap_switch == 1)||(auto_ap_login->auto_login->ifnum<=0)))
				     fprintf(cgiOut,"<select name=base_interface id=base_interface style=width:100px disabled>");
				   else
				   	 fprintf(cgiOut,"<select name=base_interface id=base_interface style=width:100px>");
			       fprintf(cgiOut,"<option value=>");
				   if((flag2 == 1))
				   {
				   	   if((auto_ap_login)&&(auto_ap_login->auto_login))
				   	   {
						   autoap_head = auto_ap_login->auto_login->auto_ap_if;
						   while(autoap_head)
						   {
							 fprintf(cgiOut,"<option value=%s>%s",autoap_head->ifname,autoap_head->ifname);
							 autoap_head = autoap_head->ifnext;
						   }
				   	   }
				   }
				   fprintf(cgiOut,"</select>");				   
				 }
				 else
				 {
  				    fprintf(cgiOut,"%s",search(lpublic,"exec_shell_fail"));
				 }
				fprintf(cgiOut,"</td>"\
                "</tr>");			
				}			
                fprintf(cgiOut,"<tr height=30>"\
                 "<td>%sAP%s:</td>",search(lwlan,"dynamic"),search(lpublic,"policy"));
                 fprintf(cgiOut,"<td align=left>");
				    if((flag1 == 1)&&(auto_ap_login->auto_login)&&(auto_ap_login->auto_login->ifnum<=0))
				      fprintf(cgiOut,"<select name=auto_ap_switch id=auto_ap_switch style=width:100px disabled>");
					else
					  fprintf(cgiOut,"<select name=auto_ap_switch id=auto_ap_switch style=width:100px>");
				    fprintf(cgiOut,"<option value=>"\
  				    "<option value=disable>disable"\
  				    "<option value=enable>enable"\
	              "</select></td>"\
				  "<td align=left style=\"padding-left:20px\"><font color=red>(%s)</font></td>",search(lwlan,"auto_ap_switch"));
                fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=30>"\
                 "<td>%s%sAP%s:</td>",search(lpublic,"log_save"),search(lwlan,"dynamic"),search(lpublic,"policy"));
                 fprintf(cgiOut,"<td align=left>");
				   if((flag1 == 1)&&(auto_ap_login->auto_login)&&(auto_ap_login->auto_login->auto_ap_switch==1))
				     fprintf(cgiOut,"<select name=save_auto_ap id=save_auto_ap style=width:100px disabled>");
				   else
				   	 fprintf(cgiOut,"<select name=save_auto_ap id=save_auto_ap style=width:100px>");
                     fprintf(cgiOut,"<option value=>"\
				     "<option value=disable>disable"\
  				     "<option value=enable>enable"\
	               "</select>"\
	             "</td>"\
				 "<td align=left style=\"padding-left:20px\"><font color=red>(%s)</font></td>",search(lwlan,"save_auto_ap_switch"));
                fprintf(cgiOut,"</tr>"\
				"<tr height=30>"\
                 "<td>%s:</td>",search(lwlan,"delet_bind_inter"));
                 fprintf(cgiOut,"<td align=left>");
				   if((flag1 == 1)&&(auto_ap_login->auto_login)&&(auto_ap_login->auto_login->auto_ap_switch==1))
				     fprintf(cgiOut,"<select name=del_bind_inter id=del_bind_inter style=width:100px disabled>");
				   else
				   	 fprintf(cgiOut,"<select name=del_bind_inter id=del_bind_inter style=width:100px>");
                     fprintf(cgiOut,"<option value=>");
					 if(flag2 == 1)
					 {
					 	 if((auto_ap_login)&&(auto_ap_login->auto_login))
					 	 {
							 autoap_head = auto_ap_login->auto_login->auto_ap_if;
							 while(autoap_head)
							 {
								fprintf(cgiOut,"<option value=%s>%s",autoap_head->ifname,autoap_head->ifname);
								autoap_head = autoap_head->ifnext;
							 }
					 	 }
					 }
	               fprintf(cgiOut,"</select>"\
	             "</td>"\
				 "<td align=left style=\"padding-left:20px\"><font color=red>(%s)</font></td>",search(lwlan,"con_unbind"));
                fprintf(cgiOut,"</tr>"\
					"<tr>"\
					"<td><input type=hidden name=encry_conAutoAp value=%s></td>",m);
				    fprintf(cgiOut,"<td colspan=2><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
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
if(result1 == 1)
{
  Free_wlan_head(WLANINFO);
}
if(result2 == 1)
{
  Free_auto_ap_config(auto_ap_login);
}
free_instance_parameter_list(&paraHead1);
return 0;
}


void config_AutoAp(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)   /*返回0表示失败，返回1表示成功*/
{
  int ret1=0,ret2=0,ret3=0,ret4=0,ret5=0;
  char del_inter[20] = { 0 };
  int result = cgiFormNotFound,i = 0,bind_result = 0;
  char **responses;
  char base_inter[20] = { 0 };
  char l_bss_num[10] = { 0 };
  char state[20] = { 0 };
  char save_state[20] = { 0 };  
  char alt[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  int hidden = 1;


  /*************delete binding interface******************/
  memset(del_inter,0,sizeof(del_inter));
  cgiFormStringNoNewlines("del_bind_inter",del_inter,20);
  if(strcmp(del_inter,"")!=0) 
  {
  	ret5=set_wirelesscontrol_auto_ap_binding_l3_interface(ins_para->parameter,ins_para->connection,"downlink",del_inter);
	switch(ret5)
    {
      case SNMPD_CONNECTION_ERROR:
	  case 0:{
			   ShowAlert(search(lwlan,"unbind_auto_ap_inter_fail"));
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
		        ShowAlert(search(lpublic,"input_para_illegal"));
		        hidden = 0;
			    break;  
	    	  }
	  case -3:{
		        ShowAlert(search(lwlan,"disable_auto_ap_switch"));
		        hidden = 0;
			    break;  
	    	  }
	  case -4:{
		        memset(alt,0,sizeof(alt));
			    strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
				strncat(alt,del_inter,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"inter_err_noindex_isdown"),sizeof(alt)-strlen(alt)-1);
			    ShowAlert(alt);
		        hidden = 0;
			    break;  
	    	  }
	  case -5:{
		        memset(alt,0,sizeof(alt));
			    strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
				strncat(alt,del_inter,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"inter_is_down"),sizeof(alt)-strlen(alt)-1);
			    ShowAlert(alt);
		        hidden = 0;
			    break;  
	    	  }
	  case -6:{
		        memset(alt,0,sizeof(alt));
			    strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
				strncat(alt,del_inter,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"inter_is_no_flag"),sizeof(alt)-strlen(alt)-1);
			    ShowAlert(alt);
		        hidden = 0;
			    break;  
	    	  }
	  case -7:{
		        memset(alt,0,sizeof(alt));
			    strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
				strncat(alt,del_inter,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"inter_no_index"),sizeof(alt)-strlen(alt)-1);
			    ShowAlert(alt);
		        hidden = 0;
			    break;  
	    	  }
	  case -8:{
		        memset(alt,0,sizeof(alt));
			    strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
				strncat(alt,del_inter,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"inter_error"),sizeof(alt)-strlen(alt)-1);
			    ShowAlert(alt);
		        hidden = 0;
			    break;  
	    	  }
	  case -9:{
		        ShowAlert(search(lwlan,"interface_bind_in_other_hansi"));
		        hidden = 0;
			    break;  
	    	  }
	}
  }  

  
  /**************************binding interface***************************/
  result = cgiFormStringMultiple("bind_interface", &responses);
  
  if(result != cgiFormNotFound) 
  {
    i = 0;
	bind_result=1;
    while((responses[i])&&(bind_result==1))
    {
		ret1=set_wirelesscontrol_auto_ap_binding_l3_interface(ins_para->parameter,ins_para->connection,"uplink",responses[i]);
		switch(ret1)
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:{
				   ShowAlert(search(lwlan,"bind_auto_ap_inter_fail"));
				   bind_result = 0;
				   break;
				 } 
		  case 1:break;
		  case -1:{
					ShowAlert(search(lpublic,"interface_too_long"));
					bind_result = 0;
					break;	
				  }
		  case -2:{
					ShowAlert(search(lpublic,"input_para_illegal"));
					bind_result = 0;
					break;	
				  }
		  case -3:{
					ShowAlert(search(lwlan,"disable_auto_ap_switch"));
					bind_result = 0;
					break;	
				  }
		  case -4:{
					memset(alt,0,sizeof(alt));
					strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
					strncat(alt,responses[i],sizeof(alt)-strlen(alt)-1);
					strncat(alt,search(lpublic,"inter_err_noindex_isdown"),sizeof(alt)-strlen(alt)-1);
					ShowAlert(alt);
					bind_result = 0;
					break;	
				  }
		  case -5:{
					memset(alt,0,sizeof(alt));
					strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
					strncat(alt,responses[i],sizeof(alt)-strlen(alt)-1);
					strncat(alt,search(lpublic,"inter_is_down"),sizeof(alt)-strlen(alt)-1);
					ShowAlert(alt);
					bind_result = 0;
					break;	
				  }
		  case -6:{
					memset(alt,0,sizeof(alt));
					strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
					strncat(alt,responses[i],sizeof(alt)-strlen(alt)-1);
					strncat(alt,search(lpublic,"inter_is_no_flag"),sizeof(alt)-strlen(alt)-1);
					ShowAlert(alt);
					bind_result = 0;
					break;	
				  }
		  case -7:{
					memset(alt,0,sizeof(alt));
					strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
					strncat(alt,responses[i],sizeof(alt)-strlen(alt)-1);
					strncat(alt,search(lpublic,"inter_no_index"),sizeof(alt)-strlen(alt)-1);
					ShowAlert(alt);
					bind_result = 0;
					break;	
				  }
		  case -8:{
					memset(alt,0,sizeof(alt));
					strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
					strncat(alt,responses[i],sizeof(alt)-strlen(alt)-1);
					strncat(alt,search(lpublic,"inter_error"),sizeof(alt)-strlen(alt)-1);
					ShowAlert(alt);
					bind_result = 0;
					break;	
				  }
		  case -9:{
		            ShowAlert(search(lwlan,"interface_bind_in_other_hansi"));
		            bind_result = 0;
			        break;  
	    	      }
		}      
        i++;
    }
	cgiStringArrayFree(responses);
	if(bind_result==0)
	  hidden = 0;
  }

  
  /******************set wirelesscontrol auto ap binding wlan*******************/

  result = cgiFormStringMultiple("bind_wlan", &responses);

  memset(base_inter,0,sizeof(base_inter));
  cgiFormStringNoNewlines("base_interface",base_inter,20);


  if((result == cgiFormNotFound)||(strcmp(base_inter,"")==0))
  {
  	if(!((result == cgiFormNotFound)&&(strcmp(base_inter,"")==0)))
  	{
  		if(result == cgiFormNotFound)
  		{
  		  ShowAlert(search(lwlan,"select_wlan"));
		  hidden = 0;
  		}
		else if(strcmp(base_inter,"")==0)
		{
  		  ShowAlert(search(lwlan,"select_inter_for_wlan"));
		  hidden = 0;
  		}
  			
  	}
  }
  else if((result != cgiFormNotFound)&&(strcmp(base_inter,"")!=0))
  {
  	  i = 0;
	  bind_result=1;
      while((responses[i])&&(bind_result==1))
      {
		  ret3=set_wirelesscontrol_auto_ap_binding_wlan(ins_para->parameter,ins_para->connection,responses[i],base_inter); 
		  switch(ret3)
		  {
		    case SNMPD_CONNECTION_ERROR:
			case 0:{
					 ShowAlert(search(lwlan,"set_auto_ap_wlan_fail"));
					 bind_result = 0;
					 break;
				   }
			case 1:break;
			case -1:{
					  ShowAlert(search(lpublic,"interface_too_long"));
					  bind_result = 0;
					  break;  
					}
			case -2:{
				      memset(alt,0,sizeof(alt));
					  strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
					  memset(max_wlan_num,0,sizeof(max_wlan_num));
					  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
					  strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
					  strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
					  ShowAlert(alt);
					  bind_result = 0;
					  break;
					}	
			case -3:{
					  ShowAlert(search(lwlan,"wlan_not_exist"));
					  bind_result = 0;
					  break;
					}
			case -4:{
					  memset(alt,0,sizeof(alt));
					  strncpy(alt,"wlan",sizeof(alt)-1);
					  strncat(alt,responses[i],sizeof(alt)-strlen(alt)-1);
					  strncat(alt,search(lpublic,"dont_bind_inter"),sizeof(alt)-strlen(alt)-1);
					  ShowAlert(alt);
					  bind_result = 0;
					  break;
					}
			case -5:{
					  ShowAlert(search(lwlan,"inter_not_auto_login_inter"));
					  bind_result = 0;
					  break;
					}
			case -6:{
					  ShowAlert(search(lwlan,"auto_ap_inter_not_set"));
					  bind_result = 0;
					  break;
					}
			case -7:{
					  memset(alt,0,sizeof(alt));
					  strncpy(alt,search(lwlan,"auto_ap_inter_wlan_num1"),sizeof(alt)-1);
					  strncat(alt,base_inter,sizeof(alt)-strlen(alt)-1);
					  strncat(alt,search(lwlan,"auto_ap_inter_wlan_num2"),sizeof(alt)-strlen(alt)-1);
					  memset(l_bss_num,0,sizeof(l_bss_num));
					  snprintf(l_bss_num,sizeof(l_bss_num)-1,"%d",L_BSS_NUM);
					  strncat(alt,l_bss_num,sizeof(alt)-strlen(alt)-1);
					  strncat(alt,search(lwlan,"auto_ap_inter_wlan_num3"),sizeof(alt)-strlen(alt)-1);
					  ShowAlert(alt);
					  bind_result = 0;
					  break;
					}	
			case -8:{
					  ShowAlert(search(lwlan,"disable_auto_ap_switch"));
					  bind_result = 0;
					  break;
					}
			case -9:{
					  ShowAlert(search(lpublic,"no_local_interface"));
					  bind_result = 0;
					  break;
					}
			case -10:{
				        memset(alt,0,sizeof(alt));
					    strncpy(alt,search(lpublic,"inter"),sizeof(alt)-1);
						strncat(alt,base_inter,sizeof(alt)-strlen(alt)-1);
						strncat(alt,search(lpublic,"inter_err_noindex_isdown"),sizeof(alt)-strlen(alt)-1);
					    ShowAlert(alt);
				        bind_result = 0;
					    break;  
			    	  }
			case -11:{
					   ShowAlert(search(lpublic,"error"));
					   bind_result = 0;
					   break;
					 }
		  }
		  i++;
      }
	  cgiStringArrayFree(responses);
	  if(bind_result==0)
	    hidden = 0;
  }

  
  /******************set wirelesscontrol auto ap save config switch*******************/
  memset(save_state,0,sizeof(save_state));
  cgiFormStringNoNewlines("save_auto_ap",save_state,20);  
  if(strcmp(save_state,"")!=0) 
  {
    ret4=set_wirelesscontrol_auto_ap_save_config_switch(ins_para->parameter,ins_para->connection,save_state); 
	switch(ret4)
    {
      case SNMPD_CONNECTION_ERROR:
	  case 0:{
			   ShowAlert(search(lwlan,"set_auto_ap_save_fail"));
			   hidden = 0;
               break;
	    	 } 
      case 1:break;
	  case -2:{
		        ShowAlert(search(lwlan,"disable_auto_ap_switch"));                    
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

  
  /******************set wirelesscontrol auto ap switch*******************/
  memset(state,0,sizeof(state));
  cgiFormStringNoNewlines("auto_ap_switch",state,20);	

  if(strcmp(state,"")!=0) 
  {
	  ret2=set_wirelesscontrol_auto_ap_switch(ins_para->parameter,ins_para->connection,state);  
	  switch(ret2)
	  {
	  	  case SNMPD_CONNECTION_ERROR:
		  case 0:{
				   ShowAlert(search(lwlan,"set_auto_ap_use_fail"));
				   hidden = 0;
				   break;
				 }
		  case 1:break;
		  case -2:{
					ShowAlert(search(lwlan,"auto_ap_inter_not_set"));
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
  
  if(hidden==1)
  	ShowAlert(search(lpublic,"oper_succ"));  
}


