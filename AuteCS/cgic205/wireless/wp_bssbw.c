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
* wp_bssbw.c
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
#include "ws_sta.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

char *mac_policy[] = {   /*wlan mac policy*/
	"none",
	"black",
	"white"
};


int ShowBssMacPage(char *m,char *rid,char *Wid,char *n,struct list *lpublic,struct list *lwlan);    
void BssMacPolicy(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);

int cgiMain()
{  
  char encry[BUF_LEN] = { 0 };  
  char *str = NULL;   
  char radio_id[10] = { 0 };
  char Wid[10] = { 0 };
  char macChoice[10] = { 0 };  
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(radio_id,0,sizeof(radio_id));
  memset(Wid,0,sizeof(Wid));
  memset(macChoice,0,sizeof(macChoice));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowBssMacPage(encry,"1","1","none",lpublic,lwlan);
  }
  else                    
  {      
    cgiFormStringNoNewlines("radio_id",radio_id,10);
	cgiFormStringNoNewlines("wlan_id",Wid,10);
    cgiFormStringNoNewlines("bss_mac_select",macChoice,10);	
    cgiFormStringNoNewlines("encry_bssmac",encry,BUF_LEN);
	str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowBssMacPage(encry,radio_id,Wid,macChoice,lpublic,lwlan);
  } 
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

