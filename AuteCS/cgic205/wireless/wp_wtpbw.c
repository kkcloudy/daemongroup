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
* wp_wtpbw.c
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
#include "ws_sta.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

char *mac_policy[] = {   /*wlan mac policy*/
	"none",
	"black",
	"white"
};


int ShowWtpMacPage(char *m,char *wid,char *n,struct list *lpublic,struct list *lwlan);    
void WtpMacPolicy(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);

int cgiMain()
{  
  char encry[BUF_LEN] = { 0 };  
  char *str = NULL;   
  char wtp_id[10] = { 0 };
  char macChoice[10] = { 0 };  
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(wtp_id,0,sizeof(wtp_id));
  memset(macChoice,0,sizeof(macChoice));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowWtpMacPage(encry,"1","none",lpublic,lwlan);
  }
  else                    
  {      
    cgiFormStringNoNewlines("wtp_id",wtp_id,10);	
    cgiFormStringNoNewlines("wtp_mac_select",macChoice,10);	
    cgiFormStringNoNewlines("encry_wtpmac",encry,BUF_LEN);
	str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowWtpMacPage(encry,wtp_id,macChoice,lpublic,lwlan);
  } 
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWtpMacPage(char *m,char *wid,char *n,struct list *lpublic,struct list *lwlan)
{  
  int i = 0,result = 0,macPolicyChoice = 0;  
  DCLI_WTP_API_GROUP_ONE *head = NULL;
  WID_WTP *q = NULL;
  int wnum = 0;             /*存放wtp的个数*/
  char *endptr = NULL;
  int wtp_id = 0;
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WTP MAC Policy</title>");
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
  if(cgiFormSubmitClicked("wtpmac_apply") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		WtpMacPolicy(paraHead1,lpublic,lwlan);
	}
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
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=wtpmac_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
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
				  fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>MAC </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"mac_filter"));   /*突出显示*/
			   	  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
                  for(i=0;i<0;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:30px\">"\
              "<table width=370 border=0 cellspacing=0 cellpadding=0>");
  if(paraHead1)
  {
	  result=show_wtp_list_new_cmd_func(paraHead1->parameter,paraHead1->connection,&head);	
  }				
  if(result == 1)
  {
  	if((head)&&(head->WTP_INFO))
  	{
  	  wnum = head->WTP_INFO->list_len;
  	}
  }
  fprintf(cgiOut,"<tr height=30>"\
	  "<td>%s ID:</td>",search(lpublic,"instance"));
	  fprintf(cgiOut,"<td align=left colspan=2>"\
		  "<select name=instance_id id=instance_id style=width:80px onchange=instanceid_change(this,\"wp_wtpbw.cgi\",\"%s\")>",m);  
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
    "<td width=70>WTP ID:</td>"\
    "<td width=150 align=left><select name=wtp_id id=wtp_id style=width:80px>");
  	wtp_id= strtoul(wid,&endptr,10);
	if(result == 1)
    {
  	  if((head)&&(head->WTP_INFO))
      {
		for(i=0,q=head->WTP_INFO->WTP_LIST;((i<wnum)&&(NULL != q));i++,q=q->next)
		{
		  if(q->WTPID==wtp_id)
			fprintf(cgiOut,"<option value=%d selected=selected>%d",q->WTPID,q->WTPID);
		  else
			fprintf(cgiOut,"<option value=%d>%d",q->WTPID,q->WTPID);
		}
  	  }
    }
    fprintf(cgiOut,"</select></td>"\
    "<td width=150>&nbsp;</td>"\
  "</tr>"\
  "<tr height=30>"\
    "<td>%s:</td>",search(lwlan,"type"));
    fprintf(cgiOut,"<td align=left><select name=wtp_mac_select id=wtp_mac_select style=width:80px onchange=\"javascript:this.form.submit();\">");
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
	cgiFormSelectSingle("wtp_mac_select", mac_policy, 3, &macPolicyChoice, 0);
    if(macPolicyChoice==0)
      fprintf(cgiOut,"<input type=text name=wtp_mac disabled=\"disabled\">");
	else
	  fprintf(cgiOut,"<input type=text name=wtp_mac maxLength=17>");
	fprintf(cgiOut,"</td>");
      fprintf(cgiOut,"<td align=left><font color=red>%s</font></td>",search(lpublic,"mac_format"));
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
    "<td><input type=hidden name=encry_wtpmac value=%s></td>",m);
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
{
  Free_wtp_list_new_head(head);
}
free_instance_parameter_list(&paraHead1);
return 0;
}


