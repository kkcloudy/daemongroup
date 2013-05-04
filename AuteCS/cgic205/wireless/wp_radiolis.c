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
* wp_radiolis.c
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

#define MAX_PAGE_NUM 24     /*每页显示的最多Radio个数*/   

void ShowRadioListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char *str = NULL;                                
  char page_no[5] = { 0 };    
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
  	if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound )  /*点击翻页进入该页面*/
	{
	  pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowRadioListPage(encry,pno,str,lpublic,lwlan);
	}
	else
      ShowRadioListPage(encry,0,str,lpublic,lwlan);
  }

  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowRadioListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan)
{      
  DCLI_RADIO_API_GROUP_ONE *head = NULL;           /*存放radio信息的链表头*/       
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int rnum = 0;                   /*存放radio的个数*/
  int i = 0,result = 0,retu = 1,cl = 1;                        /*颜色初值为#f9fafe*/  
  char RType[10] = { 0 };  
  char radio_id[10] = { 0 };  
  char select_insid[10] = { 0 };
  int limit,start_radiono,end_radiono,radiono_page,total_pnum;    /*start_radiono表示要显示的起始radio global id，end_radiono表示要显示的结束radio global id，radiono_page表示本页要显示的radio数，total_pnum表示总页数*/
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
  fprintf(cgiOut,"<title>Radio</title>");
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
	  "function page_change(obj)"\
	  "{"\
	     "var page_num = obj.options[obj.selectedIndex].value;"\
	   	 "var url = 'wp_radiolis.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
	   	 "window.location.href = url;"\
	   	"}", m , select_insid);
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
  "</script>"\
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RF</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>Radio</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");  
				  retu=checkuser_group(t);
				  if(retu==0)/*管理员*/
				  {
				    fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_bssbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  if(paraHead1)
				  {
					  result=show_radio_list(paraHead1->parameter,paraHead1->connection,&head);
				  }
				  if(result == 1)
				  {
				    rnum = head->radio_num;
				  }
				  total_pnum=((rnum%MAX_PAGE_NUM)==0)?(rnum/MAX_PAGE_NUM):((rnum/MAX_PAGE_NUM)+1);
				  start_radiono=n*MAX_PAGE_NUM;   
				  end_radiono=(((n+1)*MAX_PAGE_NUM)>rnum)?rnum:((n+1)*MAX_PAGE_NUM);
				  radiono_page=end_radiono-start_radiono;
				  if((radiono_page<(MAX_PAGE_NUM/2))||(rnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个radio*/
				  	limit=9;
				  else if((radiono_page<MAX_PAGE_NUM)||(rnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个radio*/
				  	     limit=19;
				       else         /*大于30个翻页*/
					   	 limit=21;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
              "<table width=618 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
			  "<tr style=\"padding-bottom:15px\">"\
				  "<td width=70>%s ID:</td>",search(lpublic,"instance"));
				  fprintf(cgiOut,"<td width=693>"\
					  "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_radiolis.cgi\",\"%s\")>",m);
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
    "<td colspan=3 valign=top align=center style=\"padding-top:5px; padding-bottom:10px\">");     
	if(result == 1)    /*显示所有radio的信息，head返回radio信息链表的链表头*/
	{   
	  fprintf(cgiOut,"<table width=618 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(rnum>0)           /*如果radio存在*/
	  {		   
   	    fprintf(cgiOut,"<table frame=below rules=rows width=618 border=1>");
		fprintf(cgiOut,"<tr align=left>"\
        "<th width=90><font id=yingwen_thead>Radio ID</font></th>"\
        "<th width=75><font id=yingwen_thead>AP ID</font></th>"\
        "<th width=70>%s</th>",search(lwlan,"channel"));
        fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"tx_power"));
		fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"ad_state"));
		fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"op_state"));
		fprintf(cgiOut,"<th width=90><font id=%s>%s%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"rate"),search(lpublic,"count"));
		fprintf(cgiOut,"<th width=40><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"type"));
        fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
        "</tr>");
		for(i=start_radiono;i<end_radiono;i++)
		{ 
		  memset(menu,0,sizeof(menu));
		  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
		  snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
		  strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
		  if(head->RADIO[i])
		  {
			  snprintf(radio_id,sizeof(radio_id)-1,"%d",head->RADIO[i]->Radio_G_ID);	/*int转成char*/
		  }
		  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
		  if(head->RADIO[i])
		  {
			  fprintf(cgiOut,"<td>%d</td>",head->RADIO[i]->Radio_G_ID);
			  fprintf(cgiOut,"<td>%d</td>",head->RADIO[i]->WTPID);
			  if(head->RADIO[i]->Radio_Chan==0)
			  {
				fprintf(cgiOut,"<td>%s</td>","auto");
			  }
			  else
			  {
				fprintf(cgiOut,"<td>%d</td>",head->RADIO[i]->Radio_Chan);
			  }
			  if((head->RADIO[i]->Radio_TXP == 0)||(head->RADIO[i]->Radio_TXP == 100))
			  {
				fprintf(cgiOut,"<td>%s</td>","auto");
			  }
			  else
			  {
				fprintf(cgiOut,"<td>%d</td>",head->RADIO[i]->Radio_TXP);
			  }
		  }
		  if((head->RADIO[i])&&(head->RADIO[i]->AdStat== 2))
            fprintf(cgiOut,"<td>disable</td>");
		  else
			fprintf(cgiOut,"<td>enable</td>");
		  if((head->RADIO[i])&&(head->RADIO[i]->OpStat== 2))
            fprintf(cgiOut,"<td>disable</td>");
		  else
			fprintf(cgiOut,"<td>enable</td>");
		  if(head->RADIO[i])
		  {
			  fprintf(cgiOut,"<td>%d</td>",head->RADIO[i]->Support_Rate_Count);
		  }
		  memset(RType,0,sizeof(RType));
		  if(head->RADIO[i])
		  {
			  Radio_Type(head->RADIO[i]->Radio_Type,RType);
		  }
		  fprintf(cgiOut,"<td>%s</td>",RType);
		  fprintf(cgiOut,"<td>"\
		                           "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(rnum-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");
								   if(retu==0)/*管理员*/
	                                 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_radcon.cgi?UN=%s&ID=%s&FL=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,"1",n,select_insid,search(lpublic,"configure"));      
	                               if(head->RADIO[i])
								   {
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_rdtail.cgi?UN=%s&ID=%s&FL=%s&WID=%d&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,"1",head->RADIO[i]->WTPID,n,select_insid,search(lpublic,"details"));
								   }
								   else
								   {
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_rdtail.cgi?UN=%s&ID=%s&FL=%s&WID=%d&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,"1",0,n,select_insid,search(lpublic,"details"));
								   }
								   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_radioblack.cgi?UN=%s&ID=%s&FL=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,"1",n,select_insid,search(lwlan,"black")); 
								   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_radiowhite.cgi?UN=%s&ID=%s&FL=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,"1",n,select_insid,search(lwlan,"white")); 
								   fprintf(cgiOut,"</div>"\
                                   "</div>"\
                                   "</div>"\
          "</td>"\
          "</tr>");
		  cl=!cl;
		}		  
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no radio exist*/
		fprintf(cgiOut,"%s",search(lwlan,"no_radio"));
	  fprintf(cgiOut,"</td></tr>");
	  if(rnum>MAX_PAGE_NUM)               /*大于30个radio时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(n!=0)          /**/
		  fprintf(cgiOut,"<td align=left width=100><a href=wp_radiolis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,select_insid,search(lpublic,"up_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
		fprintf(cgiOut,"<td align=center width=418>%s",search(lpublic,"jump_to_page1"));
										 fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this)>");
										 for(i=0;i<total_pnum;i++)
										 {
										   if(i==n)
											 fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
										   else
											 fprintf(cgiOut,"<option value=%d>%d",i,i+1);
										 }
										 fprintf(cgiOut,"</select>"\
										 "%s</td>",search(lpublic,"jump_to_page2"));
		if(n!=((rnum-1)/MAX_PAGE_NUM))
		  fprintf(cgiOut,"<td align=right width=100><a href=wp_radiolis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,select_insid,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }  
      fprintf(cgiOut,"</table>");
	}
	else if((result == 0)||(result ==  SNMPD_CONNECTION_ERROR))
      fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
	else if(result == -1)
		fprintf(cgiOut,"%s",search(lwlan,"no_radio"));
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
if((result == 1))
  Free_radio_head(head);
free_instance_parameter_list(&paraHead1);
}


