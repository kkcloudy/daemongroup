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
* wp_wtplis.c
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

#define MAX_PAGE_NUM 25     /*每页显示的最多AP个数*/   


void ShowWtpListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan);    
void DeleteWtp(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };  
  char page_no[5] = { 0 };  
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
	if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound )  /*点击翻页进入该页面*/
	{
	  pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowWtpListPage(encry,pno,str,lpublic,lwlan);
	}
	else
	  ShowWtpListPage(encry,0,str,lpublic,lwlan);	
  }
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWtpListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan)
{  
  char IsDeleete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  char stri[8]={0};
  DCLI_WTP_API_GROUP_ONE *head = NULL;
  WID_WTP *q = NULL;
  char wtp_state[20] = { 0 };
  char update_id[10]={0};
  int ret_ip = 0;
  char ip[WTP_WTP_IP_LEN+1] = { 0 };
  int wnum = 0;            
  int i = 0,result = 0,retu = 1,cl = 1;                        /*颜色初值为#f9fafe*/  
  int limit = 0,start_wtpno = 0,end_wtpno = 0,wtpno_page = 0,total_pnum = 0;    /*start_wtpno表示要显示的起始wtp id，end_wtpno表示要显示的结束wtp id，wtpno_page表示本页要显示的wtp数，total_pnum表示总页数*/
  char wtp_id[10] = { 0 };  
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  char select_insid[10] = { 0 };
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
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
    "#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
"</style>"\
"</head>");
  fprintf(cgiOut,"<script type=\"text/javascript\" src=/jquery-1.8.3.min.js></script>");
  
	  fprintf(cgiOut, "<script type=\"text/javascript\">"\
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
	   	 "var url = 'wp_wtplis.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
	   	 "window.location.href = url;"\
	   	"}", m , select_insid);

	  fprintf(cgiOut,"function sender()"\
	    "{"\
	    	  "var start_wtpno = document.getElementById('startnum').value;"\
	    	  "var end_wtpno = document.getElementById('endtnum').value;"\
	    	  "var select_insid = document.getElementById('slothansi').value;"\
		  "var i = 0;"\
		  "var j = 0;"\
		      "$.post(\"wp_ajax.cgi\",{start_global:start_wtpno,end_global:end_wtpno,select_insid:select_insid},function(result){"\
	  			"var mycars=new Array();"\
		  		"mycars=result.split(\"^\");"\
				  "for(i=0,j=start_wtpno;i<mycars.length,j<end_wtpno;i++,j++){"\
				  	 "var obj=\"up\";"\
					  "obj +=j;"\
					 "document.getElementById(obj).innerHTML = mycars[i];"\
				   "}"\
		      "});"\
		      "setTimeout(\"sender()\",3000);"
	    "}");

	  
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
	  "</script>"\
  "<body onload='sender()'>");	  
  memset(IsDeleete,0,sizeof(IsDeleete));
  cgiFormStringNoNewlines("DeletWtp", IsDeleete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(wtp_id,0,sizeof(wtp_id));
    cgiFormStringNoNewlines("WtpID", wtp_id, 10);
	if(paraHead1)
	{
		DeleteWtp(paraHead1,wtp_id,lpublic,lwlan);
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>AP</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(t);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                    fprintf(cgiOut,"</tr>");
		    
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpgrouplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"ap_group_list"));                       
                    fprintf(cgiOut,"</tr>");
		    
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));                       
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

				  total_pnum=((wnum%MAX_PAGE_NUM)==0)?(wnum/MAX_PAGE_NUM):((wnum/MAX_PAGE_NUM)+1);
				  start_wtpno=n*MAX_PAGE_NUM;   
				  end_wtpno=(((n+1)*MAX_PAGE_NUM)>wnum)?wnum:((n+1)*MAX_PAGE_NUM);
				  wtpno_page=end_wtpno-start_wtpno;
				  if((wtpno_page<(MAX_PAGE_NUM/2))||(wnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个wtp*/
				  	limit=2;
				  else if((wtpno_page<MAX_PAGE_NUM)||(wnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个wtp*/
			  	    limit=13;
			      else         /*大于30个翻页*/
				  	limit=14;
				  if(retu==1)  /*普通用户*/
				  	limit+=4;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
		  fprintf(cgiOut,"<input type=hidden name=startnum id=startnum value=%d>",start_wtpno);
		  fprintf(cgiOut,"<input type=hidden name=endtnum id=endtnum value=%d>",end_wtpno);
		  fprintf(cgiOut,"<input type=hidden name=slothansi id=slothansi value=%s>",select_insid);
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
   "<table width=773 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
	"<tr style=\"padding-bottom:15px\">"\
	   "<td width=70>%s ID:</td>",search(lpublic,"instance"));
	   fprintf(cgiOut,"<td width=703>"\
		   "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wtplis.cgi\",\"%s\")>",m);
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
    "<td colspan=2 align=center valign=top style=\"padding-top:5px; padding-bottom:10px\">");
			if(result == 1)
	        { 
	          fprintf(cgiOut,"<table width=773 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
              "<td align=left colspan=3>");
	          if(wnum>0)       /*如果WTP存在*/
	          {			          
				fprintf(cgiOut,"<table frame=below rules=rows width=773 border=1>"\
				"<tr align=left>"\
				"<th width=150><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
                fprintf(cgiOut,"<th width=30><font id=yingwen_thead>ID</font></th>"\
                "<th width=110><font id=yingwen_thead>MAC</font></th>"\
                "<th width=130><font id=yingwen_thead>IP</font></th>"\
                "<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"model"));					
				fprintf(cgiOut,"<th width=70><font id=%s>%s</font><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"run"),search(lpublic,"menu_thead"),search(lwlan,"state"));
				fprintf(cgiOut,"<th width=50><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"state"));
				fprintf(cgiOut,"<th width=140><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"location"));
	            fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
                "</tr>");
				q=head->WTP_INFO->WTP_LIST;
				for(i=0;i<start_wtpno;i++)
				{
					if(q)
					{
						q=q->next;
					}
				}
				
		        for(i=start_wtpno;i<end_wtpno;i++)
		        {	
				  memset(menu,0,sizeof(menu));
				  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
		          snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
		          strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
                  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
				  if(q)
				  {
				  	if(q->WTPNAME)
				  	{
						fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",q->WTPNAME);
				  	}
					fprintf(cgiOut,"<td>%d</td>",q->WTPID);
					if(q->WTPMAC)
					{
						fprintf(cgiOut,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
					}
				  }
				  memset(ip,0,sizeof(ip));
				  if((q)&&(q->WTPIP))
				  {
					  ret_ip= wtp_check_wtp_ip_addr(ip,q->WTPIP);
					  if(ret_ip != 1)
						fprintf(cgiOut,"<td>%s</td>",q->WTPIP);
					  else
						fprintf(cgiOut,"<td>%s</td>",ip);
				  }
				  if((q)&&(q->WTPModel))
				  {
					  fprintf(cgiOut,"<td>%s</td>",q->WTPModel);
				  }
				  memset(wtp_state,0,sizeof(wtp_state));
				  if(q)
				  {
					  CheckWTPStatePercent(wtp_state, q->WTPStat, q->image_data_percent);
				  }
				  memset(stri,0,sizeof(stri));
				  memset(update_id,0,sizeof(update_id));
				  sprintf(stri, "%d", i);
				  strcat(update_id,"up");
				  strcat(update_id,stri);
				  fprintf(cgiOut,"<td id=%s>%s</td>",update_id,wtp_state);
				  if((q)&&(q->isused==1))
				    fprintf(cgiOut,"<td>used</td>");
				  else
				  	fprintf(cgiOut,"<td>unused</td>");
				  fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",q->location);
				  memset(wtp_id,0,sizeof(wtp_id));
				  if(q)
				  {
					  snprintf(wtp_id,sizeof(wtp_id)-1,"%d",q->WTPID);	   /*int转成char*/
				  }
			      fprintf(cgiOut,"<td>"\
				 	              "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(wnum-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");
								   if(retu==0)  /*管理员*/
								   {
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpnew.cgi?UN=%s target=mainFrame>%s</a></div>",m,search(lpublic,"create"));
									 if((n>0)&&(n==((wnum-1)/MAX_PAGE_NUM))&&(((wnum-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtplis.cgi?UN=%s&WtpID=%s&PN=%d&DeletWtp=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,wtp_id,n-1,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
									 else
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtplis.cgi?UN=%s&WtpID=%s&PN=%d&DeletWtp=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,wtp_id,n,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpcon.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wtp_id,n,select_insid,search(lpublic,"configure"));
								   }
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpdta.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wtp_id,n,select_insid,search(lpublic,"details"));
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_stakind.cgi?UN=%s&ID=%s&ST=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wtp_id,"wtp",n,select_insid,search(lwlan,"station"));                             
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_rogapkind.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%sAP</a></div>",m,wtp_id,n,select_insid,search(lwlan,"rogue"));                             
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_neiapkind.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%sAP</a></div>",m,wtp_id,n,select_insid,search(lwlan,"neighbor"));                             
								   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpblack.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wtp_id,n,select_insid,search(lwlan,"black")); 
								   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpwhite.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wtp_id,n,select_insid,search(lwlan,"white"));        
								   fprintf(cgiOut,"</div>"\
                                   "</div>"\
                                   "</div>"\
				 	"</td></tr>");
  	             cl=!cl;
				 if(q)
				 {
					 q=q->next;
				 }
		        }	

				  /*fprintf(cgiOut,"<tr height=30>"\
						  "<td>get time:</td>");
				  fprintf(cgiOut,"<td id=\"current_time\"></td>"\
					 "<td><input type=\"button\" value=\"提交\" onclick=\"sender();\" /></td></tr>");*/

			
				  fprintf(cgiOut,"</table>");
	          }
	          else				 /*no wlan exist*/
		        fprintf(cgiOut,"%s",search(lwlan,"no_wtp"));
			  fprintf(cgiOut,"</td></tr>");
			  if(wnum>MAX_PAGE_NUM)               /*大于30个wtp时，显示翻页的链接*/
			  {
			    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
				if(n!=0)
			      fprintf(cgiOut,"<td align=left width=100><a href=wp_wtplis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,select_insid,search(lpublic,"up_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
				fprintf(cgiOut,"<td align=center width=393>%s",search(lpublic,"jump_to_page1"));
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
			      fprintf(cgiOut,"<td align=right width=100><a href=wp_wtplis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,select_insid,search(lpublic,"down_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
			    fprintf(cgiOut,"</tr>");
			  }
              fprintf(cgiOut,"</table>");
	        }
			else if(result == -1)
			  fprintf(cgiOut,"%s",search(lwlan,"no_wtp"));	  
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
if(result==1)
{
  Free_wtp_list_new_head(head);
}
free_instance_parameter_list(&paraHead1);
}

void DeleteWtp(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan)
{
  char *endptr = NULL;  
  int wtpid = 0,ret = 0;  
  char alt[100] = { 0 };
  char max_wtp_num[10] = { 0 };

  wtpid=strtoul(ID,&endptr,10);        /*char转成int*/ 
  ret=delete_wtp(ins_para->parameter,ins_para->connection,wtpid); /*返回0表示删除失败，返回1表示删除成功*/
																 /*返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																 /*返回-2表示wtp id not exist，返回-3表示please unused first*/
																 /*返回-4表示error，返回-5表示input wtp has some radios interface in ebr,please delete it first*/
																 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
  switch(ret)
  {
    case SNMPD_CONNECTION_ERROR:
	case 0:{
			ShowAlert(search(lwlan,"delete_wtp_fail"));
	        break;
		   }
	case 1:{                  
			 ShowAlert(search(lwlan,"delete_wtp_succ"));
		     break;
		   }
	case -1:memset(alt,0,sizeof(alt));
			strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
			memset(max_wtp_num,0,sizeof(max_wtp_num));
			snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
			strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
			strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
  	        ShowAlert(alt);
   		    break;
	case -2:ShowAlert(search(lwlan,"wtp_not_exist"));
			break;
	case -3:ShowAlert(search(lwlan,"unuse_wtp"));
			break;
	case -4:ShowAlert(search(lpublic,"error"));
			break;
	case -5:ShowAlert(search(lwlan,"delete_inter_from_ebr"));
			break;
  }
}