int ShowBssMacPage(char *m,char *rid,char *Wid,char *n,struct list *lpublic,struct list *lwlan)
{  
  int i = 0,result = 0,macPolicyChoice = 0;  
  DCLI_RADIO_API_GROUP_ONE *head = NULL;         /*存放wlan信息的链表头*/   
  char *endptr = NULL;
  int radio_id = 0;
  int instance_id = 0;
  char select_insid[10] = { 0 };
  int ret = 0;
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  int wlan_id = 0;
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
  fprintf(cgiOut,"<title>BSS MAC Policy</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>"\
  "<body>");
  if(cgiFormSubmitClicked("bssmac_apply") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		BssMacPolicy(paraHead1,lpublic,lwlan);
	} 
  }
  if(paraHead1)
  {
	  result=show_radio_list(paraHead1->parameter,paraHead1->connection,&head);
  } 
  
	fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RF</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=bssmac_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
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
  					  "<td align=left id=tdleft><a href=wp_radiolis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>Radio</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>MAC </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"mac_filter"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");  
                  for(i=0;i<4;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
              "<table width=370 border=0 cellspacing=0 cellpadding=0>"\
   "<tr height=30>"\
	   "<td width=70>%s ID:</td>",search(lpublic,"instance"));
	   fprintf(cgiOut,"<td width=300 align=left colspan=2>"\
		   "<select name=instance_id id=instance_id style=width:80px onchange=instanceid_change(this,\"wp_bssbw.cgi\",\"%s\")>",m);
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
  "</tr>");
  if(result == 1)
  {
	  fprintf(cgiOut,"<tr height=30>"\
	    "<td width=70>Radio ID:</td>"\
	    "<td width=150 align=left><select name=radio_id id=radio_id style=width:80px onchange=\"javascript:this.form.submit();\">");
	    radio_id= strtoul(rid,&endptr,10);
		if((radio_id == 1)&&(head)&&(head->RADIO[0]))
		{
			radio_id = head->RADIO[0]->Radio_G_ID;
		}
		for(i=0;i<head->radio_num;i++)
		{
			if((head)&&(head->RADIO[i]))
			{
				if(head->RADIO[i]->Radio_G_ID==radio_id)
				  fprintf(cgiOut,"<option value=%d selected=selected>%d",head->RADIO[i]->Radio_G_ID,head->RADIO[i]->Radio_G_ID);
				else
				  fprintf(cgiOut,"<option value=%d>%d",head->RADIO[i]->Radio_G_ID,head->RADIO[i]->Radio_G_ID);
			}
		}
	    fprintf(cgiOut,"</select></td>"\
	    "<td width=150>&nbsp;</td>"\
	  "</tr>"\
	  "<tr height=30>"\
	    "<td>WLAN ID:</td>"\
	    "<td align=left><select name=wlan_id id=wlan_id style=width:80px>");
		wlan_id= strtoul(Wid,&endptr,10);
		if(paraHead1)
		{
			ret = show_radio_one(paraHead1->parameter,paraHead1->connection,radio_id,&radio);
		} 
		if((ret==1)&&(radio->wlan_num>0))
		{
			for(i=0;i<radio->wlan_num;i++)		
			{
				if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->WlanId))
				{
					if(radio->RADIO[0]->WlanId[i]==wlan_id)
					  fprintf(cgiOut,"<option value=%d selected=selected>%d",radio->RADIO[0]->WlanId[i],radio->RADIO[0]->WlanId[i]);
					else
					  fprintf(cgiOut,"<option value=%d>%d",radio->RADIO[0]->WlanId[i],radio->RADIO[0]->WlanId[i]);
				}
			}
		}
		if(ret==1)
	    {
	      Free_radio_one_head(radio);
	    }	
		fprintf(cgiOut,"</select></td>"\
	    "<td >&nbsp;</td>"\
	  "</tr>"\
	  "<tr height=30>"\
	    "<td>%s:</td>",search(lwlan,"type"));
	    fprintf(cgiOut,"<td align=left><select name=bss_mac_select id=bss_mac_select style=width:80px onchange=\"javascript:this.form.submit();\">");
		for(i=0;i<3;i++)
	    if(strcmp(mac_policy[i],n)==0)              /*显示上次选中的mac policy*/
	      fprintf(cgiOut,"<option value=%s selected=selected>%s",mac_policy[i],mac_policy[i]);
	    else			  	
		  fprintf(cgiOut,"<option value=%s>%s",mac_policy[i],mac_policy[i]);
		fprintf(cgiOut,"</select></td>"\
	    "<td >&nbsp;</td>"\
	  "</tr>"\
	  "<tr height=30>"\
	    "<td>MAC:</td>"\
	    "<td align=left>");
		cgiFormSelectSingle("bss_mac_select", mac_policy, 3, &macPolicyChoice, 0);
	    if(macPolicyChoice==0)
	      fprintf(cgiOut,"<input type=text name=bss_mac disabled=\"disabled\" style=\"background-color:#cccccc\">");
		else
		  fprintf(cgiOut,"<input type=text name=bss_mac maxLength=17 onkeypress=\"return event.keyCode!=32\">");
		fprintf(cgiOut,"</td>");
	      fprintf(cgiOut,"<td align=left><font color=red>%s</font></td>",search(lpublic,"mac_format"));
	  fprintf(cgiOut,"</tr>");
  }  
  else if(result == -1)
  {
	  fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",search(lwlan,"no_radio"));
  }
  else
  {
	  fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",search(lpublic,"contact_adm"));
  }
  fprintf(cgiOut,"<tr>"\
    "<td><input type=hidden name=encry_bssmac value=%s></td>",m);
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
if(result == 1)
  Free_radio_head(head);
free_instance_parameter_list(&paraHead1);
return 0;
}


