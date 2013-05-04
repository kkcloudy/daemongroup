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
* wp_wqoslis.c
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
#include "ws_dcli_wqos.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


void ShowWqosListPage(char *m,char *n,struct list *lpublic,struct list *lwlan);    
void DeleteWqos(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan);



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
    ShowWqosListPage(encry,str,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWqosListPage(char *m,char *n,struct list *lpublic,struct list *lwlan)
{ 
  DCLI_WQOS *wqos = NULL;
  char IsDeleete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  char select_insid[10] = { 0 };
  char wqos_id[10] = { 0 };   
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int i = 0,retu = 1,cl = 1,limit = 0,result = 0;                        /*颜色初值为#f9fafe*/
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WQOS</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
    "#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
"</style>"\
"</head>"\
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
  
  memset(IsDeleete,0,sizeof(IsDeleete));
  cgiFormStringNoNewlines("DeletWqos", IsDeleete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(wqos_id,0,sizeof(wqos_id));
    cgiFormStringNoNewlines("WqosID", wqos_id, 10);
	if(paraHead1)
	{
		DeleteWqos(paraHead1,wqos_id,lpublic,lwlan);
	}
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=%s>%s</font><font id=titleen>QOS</font></td>",search(lpublic,"title_style"),search(lwlan,"wireless"));
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
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san>QOS</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lwlan,"wireless"),search(lpublic,"menu_san"),search(lpublic,"list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>");
					if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
					  fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_wqosnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=%s> %s</font><font id=yingwen_san>QOS</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));                       
					else
					  fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_wqosnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=yingwen_san>QOS</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));                       
                    fprintf(cgiOut,"</tr>");
				  }				  
				  if(paraHead1)
				  {
					  result=show_wireless_qos_profile_list(paraHead1->parameter,paraHead1->connection,&wqos);
				  }

				  limit=6;
				  if(result == 1)
				  {
					if(wqos->qos_num>=8)
					  limit+=6;
				  }
				  
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
              "<table width=763 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr style=\"padding-bottom:15px\">"\
	"<td width=70>%s ID:</td>",search(lpublic,"instance"));
	fprintf(cgiOut,"<td width=693>"\
		"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wqoslis.cgi\",\"%s\")>",m);
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
    "<td colspan=2 valign=top align=center style=\"padding-top:5px; padding-bottom:10px\">");
    if(result == 1) 
	{ 
	  fprintf(cgiOut,"<table width=763 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=2>");
		fprintf(cgiOut,"<table frame=below rules=rows width=243 border=1>"\
		"<tr align=left>"\
		"<th width=50><font id=yingwen_thead>ID</font></th>"\
        "<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
		fprintf(cgiOut,"<th width=80><font id=yingwen_thead>wmm_map</font></th>"\
        "<th width=13>&nbsp;</th>"\
        "</tr>");
		for(i=0;i<wqos->qos_num;i++)
		{
		  memset(menu,0,sizeof(menu));
		  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
		  snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
		  strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
		  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
		  if((wqos)&&(wqos->qos[i]))
		  {
			  fprintf(cgiOut,"<td>%d</td>",wqos->qos[i]->QosID);
		  }
		  if((wqos)&&(wqos->qos[i])&&(wqos->qos[i]->name))
		  {
			  fprintf(cgiOut,"<td>%s</td>",wqos->qos[i]->name);
		  }
		  if((wqos)&&(wqos->qos[i])&&(wqos->qos[i]->radio_qos[0])&&(wqos->qos[i]->radio_qos[0]->mapstate==1))
		  	fprintf(cgiOut,"<td>enable</td>");
		  else
		  	fprintf(cgiOut,"<td>disable</td>");
		  if((wqos)&&(wqos->qos[i]))
		  {
			  snprintf(wqos_id,sizeof(wqos_id)-1,"%d",wqos->qos[i]->QosID);  /*int转成char*/
		  }
		  fprintf(cgiOut,"<td>"\
		                           "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(wqos->qos_num-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");
								   if(retu==0)  /*管理员*/
								   {
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wqosnew.cgi?UN=%s target=mainFrame>%s</a></div>",m,search(lpublic,"create"));                             
									 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wqoslis.cgi?UN=%s&WqosID=%s&DeletWqos=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,wqos_id,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));                             
	                                 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wqoscon.cgi?UN=%s&ID=%s&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wqos_id,select_insid,search(lpublic,"configure"));                             
								   }
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wqosdta.cgi?UN=%s&ID=%s&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wqos_id,select_insid,search(lpublic,"details"));
								   fprintf(cgiOut,"</div>"\
                                   "</div>"\
                                   "</div>"\
          "</td></tr>");
		  cl=!cl;
		}	
		fprintf(cgiOut,"</table>");
	  fprintf(cgiOut,"</td></tr>"\
        "</table>");
	}
	else if(result == -1)
	  fprintf(cgiOut,"%s",search(lwlan,"no_wqos"));
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
  Free_qos_head(wqos);
}
free_instance_parameter_list(&paraHead1);
}

void DeleteWqos(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan)
{
  int ret = 0;
  char qos_num[10] = { 0 };
  char temp[100] = { 0 };

  ret=delete_qos(ins_para->parameter,ins_para->connection,ID);/*返回0表示 删除失败，返回1表示删除成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
							                       			 /*返回-3表示qos id does not exist，返回-4表示this qos profile be used by some radios,please disable them first，返回-5表示error*/
															 /*返回-6表示this qos now be used by some radios,please delete them*/
  switch(ret)
  {
    case SNMPD_CONNECTION_ERROR:
	case 0:ShowAlert(search(lwlan,"delete_wqos_fail"));
		   break;
	case 1:ShowAlert(search(lwlan,"delete_wqos_succ"));
		   break;
	case -1:ShowAlert(search(lpublic,"unknown_id_format"));
			break;
	case -2:{
		      memset(temp,0,sizeof(temp));
			  strncpy(temp,search(lwlan,"qos_id_1"),sizeof(temp)-1);
			  memset(qos_num,0,sizeof(qos_num));
			  snprintf(qos_num,sizeof(qos_num)-1,"%d",QOS_NUM-1);   /*int转成char*/
			  strncat(temp,qos_num,sizeof(temp)-strlen(temp)-1);
			  strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  ShowAlert(temp);
 			  break;
		    }
	case -3:ShowAlert(search(lwlan,"wqos_not_exist"));
			break;
	case -4:ShowAlert(search(lwlan,"wqos_used"));
	        break;
	case -5:ShowAlert(search(lpublic,"error"));
			break;
	case -6:ShowAlert(search(lwlan,"delete_bind_radio_first"));
			break;
  }
}