void WtpMacPolicy(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{
  int id = 0,ret1 = 0,ret2 = 0,flag = 1;    
  char *endptr = NULL;  
  char ID[20] = { 0 };
  char mac_select[20] = { 0 };
  char mac[20] = { 0 };
  char alt[100] = { 0 };
  char max_wtp_num[10] = { 0 };
  memset(ID,0,sizeof(ID));
  cgiFormStringNoNewlines("wtp_id",ID,20);   
  memset(mac_select,0,sizeof(mac_select));
  cgiFormStringNoNewlines("wtp_mac_select",mac_select,20);  
  memset(mac,0,sizeof(mac));
  cgiFormStringNoNewlines("wtp_mac",mac,20);

  if(strcmp(ID,"")==0)       /*没有wtp*/
  {
    ShowAlert(search(lwlan,"no_wtp"));  
	flag = 0;
  }
  else
  {    
	id= strtoul(ID,&endptr,10);
	ret1=wtp_use_none_black_white(ins_para->parameter,ins_para->connection,id,mac_select);/*返回0表示失败，返回1表示成功*/
																					     /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																					     /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																					     /*返回-3表示wtp isn't existed*/
	switch(ret1)
	{
	  case SNMPD_CONNECTION_ERROR:
      case 0:ShowAlert(search(lpublic,"oper_fail"));
	         flag = 0;
    	     break;
	  case 1:break;
	  case -1:memset(alt,0,sizeof(alt));
			  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
			  memset(max_wtp_num,0,sizeof(max_wtp_num));
			  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
			  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
			  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
	  		  ShowAlert(alt);
	          flag = 0;
    	      break;
	  case -2:ShowAlert(search(lpublic,"input_para_illegal"));
			  flag = 0;
			  break;
	  case -3:ShowAlert(search(lwlan,"wtp_not_exist"));
	          flag = 0;
    	      break;
	}
	if((strcmp(mac_select,"none")!=0)&&(strcmp(mac,"")!=0))
	{
	  id= strtoul(ID,&endptr,10);	  
	  ret2=wtp_add_black_white(ins_para->parameter,ins_para->connection,id,mac_select,mac);/*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																						  /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																						  /*返回-3返回Unknown mac addr format*/
																						  /*返回-4表示wtp is not existed，返回-5表示mac add already*/
      switch(ret2)
      {
      	case SNMPD_CONNECTION_ERROR:
        case 0:ShowAlert(search(lpublic,"oper_fail"));
		       flag = 0;
			   break;
	    case 1:break;      
        case -1:memset(alt,0,sizeof(alt));
			    strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
			    memset(max_wtp_num,0,sizeof(max_wtp_num));
			    snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
			    strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
			    strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
		        flag = 0;
    	        break;
		case -2:ShowAlert(search(lpublic,"input_para_illegal"));
			    flag = 0;
			    break;	
		case -3:ShowAlert(search(lwlan,"mac_form"));
		        flag = 0;
    	        break;
        case -4:ShowAlert(search(lwlan,"wtp_not_exist"));
		        flag = 0;
                break;		
		case -5:ShowAlert(search(lwlan,"mac_add_already"));
			    flag = 0;
			    break;
      }   
	}
  }
  if(flag)
  	ShowAlert(search(lpublic,"oper_succ"));  
}

