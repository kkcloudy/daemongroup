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
* wp_widsHistory.c
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
#include "wcpss/wid/WID.h"
#include "ws_dcli_ac.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


void ShowWidsHistoryPage(char *m,char *n,struct list *lpublic,struct list *lwlan);    
void ClearWidsDeviceList(struct list *lpublic,struct list *lwlan);



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
    cgiFormStringNoNewlines("encry_widsDev",encry,BUF_LEN);
  }  
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowWidsHistoryPage(encry,str,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWidsHistoryPage(char *m,char *n,struct list *lpublic,struct list *lwlan)
{    
  int i = 0,result = 0,retu = 1,cl = 1,limit = 0;
  DCLI_AC_API_GROUP_TWO *LIST = NULL;
  int len = 0;
  struct tag_wids_device_ele *head = NULL;
  char attacktype[WIDS_TYPE_LEN] = { 0 };
  char frametype[WIDS_TYPE_LEN] = { 0 };
  int device_num = 0;  
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WIDS</title>");
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
  if(cgiFormSubmitClicked("ClearWidsDev_apply") == cgiFormSuccess)
  {
    ClearWidsDeviceList(lpublic,lwlan);
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
          "<tr>"\
          "<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
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
				  retu=checkuser_group(n);				  
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_showWids.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"show_wids"));						 
				  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_conWids.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"con_wids"));						
					  fprintf(cgiOut,"</tr>");				  
				  }
				  fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"history"));	 /*突出显示*/
				  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_widsStatistics.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"stat_info")); 					  
				  fprintf(cgiOut,"</tr>");
				  if(paraHead1)
				  {
					  result=show_wids_device_list_cmd_func(paraHead1->parameter,paraHead1->connection,&LIST);
				  }
				  if(result == 1)
				  {
				  	if((LIST)&&(LIST->wids_device_list)&&(LIST->wids_device_list->wids_device_info!= NULL))
					{
						len = LIST->wids_device_list->count;
						head = LIST->wids_device_list->wids_device_info;
						device_num = len;
					}
				  }

				  if(device_num>1)
				  	limit+=device_num-1;
				  if(retu==1)  /*普通用户*/
				  	limit+=1;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
              "<table width=670 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
   "<tr style=\"padding-bottom:15px\">"\
	"<td width=70>%s ID:</td>",search(lpublic,"instance"));
	fprintf(cgiOut,"<td width=600>"\
		"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_widsHistory.cgi\",\"%s\")>",m);
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
    "<td colspan=2 valign=top align=center style=\"padding-bottom:10px\">");
	if(result == 1)    /*显示所有wlan的信息，wlan_head返回wlan信息链表的链表头*/
	{ 
	  fprintf(cgiOut,"<table width=670 border=0 cellspacing=0 cellpadding=0>"\
	  "<tr align=right>"\
	    "<td><input type=submit style=\"width:50px; margin-left:5px\" border=0 name=ClearWidsDev_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lpublic,"log_clear"));
	  fprintf(cgiOut,"</tr>"\
      "<tr>"\
      "<td align=left>");
	  if(device_num>0)           /*如果WLAN存在*/
	  {		
		fprintf(cgiOut,"<table frame=below rules=rows width=600 border=1>"\
		"<tr align=left>"\
        "<th width=140><font id=yingwen_thead>MAC</font></th>"\
        "<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"attacktype"));
		fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"frametype"));
        fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"attackcount"));
        fprintf(cgiOut,"<th width=180><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"fst_attack"));
        fprintf(cgiOut,"</tr>");
		for(i=0;i<device_num;i++)
		{
			if(head)
			{
				fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
				  fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",head->bssid[0],head->bssid[1],head->bssid[2],head->bssid[3],head->bssid[4],head->bssid[5]);
				  memset(attacktype,0,sizeof(attacktype));
				  memset(frametype,0,sizeof(frametype));
				  CheckWIDSType(attacktype,frametype,head->attacktype,head->frametype);
				  fprintf(cgiOut,"<td>%s</td>",attacktype);
				  fprintf(cgiOut,"<td>%s</td>",frametype);
				  fprintf(cgiOut,"<td>%d</td>",head->attackcount);
				  fprintf(cgiOut,"<td>%s</td>",ctime(&head->fst_attack));
				fprintf(cgiOut,"</tr>");
				cl=!cl;
				head = head->next;
			}
		}	
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no wlan exist*/
		fprintf(cgiOut,"%s",search(lwlan,"no_wids_device"));
	  fprintf(cgiOut,"</td></tr>"\
	  "<tr>"\
		"<td><input type=hidden name=encry_widsDev value=%s></td>",m);
		fprintf(cgiOut,"</tr>"\
	  "<tr>"\
		"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
	  fprintf(cgiOut,"</tr>"\
      "</table>");
	}
	else if(result==-1)
	  fprintf(cgiOut,"%s",search(lwlan,"no_wids_device"));	
	else if(result==-2)
	  fprintf(cgiOut,"%s",search(lpublic,"error"));
	else
      fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));			
	fprintf(cgiOut,"</td>"\
  "</tr>"\
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
  Free_wids_device_head(LIST);
}  
free_instance_parameter_list(&paraHead1);
}

void ClearWidsDeviceList(struct list *lpublic,struct list *lwlan)
{
  int ret = 0;
  char *endptr = NULL; 
  char ins_id[10] = { 0 };
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;

  memset(ins_id,0,sizeof(ins_id));
  cgiFormStringNoNewlines( "instance_id", ins_id, 10 );

  get_slotID_localID_instanceID(ins_id,&ins_para);	
  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

  if(paraHead1)
  {
	  ret=clear_wids_device_list_cmd_func(paraHead1->parameter,paraHead1->connection);
  }
  switch(ret)
  {
    case SNMPD_CONNECTION_ERROR:
	case 0:ShowAlert(search(lwlan,"clear_wids_dev_fail"));
		   break;
	case 1:ShowAlert(search(lwlan,"clear_wids_dev_succ"));
		   break;
	case -1:ShowAlert(search(lpublic,"error"));
			break;
  }
  free_instance_parameter_list(&paraHead1);
}


