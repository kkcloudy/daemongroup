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
* wp_wcapill.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
********************************************************************************/


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


#define MAX_PAGE_NUM 25     /*每页显示的最多非法AP个数*/ 

void ShowRogueApListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwcontrol,struct list *lwlan);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;                                
  char page_no[5] = { 0 };    
  char *endptr = NULL;  
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwcontrol = NULL;     /*解析wcontrol.txt文件的链表头*/  
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwcontrol=get_chain_head("../htdocs/text/wcontrol.txt");
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
      ShowRogueApListPage(encry,pno,str,lpublic,lwcontrol,lwlan);
	}
	else
      ShowRogueApListPage(encry,0,str,lpublic,lwcontrol,lwlan);
  }
  release(lpublic);  
  release(lwcontrol);
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowRogueApListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwcontrol,struct list *lwlan)
{  
  int rogue_ap_num = 0;
  DCLI_AC_API_GROUP_TWO *LIST = NULL;
  int len = 0;
  int j = 0;
  struct Neighbor_AP_ELE *head = NULL;
  int i = 0,result = 0,retu = 1,cl = 1;                  /*颜色初值为#f9fafe*/  
  int limit = 0,start_illapno = 0,end_illapno = 0,illapno_page = 0,total_pnum = 0;    /*start_illapno表示要显示的起始非法AP，end_illapno表示要显示的结束非法AP id，illapno_page表示本页要显示的非法AP数，total_pnum表示总页数*/
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Rogue AP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"</head>");
  
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
  
  fprintf(cgiOut,"<script type=\"text/javascript\">"\
  "function page_change(obj)"\
  "{"\
	 "var page_num = obj.options[obj.selectedIndex].value;"\
	 "var url = 'wp_wcapill.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
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
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwcontrol,"adv_conf"));
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
	              fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wcsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"wc_info"));                       
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(t);
				  if(retu==0)  /*管理员*/
				  {
                    
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wcwtp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"wc_config"));                       
                    fprintf(cgiOut,"</tr>");
				  }    
				  fprintf(cgiOut,"<tr height=25>"\
				    "<td align=left id=tdleft><a href=wp_apsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"ap_info"));                       
				  fprintf(cgiOut,"</tr>");
				  if(retu == 0)
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_apcon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"ap_config")); 					
					fprintf(cgiOut,"</tr>");
				  }
				  fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_rrmsumary.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"rrm_info"));                       
 				    fprintf(cgiOut,"</tr>");
				  if(retu == 0)
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_rrmcon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwcontrol,"rrm_config")); 					
				    fprintf(cgiOut,"</tr>");
				  }
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwcontrol,"illegal"),search(lpublic,"menu_san"),search(lwcontrol,"list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  if(paraHead1)
				  {
					  result=show_rogue_ap_list(paraHead1->parameter,paraHead1->connection,&LIST);
				  }
				  if(result == 1)
				  {
				  	if((LIST)&&(LIST->rouge_ap_list))
				  	{
				  		len = LIST->rouge_ap_list->neighborapInfosCount;
						for(j=0;j<len;j++)
						{
							rogue_ap_num ++;
						}
				  	}
				  }

				  total_pnum=((rogue_ap_num%MAX_PAGE_NUM)==0)?(rogue_ap_num/MAX_PAGE_NUM):((rogue_ap_num/MAX_PAGE_NUM)+1);
				  start_illapno=n*MAX_PAGE_NUM;   
				  end_illapno=(((n+1)*MAX_PAGE_NUM)>rogue_ap_num)?rogue_ap_num:((n+1)*MAX_PAGE_NUM);
				  illapno_page=end_illapno-start_illapno;
				  if((illapno_page<(MAX_PAGE_NUM/2))||(rogue_ap_num==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个非法AP*/
				  	limit=5;
				  else if((illapno_page<MAX_PAGE_NUM)||(rogue_ap_num==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个非法AP*/
				  	     limit=15;
				       else         /*大于30个翻页*/
					   	 limit=17;
				  if(retu==1)  /*普通用户*/
				  	limit+=3;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }				  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
              "<table width=790 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
   "<tr style=\"padding-bottom:15px\">"\
	"<td width=70>%s ID:</td>",search(lpublic,"instance"));
	fprintf(cgiOut,"<td width=720>"\
		"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wcapill.cgi\",\"%s\")>",m);
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
	if(result == 1)    /*显示所有非法AP的信息，head返回非法AP信息链表的链表头*/
	{   
	  fprintf(cgiOut,"<table width=790 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(rogue_ap_num>0)           /*如果非法AP存在*/
	  {		   
   	    fprintf(cgiOut,"<table frame=below rules=rows width=790 border=1>");
		fprintf(cgiOut,"<tr align=left>"\
		"<th width=130><font id=yingwen_thead>MAC</font></th>"\
        "<th width=65><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"rate"));
		fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"channel"));
        fprintf(cgiOut,"<th width=130><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"rssi"));
		fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"noise"));
		fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"bea_int"));
		fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"capab"));
		fprintf(cgiOut,"<th width=110><font id=yingwen_thead>ESSID</font></th>"\
		"<th width=85><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwcontrol,"ie_info"));
        fprintf(cgiOut,"</tr>");
		if((LIST)&&(LIST->rouge_ap_list)&&(LIST->rouge_ap_list->neighborapInfos != NULL))
	  	{
			head = LIST->rouge_ap_list->neighborapInfos;
			for(i=0;i<start_illapno;i++)
			{
				if(head != NULL)
				{
					head = head->next;
				}
			}
	  	}
		for(i=start_illapno;i<end_illapno;i++)
		{ 
		  if(head != NULL)
		  {
			  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
			  fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],head->BSSID[5]);
			  fprintf(cgiOut,"<td>%d</td>",head->Rate);
			  fprintf(cgiOut,"<td>%d</td>",head->Channel);
			  fprintf(cgiOut,"<td>%d</td>",head->RSSI);
			  fprintf(cgiOut,"<td>%d</td>",head->NOISE);
			  fprintf(cgiOut,"<td>%d</td>",head->BEACON_INT);
			  fprintf(cgiOut,"<td>%d</td>",head->capabilityinfo);
			  if(head->ESSID)
			  {
				  fprintf(cgiOut,"<td>%s</td>",head->ESSID);
			  }
			  if(head->IEs_INFO)
			  {
				  fprintf(cgiOut,"<td>%s</td>",head->IEs_INFO);
			  }
			  fprintf(cgiOut,"</tr>");
			  cl=!cl;
			  head = head->next;	  
		  }
		}		  
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no rogue ap exist*/
		fprintf(cgiOut,"%s",search(lwcontrol,"no_rogue_ap"));
	  fprintf(cgiOut,"</td></tr>");
	  if(rogue_ap_num>MAX_PAGE_NUM)               /*大于30个非法AP时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(n!=0)
		  fprintf(cgiOut,"<td align=left width=100><a href=wp_wcapill.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,select_insid,search(lpublic,"up_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
		fprintf(cgiOut,"<td align=center width=590>%s",search(lpublic,"jump_to_page1"));
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
		if(n!=((rogue_ap_num-1)/MAX_PAGE_NUM))
		  fprintf(cgiOut,"<td align=right width=100><a href=wp_wcapill.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,select_insid,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }  
      fprintf(cgiOut,"</table>");
	}
	else if(result == 2)
	  fprintf(cgiOut,"%s",search(lwcontrol,"no_rogue_ap"));	
	else if(result == -1)
	  fprintf(cgiOut,"%s",search(lwcontrol,"enable_radio_resource"));	
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
  Free_rogue_ap_head(LIST);
}
free_instance_parameter_list(&paraHead1);
}


