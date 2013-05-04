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
* wp_stalis.c
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
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


void ShowStationPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 }; 
  char page_no[10] = { 0 };  
  char *str = NULL;        
  char *endptr = NULL;  
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(page_no,0,sizeof(page_no));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {
    if(cgiFormStringNoNewlines("PN", page_no, 10)!=cgiFormNotFound )  /*点击翻页进入该页面*/
    {
      pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowStationPage(encry,pno,str,lpublic,lwlan);
	}
	else
      ShowStationPage(encry,0,str,lpublic,lwlan);
  }
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowStationPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan)
{  
  char menu[15]={ 0 };
  char menu_id[10] = { 0 };
  int bnum = 0,limit = 0;
  int i = 0,j = 0,result = 0,cl = 1,retu = 1;                             /*颜色初值为#f9fafe*/
  char mac[30] = { 0 };  
  struct dcli_ac_info *ac = NULL;
  struct dcli_bss_info *bss = NULL;
  struct dcli_sta_info *sta = NULL;
  char select_insid[10] = { 0 };
  int hourz = 0,minz = 0,secz = 0;
  //time_t online_time,now_sysrun;
  time_t online_time;
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Station</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
  	"th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
  	"#div1{ width:92px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:90px; height:15px; padding-left:5px; padding-top:3px}"\
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
   "function page_change(obj,instanceid)"\
   "{"\
	  "var page_num = obj.options[obj.selectedIndex].value;"\
	  "var url = 'wp_stalis.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID='+instanceid;"\
	  "window.location.href = url;"\
   "}", m);
  fprintf(cgiOut,"</script>"\
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
		"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"station"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>"\
		"<td width=62 align=center><a href=wp_stasumary.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,select_insid,search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=center><a href=wp_stasumary.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,select_insid,search(lpublic,"img_cancel"));
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
						  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"sta_list"));   /*突出显示*/
						fprintf(cgiOut,"</tr>"\
						"<tr height=25>"\
  					      "<td align=left id=tdleft><a href=wp_seachsta.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"search_sta"));                       
                        fprintf(cgiOut,"</tr>");
						if(paraHead1)
					    {
							result=show_station_list_by_group(paraHead1->parameter,paraHead1->connection,&ac);
					    }
						if((result == 1)&&(ac))
						{
							bnum = ac->num_bss_wireless;
						}

						if(bnum>1)                 /*需要翻页*/
							limit=13;
						else
							limit=12;
						if((result == 1)&&(ac))
						{
							bss=ac->bss_list;
							for(i=0;i<n;i++)
							{
								if(bss)
								{
									bss=bss->next;	/*移动到第i个bss信息节点处*/					  
								}
							}
						}

						if(bss)
						{
							limit+=bss->num_sta;		
						}

						for(i=0;i<limit;i++)
						{
						  fprintf(cgiOut,"<tr height=25>"\
							"<td id=tdleft>&nbsp;</td>"\
						  "</tr>");
						}
					  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
 "<table width=603 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");
	fprintf(cgiOut,"<tr style=\"padding-bottom:15px\">\n");
	fprintf(cgiOut,"<td>%s ID:&nbsp;&nbsp;",search(lpublic,"instance"));
    fprintf(cgiOut,"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_stalis.cgi\",\"%s\")>",m);
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
	fprintf(cgiOut,"</select>");
  fprintf(cgiOut,"</td>\n");
  fprintf(cgiOut,"</tr>\n");
  fprintf(cgiOut,"<tr valign=middle>"\
    "<td>");	

			if((result==1)&&(bnum>0)&&(NULL != bss))			
			{					
				  fprintf(cgiOut,"<table width=603 border=0 cellspacing=0 cellpadding=0>"\
		          "<tr align=left height=10 valign=top>"\
		            "<td colspan=3 id=thead1>%s</td>",search(lwlan,"sta_list"));
		          fprintf(cgiOut,"</tr>"\
	              "<tr>"\
	                "<td colspan=3 align=left style=\"padding-left:20px\">"\
				    "<table frame=below rules=rows width=320 border=1>"\
				    "<tr align=left>"\
				     "<td id=td1 width=170>WLAN ID</td>"\
				     "<td id=td2 width=150>%d</td>",bss->WlanID);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>AP ID</td>"\
					  "<td id=td2>%d</td>",bss->WtpID);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>BSS %s</td>",search(lwlan,"sta_num"));			  
					  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_sta);
					fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>Radio_G_ID</td>"\
					  "<td id=td2>%d</td>",bss->Radio_G_ID);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>Radio_L_ID</td>"\
					  "<td id=td2>%d</td>",bss->Radio_L_ID);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>BSS%s</td>",search(lwlan,"index"));
				      fprintf(cgiOut,"<td id=td2>%d</td>",bss->BSSIndex);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>%sID</td>",search(lwlan,"security"));			  
					  fprintf(cgiOut,"<td id=td2>%d</td>",bss->SecurityID);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>%s</td>",search(lwlan,"bss_assoc_num"));			  
					  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_assoc);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>%s</td>",search(lwlan,"bss_reassoc_num"));			  
					  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_reassoc);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>%s</td>",search(lwlan,"bss_assoc_failure_num"));			  
					  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_assoc_failure);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>BSS%s</td>",search(lpublic,"uplink_traffic_limit_threshold"));			  
					  fprintf(cgiOut,"<td id=td2>%u kbps</td>",bss->traffic_limit);
				    fprintf(cgiOut,"</tr>"\
					"<tr align=left>"\
					  "<td id=td1>BSS%s</td>",search(lpublic,"downlink_traffic_limit_threshold"));			  
					  fprintf(cgiOut,"<td id=td2>%u kbps</td>",bss->send_traffic_limit);
				    fprintf(cgiOut,"</tr>"\
				    "</table></td>"\
					"</tr>"\
					"<tr>"\
					"<td colspan=3 align=left style=\"padding-left:20px; padding-top:20px\"><table frame=below rules=rows width=603 border=1>"\
					"<tr height=20>"\
					  "<th align=left width=130 id=td1>MAC</th>"\
					  "<th align=left width=120 id=td1>IP</th>"\
					  "<th align=left width=120 id=td1>%s</th>",search(lwlan,"resbyte"));
					  fprintf(cgiOut,"<th align=left width=120 id=td1>%s</th>",search(lwlan,"trabyte"));
					  fprintf(cgiOut,"<th align=left width=100 id=td1>%s</th>",search(lwlan,"on_time"));
					  fprintf(cgiOut,"<th align=left width=13 id=td1>&nbsp;</th>");
					fprintf(cgiOut,"</tr>");
					sta = NULL;
					for( j=0,sta = bss->sta_list;
						 (j<bss->num_sta)&&(NULL != sta);
						 j++,sta = sta->next )
					{
					  /*get_sysruntime(&now_sysrun);
					  online_time = now_sysrun - sta->StaTime+sta->sta_online_time;*/
					  online_time=sta->sta_online_time_new;
					  hourz=online_time/3600;
					  minz=(online_time-hourz*3600)/60;
					  secz=(online_time-hourz*3600)%60;

					  memset(menu,0,sizeof(menu));
					  strncpy(menu,"menuLists",sizeof(menu)-1);
					  memset(menu_id,0,sizeof(menu_id));
					  snprintf(menu_id,sizeof(menu_id)-1,"%d",j+1); 
			          strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
					  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
	        		  fprintf(cgiOut,"<td id=td2>%02X:%02X:%02X:%02X:%02X:%02X</td>",MAC2STRZ(sta->addr));
					  memset(mac,0,sizeof(mac));
					  snprintf(mac,sizeof(mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STRZ(sta->addr));
					  if(sta->ip)
					  {
						  fprintf(cgiOut,"<td id=td2>%s</td>",sta->ip);
					  }
					  fprintf(cgiOut,"<td id=td2>%lld</td>",sta->rxbytes);
					  fprintf(cgiOut,"<td id=td2>%lld</td>",sta->txbytes);
					  fprintf(cgiOut,"<td id=td2>%d:%d:%d</td>",hourz,minz,secz);
					  fprintf(cgiOut,"<td align=left>"\
				  	               "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(500-j),menu,menu);
	                               fprintf(cgiOut,"<img src=/images/detail.gif>"\
	                               "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
	                               fprintf(cgiOut,"<div id=div1>");
								   retu=checkuser_group(t);
								   if(retu==0)/*管理员*/
								   {
	                                 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_stacon.cgi?UN=%s&RID=%d&WLANID=%d&MAC=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,bss->Radio_G_ID,bss->WlanID,mac,n,select_insid,search(lpublic,"configure"));
								   }
								   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_stadta.cgi?UN=%s&MAC=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,mac,n,select_insid,search(lpublic,"details"));
								   if(retu==0)/*管理员*/
								   {
									 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_stakick.cgi?UN=%s&Nm=%s&INSTANCE_ID=%s&Type=%s target=mainFrame>%s</a></div>",m,mac,select_insid,"stalis",search(lwlan,"kick"));
								   }
	                               fprintf(cgiOut,"</div>"\
	                               "</div>"\
	                               "</div>"\
				  	 "</td>");
					  fprintf(cgiOut,"</tr>");
					  cl=!cl;
					} 
				  fprintf(cgiOut,"</table>"\
					  "</td>"\
					"</tr>");
	                if(bnum>1)               /*bss多余一个时，显示翻页的链接*/
				    {
				      fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
					  if(n!=0)          
				        fprintf(cgiOut,"<td align=left width=100><a href=wp_stalis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,select_insid,search(lpublic,"up_page"));
					  else
						fprintf(cgiOut,"<td width=100>&nbsp;</td>");
					  fprintf(cgiOut,"<td align=center width=403>%s",search(lpublic,"jump_to_page1"));
					   fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this,'%s')>",select_insid);
					   for(i=0;i<bnum;i++)
					   {
						 if(i==n)
						   fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
						 else
						   fprintf(cgiOut,"<option value=%d>%d",i,i+1);
					   }
					   fprintf(cgiOut,"</select>"\
					   "%s</td>",search(lpublic,"jump_to_page2"));
					  if(n!=(bnum-1))
				        fprintf(cgiOut,"<td align=right width=100><a href=wp_stalis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,select_insid,search(lpublic,"down_page"));				  
					  else
						fprintf(cgiOut,"<td width=100>&nbsp;</td>");
				      fprintf(cgiOut,"</tr>");
				    }				
				  fprintf(cgiOut,"</table>");
        }
		else if(bnum==0)
		  fprintf(cgiOut,"%s",search(lwlan,"no_bss"));
		else if((result==0)||result == SNMPD_CONNECTION_ERROR)
          fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
        else
	      fprintf(cgiOut,"%s",search(lpublic,"error"));	
		
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
	Free_sta_summary(ac);
}
free_instance_parameter_list(&paraHead1);
}