void BssMacPolicy(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{
  int rid = 0,ret1 = 0,ret2 = 0,flag = 1;    
  char *endptr = NULL;  
  char RID[20] = { 0 };
  char WID[20] = { 0 };
  char mac_select[20] = { 0 };
  char mac[50] = { 0 };
  char temp[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  char max_radio_num[10] = { 0 };
  memset(RID,0,sizeof(RID));
  cgiFormStringNoNewlines("radio_id",RID,20);   
  memset(WID,0,sizeof(WID));
  cgiFormStringNoNewlines("wlan_id",WID,20);   
  memset(mac_select,0,sizeof(mac_select));
  cgiFormStringNoNewlines("bss_mac_select",mac_select,20);  
  memset(mac,0,sizeof(mac));
  cgiFormStringNoNewlines("bss_mac",mac,50);

  if(strcmp(RID,"")==0)       /*没有radio*/
  {
    ShowAlert(search(lwlan,"no_radio"));  
	flag = 0;
  }
  else if(strcmp(WID,"")==0)       /*没有绑定WLAN*/
  {
    ShowAlert(search(lwlan,"no_wlan"));  
	flag = 0;
  }
  else 
  {    
	rid= strtoul(RID,&endptr,10);					   /*char转成int，10代表十进制*/		
	ret1=radio_bss_use_none_black_white(ins_para->parameter,ins_para->connection,rid,WID,mac_select);
	switch(ret1)
	{
	  case SNMPD_CONNECTION_ERROR:
      case 0:ShowAlert(search(lpublic,"oper_fail"));
	         flag = 0;
    	     break;
	  case 1:break;
	  case -1:memset(temp,0,sizeof(temp));
			  strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
			  memset(max_radio_num,0,sizeof(max_radio_num));
			  snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
			  strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
			  strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
	  		  ShowAlert(temp);
	          flag = 0;
    	      break;
	  case -2:memset(temp,0,sizeof(temp));
			  strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
			  memset(max_wlan_num,0,sizeof(max_wlan_num));
			  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);   /*int转成char*/
			  strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
			  strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
	  		  ShowAlert(temp);
	          flag = 0;
    	      break;
	  case -3:ShowAlert(search(lwlan,"bss_not_exist"));
	          flag = 0;
    	      break;
	  case -4:ShowAlert(search(lpublic,"unknown_id_format"));
			  flag = 0;
			  break;
	  case -5:ShowAlert(search(lwlan,"wlan_not_exist"));
			  flag = 0;
			  break;
	  case -6:ShowAlert(search(lwlan,"rad_dont_bind_wlan"));
			  flag = 0;
			  break;
	  case -7:ShowAlert(search(lwlan,"mac_add_already"));
			  flag = 0;
			  break;
	  case -8:ShowAlert(search(lwlan,"radio_id_not_exist"));
			  flag = 0;
			  break;
	}
	if((strcmp(mac_select,"none")!=0)&&(strcmp(mac,"")!=0))
	{
		if(strchr(mac,' ')==NULL)   /*不包含空格*/
		{
			ret2=radio_bss_add_black_white(ins_para->parameter,ins_para->connection,rid,WID,mac_select,mac);
			switch(ret2)
			{
			  case SNMPD_CONNECTION_ERROR:
			  case 0:ShowAlert(search(lpublic,"oper_fail"));
					 flag = 0;
					 break;
			  case 1:break; 	 
			  case -1:memset(temp,0,sizeof(temp));
					  strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
					  memset(max_radio_num,0,sizeof(max_radio_num));
					  snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
					  strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
					  strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
					  ShowAlert(temp);
					  flag = 0;
					  break;
			  case -2:{
						memset(temp,0,sizeof(temp));
						strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
						memset(max_wlan_num,0,sizeof(max_wlan_num));
						snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
						strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
						strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
						ShowAlert(temp);
						flag = 0;
						break;
					  }
			  case -3:ShowAlert(search(lwlan,"mac_form"));
					  flag = 0;
					  break;
			  case -4:ShowAlert(search(lwlan,"bss_not_exist"));
					  flag = 0;
					  break;
			  case -5:ShowAlert(search(lwlan,"mac_add_already"));
					  flag = 0;
					  break;
			  case -6:ShowAlert(search(lpublic,"unknown_id_format"));
					  flag = 0;
					  break;
			  case -7:ShowAlert(search(lwlan,"wlan_not_exist"));
					  flag = 0;
					  break;
			  case -8:ShowAlert(search(lwlan,"rad_dont_bind_wlan"));
					  flag = 0;
					  break;
			  case -9:ShowAlert(search(lwlan,"radio_id_not_exist"));
					  flag = 0;
					  break;
			}	
		}
		else
		{
		  ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
		  flag = 0;
		}
	}
  }
  if(flag)
  	ShowAlert(search(lpublic,"oper_succ"));  
}


