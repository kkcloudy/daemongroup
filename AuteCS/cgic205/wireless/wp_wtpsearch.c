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
* wp_wtpsearch.c
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
#include "ws_dcli_acl.h"
#include "ws_dbus_list_interface.h"


#define MAX_PAGE_NUM 20     /*每页显示的最多AP个数*/  

struct mac_address_profile
{
    unsigned char	macaddr[6];
};


char *search_state[] = {   /*security type*/
	"",
	"join",
	"configure",
	"datacheck",
	"run",
	"quit",
	"imagedata",
	"bak_run",
};


void ShowWtpSearchPage(char *m,int n,char *t,char *type,struct list *lpublic,struct list *lwlan);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };  
  char page_no[5] = { 0 };  
  char search_type[10] = { 0 };  
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
  memset(search_type,0,sizeof(search_type));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    if(cgiFormStringNoNewlines("search_type",search_type,10)==cgiFormNotFound)
  	  strncpy(search_type,"by_state",sizeof(search_type)-1);
  }
  else
  {
  	cgiFormStringNoNewlines("encry_wtpsearch",encry,BUF_LEN);
	cgiFormStringNoNewlines("search_type",search_type,10);
  }
  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
  {
	if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound )  /*点击翻页进入该页面*/
	{
	  pno= strtoul(page_no,&endptr,10);   /*char转成int，10代表十进制*/ 
	  ShowWtpSearchPage(encry,pno,str,search_type,lpublic,lwlan);
	}
	else
	  ShowWtpSearchPage(encry,0,str,search_type,lpublic,lwlan);   
  }

  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWtpSearchPage(char *m,int n,char *t,char *type,struct list *lpublic,struct list *lwlan)
{    
  DCLI_WTP_API_GROUP_ONE *head = NULL;
  WID_WTP *q = NULL;   
  char wtp_state[20] = { 0 };
  int ret_ip = 0;
  char wtp_ip[WTP_WTP_IP_LEN+1] = { 0 };
  unsigned long ipaddr = 0;
  unsigned long mask = 0;
  char apip[WID_SYSTEM_CMD_LENTH] = { 0 };
  char *delim=":";
  char *papip = NULL;
  int wnum = 0,pnum_flag = 0,dis_flag = 0;   
  struct mac_address_profile  macaddr;
  struct mac_address_profile  macmask;
  int i = 0,result = 0,retu = 1,cl = 1;                        /*颜色初值为#f9fafe*/  
  int limit = 0,start_wtpno = 0,end_wtpno = 0,wtpno_page = 0,total_pnum = 0;    /*start_wtpno表示要显示的起始wtp id，end_wtpno表示要显示的结束wtp id，wtpno_page表示本页要显示的wtp数，total_pnum表示总页数*/
  char ap_state[15] = { 0 };
  char flag[5] = { 0 };
  char ip1[4] = { 0 };
  char ip2[4] = { 0 };
  char ip3[4] = { 0 };
  char ip4[4] = { 0 };
  char ip[20] = { 0 };  
  char mask1[4] = { 0 };
  char mask2[4] = { 0 };
  char mask3[4] = { 0 };
  char mask4[4] = { 0 };
  char ip_mask[20] = { 0 };
  char mac[20] = { 0 };
  char mac_mask[20] = { 0 };
  char select_insid[10] = { 0 };
  char tmp_ip[20] = { 0 };
  char tmp_mask[20] = { 0 };
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
  fprintf(cgiOut,"<title>Search Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  "</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>"\
  "<body>"\
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
          "<td width=62 align=center><input id=but type=submit name=wtpsearch_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
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
				  "<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> AP</font></td>",search(lpublic,"menu_san"),search(lpublic,"Search"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");				  
				  retu=checkuser_group(t);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
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
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
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
				  
				  memset(flag,0,sizeof(flag));
				  cgiFormStringNoNewlines("FL",flag,5); 
				  
				  if((cgiFormSubmitClicked("wtpsearch_apply") == cgiFormSuccess)||(strcmp(flag,"")!=0))/*点击确定按钮或者点击翻页进入*/
				  {
					if(strcmp(type,"by_state")==0)/*按状态查找*/
					{
					  memset(ap_state,0,sizeof(ap_state));
					  cgiFormStringNoNewlines("search_state",ap_state,15); 
					  if(strcmp(ap_state,""))
					  {
					    if(paraHead1)
						{
							result=show_wtp_list_by_state_func(paraHead1->parameter,paraHead1->connection,&head);
						} 
						if(result == 1)
					    {
					  	  if((head)&&(head->WTP_INFO))
					      {
							for (i = 0,q = head->WTP_INFO->WTP_LIST; ((i < head->num)&&(NULL != q)); i++,q = q->next) 
							{	
								memset(wtp_state,0,sizeof(wtp_state));
								CheckWTPState(wtp_state,q->WTPStat);  
								if(strcmp(ap_state,wtp_state)==0)
								{									
									wnum ++;
								}
							}
					  	  }
					    }
					  }
					  else
					  	ShowAlert(search(lwlan,"select_state"));
					}
					else if(strcmp(type,"by_ip")==0)/*按IP查找*/
					{
					  /*get ip addr*/
					  memset(ip,0,sizeof(ip)); 
					  memset(ip1,0,sizeof(ip1));
					  cgiFormStringNoNewlines("ip1",ip1,4);	
					  strncat(ip,ip1,sizeof(ip)-strlen(ip)-1);
					  strncat(ip,".",sizeof(ip)-strlen(ip)-1);
					  memset(ip2,0,sizeof(ip2));
					  cgiFormStringNoNewlines("ip2",ip2,4); 
					  strncat(ip,ip2,sizeof(ip)-strlen(ip)-1);	
					  strncat(ip,".",sizeof(ip)-strlen(ip)-1);
					  memset(ip3,0,sizeof(ip3));
					  cgiFormStringNoNewlines("ip3",ip3,4); 
					  strncat(ip,ip3,sizeof(ip)-strlen(ip)-1);	
					  strncat(ip,".",sizeof(ip)-strlen(ip)-1);
					  memset(ip4,0,sizeof(ip4));
					  cgiFormStringNoNewlines("ip4",ip4,4);
					  strncat(ip,ip4,sizeof(ip)-strlen(ip)-1);
					  if((strcmp(ip1,"")==0)||(strcmp(ip2,"")==0)||(strcmp(ip3,"")==0)||(strcmp(ip4,"")==0))
					    ShowAlert(search(lpublic,"ip_not_null"));
					  else
					  {
						/*get ip mask*/
						memset(ip_mask,0,sizeof(ip_mask)); 
						memset(mask1,0,sizeof(mask1));
						cgiFormStringNoNewlines("ip_mask1",mask1,4);	  
						strncat(ip_mask,mask1,sizeof(ip_mask)-strlen(ip_mask)-1);
						strncat(ip_mask,".",sizeof(ip_mask)-strlen(ip_mask)-1);
						memset(mask2,0,sizeof(mask2));
						cgiFormStringNoNewlines("ip_mask2",mask2,4); 
						strncat(ip_mask,mask2,sizeof(ip_mask)-strlen(ip_mask)-1);  
						strncat(ip_mask,".",sizeof(ip_mask)-strlen(ip_mask)-1);
						memset(mask3,0,sizeof(mask3));
						cgiFormStringNoNewlines("ip_mask3",mask3,4); 
						strncat(ip_mask,mask3,sizeof(ip_mask)-strlen(ip_mask)-1);  
						strncat(ip_mask,".",sizeof(ip_mask)-strlen(ip_mask)-1);
						memset(mask4,0,sizeof(mask4));
						cgiFormStringNoNewlines("ip_mask4",mask4,4);		 
						strncat(ip_mask,mask4,sizeof(ip_mask)-strlen(ip_mask)-1);
						if((strcmp(mask1,"")==0)||(strcmp(mask2,"")==0)||(strcmp(mask3,"")==0)||(strcmp(mask4,"")==0) )
						  ShowAlert(search(lpublic,"mask_not_null"));
						else
						{
						  memset(tmp_ip,0,sizeof(tmp_ip));
						  strncpy(tmp_ip,ip,sizeof(tmp_ip)-1);
						  memset(tmp_mask,0,sizeof(tmp_mask));
						  strncpy(tmp_mask,ip_mask,sizeof(tmp_mask)-1);
						  if(paraHead1)
						  {
							  result=show_wtp_list_by_ip_func(paraHead1->parameter,paraHead1->connection,tmp_ip,tmp_mask,&head);
						  } 
						  if(result == 1)
						  {
						  	if((head)&&(head->WTP_INFO))
						  	{
							  for (i = 0,q = head->WTP_INFO->WTP_LIST; ((i < head->num)&&(NULL != q)); i++,q=q->next) 
							  {	
								memset(wtp_ip,0,sizeof(wtp_ip));
								if(q->WTPIP)
								{
								    ret_ip= wtp_check_wtp_ip_addr(wtp_ip,q->WTPIP);					  
								    if(ret_ip != 1)
								    {
								  	  memset(apip,0,sizeof(apip));
									  papip = NULL;

									  strncpy(apip,q->WTPIP,sizeof(apip)-1);
									  papip = strtok(apip,delim);

									  memset(tmp_ip,0,sizeof(tmp_ip));
									  strncpy(tmp_ip,ip,sizeof(tmp_ip)-1);
									  memset(tmp_mask,0,sizeof(tmp_mask));
									  strncpy(tmp_mask,ip_mask,sizeof(tmp_mask)-1);
									  ipaddr = dcli_ip2ulong((char*)tmp_ip);
									  mask = dcli_ip2ulong((char*)tmp_mask);
									
									  if((papip)&&(1 == check_ip_with_mask(ipaddr,mask,papip)))
									  {
									  	wnum ++;
								  	  }
								    }						
								}
							  }
						  	}
						  }
						}
					  }					    
					}
					else if(strcmp(type,"by_mac")==0)/*按MAC查找*/
					{
					  /*get mac addr*/
					  memset(mac,0,sizeof(mac));
					  cgiFormStringNoNewlines("mac",mac,20);
					  if(strcmp(mac,"")==0)
					  	ShowAlert(search(lpublic,"mac_not_null"));
					  else
					  {
					    /*get mac mask*/
					    memset(mac_mask,0,sizeof(mac_mask));
					    cgiFormStringNoNewlines("mac_mask",mac_mask,20);
						if(strcmp(mac_mask,"")==0)
						  ShowAlert(search(lpublic,"macmask_not_null"));
						else
						{
						  if(paraHead1)
						  {
							  result=show_wtp_list_by_macex_func(paraHead1->parameter,paraHead1->connection,mac,mac_mask,&head);
						  } 
						  if(result == 1)
						  {
						  	if((head)&&(head->WTP_INFO))
						  	{
							  for (i = 0,q = head->WTP_INFO->WTP_LIST; ((i < head->num)&&(NULL != q)); i++,q=q->next) 
							  {	
								wid_parse_mac_addr((char *)mac,&macaddr);
							    wid_parse_mac_addr((char *)mac_mask,&macmask);
					            if((q->WTPMAC)&&(1 == check_mac_with_mask(&macaddr,&macmask,q->WTPMAC)))
					            {
					              wnum ++;
					            }
							  }
						  	}
						  }
						}
					  }
					}
				  }
				  fprintf(cgiOut,"<script type=\"text/javascript\">"\
				  "function page_change(obj)"\
				  "{"\
					 "var page_num = obj.options[obj.selectedIndex].value;"\
					 "var url = 'wp_wtpsearch.cgi?UN=%s&PN='+page_num+'&FL=%d&search_type=%s&search_state=%s&ip1=%s&ip2=%s&ip3=%s&ip4=%s&ip_mask1=%s&ip_mask2=%s&ip_mask3=%s&ip_mask4=%s&mac=%s&mac_mask=%s&INSTANCE_ID=%s';"\
					 "window.location.href = url;"\
				  "}", m,1,type,ap_state,ip1,ip2,ip3,ip4,mask1,mask2,mask3,mask4,mac,mac_mask,select_insid);
				  fprintf(cgiOut,"</script>");


				  total_pnum=((wnum%MAX_PAGE_NUM)==0)?(wnum/MAX_PAGE_NUM):((wnum/MAX_PAGE_NUM)+1);
				  start_wtpno=n*MAX_PAGE_NUM;   
				  end_wtpno=(((n+1)*MAX_PAGE_NUM)>wnum)?wnum:((n+1)*MAX_PAGE_NUM);
				  wtpno_page=end_wtpno-start_wtpno;
				  if((wtpno_page<(MAX_PAGE_NUM/2))||(wnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个wtp*/
				  	limit=3;
				  else if((wtpno_page<MAX_PAGE_NUM)||(wnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个wtp*/
			  	    limit=11;
			      else         /*大于30个翻页*/
				  	limit=13;
				  
				  if(strcmp(type,"by_state"))/*按IP或按MAC查找*/
				  	limit+=1;
				  
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
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
   "<table width=730 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
	 "<tr height=30>"\
	  "<td width=70>%s ID:</td>",search(lpublic,"instance"));
	  fprintf(cgiOut,"<td width=660 align=left>"\
		  "<select name=instance_id id=instance_id style=width:90px onchange=instanceid_change(this,\"wp_wtpsearch.cgi\",\"%s\")>",m);  
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
    "<tr>"
      "<td colspan=2 valign=top>");
		if(strcmp(search(lpublic,"search_type"),"Search Type")==0)
        {
          fprintf(cgiOut,"<table width=330 border=0 cellspacing=0 cellpadding=0>"\
			"<tr height=30>"\
              "<td width=90>%s:</td>",search(lpublic,"search_type"));
		}		
		else
		{
          fprintf(cgiOut,"<table width=310 border=0 cellspacing=0 cellpadding=0>"\
			"<tr height=30>"\
              "<td width=70>%s:</td>",search(lpublic,"search_type"));
		}         
			
		   fprintf(cgiOut,"<td width=240>"\
			 "<select name=search_type id=search_type style=width:90px onchange=\"javascript:this.form.submit();\">");
		     if(strcmp(type,"by_state")==0)
		     {
			    fprintf(cgiOut,"<option value=by_state selected=selected>%s",search(lpublic,"by_state"));
				fprintf(cgiOut,"<option value=by_ip>%s",search(lpublic,"by_ip"));
				fprintf(cgiOut,"<option value=by_mac>%s",search(lpublic,"by_mac"));
		     }
			 else if(strcmp(type,"by_ip")==0)
			 {
			    fprintf(cgiOut,"<option value=by_state>%s",search(lpublic,"by_state"));
				fprintf(cgiOut,"<option value=by_ip selected=selected>%s",search(lpublic,"by_ip"));
				fprintf(cgiOut,"<option value=by_mac>%s",search(lpublic,"by_mac"));
		     }
			 else if(strcmp(type,"by_mac")==0) 
			 {
			    fprintf(cgiOut,"<option value=by_state>%s",search(lpublic,"by_state"));
				fprintf(cgiOut,"<option value=by_ip>%s",search(lpublic,"by_ip"));
				fprintf(cgiOut,"<option value=by_mac selected=selected>%s",search(lpublic,"by_mac"));
		     }
              fprintf(cgiOut,"</select>"\
			"</td>"\
          "</tr>");
	
		if(strcmp(type,"by_ip")==0)/*按IP查找*/
		{
		  memset(ip1,0,sizeof(ip1));
		  cgiFormStringNoNewlines("ip1",ip1,4);	
		  memset(ip2,0,sizeof(ip2));
		  cgiFormStringNoNewlines("ip2",ip2,4); 
		  memset(ip3,0,sizeof(ip3));
		  cgiFormStringNoNewlines("ip3",ip3,4); 
		  memset(ip4,0,sizeof(ip4));
		  cgiFormStringNoNewlines("ip4",ip4,4);
		  fprintf(cgiOut,"<tr height=30>"\
            "<td>IP:</td>"\
            "<td align=left>"\
		      "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
		        "<input type=text name='ip1' maxlength=3 class=a3 value=\"%s\" onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",ip1,search(lpublic,"ip_error"));
		        fprintf(cgiOut,"<input type=text name='ip2' maxlength=3 value=\"%s\" class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",ip2,search(lpublic,"ip_error"));
		        fprintf(cgiOut,"<input type=text name='ip3' maxlength=3 value=\"%s\" class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",ip3,search(lpublic,"ip_error"));
		        fprintf(cgiOut,"<input type=text name='ip4' maxlength=3 value=\"%s\" class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",ip4,search(lpublic,"ip_error"));
		      fprintf(cgiOut,"</div>"\
		    "</td>"\
          "</tr>");
		  memset(mask1,0,sizeof(mask1));
	      cgiFormStringNoNewlines("ip_mask1",mask1,4);	
	      memset(mask2,0,sizeof(mask2));
	      cgiFormStringNoNewlines("ip_mask2",mask2,4); 
	      memset(mask3,0,sizeof(mask3));
	      cgiFormStringNoNewlines("ip_mask3",mask3,4); 
	      memset(mask4,0,sizeof(mask4));
	      cgiFormStringNoNewlines("ip_mask4",mask4,4);
		  fprintf(cgiOut,"<tr height=30>"\
            "<td>%s:</td>",search(lpublic,"mask"));
            fprintf(cgiOut,"<td align=left>"\
		      "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
      	        "<input type=text name='ip_mask1' maxlength=3 class=a3 value=\"%s\" onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",mask1,search(lpublic,"ip_error"));
      	        fprintf(cgiOut,"<input type=text name='ip_mask2' maxlength=3 class=a3 value=\"%s\" onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",mask2,search(lpublic,"ip_error"));
      	   	    fprintf(cgiOut,"<input type=text name='ip_mask3' maxlength=3 class=a3 value=\"%s\" onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",mask3,search(lpublic,"ip_error"));
      	    	fprintf(cgiOut,"<input type=text name='ip_mask4' maxlength=3 class=a3 value=\"%s\" onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",mask4,search(lpublic,"ip_error"));
		    	fprintf( cgiOut,"</div>"\
		 	"</td>"\
          "</tr>");
		}
		else if(strcmp(type,"by_mac")==0)/*按MAC查找*/
		{
		  memset(mac,0,sizeof(mac));
		  cgiFormStringNoNewlines("mac",mac,20);
		  fprintf(cgiOut,"<tr height=30>"\
            "<td>MAC:</td>"\
            "<td><input type=text name=mac value=\"%s\" size=18 maxLength=17><font color=red style=\"padding-left:10px\">%s</font></td>",mac,search(lpublic,"mac_format"));
          fprintf(cgiOut,"</tr>");
		  memset(mac_mask,0,sizeof(mac_mask));
		  cgiFormStringNoNewlines("mac_mask",mac_mask,20);
          fprintf(cgiOut,"<tr height=30>"\
            "<td>%s:</td>",search(lpublic,"mask"));
            fprintf(cgiOut,"<td><input type=text name=mac_mask value=\"%s\" size=18 maxLength=17><font color=red style=\"padding-left:10px\">%s</font></td>",mac_mask,search(lpublic,"mac_format"));
          fprintf(cgiOut,"</tr>");
		}
		else /*按状态查找*/
		{
		  memset(ap_state,0,sizeof(ap_state));
		  cgiFormStringNoNewlines("search_state",ap_state,15); 
		  fprintf(cgiOut,"<tr height=30>"\
            "<td>%s:</td>",search(lpublic,"l_state"));
            fprintf(cgiOut,"<td>"\
			  "<select name=search_state id=search_state style=width:90px>");
			  for(i=0;i<8;i++)
			    if(strcmp(search_state[i],ap_state)==0)              /*显示上次选中的state*/
	              fprintf(cgiOut,"<option value=\"%s\" selected=selected>%s",search_state[i],search_state[i]);
	            else			  	
	              fprintf(cgiOut,"<option value=\"%s\">%s",search_state[i],search_state[i]);
              fprintf(cgiOut,"</select>"\
			"</td>"\
          "</tr>");
		}
        fprintf(cgiOut,"</table>"\
      "</td>"\
    "</tr>");
			if((result == 1)&&(head)&&(head->WTP_INFO))
	        { 
			  fprintf(cgiOut,"<tr>"\
				"<td colspan=2 align=center valign=top style=\"padding-top:20px; padding-bottom:10px\">");
	          fprintf(cgiOut,"<table width=730 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
              "<td align=left colspan=3>");
	          if(wnum>0)       /*如果WTP存在*/
	          {			          
				fprintf(cgiOut,"<table frame=below rules=rows width=730 border=1>"\
				"<tr align=left>"\
				"<th width=150><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
                fprintf(cgiOut,"<th width=40><font id=yingwen_thead>ID</font></th>"\
                "<th width=130><font id=yingwen_thead>MAC</font></th>"\
                "<th width=150><font id=yingwen_thead>IP</font></th>"\
                "<th width=110><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"model"));					
				fprintf(cgiOut,"<th width=90><font id=%s>%s</font><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"run"),search(lpublic,"menu_thead"),search(lwlan,"state"));
				fprintf(cgiOut,"<th width=60><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"state"));
                fprintf(cgiOut,"</tr>");
				q = head->WTP_INFO->WTP_LIST;
				pnum_flag = 0;
				if(n>0)/*不是第一页*/
				{
					for (i = 0; i < head->num; i++) 
					{
						if(strcmp(type,"by_ip")==0)/*按IP查找*/
						{
							memset(wtp_ip,0,sizeof(wtp_ip));
							if((q)&&(q->WTPIP))
							{
								ret_ip= wtp_check_wtp_ip_addr(wtp_ip,q->WTPIP); 				
								if(ret_ip != 1)
								{
									memset(apip,0,sizeof(apip));
									papip = NULL;
							
									strncpy(apip,q->WTPIP,sizeof(apip)-1);
									papip = strtok(apip,delim);

									memset(tmp_ip,0,sizeof(tmp_ip));
								    strncpy(tmp_ip,ip,sizeof(tmp_ip)-1);
								    memset(tmp_mask,0,sizeof(tmp_mask));
								    strncpy(tmp_mask,ip_mask,sizeof(tmp_mask)-1);
									ipaddr = dcli_ip2ulong((char*)tmp_ip);
									mask = dcli_ip2ulong((char*)tmp_mask);
							
									if((papip)&&(1 == check_ip_with_mask(ipaddr,mask,papip)))
									{
										pnum_flag ++;
									}
								}						
							}
						}
						else if(strcmp(type,"by_mac")==0)/*按MAC查找*/
						{
							wid_parse_mac_addr((char *)mac,&macaddr);
						    wid_parse_mac_addr((char *)mac_mask,&macmask);
							if((q)&&(q->WTPMAC))
							{
								if(1 == check_mac_with_mask(&macaddr,&macmask,q->WTPMAC))
								{
								  pnum_flag ++;
								}
							}
						}
						else /*按状态查找*/
						{	
							memset(wtp_state,0,sizeof(wtp_state));
							if((q))
							{
								CheckWTPState(wtp_state,q->WTPStat);  
							}
							if(strcmp(ap_state,wtp_state)==0)
							{									
								pnum_flag ++;
							}
						}							
						if(pnum_flag > start_wtpno)
						{
							break;
						}
						else
						{
							if(q)
							{
								q = q->next;	
							}
						}
					}
				}


				dis_flag = 0;
				if(strcmp(type,"by_state")==0)/*按状态查找*/
				{					
					while(dis_flag < wtpno_page)
			        {	
					  memset(wtp_state,0,sizeof(wtp_state));
					  if(q)
					  {
						  CheckWTPState(wtp_state,q->WTPStat);
						  if(strcmp(ap_state,wtp_state)==0)
						  {
							  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
							  if(q->WTPNAME)
							  {
								  fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",q->WTPNAME);
							  }
							  fprintf(cgiOut,"<td>%d</td>",q->WTPID);
							  if(q->WTPMAC)
							  {
								  fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
							  }
							  memset(wtp_ip,0,sizeof(wtp_ip));
							  if(q->WTPIP)
							  {
								  ret_ip= wtp_check_wtp_ip_addr(wtp_ip,q->WTPIP);
								  if(ret_ip != 1)
									fprintf(cgiOut,"<td>%s</td>",q->WTPIP);
								  else
									fprintf(cgiOut,"<td>%s</td>",wtp_ip);
							  }
							  if(q->WTPModel)
							  {
								  fprintf(cgiOut,"<td>%s</td>",q->WTPModel);
							  }
							  fprintf(cgiOut,"<td>%s</td>",wtp_state);
							  if(q->isused==1)
								fprintf(cgiOut,"<td>used</td>");
							  else
								fprintf(cgiOut,"<td>unused</td>");	
							  fprintf(cgiOut,"</tr>");
							  cl=!cl;						  
							  dis_flag ++;
						  }
						  q=q->next;
					  }
			        }				
				}
				else if(strcmp(type,"by_ip")==0)/*按IP查找*/
				{
					while(dis_flag < wtpno_page)
			        {		        
					  memset(wtp_ip,0,sizeof(wtp_ip));
					  if((q)&&((q->WTPIP)))
					  {
						  ret_ip= wtp_check_wtp_ip_addr(wtp_ip,q->WTPIP);					  
						  if(ret_ip != 1)
						  {
							memset(apip,0,sizeof(apip));
							papip = NULL;
						  
							strncpy(apip,q->WTPIP,sizeof(apip)-1);
							papip = strtok(apip,delim);
						  
							memset(tmp_ip,0,sizeof(tmp_ip));
							strncpy(tmp_ip,ip,sizeof(tmp_ip)-1);
							memset(tmp_mask,0,sizeof(tmp_mask));
							strncpy(tmp_mask,ip_mask,sizeof(tmp_mask)-1);
							ipaddr = dcli_ip2ulong((char*)tmp_ip);
							mask = dcli_ip2ulong((char*)tmp_mask);
							
							if((papip)&&(1 == check_ip_with_mask(ipaddr,mask,papip)))
							{
							  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
							  if(q->WTPNAME)
							  {
								  fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",q->WTPNAME);
							  }
							  fprintf(cgiOut,"<td>%d</td>",q->WTPID);
							  if(q->WTPMAC)
							  {
								  fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
							  }
							  fprintf(cgiOut,"<td>%s</td>",q->WTPIP);
							  if(q->WTPModel)
							  {
								  fprintf(cgiOut,"<td>%s</td>",q->WTPModel);
							  }
							  memset(wtp_state,0,sizeof(wtp_state));
							  CheckWTPState(wtp_state,q->WTPStat);
							  fprintf(cgiOut,"<td>%s</td>",wtp_state);
							  if(q->isused==1)
								fprintf(cgiOut,"<td>used</td>");
							  else
								fprintf(cgiOut,"<td>unused</td>");	
							  fprintf(cgiOut,"</tr>");
							  cl=!cl;
							  dis_flag ++;
							}
						  }
						  q=q->next;
					  }
			        }				
				}
				else if(strcmp(type,"by_mac")==0) /*按MAC查找*/
				{
			        while(dis_flag < wtpno_page)
			        {	
			          wid_parse_mac_addr((char *)mac,&macaddr);
					  wid_parse_mac_addr((char *)mac_mask,&macmask);
					  if((q)&&(q->WTPMAC))
					  {
						  if(1 == check_mac_with_mask(&macaddr,&macmask,q->WTPMAC))
						  {
							  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
							  if(q->WTPNAME)
							  {
								  fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",q->WTPNAME);
							  }
							  fprintf(cgiOut,"<td>%d</td>",q->WTPID);
							  fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
							  memset(wtp_ip,0,sizeof(wtp_ip));
							  if(q->WTPIP)
							  {
								  ret_ip= wtp_check_wtp_ip_addr(wtp_ip,q->WTPIP);
								  if(ret_ip != 1)
									fprintf(cgiOut,"<td>%s</td>",q->WTPIP);
								  else
									fprintf(cgiOut,"<td>%s</td>",wtp_ip);
							  }
							  if(q->WTPModel)
							  {
								  fprintf(cgiOut,"<td>%s</td>",q->WTPModel);
							  }
							  memset(wtp_state,0,sizeof(wtp_state));
							  CheckWTPState(wtp_state,q->WTPStat);
							  fprintf(cgiOut,"<td>%s</td>",wtp_state);
							  if(q->isused==1)
								fprintf(cgiOut,"<td>used</td>");
							  else
								fprintf(cgiOut,"<td>unused</td>");	
							  fprintf(cgiOut,"</tr>");
							  cl=!cl;
							  dis_flag ++;
						  }
						  q=q->next;
					  }
			        }				
				}						
				fprintf(cgiOut,"</table>");
	          }
	          else				 /*no ap exist*/
		        fprintf(cgiOut,"<font color=red>%s</font>",search(lwlan,"no_wtp"));
			  fprintf(cgiOut,"</td></tr>");
			  if(wnum>MAX_PAGE_NUM)               /*大于MAX_PAGE_NUM个AP时，显示翻页的链接*/
			  {
			    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
				if(n!=0)
			      fprintf(cgiOut,"<td align=left width=100><a href=wp_wtpsearch.cgi?UN=%s&PN=%d&FL=%d&search_type=%s&search_state=%s&ip1=%s&ip2=%s&ip3=%s&ip4=%s&ip_mask1=%s&ip_mask2=%s&ip_mask3=%s&ip_mask4=%s&mac=%s&mac_mask=%s&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,1,type,ap_state,ip1,ip2,ip3,ip4,mask1,mask2,mask3,mask4,mac,mac_mask,select_insid,search(lpublic,"up_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
				fprintf(cgiOut,"<td align=center width=380>%s",search(lpublic,"jump_to_page1"));
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
				if(n!=((wnum-1)/MAX_PAGE_NUM))
			      fprintf(cgiOut,"<td align=right width=100><a href=wp_wtpsearch.cgi?UN=%s&PN=%d&FL=%d&search_type=%s&search_state=%s&ip1=%s&ip2=%s&ip3=%s&ip4=%s&ip_mask1=%s&ip_mask2=%s&ip_mask3=%s&ip_mask4=%s&mac=%s&mac_mask=%s&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,1,type,ap_state,ip1,ip2,ip3,ip4,mask1,mask2,mask3,mask4,mac,mac_mask,select_insid,search(lpublic,"down_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
			    fprintf(cgiOut,"</tr>");
			  }
              fprintf(cgiOut,"</table>"\
				"</td>"\
			  "</tr>");
	        }
			else if(result == 2)
			{
				fprintf(cgiOut,"<tr>"\
				  "<td colspan=2 align=center valign=top style=\"padding-top:20px; padding-bottom:10px\"><font color=red>%s</font></td>",search(lwlan,"no_wtp"));
			    fprintf(cgiOut,"</tr>");
			}				
  fprintf(cgiOut,"<tr>"\
    "<td colspan=2>"\
      "<table border=0 cellspacing=0 cellpadding=0>"\
        "<tr>"\
          "<td><input type=hidden name=encry_wtpsearch value=%s></td>",m);
		  fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
        fprintf(cgiOut,"</tr>"\
      "</table>"\
    "</td>"\
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
if(result==1)
{
  Free_wtp_list_new_head(head);
}
free_instance_parameter_list(&paraHead1);
}


