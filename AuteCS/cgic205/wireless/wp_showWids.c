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
* wp_showWids.c
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
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

void ShowWidsPage(char *m,char *n,struct list *lpublic,struct list *lwlan);  


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
  cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowWidsPage(encry,str,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWidsPage(char *m,char *n,struct list *lpublic,struct list *lwlan)
{ 
  int i = 0,retu = 1,result = 0;
  char select_insid[10] = { 0 };
  DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;   
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
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=%s>%s %s</font></td>",search(lpublic,"title_style"),search(lwlan,"wids"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>"\
    	"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
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
				  fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"show_wids"));   /*突出显示*/
				  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_conWids.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"con_wids"));						
					  fprintf(cgiOut,"</tr>");				  
				  }
                  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_widsHistory.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"history"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_widsStatistics.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"stat_info"));                       
                  fprintf(cgiOut,"</tr>");
				  for(i=0;i<4;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
			 "<table width=763 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
			 "<tr style=\"padding-bottom:15px\">"\
			   "<td width=70>%s ID:</td>",search(lpublic,"instance"));
			   fprintf(cgiOut,"<td width=693>"\
				   "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_showWids.cgi\",\"%s\")>",m);
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
			  "<tr valign=middle>"\
				"<td colspan=2 align=center>");
				if(paraHead1)
				{
					result=show_ap_wids_set_cmd_func(paraHead1->parameter,paraHead1->connection,&WTPINFO);
				}
				if((result==1)&&(WTPINFO))
				{
				   fprintf(cgiOut,"<table width=763 border=0 cellspacing=0 cellpadding=0>"\
				   "<tr align=left height=10 valign=top>"\
				   "<td id=thead1>%s%s</td>",search(lwlan,"wids"),search(lpublic,"info"));
				   fprintf(cgiOut,"</tr>"\
				   "<tr>"\
				   "<td align=left style=\"padding-left:20px;padding-top:10px\">");
				   if(strcmp(search(lwlan,"wids"),"WIDS")==0)
				     fprintf(cgiOut,"<table frame=below rules=rows width=240 border=1>");
				   else
				   	 fprintf(cgiOut,"<table frame=below rules=rows width=220 border=1>");
				     fprintf(cgiOut,"<tr align=left>");
					 if(strcmp(search(lwlan,"wids"),"WIDS")==0)
				       fprintf(cgiOut,"<td id=td1 width=180>%s</td>",search(lwlan,"flooding"));
					 else
					   fprintf(cgiOut,"<td id=td1 width=160>%s</td>",search(lwlan,"flooding"));
				       if(WTPINFO->wids.flooding==1)
			             fprintf(cgiOut,"<td id=td2 width=60>enable</td>");
			           else
			         	 fprintf(cgiOut,"<td id=td2 width=60>disable</td>");
				  	 fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s</td>",search(lwlan,"spoofing"));
					   if(WTPINFO->wids.sproof==1)
						 fprintf(cgiOut,"<td id=td2>enable</td>");
					   else
						 fprintf(cgiOut,"<td id=td2>disable</td>");
				  	 fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s</td>",search(lwlan,"weakiv"));
					   if(WTPINFO->wids.weakiv==1)
						 fprintf(cgiOut,"<td id=td2>enable</td>");
					   else
						 fprintf(cgiOut,"<td id=td2>disable</td>");
				    fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s</td>",search(lwlan,"interval"));
					   fprintf(cgiOut,"<td id=td2>%d %s</td>",WTPINFO->interval,search(lpublic,"second"));
				    fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s</td>",search(lwlan,"probe_threshold"));
					   fprintf(cgiOut,"<td id=td2>%d</td>",WTPINFO->probethreshold);
				    fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s</td>",search(lwlan,"other_threshold"));
					   fprintf(cgiOut,"<td id=td2>%d</td>",WTPINFO->otherthreshold);
				    fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s</td>",search(lwlan,"last_time_in_black"));
					   fprintf(cgiOut,"<td id=td2>%d %s</td>",WTPINFO->lasttime,search(lpublic,"second"));
				    fprintf(cgiOut,"</tr>"\
				  "</table></td>"\
				  "</tr>"\
				  "</table>");
				}
				else if(result==-1)
				  fprintf(cgiOut,"%s",search(lpublic,"error"));	
				else
				  fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));		
				fprintf(cgiOut,"</td>"\
			 " </tr>"\
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
	free_show_ap_wids_set_cmd(WTPINFO);
}
free_instance_parameter_list(&paraHead1);
}


